#ifndef FUNCTION_H
#define FUNCTION_H

# include "inthw.h"
// Blocking function that writes a serial character when the UART buffer is not full
extern void putcUart0(char c);

// Blocking function that writes a string when the UART buffer is not full
extern void putsUart0(char* str);

// Blocking function that returns with serial data once the buffer is not empty
extern char getcUart0();

extern uint32_t init_left_tire;
extern uint32_t init_right_tire;
extern uint32_t left_tire_count;       //for the inside functionality for every comand
extern uint32_t right_tire_count;      // for the inside functionality for every command
//extern uint32_t left_tire_count_total;     //for the total distance of left tire only resetes when stop
//extern uint32_t right_tire_count_total;//for the total distance of right tire only resets when stop
extern int32_t left_tire_cmp_val;
extern int32_t right_tire_cmp_val;
extern int32_t angle_of_robot;     // the degree of rotation
extern uint32_t distance;      // the distance input
extern uint8_t set_distance; // a lock for distance
extern uint8_t LOCK_DIR;     // allows to see wich direction whe are moving towards 1 is forward 2 is revreser 0 is no direction
extern uint8_t lOCK_ROT;     // allows to deterime the wheter rotation is being set or not
extern uint8_t LOCK_Wheesl;  //LOCK_Wheesl is used to set the dircetion so no inturput hapens when stoping and steer controle
extern uint8_t offset;
extern uint8_t motion;
//for the remote
extern uint32_t total_time;
extern uint32_t remote_val;

extern uint32_t pres_time;
//extern uint32_t cur_time;
extern uint32_t past_time;
extern uint32_t dis_sonar0;
extern uint32_t dis_sonar0_time;
extern uint8_t STATE;
extern uint8_t Bits_Read;
extern uint8_t TURN_OFF_ON_AUTO;
extern uint8_t halt_rob_rot;
extern  uint8_t modes_main;
extern uint8_t rob_is_stop;
extern uint8_t phase;


//void intTostring(int val);
//void print_wheel(int wheel_left,int wheel_right);
void fallinedge();
void timer1Isr(void);
void timer2Isr(void);
void getsUart0(USER_DATA *data);
//void parseFields(USER_DATA *data);
//char* getFieldString(USER_DATA* data,uint8_t fieldnumber);
//int32_t getFieldInteger(USER_DATA* data,uint8_t fieldnumber);
//bool isCommand(USER_DATA* data,const char strCommand[],uint8_t minArguments);
//bool Stringcmp( const char str[],const char strCommand[]);
void Motor_Driver(int val1,int val2,int val3,int val4);
void forward();
void reverse();
int32_t compvale(int32_t speedP);
void forwardSpeed(int speedP);
void reverseSpeed(int speedP);
void cw();
void ccw();
void stop();
void Distance(void);
void timer3ISR(void);
void timer4Isr(void);

//void REMOTE_CONT(void);

void is_motion();
void wtimer5ISR();//


void ping_distance(void);
void rot_auto(void);
void putty(void);

int32_t ret_ran_num(int32_t upper_B);
#endif
