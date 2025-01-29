#include "function.h"
#include <time.h>
#include <stdlib.h>

// Initialize Hardware
uint32_t init_left_tire =0;
uint32_t init_right_tire=0;
uint32_t left_tire_count=0;       //for the inside functionality for every comand
uint32_t right_tire_count=0;      // for the inside functionality for every command
//uint32_t left_tire_count_total=0;     //for the total distance of left tire only resetes when stop
//uint32_t right_tire_count_total=0;//for the total distance of right tire only resets when stop
int32_t left_tire_cmp_val;
int32_t right_tire_cmp_val;
int32_t angle_of_robot;     // the degree of rotation
uint32_t distance =0;      // the distance input
uint8_t set_distance = 0; // a lock for distance
uint8_t LOCK_DIR=0;     // allows to see wich direction whe are moving towards 1 is forward 2 is revreser 0 is no direction
uint8_t lOCK_ROT=0;     // allows to deterime the wheter rotation is being set or not
uint8_t LOCK_Wheesl=0;  //LOCK_Wheesl is used to set the dircetion so no inturput hapens when stoping and steer controle
uint8_t offset =0;
uint8_t motion=0;
//for the remote
uint32_t total_time=0;
uint32_t remote_val=0;
uint32_t dis_sonar0=0;
uint32_t dis_sonar0_time=0;

uint32_t pres_time=0;
//uint32_t cur_time=0;
uint32_t past_time=0;
uint8_t STATE =0;
uint8_t Bits_Read =0;
uint8_t TURN_OFF_ON_AUTO=0;
uint8_t halt_rob_rot =0;
uint8_t modes_main=0;
uint8_t rob_is_stop=0;
uint8_t phase=0;


void intTostring(int val){
    char ret [5];
         int thos = val / 1000;
        if (thos >= 0) {
            char temp = '0' + thos;
            val -= (thos * 1000);
            ret[0] = temp;
        }

        int hun = val / 100;
        if (hun >= 0) {
            char temp = '0' + hun;
            val -= (hun * 100);
            ret[1] = temp;
        }

        int ten = val / 10;
        if (ten >= 0) {
            char temp = '0' + ten;
            val -= (ten * 10);
            ret[2] = temp;
        }

        int one = val / 1;
        if (one >= 0) {
            char temp = '0' + one;
            val -= (one * 1);
            ret[3] = temp;
        }

        ret[4] = '\0'; // Null-terminate the string
        putsUart0(ret);
}
void print_wheel(int wheel_left,int wheel_right){
    putsUart0("left wheel: ");
    intTostring(wheel_left);
    putsUart0("right wheel: ");
    intTostring(wheel_right);
}






