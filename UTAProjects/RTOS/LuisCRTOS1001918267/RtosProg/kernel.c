// Kernel functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "mm.h"
#include "kernel.h"

#include "AssemblyHeader.h"//for psp
#include "shell.h"

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------


// mutex
typedef struct _mutex
{
    bool lock;
    uint8_t queueSize;
    uint8_t processQueue[MAX_MUTEX_QUEUE_SIZE];
    uint8_t lockedBy;
} mutex;
mutex mutexes[MAX_MUTEXES];

// semaphore
typedef struct _semaphore
{
    uint8_t count;
    uint8_t queueSize;
    uint8_t processQueue[MAX_SEMAPHORE_QUEUE_SIZE];
} semaphore;
semaphore semaphores[MAX_SEMAPHORES];

// task states
#define STATE_INVALID           0 // no task
#define STATE_STOPPED           1 // stopped, all memory freed
#define STATE_READY             2 // has run, can resume at any time
#define STATE_DELAYED           3 // has run, but now awaiting timer
#define STATE_BLOCKED_MUTEX     4 // has run, but now blocked by semaphore
#define STATE_BLOCKED_SEMAPHORE 5 // has run, but now blocked by semaphore

// task
uint8_t taskCurrent = 0;          // index of last dispatched task
uint8_t taskCount = 0;            // total number of valid tasks
bool switchTime=false;

// control
bool priorityScheduler = true;    // priority (true) or round-robin (false)
bool priorityInheritance = false;// false; // priority inheritance for mutexes
bool preemption = true;          // preemption (true) or cooperative (false) //cahnge bak to flase

// tcb
#define NUM_PRIORITIES   16
struct _tcb
{
    uint8_t state;                 // see STATE_ values above
    void *pid;                     // used to uniquely identify thread (add of task fn)
    void *spInit;                  // original top of stack
    void *sp;                      // current stack pointer
    uint8_t priority;              // 0=highest
    uint8_t currentPriority;       // 0=highest (needed for pi)
    uint32_t stackBytes;            //size of stack need for restart treasd
    uint32_t ticks;                // ticks until sleep complete
    uint64_t TIMEA;                 //writes A reads B WHEN FALSE
    uint64_t TIMEB;                 //WRITES B reads A when true;

    uint64_t srd;                  // MPU subregion disable bits
    char name[16];                 // name of task used in ps command
    uint8_t mutex;                 // index of the mutex in use or blocking the thread
    uint8_t semaphore;             // index of the semaphore that is blocking the thread
} tcb[MAX_TASKS];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------



void InitWTimer(){

        // Enable the Wide Timer module 0 (WTIMER0)
        SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;  // Enable WTIMER0
        _delay_cycles(3);  // Allow time for the Wide Timer to be ready

        WTIMER0_CTL_R &= ~TIMER_CTL_TAEN;  // Turn off the timer before reconfiguring
        WTIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;  // Configure WTIMER0 as a 32-bit timer (both A and B)
        WTIMER0_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR;// Set the timer mode to One-Shot (TAMR_TAMR_1_SHOT) with down-counting (TACDIR)
        WTIMER0_TAILR_R = 40000000;   // Set the load value for the timer (40000000 for 2.5 seconds with a 16 MHz clock)

        NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_USAGE|NVIC_SYS_HND_CTRL_BUS|NVIC_SYS_HND_CTRL_MEM;//enable the mpu seg and hard fault
       NVIC_FAULT_STAT_R |=NVIC_FAULT_STAT_DIV0;

}


bool initMutex(uint8_t mutex)
{
    bool ok = (mutex < MAX_MUTEXES);
    if (ok)
    {
        mutexes[mutex].lock = false;
        mutexes[mutex].lockedBy = 0;
    }
    return ok;
}

bool initSemaphore(uint8_t semaphore, uint8_t count)
{
    bool ok = (semaphore < MAX_SEMAPHORES);
    {
        semaphores[semaphore].count = count;
    }
    return ok;
}

