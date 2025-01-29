// Shell functions
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
#include "tm4c123gh6pm.h"
#include "shell.h"
#include "uart0.h"//maybe remove
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
// REQUIRED: Add header files here for your strings functions, ...
#define MAX_CHARS 80
#define MAX_FIELDS 5
typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
// Blocking function that writes a serial character when the UART buffer is not full
extern void putcUart0(char c);

// Blocking function that writes a string when the UART buffer is not full
extern void putsUart0(char* str);

// Blocking function that returns with serial data once the buffer is not empty
extern char getcUart0();

void getsUart0(USER_DATA *data){
    int count =0;
    while(1){
        if(kbhitUart0()){
        char letter=getcUart0();
        //printable charter or space
        if ((letter<127 && letter>=32) || letter==32){
            data->buffer[count]=letter;
            count++;
            if(count ==MAX_CHARS){
                data->buffer[count]='\0';
                break;
            }
        }
        //if backspace decement count
        if((letter==8 || letter==127)&& count >0){
            count=count-1;
        }
        //add null
        if(letter==13){
            data->buffer[count]='\0';
            break;
        }
    }
        else{
            __asm(" SVC #1");//yeild();
        }
    }
    return;
}
void parseFields(USER_DATA *data){
        int count=0;
        int countS=0;
        int flag =0;
        data->fieldCount=countS;
        while(true){
            char temp=data->buffer[count];
            if(temp=='\0')break;
            if(countS==MAX_FIELDS)break;
            //checks if it s a valid input
            if((temp >='A' && temp <='Z') || (temp>='a' &&  temp<='z') || (temp>='0' && temp<='9')){
                //checks a-z A-Zin the buffer
                if( (temp >='A' && temp <='Z') || (temp>='a' &&  temp<='z')){
                    if(flag==0){
                    data->fieldType[countS]='a';
                    data->fieldPosition[countS]=count;
                    flag=1;
                    countS++;
                    }
                }
                  //0-9 in the buffer
                if ((temp>='0' && temp<='9')){
                    if(flag==0){
                        data->fieldType[countS]='n';

                        data->fieldPosition[countS]=count;
                        flag=1;
                        countS++;
                     }
                 }
            }
             else{//ads null to delimter
                 data->buffer[count]='\0';
                 flag=0;
             }
             count++;
          }

        data->fieldCount=countS;
          return;
}

size_t LenOfStr(const char *str){
    const char *s=str;
    while(*s){
        s++;
    }
    return (s-str);
}
char* getFieldString(USER_DATA* data,uint8_t fieldnumber){
    int count = data->fieldCount;
    char *ret=NULL;
    if(fieldnumber<=count){
        if(data->fieldType[fieldnumber]=='a' || data->fieldType[fieldnumber]=='n'){ //so i can get a num in string form
            int field =data->fieldPosition[fieldnumber];

                ret=&(data->buffer[field]);
        }
    }
    return ret;
}

int32_t getFieldInteger(USER_DATA* data,uint8_t fieldnumber){
    int count = data->fieldCount;
    int ret =0;
    if(fieldnumber<=count){
        if(data->fieldType[fieldnumber]=='n'){
            int field =data->fieldPosition[fieldnumber];

            ret=atoi(&(data->buffer[field]));
        }
    }
    return ret;
}
bool isHex(const char *str){
    if(str==NULL||*str=='\0')return false;
    if(str[0]=='0'&&(str[1]=='x' ||str[1]=='X')){
        str+=2;
    }
    else return false;


    while(*str){
        if(!((*str >= '0'&& *str <='9')|| (*str >='A'&& *str <='F')||(*str >='a'&& *str <='f')) ){
            return false;
        }
        str++;

    }
    return true;

}



uint32_t STRhexToint(const char *str){
    uint32_t ret=0;
    str+=2;//skips 0x

    while(*str){
        char let= *str;
        uint32_t val;
        if(let>='0' && let<='9'){
            val= let -'0';
        }
        else if(let>='A'&& let<='F'){
            val =let -'A'+10;
        }
        else if(let>='a'&& let<='f'){
            val =let -'a'+10;
        }
        if(ret>(UINT32_MAX -val)/16)return 0;
        ret=ret*16+val;
        str++;
    }
    return ret;

}



