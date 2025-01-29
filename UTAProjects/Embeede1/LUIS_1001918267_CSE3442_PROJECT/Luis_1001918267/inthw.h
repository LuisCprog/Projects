#ifndef INTHW_H
#define INTHW_H

#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "wait.h"

#define MAX_CHARS 80
#define MAX_FIELDS 5
typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

// Bitband aliases
#define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))//PF1
#define BLUE_LED     (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4)))//PF2
#define GREEN_LED     (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))//PF3

//port D
//for sonar echo 0x400073FC
#define ECHO_0      (*((volatile uint32_t *)(0x42000000 + (0x400073FC-0x40000000)*32 + 6*4))) //PD6
#define TRIG_0      (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 3*4))) //was PB3

#define MOTION      (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 2*4))) //PA2
//port C
//sensor for wheel
#define SPEEDONE    (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 4*4))) //PC4
#define SPEEDTWO    (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 6*4))) //PC6
//remote mask


#define REMOTE      (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 5*4))) //PC5
//PortB PWMS
#define PWM1_AD     (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 4*4))) //PB4
#define PWM2_AD     (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 5*4))) //PB5
#define PWM3_AD     (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 6*4))) //PB6
#define PWM4_AD     (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 7*4))) //PB7

//PortE sleep
#define SLEEP_AD    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 1*4))) //PE1
//portC mask
#define SPEEDONE_MASK 16
#define SPEEDTWO_MASK 64
// PortF masks portf pf1 and pf2
#define RED_LED_MASK 2
#define BLUE_LED_MASK 4
#define GREEN_LED_MASK 12
#define SLEEP_MASK 2
// PortB mask
#define PWM1_MASK   16
#define PWM2_MASK   32
#define PWM3_MASK   64
#define PWM4_MASK   128
#define REMOTE_MASK 32
#define TRIG_0_MASK 8
#define MOTION_MASK 4
//port D mask
#define ECHO_0_MASK 64
//steering
#define ALPHA 5;


//speed
#define UPPER_SPEED 1023
#define LOWER_SPEED 750//640 lower but -25 take in considration for moter dispartioy
#define BLINKS 10

void initINTER();

void Remote_wideTimer();

void initTimer();

void initHw();

void initPWM();
void sonar_init();
void PIR_init();
#endif