// REQUIRED: initialize systick for 1ms system timer
void initRtos(void)
{
    uint8_t i;
    // no tasks running
    taskCount = 0;
    // clear out tcb records
    for (i = 0; i < MAX_TASKS; i++)
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
    }
    InitWTimer();//enables timer for cpu time and the faults
    allowFlashAccess();
    allowPeripheralAccess();
    setupSramAccess();
    MPUON();
    NVIC_ST_CTRL_R |=NVIC_ST_CTRL_CLK_SRC|NVIC_ST_CTRL_INTEN;
    NVIC_ST_RELOAD_R |=40000;//set the 1 ms inturput
    NVIC_ST_CTRL_R|=NVIC_ST_CTRL_ENABLE;//turns on the sytick
}

int ComPrio(const void *a , const void*b){
    uint8_t Indexa=*(uint8_t*)a;
    uint8_t Indexb=*(uint8_t*)b;


        return (int)(tcb[Indexb].priority-tcb[Indexa].priority);
}


//my function for redundant checking checks the prio
uint8_t loopForPrio(uint8_t prio, uint8_t taskRan){
    uint8_t run=taskRan+1;
    for(run;run<taskCount;run++){//check the next task that was ran in the prio leve
        if(tcb[run].state == STATE_READY && tcb[run].currentPriority==prio){
                            return run;
                            break;
        }
    }
    for(run=0;run<taskCount;run++){//check at the being if not found at the end
        if(tcb[run].state == STATE_READY && tcb[run].currentPriority==prio){
            return run;
            break;

        }
    }
}

// REQUIRED: Implement prioritization to NUM_PRIORITIES
uint8_t rtosScheduler(void)
{

    static uint8_t task = 0xFF;
    //these can be used if nessary later
    static uint8_t Prio0T=0;
    static uint8_t Prio4T=0;//index of the array that was used in the array
    static uint8_t Prio8T=0;
    static uint8_t Prio12T=0;
   // static uint8_t Prio15T=0;

    if (priorityScheduler) {// have to fix this becuase not runing the shell in prio
        uint8_t run=0;
        uint8_t taskP = 15;
        for ( run=0; run < taskCount; run++) {
            if (tcb[run].state == STATE_READY && taskP>tcb[run].currentPriority) {
                taskP=tcb[run].currentPriority;
                if(taskP==0)break;
            }
        }
        switch (taskP){
        case 0:
            task= loopForPrio(taskP, Prio0T);
            Prio0T=task;
            return task;
        case 4:
            task= loopForPrio(taskP, Prio4T);
            Prio4T=task;
            return task;
        case 8:
            task= loopForPrio(taskP, Prio8T);
            Prio12T=task;
            return task;
        case 12:
            task= loopForPrio(taskP, Prio12T);
            Prio12T=task;
            return task;
        case 15:
            task= 15;
            return task;
        }
    }
    if(!priorityScheduler){//for round robin schedualer
        bool ok;

            ok = false;
        while (!ok)
           {
               task++;
               if (task >= MAX_TASKS)
                   task = 0;
               ok = (tcb[task].state == STATE_READY);
           }
    }
    return task;
}

// REQUIRED: modify this function to start the operating system
// by calling scheduler, set srd bits, setting PSP, ASP bit, call fn with fn add in R0
// fn set TMPL bit, and PC <= fn
void startRtos(void)
{
    uint64_t SrdMask=createNoSramAccessMask();
    addSramAccessWindow(&SrdMask,0x20007C00,0x401);
    applySramAccessMask(SrdMask);
    SetPSP(0x20008000);
    SetASP();
    SetTMPL(1);
   // launchFirstTask();
    __asm(" SVC #0");
}