bool isCommand(USER_DATA* data,const char strCommand[],uint8_t minArguments){
    bool ret=true;
    int check=data->fieldCount;
    if((check)<minArguments)ret=false;
    else{
        if(!(LenOfStr(strCommand)==LenOfStr(data->buffer)))return false;
        int run=0;

        while(true){
            char temp=data->buffer[run];
            if(temp=='\0') break;
            if(temp!=strCommand[run]){
                ret=false;
                break;
            }
            run++;
        }
    }
    return ret;

}


bool Stringcmp( const char str[],const char strCommand[]){
    int run=0;
    while(true){
        if(str[run]=='\0')break;
        if(str[run]!=strCommand[run]){
            return false;
        }
        run++;
    }
    return true;
}



void* ps(PS*ptr){
    __asm(" SVC #17");
        return;
}

void* kill(uint32_t pid){
    __asm(" SVC #15");
    return;
}
void* pkill(const char name[]){//unit32
    __asm(" SVC #16");
    return;
}


void pi(bool on){
    if(on==true)putsUart0("pi on");
    else putsUart0("pi off");
    return;
}

void* MEMEINFO(meminfo* ptr){
    __asm(" SVC #19");
        return;
}

void* pidof(char name[]){//check for erros
    __asm(" SVC #14");
    return ;
}

void* ipcs(IPSC*ptr){
    __asm(" SVC #18");
       return ;
}

void*runTask(char name[]){
    __asm(" SVC #20");
    return ;
}

void SCHED(uint8_t val){
    __asm(" SVC #11");
            return ;
}

void PREON(uint8_t val){
    __asm(" SVC #9");
        return ;
}

bool procLiv(const char temp[]){
    int run=0;
    bool ret=false;
        for(run;run<3;run++){
        //if(true==Stringcmp(proc_names[run],temp)){
            ret=true;
            break;
        //}
    }
   return ret;
}