void fallinedge(){
    if( GPIO_PORTC_MIS_R & SPEEDONE_MASK){                   //right wheel
        //RED_LED ^= 1;
        right_tire_count++;
        if(4294967293<right_tire_count)right_tire_count=0;

        GPIO_PORTC_IM_R &= ~(SPEEDONE_MASK);                    // turn-off interrupts
        GPIO_PORTC_ICR_R |= SPEEDONE_MASK;                    //clear interrupts
        TIMER1_CTL_R = TIMER_CTL_TAEN;                  // turn-off debounce interrupt
    }
    if( GPIO_PORTC_MIS_R & SPEEDTWO_MASK){                    // left wheel
        //BLUE_LED ^= 1;    //TOggle the BLUE LED
        left_tire_count++;
        if(4294967293<left_tire_count)left_tire_count=0;

        GPIO_PORTC_IM_R &= ~( SPEEDTWO_MASK);                    // turn-off interrupts
        GPIO_PORTC_ICR_R |= SPEEDTWO_MASK;                     // clear the interrupts
        TIMER2_CTL_R = TIMER_CTL_TAEN;                  // turn-off debounce interrupt
    }
    if(GPIO_PORTC_MIS_R & REMOTE_MASK){
           pres_time=WTIMER0_TBV_R;
           int changeT=(pres_time-past_time);
               switch(STATE){
                   case 3:
                       GREEN_LED = 0;
                       if(551489775==remote_val){//turn on and off the remote cablity
                           TURN_OFF_ON_AUTO^=1;
                       }
                       if(TURN_OFF_ON_AUTO==0){
                           if(551494365==remote_val){//stop
                               stop();
                               left_tire_count=0;
                               right_tire_count=0;
                               LOCK_DIR=0;
                           }
                           if(551486205==remote_val){//forward
                               forward();
                               LOCK_DIR=1;
                               set_distance=0;
                               LOCK_Wheesl=0;
                           }
                           if(551518845==remote_val){//backward
                               reverse();
                               LOCK_DIR=2;
                               set_distance=0;
                               LOCK_Wheesl=0;
                           }
                           if(551510175==remote_val){//right
                               cw();
                               LOCK_DIR=0;
                               set_distance=0;
                           }
                           if(551542815==remote_val){//left
                               ccw();
                               LOCK_DIR=0;
                               set_distance=0;
                           }
                       }
                       left_tire_count=0;
                       right_tire_count=0;
                       WTIMER0_TBV_R=0;
                       STATE=0;
                       pres_time=0;
                       total_time=0;
                       Bits_Read=0;
                       remote_val=0;
                       past_time=0;
                       break;
                   case 2:
                       GREEN_LED = 1;
                       if(80000<=changeT && changeT <=100000 ){//if(2 ms <time && time <2.5 ms ) time should be 2.25 ms
                           remote_val=remote_val<<1;
                           remote_val+=1;
                           total_time+=(pres_time-past_time);//value in clock for '1'
                           past_time=pres_time;
                       }
                       if(35000<=changeT && changeT <=55000){//if( .875 ms <time && time<1.375 ms) time should be 1.125 ms
                           remote_val=remote_val<<1;
                           total_time+=(pres_time-past_time);//value in clock for '0'
                           past_time=pres_time;
                       }
                       Bits_Read++;
                       if(total_time >=2800000 || Bits_Read>=33  ){//total_time >=2800000 || Bits_Read>=33
                           STATE=3;
                      }
                       break;
                   case 1:
                       if(530000<=pres_time && pres_time<=560000){
                           STATE=2;
                           past_time=pres_time;
                       }
                       else{
                           STATE=0;
                       }
                       break;
                   case 0:
                       //RED_LED = 1;
                       STATE=1;
                       WTIMER0_TBV_R=0;
                       break;
               }


           GPIO_PORTC_ICR_R |= REMOTE_MASK;

       }
}


void timer1Isr(void)
{
    GPIO_PORTC_IM_R |= (SPEEDONE_MASK);                 // turn-off key press interrupts (diable the interupt)
    GPIO_PORTC_ICR_R |= SPEEDONE_MASK;
    TIMER1_ICR_R |= TIMER_ICR_TATOCINT;
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                  // turn-off debounce interrupt
}

void timer2Isr(void)
{
    GPIO_PORTC_IM_R |= ( SPEEDTWO_MASK);                    // turn-off key press interrupts
    GPIO_PORTC_ICR_R |= SPEEDTWO_MASK;
    TIMER2_ICR_R |= TIMER_ICR_TATOCINT;
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;                  // turn-off debounce interrupt


}