// REQUIRED:
// add task if room in task list
// store the thread name
// allocate stack space and store top of stack in sp and spInit
// set the srd bits based on the memory allocation
// initialize the created stack to make it appear the thread has run before
bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes)
{
    bool ok = false;

    uint8_t i = 0;
    bool found = false;
    if (taskCount < MAX_TASKS)
    {
        // make sure fn not already in list (prevent reentrancy)
        while (!found && (i < MAX_TASKS))
        {
            found = (tcb[i++].pid ==  fn);
        }
        if (!found)
        {
            // find first available tcb record
            i = 0;
            while (tcb[i].state != STATE_INVALID) {i++;}
            taskCurrent=i;
            tcb[i].pid = fn;
            uint32_t * curSp=(uint32_t*)mallocFromHeap(stackBytes);
            tcb[i].stackBytes=stackBytes;

            tcb[i].state = STATE_READY;

            uint8_t run=0;
            while(name[run] !='\0'){
                tcb[i].name[run] =name[run];
                run++;
            }
            tcb[i].name[run] ='\0';

            tcb[i].sp = curSp+stackBytes/4;
            tcb[i].spInit =  tcb[i].sp;
            tcb[i].priority = priority;
            tcb[i].currentPriority=priority;
            tcb[i].TIMEA=0;
            tcb[i].TIMEB=0;
            tcb[i].semaphore=4;
            uint64_t srdMask= createNoSramAccessMask();//check here
            addSramAccessWindow(&srdMask,(uint32_t)curSp,stackBytes);
            tcb[i].srd = srdMask;//for indviald srd mask
            // increment task count
            taskCount++;
            ok = true;
        }
    }
    uint32_t * ptr=tcb[i].spInit ;
    *(--ptr)=1<<24;//Xpsr flip the thumb bit
    *(--ptr)=(uint32_t)fn;//PC
    ptr-=14;
    *(--ptr)=0xFFFFFFFD; //(s/w)bxlr
    tcb[i].sp=ptr;
    return ok;
}

_fn getPID(){
    return tcb[taskCurrent].pid;
}

void runThread(uint8_t task){
        uint8_t taskrest=taskCurrent;
        taskCurrent=task;
       // tcb[task].pid = fn;
        uint32_t * curSp=(uint32_t*)mallocFromHeap(tcb[task].stackBytes);
        tcb[task].state = STATE_READY;
        tcb[task].sp = curSp+tcb[task].stackBytes/4;
        tcb[task].spInit =  tcb[task].sp;
        uint64_t srdMask= createNoSramAccessMask();//check here
        addSramAccessWindow(&srdMask,(uint32_t)curSp,tcb[task].stackBytes);
        tcb[task].srd = srdMask;//for indviald srd mask
        uint32_t * ptr=tcb[task].spInit ;
        *(--ptr)=1<<24;//Xpsr flip the thumb bit
        *(--ptr)=(uint32_t)tcb[task].pid;//PC
        ptr-=14;
        *(--ptr)=0xFFFFFFFD; //(s/w)bxlr
        tcb[task].sp=ptr;
        taskCurrent=taskrest;
        return;
}

// REQUIRED: modify this function to restart a thread
void restartThread(_fn fn)
{
    __asm(" SVC #12");
    return;

}

void killThread(_fn taskP){//fix the semaphore posting the function for readkeys
       uint8_t run=0;
       uint8_t task=0;
       bool found=false;
       for(run=0;run<taskCount;run++){
           if(tcb[run].pid==taskP && (tcb[run].state!= STATE_STOPPED)){
               task=run;
               found= true;
           }
       }

       if(found){
            freeToHeap(tcb[task].pid);
            tcb[task].srd=createNoSramAccessMask();
            tcb[task].sp = NULL;
            tcb[task].spInit =NULL;
            if(mutexes[0].lockedBy==task){//check if the muutex is beign used
                mutexes[0].lock = false;
                mutexes[0].lockedBy = 0;
                tcb[task].mutex=0;
                if(mutexes[0].queueSize>0){//set next task ready set mutex lock by next and also decemernt the quecount anddequeue
                    tcb[mutexes[0].processQueue[0]].state=STATE_READY;
                    tcb[mutexes[0].processQueue[0]].mutex=0;//setting back to none
                    mutexes[0].lockedBy=mutexes[0].processQueue[0];
                    mutexes[0].lock =true;
                    mutexes[0].queueSize--;
                    mutexes[0].processQueue[0]=mutexes[0].processQueue[1];
                    mutexes[0].processQueue[1]=0;
                }
            }
            else if(tcb[task].state ==STATE_BLOCKED_MUTEX){//cheing mutex queue
                if(mutexes[0].processQueue[0]==task){//check i
                    mutexes[0].queueSize--;
                    mutexes[0].processQueue[0]=mutexes[0].processQueue[1];
                    mutexes[0].processQueue[1]=0;
                    tcb[task].mutex=0;
                }
                else{
                    mutexes[0].queueSize--;
                    mutexes[0].processQueue[1]=0;
                    tcb[task].mutex=0;
                }

            }

            else if(tcb[task].state ==STATE_BLOCKED_SEMAPHORE){//for sema
                    for(run=0;run<3;run++){
                            if(semaphores[run].processQueue[0]==task){
                                semaphores[run].processQueue[0]=semaphores[run].processQueue[1];
                                semaphores[run].processQueue[1]=0;
                                semaphores[run].queueSize--;
                                break;
                                }
                            else if(semaphores[run].processQueue[1]==task){
                                semaphores[run].processQueue[1]=0;
                                semaphores[run].queueSize--;
                                break;
                                 }

                    }
                 }

            else if(tcb[task].semaphore!=4){
                semaphores[tcb[task].semaphore].count++;
                if(semaphores[tcb[task].semaphore].queueSize>0){
                    tcb[semaphores[tcb[task].semaphore].processQueue[0]].state=STATE_READY;
                   // tcb[semaphores[tcb[task].semaphore].processQueue[0]].semaphore=4;//5meainf its
                    semaphores[tcb[task].semaphore].queueSize--;
                    semaphores[tcb[task].semaphore].processQueue[0]=semaphores[tcb[task].semaphore].processQueue[1];
                    semaphores[tcb[task].semaphore].processQueue[1]=0;
                    semaphores[tcb[task].semaphore].count--;
                }
            }


                tcb[task].state=STATE_STOPPED;
       }

}