// REQUIRED: add processing for the shell commands through the UART here
void shell(void)
{
    USER_DATA data;
    while(true){
        if(kbhitUart0()){
            getsUart0(&data);
            parseFields(&data);

            if(isCommand(&data,"ipcs",1)){
                IPSC tempipcs[1];
                char tempName [16];
                char tempSizeque[3];
                uint8_t run=0;
                uint8_t run2=0;
                ipcs(&tempipcs);
                putsUart0("MUTEX INFO:");
                putsUart0("\n");
                putsUart0("Locked          Locked-by    QueSize     Queue");
                putsUart0("\n");
                if(tempipcs[0].lock){
                    snprintf(tempName, sizeof(tempName), "%-23s","LOCKED");
                    putsUart0(tempName);
                    putsUart0(" ");
                    snprintf(tempName, sizeof(tempName), "%-16s", tempipcs[0].lockedBy);
                    putsUart0(tempName);

                }else{
                    snprintf(tempName, sizeof(tempName), "%-23s","NOT-LOCKED");
                    putsUart0(tempName);
                    putsUart0("                ");
                }

                sprintf(tempSizeque, "%d", tempipcs[0].MUXqueueSize);
                putsUart0(tempSizeque);
                putsUart0("         ");
                for(run;run<tempipcs[0].MUXqueueSize;run++){
                    snprintf(tempName, sizeof(tempName), "%-16s", tempipcs[0].processQueue[run]);
                   putsUart0(tempName);
                   putsUart0("   ");

                }
                putsUart0("\n");
                putsUart0("\n");
                putsUart0("SEMAPHORE INFO:");
                putsUart0("\n");
                putsUart0("SEM #       Count    QueSize     Queue");
                putsUart0("\n");

                for(run=0;run<3;run++){
                    sprintf(tempSizeque, "%d", run);
                    putsUart0("   ");
                    putsUart0(tempSizeque);
                    putsUart0("         ");
                    sprintf(tempSizeque, "%d", tempipcs[0].SEMcount[run] );
                    putsUart0(tempSizeque);
                    putsUart0("         ");

                    sprintf(tempSizeque, "%d", tempipcs[0].SEMqueueSize[run] );
                    putsUart0(tempSizeque);
                    putsUart0("         ");

                    for(run2=0;run2<tempipcs[0].SEMqueueSize[run];run2++){
                            snprintf(tempName, sizeof(tempName), "%-16s", tempipcs[0].SEMprocessQueue[run][run2]);
                            putsUart0(tempName);
                            putsUart0("     ");
                    }

                    putsUart0("\n");

            }
                putsUart0("\n");
            }
            else if(isCommand(&data,"memeinfo",1)){//is done
                char tempPid[9];
                char tempsize[6];
                meminfo Tempmeminfo[12];
                uint8_t taskNUM= MEMEINFO(&Tempmeminfo);
                uint8_t run=0;
                uint32_t pidnum=0;
                putsUart0("MEMEINFO:");
                putsUart0("\n");
                putsUart0("PID            baseAddres     TopAddres   Region   Size");
                putsUart0("\n");


                for(run=0;run<taskNUM;run++){
                    pidnum= Tempmeminfo[run].pid;

                    snprintf(tempPid, sizeof(tempPid), "%08X", pidnum);
                    putsUart0("0x");
                    putsUart0(tempPid);
                    putsUart0("     ");

                    pidnum= (uint32_t)Tempmeminfo[run].taskname;
                    snprintf(tempPid, sizeof(tempPid), "%08X", pidnum);
                    putsUart0("0x");
                    putsUart0(tempPid);
                    putsUart0("     ");
                    pidnum= (uint32_t)Tempmeminfo[run].FreeAdd;
                    snprintf(tempPid, sizeof(tempPid), "%08X", pidnum);
                    putsUart0("0x");
                    putsUart0(tempPid);
                    putsUart0("     ");
                    pidnum=Tempmeminfo[run].region;
                    snprintf(tempsize, sizeof(tempsize), "%d", pidnum);
                    putsUart0(tempsize);
                    putsUart0("     ");
                    pidnum=Tempmeminfo[run].size;
                    snprintf(tempsize, sizeof(tempsize), "%d", pidnum);
                    putsUart0(tempsize);


                    putsUart0("\n");
                }

            }
            else if(isCommand(&data,"reboot",1)){
                putsUart0("reboot called");
                __asm(" SVC #7");
            }
            else if(isCommand(&data,"ps",1)){
                PS tempPS[12];
                char tempCPU[5];
                char tempPid[9];
                char nametemp[16];
                char tempState[23];
                uint8_t run=0;
                uint8_t maxNUm=12;
                uint64_t tempTest=0;



                    maxNUm=ps(&tempPS);

                putsUart0("PID ID         Process        Cpu time    State                     blocked by");
                putsUart0("\n");
                for(run=0;run<maxNUm;run++){
                    uint32_t pidnum= (uint32_t)tempPS[run].pid;
                    snprintf(tempPid, sizeof(tempPid), "%08X", pidnum);
                    putsUart0("0x");
                    putsUart0(tempPid);
                    putsUart0("     ");
                    //putsUart0(tempPS[run].name);
                    snprintf(nametemp, sizeof(nametemp), "%-16s", tempPS[run].name);
                    putsUart0(nametemp);
                    putsUart0(" ");
                    tempTest=tempPS[run].CPUtime;
                    // Calculate the whole part and fractional part
                        int whole_part = tempTest / 100;         // Whole part (1)
                        int fractional_part = tempTest % 100;    // Fractional part (0)

                        // Use sprintf to format the whole and fractional parts into a string
                        sprintf(tempCPU,"%d.%02d", whole_part, fractional_part);


                    putsUart0(tempCPU);
                    putsUart0("%");
                    putsUart0("     ");

                    switch(tempPS[run].state){
                    case 0:
                        snprintf(tempState, sizeof(tempState), "%-23s","STATE_INVALID");
                        putsUart0(tempState);
                        //putsUart0("STATE_INVALID");
                        break;
                    case 1:
                        snprintf(tempState, sizeof(tempState), "%-23s","STATE_STOPPED");
                        putsUart0(tempState);
                        //putsUart0("STATE_STOPPED");
                        break;
                   case 2:
                       snprintf(tempState, sizeof(tempState), "%-23s","STATE_READY");
                       putsUart0(tempState);
                       //putsUart0("STATE_READY");
                       break;
                   case 3:
                       snprintf(tempState, sizeof(tempState), "%-23s","STATE_DELAYED");
                       putsUart0(tempState);
                       //putsUart0("STATE_DELAYED");
                       break;
                   case 4:
                       snprintf(tempState, sizeof(tempState), "%-23s","STATE_BLOCKED_MUTEX");
                       putsUart0(tempState);
                      // putsUart0("STATE_BLOCKED_MUTEX");
                       sprintf(tempPid, "%d", tempPS[run].mutex);
                       putsUart0("     ");
                       putsUart0(tempPid);
                       break;
                   case 5:
                       snprintf(tempState, sizeof(tempState), "%-23s","STATE_BLOCKED_SEMAPHORE");
                       putsUart0(tempState);
                       //putsUart0("STATE_BLOCKED_SEMAPHORE");
                       sprintf(tempPid, "%d", tempPS[run].semaphore);
                       putsUart0("     ");
                       putsUart0(tempPid);
                       break;

                    }
                    putsUart0("\n");
                }

            }
            else if(isCommand(&data,"kill",2)){
                uint32_t pid;
                char*str=getFieldString(&data, 1);
                if(isHex(str)){
                    pid=STRhexToint(str);

                    if(kill(pid)==1)putsUart0("killed");
                    else putsUart0("not found");
                }

            }

            else if(isCommand(&data,"pkill",2)){
                char*com =getFieldString(&data, 1);

                if(pkill(com)==1)putsUart0("Killed useing pkill");
                else putsUart0("Invalid pkill command");
            }
            else if(isCommand(&data,"pi",2)){
                char*com =getFieldString(&data, 1);
                if(Stringcmp(com,"ON")){
                    pi(true);
                }
                else if(Stringcmp(com,"OFF")){
                    pi(false);
                }
                else {
                    putsUart0("Invalid pi command");
                }
           }
            else if(isCommand(&data,"preempt",2)){
                char*com =getFieldString(&data, 1);
                if(Stringcmp(com,"ON")){
                    putsUart0("preempt on");

                    PREON(1);
                }
                else if(Stringcmp(com,"OFF")){
                    putsUart0("preempt OFF");
                    PREON(0);
                }
                else{
                    putsUart0("Invalid preempt command");
                }
            }
            else if(isCommand(&data,"sched",2)){
                char*com =getFieldString(&data, 1);
                if(Stringcmp(com,"PRIO")){
                    putsUart0("sched prio");
                    SCHED(1);
                }
                else if(Stringcmp(com,"RR")){
                    putsUart0("sched rr");
                    SCHED(0);
                }
                else{
                    putsUart0("Invalid sched command");
                }
            }
            else if(isCommand(&data,"pidof",2)){//done for the pidof

                char*com =getFieldString(&data, 1);
                com=pidof(com);
                uint32_t printN=(uint32_t)com;
                if(printN==0)putsUart0("Invalid task");
                else{
                    char tempPid[9];
                    snprintf(tempPid, sizeof(tempPid), "%08X", printN);
                    putsUart0("0x");
                    putsUart0(tempPid);
                }
            }
            else if(isCommand(&data,"help",1)){
                putsUart0("Commands:\n");
                putsUart0("pidof (task)\n");
                putsUart0("preempt (ON|OFF)\n");
                putsUart0("sched (PRIO|RR)\n");
                putsUart0("pkill (task)\n");
                putsUart0("kill (pid)\n");
                putsUart0("reboot\n");
                putsUart0("ipcs\n");
                putsUart0("ps\n");
                putsUart0("memeinfo\n");
                putsUart0("(task)\n");
            }
            else {//fix this and the atoi
                int i=0;
                //for( i;i<data.fieldCount;i++){
                    char*com =getFieldString(&data, 0);
                    if (com[0] >= 'a' && com[0] <= 'z' || com[0] >= 'A' && com[0] <= 'Z' ) {
                        if(runTask(com)){
                            putsUart0("running task");
                        }else{
                            putsUart0("cant fuffill request");
                        }
                    }
                //}

            }
        }
        __asm(" SVC #1");//yeild();

            }
}
