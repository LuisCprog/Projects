#include "function.h"



/**
 * main.c
 */
int main(void)
{
        initHw();
        initUart0();
        sonar_init();
        setUart0BaudRate(115200, 40e6);
        initPWM();
        initINTER();
        initTimer();
        Remote_wideTimer();
        SLEEP_AD=1;
        PIR_init();

      // srand((unsigned)time(NULL));//seeds the num
       //inserte the while
       TIMER4_CTL_R &= ~TIMER_CTL_TAEN;//turn off timer
       uint8_t angle_found_set=0; //0 =orgin 1=45 2=90 3=135 4=180 5=225 6=270 7=315 these are the angles whic will be used to find
       int32_t distanc_toset =0;
       int32_t  temp_distanc_toset=0;
       uint8_t run=0;
while(true){
       switch(modes_main){
       case 0://chose the mode
           //stop();

           if(TURN_OFF_ON_AUTO==0){//remote controle//putty
               TIMER4_CTL_R &= ~TIMER_CTL_TAEN;//turn off timer
               if(kbhitUart0())putty();// calls

           }
           if(TURN_OFF_ON_AUTO==1){//automos
               modes_main=1;
               angle_found_set=0; //0 =orgin 1=45 2=90 3=135 4=180 5=225 6=270 7=315 these are the angles whic will be used to find
               distanc_toset =0;
               temp_distanc_toset=0;
               run=0;
           }
           break;
       case 1://checks motion and the angle

           if(TURN_OFF_ON_AUTO==0){//can break if mode is
               modes_main=0;
               break;
           }
           if(rob_is_stop!=1){
               TIMER4_CTL_R &= ~TIMER_CTL_TAEN;//turn off timer
               is_motion();
               for(run=0;run<3;run++){//finds the best distance in 90 deg
                   stop();
                   ping_distance();
                   waitMicrosecond(30e3);
                   temp_distanc_toset=dis_sonar0;
                   if(temp_distanc_toset>distanc_toset){
                       distanc_toset=temp_distanc_toset;
                       angle_found_set=run;
                    }
                    if(TURN_OFF_ON_AUTO==0){//can break if mode is
                      modes_main=0;
                      break;
                    }
                    rot_auto();
               }
               RED_LED = 0;//just to turn off
               if(modes_main==0)break;
               rot_auto();
               modes_main=2;
               for(run=0;run<angle_found_set;run++){//set the angle
                   rot_auto();
                   if(TURN_OFF_ON_AUTO==1){//can break if mode is
                       modes_main=0;
                       break;
                   }
               }
           }
           break;
       case 2://forward and stop for a lower and upper bond
           distance=ret_ran_num(distanc_toset);
           forward();
           TIMER4_CTL_R |= TIMER_CTL_TAEN;//turn on time
           set_distance=1;
           LOCK_DIR=1;//1
           LOCK_Wheesl=0;
           left_tire_count=0;
           right_tire_count=0;
           rob_is_stop=1;
           waitMicrosecond(30e3);
           modes_main=0;
           //upper and lowere srnad stops sets mode to 1 dun based of of time how dast it takes me to trvae; distance as uper lower a micro sec*/

           break;


       }
    }
}