// REQUIRED: modify this function to stop a thread
// REQUIRED: remove any pending semaphore waiting, unlock any mutexes
void stopThread(_fn fn)
{
    __asm(" SVC #10");
       return;
}



// REQUIRED: modify this function to set a thread priority
void setThreadPriority(_fn fn, uint8_t priority)
{
    __asm(" SVC #21");
          return;


}

// REQUIRED: modify this function to yield execution back to scheduler using pendsv
void yield(void)
{
    __asm(" SVC #1");

}

// REQUIRED: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses using pendsv
void sleep(uint32_t tick)
{
    __asm(" SVC #2");
}

// REQUIRED: modify this function to lock a mutex using pendsv
void lock(int8_t mutex)
{
    __asm(" SVC #3");

}

// REQUIRED: modify this function to unlock a mutex using pendsv
void unlock(int8_t mutex)
{
    __asm(" SVC #4");
}

// REQUIRED: modify this function to wait a semaphore using pendsv
void wait(int8_t semaphore)
{
    __asm(" SVC #5");
}

// REQUIRED: modify this function to signal a semaphore is available using pendsv
void post(int8_t semaphore)
{
    __asm(" SVC #6");
}

// REQUIRED: modify this function to add support for the system timer
// REQUIRED: in preemptive code, add code to request task switch
void systickIsr(void)
{
    static uint32_t Clockpast=0;
    Clockpast++;
   // in SysTickISR you will check if a task has to sleep (DELAY_STATE) and if it does decrement its tick varible by one onece the tick variable reaches zero, set it ready
    uint8_t run=0;
    for(run=0;run<taskCount;run++){
        if(tcb[run].state== STATE_DELAYED ){
                tcb[run].ticks--;
                if(tcb[run].ticks==0)tcb[run].state=STATE_READY;
            }
    }
    //add the premeption flags when set it sould call the pendsive
    if(Clockpast>500){
        switchTime^=1;
        Clockpast=0;
        for(run=0;run<taskCount;run++){
            if(switchTime){
                tcb[run].TIMEB=0;
            }
            else{
                tcb[run].TIMEA=0;
            }
        }
    }
    if(preemption)NVIC_INT_CTRL_R|=NVIC_INT_CTRL_PEND_SV;

}