void getsUart0(USER_DATA *data){
    int count =0;
    while(1){
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
char* getFieldString(USER_DATA* data,uint8_t fieldnumber){
    int count = data->fieldCount;
    char *ret=NULL;
    if(fieldnumber<=count){
        if(data->fieldType[fieldnumber]=='a'){
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

bool isCommand(USER_DATA* data,const char strCommand[],uint8_t minArguments){
    int check=data->fieldCount;
    int lock =0;
    if((check-1)>=minArguments){
        int run=0;

        while(true){
            char temp=data->buffer[run];
            if(temp=='\0') break;
            if(temp!=strCommand[run]){
                lock=1;
                break;
            }
            run++;
        }
    }
    if(lock==1)return false;
    else return true;

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

void Motor_Driver(int val1,int val2,int val3,int val4){
           PWM0_0_CMPA_R = val1;
           PWM0_0_CMPB_R = val2;//PB6
           PWM0_1_CMPA_R = val3;
           PWM0_1_CMPB_R = val4;//PB7
}

/*
void forward(){
    //for steer corection
    left_tire_cmp_val=1023;
    right_tire_cmp_val=998;

    init_left_tire=left_tire_cmp_val;
    init_right_tire=right_tire_cmp_val;

    Motor_Driver(0,1023,0,998);
}*/
void forward(){
    //for steer corection
    left_tire_cmp_val=1018;//orginal 1023
    right_tire_cmp_val=1023;//998 was the orginal ofset

    init_left_tire=left_tire_cmp_val;
    init_right_tire=right_tire_cmp_val;

    Motor_Driver(0,1018,0,1023);
}
void reverse(){
    //for steer corection
    left_tire_cmp_val=1023;
    right_tire_cmp_val=995;

    init_left_tire=left_tire_cmp_val;
    init_right_tire=right_tire_cmp_val;
    Motor_Driver(1023,0,995,0);
}

//value for comp y = (13/240)*x+523
int32_t compvale(int32_t speedP){

    int32_t val =13*speedP;
    int32_t cmp= val/240;
    cmp =cmp+523;
    if (LOWER_SPEED <=cmp && cmp <=UPPER_SPEED ){
        return cmp;
    }
    else return 1023;


}
//BLINKS*18*2*3.14/360 => 3.14 mm/hole
//x *188.4 =cmp
// max speed 9240 low speed 4200
/*
void forwardSpeed(int speedP){
        //left is       //right
    int32_t speedone=compvale(speedP);
    int32_t speedcomp=speedone-25;

    //for steer corection
    left_tire_cmp_val=speedone;
    right_tire_cmp_val=speedcomp;

    init_left_tire=left_tire_cmp_val;
    init_right_tire=right_tire_cmp_val;

    Motor_Driver(0,speedone,0,speedcomp);
}*/
void forwardSpeed(int speedP){
        //left is       //right
    int32_t speedone=compvale(speedP);
    int32_t speedcomp=speedone-5;

    //for steer corection
    left_tire_cmp_val=speedcomp;
    right_tire_cmp_val=speedone;

    init_left_tire=left_tire_cmp_val;
    init_right_tire=right_tire_cmp_val;

    Motor_Driver(0,speedcomp,0,speedone);
}
void reverseSpeed(int speedP){
    int32_t speedone=compvale(speedP);
    int32_t speedcomp=speedone-28;

    //for steer corection
       left_tire_cmp_val=speedone;
       right_tire_cmp_val=speedcomp;

       init_left_tire=left_tire_cmp_val;
       init_right_tire=right_tire_cmp_val;
    Motor_Driver(speedone,0,speedcomp,0);
}
void cw(){
    Motor_Driver(0,1023,998,0);
}
void ccw(){
    Motor_Driver(1023,0,0,998);
}
void stop(){
    rob_is_stop=0;
    print_wheel(left_tire_count,right_tire_count);
    Motor_Driver(0,0,0,0);
}

void Distance(void){
    //for distance check
    if(lOCK_ROT==1){//for rotation
                    if(offset<=0){
                        if (angle_of_robot >= 0 && angle_of_robot < 45) {
                            angle_of_robot *= 0.1111111;
                            offset += angle_of_robot;
                        } else if (angle_of_robot >= 45 && angle_of_robot < 90) {
                            angle_of_robot -= 45;
                            angle_of_robot *= 0.1111111;
                            offset += 5;
                            offset += angle_of_robot;
                        } else if (angle_of_robot >= 90 && angle_of_robot < 180) {
                            angle_of_robot -= 90;
                            angle_of_robot *= 0.1111111;
                            offset += 11;
                            offset += angle_of_robot;
                        } else if (angle_of_robot >= 180 && angle_of_robot < 269) {
                            angle_of_robot -= 180;
                            angle_of_robot *= 0.1111111;
                            offset += 26;
                            offset += angle_of_robot;
                        } else if (angle_of_robot >= 270 && angle_of_robot < 359) {
                            angle_of_robot -= 270;
                            angle_of_robot *= 0.1111111;
                            offset += 40;
                            offset += angle_of_robot;
                        } else if (angle_of_robot >= 360) {
                            angle_of_robot -= 360;
                            angle_of_robot *= 0.1111111;
                            offset += 54;
                            offset += angle_of_robot;
                        }
                    }
                    if(offset<=(left_tire_count+right_tire_count)){
                        //print_wheel(left_tire_count,right_tire_count);
                        stop();
                        lOCK_ROT=0;
                        offset=0;
                        halt_rob_rot=1;
                    }

    }
/*
    if(lOCK_ROT==1){//for rotation
                if(offset<=0){
                    if (angle_of_robot >= 0 && angle_of_robot < 45) {
                        angle_of_robot *= 0.1111111;
                        offset += angle_of_robot;
                    } else if (angle_of_robot >= 45 && angle_of_robot < 90) {
                        angle_of_robot -= 45;
                        angle_of_robot *= 0.1111111;
                        offset += 4;
                        offset += angle_of_robot;
                    } else if (angle_of_robot >= 90 && angle_of_robot < 180) {
                        angle_of_robot -= 90;
                        angle_of_robot *= 0.1111111;
                        offset += 13;
                        offset += angle_of_robot;
                    } else if (angle_of_robot >= 180 && angle_of_robot < 269) {
                        angle_of_robot -= 180;
                        angle_of_robot *= 0.1111111;
                        offset += 38;
                        offset += angle_of_robot;
                    } else if (angle_of_robot >= 270 && angle_of_robot < 359) {
                        angle_of_robot -= 270;
                        angle_of_robot *= 0.1111111;
                        offset += 58;
                        offset += angle_of_robot;
                    } else if (angle_of_robot >= 360) {
                        angle_of_robot -= 360;
                        angle_of_robot *= 0.1111111;
                        offset += 80;
                        offset += angle_of_robot;
                    }
                }
                if(offset<=(left_tire_count+right_tire_count)){//offset<=(left_tire_count+right_tire_count
                    //print_wheel(left_tire_count,right_tire_count);
                    stop();
                    lOCK_ROT=0;
                    left_tire_count=0;
                    right_tire_count=0;
                    offset=0;
                    halt_rob_rot=1;
                }

}*/
    if(set_distance==1){                                                        //lock set_distance set to one when checking for distance
                  //int cmp =distance;
                  if ((distance/14)<= (left_tire_count/3)){
                      stop();
                      set_distance=0;
                      LOCK_Wheesl=1;
                  }
              }



    TIMER0_ICR_R = TIMER_ICR_TATOCINT;               // clear interrupt flag
}

void timer3ISR(void){
    int templef=left_tire_count/3;
    if((LOCK_DIR== 1 || LOCK_DIR ==2) && LOCK_Wheesl!=1){ //for corection of stering
                if(templef>(right_tire_count+1)){
                    left_tire_cmp_val-=ALPHA;
                    right_tire_cmp_val+=ALPHA;
                }
                if(right_tire_count>(templef+1)){
                    left_tire_cmp_val+=ALPHA;
                    right_tire_cmp_val-=ALPHA;
                }

                    //(init_left_tire+50)>=left_tire_cmp_val && ((init_left_tire-50)<=left_tire_cmp_val) && ((init_right_tire+50)>=right_tire_cmp_val) &&  ((init_right_tire-50)<=right_tire_cmp_val)
                if((LOWER_SPEED <=left_tire_cmp_val && left_tire_cmp_val <=UPPER_SPEED)&& (((init_left_tire+50)>=left_tire_cmp_val) && ((init_left_tire-50)<=left_tire_cmp_val) && ((init_right_tire+50)>=right_tire_cmp_val) &&  ((init_right_tire-50)<=right_tire_cmp_val))&&(LOWER_SPEED <=right_tire_cmp_val && right_tire_cmp_val <=UPPER_SPEED)){
                    if(LOCK_DIR==1){//forward
                        Motor_Driver(0,left_tire_cmp_val,0,right_tire_cmp_val);
                    }
                    if(LOCK_DIR==2){//reverse
                        Motor_Driver(left_tire_cmp_val,0,right_tire_cmp_val,0);
                    }
                }
                else{//fix this later it sets the motor CMP values to the midlle if it is out of bound
                    left_tire_cmp_val=init_left_tire;
                    right_tire_cmp_val=left_tire_cmp_val-25;
                    if(LOCK_DIR==1){//forward
                       Motor_Driver(0,left_tire_cmp_val,0,right_tire_cmp_val);
                    }
                    if(LOCK_DIR==2){//reverse
                       Motor_Driver(left_tire_cmp_val,0,right_tire_cmp_val,0);
                    }
                }

            }
    TIMER3_ICR_R = TIMER_ICR_TATOCINT;               // clear interrupt flag
}




void timer4Isr(){//for wall dectition
    TIMER4_ICR_R |= TIMER_ICR_TATOCINT;
    ping_distance();
    if(dis_sonar0<=200){//give or take for grivence
        stop();
        left_tire_count=0;
        right_tire_count=0;
        LOCK_DIR=0;
        modes_main=1;
    }

}
void wtimer5ISR(){

    WTIMER5_ICR_R = TIMER_ICR_CAECINT;
    if(phase ==0){
        WTIMER5_TAV_R=0;
        phase=1;
    }
    else{
    dis_sonar0_time=WTIMER5_TAV_R;
    dis_sonar0=(343*dis_sonar0_time)/(80000);//speed of sound *time ofsonar/(sys_clk)
    phase=0;
  }


}

void ping_distance(){//has to set this for latter

    TRIG_0=1;
    waitMicrosecond(10);
    TRIG_0=0;



}

void rot_auto(void){
    angle_of_robot=90;//45
    cw();
    lOCK_ROT=1;
    LOCK_DIR=0;
    set_distance=0;
    left_tire_count=0;
    right_tire_count=0;
    waitMicrosecond(3000000);
}

void is_motion(){//PA2 pin checking if motion
    motion=0;
    waitMicrosecond(3000000);//waits three second
    if(MOTION==1){//motion found
        RED_LED = 1;
    }
    else RED_LED = 0;//no motion

}
int32_t ret_ran_num(int32_t upper_B){
    int32_t lower_bound=upper_B-100;
    int32_t ret=(rand()%(upper_B-100 +1))+lower_bound;
    return ret;
}
void putty(){
    USER_DATA data;
    while(TURN_OFF_ON_AUTO==0){
        if(TURN_OFF_ON_AUTO==1)break;
            getsUart0(&data);
            parseFields(&data);

            if(data.fieldCount==1){//for one command given
                if(isCommand(&data,"ccw",0)){
                    ccw();
                    LOCK_DIR=0;
                    set_distance=0;
                }
                if(isCommand(&data,"cw",0)){
                    cw();
                    LOCK_DIR=0;
                    set_distance=0;
                }
                if(isCommand(&data,"stop",0)){
                    stop();
                    left_tire_count=0;
                    right_tire_count=0;
                    LOCK_DIR=0;
                }
                if(isCommand(&data,"forward",0)){
                    forward();
                    LOCK_DIR=1;
                    set_distance=0;
                    LOCK_Wheesl=0;
                }
                if(isCommand(&data,"reverse",0)){
                    reverse();
                    LOCK_DIR=2;
                    set_distance=0;
                    LOCK_Wheesl=0;
                }
                if(isCommand(&data,"ping",0)){
                    ping_distance();
                    waitMicrosecond(30e3);
                    putsUart0("Sonar Dis: ");
                    intTostring(dis_sonar0);
                }
                if(isCommand(&data,"move?",0)){//for motion dect
                    is_motion();
                    waitMicrosecond(3000000);
                    RED_LED = 0;
                }
                if(isCommand(&data,"auto",0)){//
                    putsUart0("In Auto:");
                    TURN_OFF_ON_AUTO=1;
                    break;
                }
            }
            if(data.fieldCount==2){//for two commands given
                if(isCommand(&data,"forward",1)){
                    int32_t speedP = getFieldInteger(&data, 1);
                   forwardSpeed(speedP);
                   LOCK_DIR=1;
                   set_distance=0;
                   LOCK_Wheesl=1;
                }
                if(isCommand(&data,"reverse",1)){
                    int32_t speedP = getFieldInteger(&data, 1);
                   reverseSpeed(speedP);
                   LOCK_DIR=2;
                   set_distance=0;
                   LOCK_Wheesl=1;//for auto
                }
                if(isCommand(&data,"ccw",1)){
                    angle_of_robot= getFieldInteger(&data, 1);
                    ccw();
                    lOCK_ROT=1;
                    LOCK_DIR=0;
                    set_distance=0;
                    left_tire_count=0;
                    right_tire_count=0;
                }
                if(isCommand(&data,"cw",1)){
                    angle_of_robot= getFieldInteger(&data, 1);
                    cw();
                    lOCK_ROT=1;
                    LOCK_DIR=0;
                    set_distance=0;
                    left_tire_count=0;
                    right_tire_count=0;
               }
            }
            if(data.fieldCount==3){//for three commands given
                if(isCommand(&data,"forward",1)){
                    int32_t speedP = getFieldInteger(&data, 1);
                    distance=getFieldInteger(&data, 2);
                    forwardSpeed(speedP);
                    set_distance=1;
                    LOCK_DIR=1;//1
                    LOCK_Wheesl=1;//wheel cor
                    left_tire_count=0;
                    right_tire_count=0;
                 }
                if(isCommand(&data,"reverse",1)){
                 int32_t speedP = getFieldInteger(&data, 1);
                 distance=getFieldInteger(&data, 2);
                 reverseSpeed(speedP);
                 set_distance=1;
                 LOCK_DIR=2;//2
                 LOCK_Wheesl=1;//
                 left_tire_count=0;
                 right_tire_count=0;
                }


            }

        }
}