// REQUIRED: in coop and preemptive, modify this function to add support for task switching
// REQUIRED: process UNRUN and READY tasks differently
__attribute__((naked)) void pendSvIsr(void)//Manual Stack Management: is the what the naked attribute does
{
    if(NVIC_FAULT_STAT_R &NVIC_FAULT_STAT_IERR) NVIC_FAULT_STAT_R &= ~(NVIC_FAULT_STAT_IERR);
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_DERR)NVIC_FAULT_STAT_R &= ~(NVIC_FAULT_STAT_DERR);

    Push();//fix the push
    tcb[taskCurrent].sp=(void*)(GetPSP());//fix the push

    if(switchTime){
        tcb[taskCurrent].TIMEB+=WTIMER0_TAV_R;
    }
    else{
        tcb[taskCurrent].TIMEA+=WTIMER0_TAV_R;
    }

    WTIMER0_CTL_R &= ~TIMER_CTL_TAEN;

    taskCurrent=rtosScheduler();
    WTIMER0_TAV_R=0;
    WTIMER0_CTL_R |= TIMER_CTL_TAEN;


    applySramAccessMask(tcb[taskCurrent].srd);
    SetPSP((uint32_t)tcb[taskCurrent].sp);
    //tcb[taskCurrent].sp =(void*)(GetPSP()+36);
    Pop();
}




// REQUIRED: modify this function to add support for the service call
// REQUIRED: in preemptive code, add code to handle synchronization primitives
void svCallIsr(void)
{
        uint32_t *PSP =(uint32_t*) GetPSP();
        meminfo (*tempmemin)= NULL;//(meminfo*)(*PSP);

        PS (*tempPS)= NULL;//(PS*)(*PSP);
        IPSC (*temIPSC)=NULL;//(IPSC*)(*PSP);

        uint8_t *pc=(uint8_t*)*(PSP+6);


        uint8_t SVCNUM =*(pc-2);
        uint32_t NUMref= *PSP;
         uint8_t priorityTemp=(uint8_t)*(PSP+1);


        uint32_t *retPtr;
        uint64_t srdMask=0;
        uint64_t Numtop=0;

        MPUEntry *MAllocINfo =NULL;
        char *name =(char*)(*PSP);

        uint8_t run=0;
        uint8_t run2=0;
        uint8_t run3=0;

        bool found=false;
        //static temprun=0;

        switch(SVCNUM){
        case 0://frist task runing
            taskCurrent=rtosScheduler();
            applySramAccessMask(tcb[taskCurrent].srd);
            SetPSP((uint32_t)tcb[taskCurrent].sp);
          //  tcb[taskCurrent].sp =(void*)(GetPSP()+36);
            Pop();
            break;
        case 1://for yeild
            NVIC_INT_CTRL_R|=NVIC_INT_CTRL_PEND_SV;//a pends isvcall
            break;

        case 2://for sleep
            tcb[taskCurrent].ticks=NUMref;//get the num of ticks
            tcb[taskCurrent].state= STATE_DELAYED;
            NVIC_INT_CTRL_R|=NVIC_INT_CTRL_PEND_SV;
            break;
        case 3://for lock
            if(NUMref < MAX_MUTEXES){
                if(mutexes[NUMref].lock == false){
                    mutexes[NUMref].lock =true;
                    mutexes[NUMref].lockedBy =taskCurrent;
                }
                else{
                    if(mutexes[NUMref].queueSize<MAX_MUTEX_QUEUE_SIZE){
                        tcb[taskCurrent].state= STATE_BLOCKED_MUTEX;
                        tcb[taskCurrent].mutex=NUMref;
                        mutexes[NUMref].processQueue[mutexes[NUMref].queueSize]=taskCurrent;
                        mutexes[NUMref].queueSize++;
                        NVIC_INT_CTRL_R|=NVIC_INT_CTRL_PEND_SV;//yield();
                    }
                }

            }
            break;
        case 4://unlock
            if(mutexes[NUMref].lockedBy ==taskCurrent){
                mutexes[NUMref].lock = false;
                mutexes[NUMref].lockedBy = 0;
                if(mutexes[NUMref].queueSize>0){//set next task ready set mutex lock by next and also decemernt the quecount anddequeue
                    tcb[mutexes[NUMref].processQueue[0]].state=STATE_READY;
                    tcb[mutexes[NUMref].processQueue[0]].mutex=0;//setting back to none
                    mutexes[NUMref].lockedBy=mutexes[NUMref].processQueue[0];
                    mutexes[NUMref].lock =true;
                    mutexes[NUMref].queueSize--;
                    mutexes[NUMref].processQueue[0]=mutexes[NUMref].processQueue[1];
                    mutexes[NUMref].processQueue[1]=0;
                }
            }
            else{//kill proce by free from heap make task set as
                killThread(tcb[taskCurrent].pid); //this code clear srd bits for the process deque for its quee if it is inside a queu
                NVIC_INT_CTRL_R|=NVIC_INT_CTRL_PEND_SV;
            }

            break;
        case 5://for wait using the semaphore
                if(semaphores[NUMref].count>0){
                    semaphores[NUMref].count--;
                    }
                else{
                    semaphores[NUMref].processQueue[semaphores[NUMref].queueSize]=taskCurrent;
                    semaphores[NUMref].queueSize++;
                    tcb[taskCurrent].state=STATE_BLOCKED_SEMAPHORE;
                    tcb[taskCurrent].semaphore=NUMref;
                    NVIC_INT_CTRL_R|=NVIC_INT_CTRL_PEND_SV;
                    }
            break;
        case 6://for the post using hte semphores
            semaphores[NUMref].count++;
            if(semaphores[NUMref].queueSize>0){
                tcb[semaphores[NUMref].processQueue[0]].state=STATE_READY;
                //tcb[semaphores[NUMref].processQueue[0]].semaphore=4;//no longer using sem
                semaphores[NUMref].queueSize--;
                semaphores[NUMref].processQueue[0]=semaphores[NUMref].processQueue[1];
                semaphores[NUMref].processQueue[1]=0;
                semaphores[NUMref].count--;
            }

            break;
        case 7://for the reboot command
            NVIC_APINT_R =(NVIC_APINT_VECTKEY |4);
            break;
        case 8: //for the IPCS command display allt he memeorty info
            break;
        case 9://preempt on =1 off=0
            if(NUMref==1)preemption=true;
            else preemption = false;
            return;
        case 10: //stop trhead
            run=0;
            for(run=0;run<taskCount;run++){
                uint32_t temp=(uint32_t )tcb[run].pid;
                if(temp==NUMref && !(tcb[run].state==STATE_STOPPED)){
                    run2=run;
                    found =true;
                    killThread(tcb[run].pid);
                    break;
                }
            }
            return;
        case 11: // Prioty sched
            if(NUMref==1)priorityScheduler=true;
            else priorityScheduler=false;
            return;
        case 12://taskrest
                run=0;
                for(run=0;run<taskCount;run++){
                    uint32_t temp=(uint32_t )tcb[run].pid;
                    if(temp==NUMref && tcb[run].state==STATE_STOPPED){
                        runThread(run);
                        break;
                    }
                }
               return;

            //load into reg R0 drefrence to get the name of number
        case 13: //for malloc function wraper
            retPtr=(uint32_t*)mallocFromHeap(NUMref);
            srdMask= tcb[taskCurrent].srd;
            addSramAccessWindow(&srdMask,(uint32_t)retPtr,NUMref);
            tcb[taskCurrent].srd = srdMask;//for indviald srd mask
            applySramAccessMask(tcb[taskCurrent].srd);
            *(PSP)=retPtr;
            return;
        case 14://pid of comand
            for(run=0;run<taskCount;run++){
                run2=0;
                while(tcb[run].name[run2] !='\0'){
                    if(tcb[run].name[run2] ==name[run2]){
                        found=true;
                    }
                    else{
                        found=false;
                        break;
                    }
                    run2++;
                }
                if(found){
                    *(PSP)=(tcb[run].pid);
                    return;
                }
            }
            *(PSP)=0;
            return;
        case 15://kill given the pid
            for(run=0;run<taskCount;run++){
                uint32_t temp=(uint32_t )tcb[run].pid;
                if(temp==NUMref){
                    killThread(tcb[run].pid);//stopThread(tcb[run].pid);
                    *(PSP)=1;
                    return;
                }
            }
            *(PSP)=0;
            return ;

        case 16://pkill given the name
            for(run;run<taskCount;run++){
                run2=0;
                while(tcb[run].name[run2] !='\0'){
                    if(tcb[run].name[run2] ==name[run2]){
                        found=true;
                    }
                    else{
                        found=false;
                        break;
                    }
                    run2++;
                }
                if(found & (tcb[run].state!= STATE_STOPPED) ){
                    killThread(tcb[run].pid);//stopThread(tcb[run].pid);

                    *(PSP)=1;
                    return;
                }
            }
            *(PSP)=0;
            return;

        case 17://ps
            tempPS=(PS*)(*PSP);
            run=0;
            srdMask=0;
            for(run=0;run<taskCount;run++){
                if(switchTime)
                    srdMask+=tcb[run].TIMEA;

                else{
                    srdMask+=tcb[run].TIMEB;
                }
            }

            for(run=0;run<taskCount;run++){
                run2=0;
                while(tcb[run].name[run2] !='\0'){
                    tempPS->name[run2]=tcb[run].name[run2];
                    run2++;
                }
                tempPS->name[run2] ='\0';
                tempPS->pid=tcb[run].pid;                     // used to uniquely identify thread (add of task fn)
                if(switchTime){
                    Numtop=tcb[run].TIMEA*100*100;
                }
                else{
                    Numtop=tcb[run].TIMEB*100*100;
                }
                Numtop=Numtop/srdMask;
                tempPS->CPUtime=Numtop;
                tempPS->state=tcb[run].state; ///if 4 blockedd by semapot five blocked by mux
                tempPS->mutex=tcb[run].mutex;                 // index of the mutex in use or blocking the thread
                tempPS->semaphore=tcb[run].semaphore;
                tempPS++;
            }
            *(PSP)=taskCount;
            return;

        case 18://ipcs

            temIPSC=(IPSC*)(*PSP);
                temIPSC->lock=mutexes[0].lock;
                temIPSC->MUXqueueSize=mutexes[0].queueSize;
                for(run=0;run<mutexes[0].queueSize;run++){
                    run2=0;
                    while(tcb[mutexes[0].processQueue[run]].name[run2]!='\0'){
                        temIPSC->processQueue[run][run2]=tcb[mutexes[0].processQueue[run]].name[run2] ;
                        run2++;
                    }
                    temIPSC->processQueue[run][run2]='\0';
                }
                run2=0;
                while(tcb[mutexes[0].lockedBy].name[run2] != '\0'){
                    temIPSC->lockedBy[run2]=tcb[mutexes[0].lockedBy].name[run2];

                    run2++;
                }
                temIPSC->lockedBy[run2]='\0';

                for(run=0;run<3;run++){
                    temIPSC->SEMcount[run]=semaphores[run].count;
                    temIPSC->SEMqueueSize[run]=semaphores[run].queueSize;
                    for(run3=0;run3<semaphores[run].queueSize;run3++){
                        run2=0;
                        while(tcb[semaphores[run].processQueue[run3]].name[run2] !='\0'){
                            temIPSC->SEMprocessQueue[run][run3][run2]= tcb[semaphores[run].processQueue[run3]].name[run2];
                            run2++;
                        }
                        temIPSC->SEMprocessQueue[run][run3][run2]='\0';
                    }

                }

                return;
        case 19://memeinfo is done
            run3=0;
            tempmemin= (meminfo*)(*PSP);
            MAllocINfo=meminfoCall(&run3);
            for(run=0;run<run3;run++){
                tempmemin->pid=(uint32_t)(MAllocINfo->PID);
                tempmemin->FreeAdd=MAllocINfo->FreeAdd;
                tempmemin->region=MAllocINfo->region;
                tempmemin->size= (uint32_t)MAllocINfo->size;
                tempmemin->taskname=(uint32_t)MAllocINfo->pid;
                tempmemin++;
                MAllocINfo++;
            }
            *(PSP)=run3;
            return;

        case 20: //restared thread /run ing thered
            run=0;
            run3=0;
            for(run=0;run<taskCount;run++){
            run2=0;
            while(tcb[run].name[run2] !='\0'){
                if(tcb[run].name[run2] ==name[run2]){
                    run3=run;
                    found=true;
                }
                else{
                    found=false;
                    break;
                }
                run2++;
            }
            if(found && (tcb[run3].state== STATE_STOPPED) ){
                runThread(run3);
                *(PSP)=(1);
                return;
            }
            }
            *(PSP)=0;
            return;

        case 21://setThreadPriority
           run=0;
                for(run=0;run<taskCount;run++){
                    if(NUMref== (uint32_t)tcb[run].pid){
                        tcb[run].currentPriority=priorityTemp;
                        return;
                    }
                }
            break;
        }

}

