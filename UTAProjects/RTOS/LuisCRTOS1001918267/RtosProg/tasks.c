// Tasks
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
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "wait.h"
#include "kernel.h"
#include "tasks.h"

#define GREENT_LED  PORTF,3
#define BLUE_LED   PORTF,2 // on-board blue LED
#define RED_LED    PORTA,2 // off-board red LED
#define ORANGE_LED PORTA,3 // off-board orange LED
#define YELLOW_LED PORTA,4 // off-board yellow LED
#define GREEN_LED  PORTE,0 // off-board green LED


#define SW0_BUTTON PORTC,4//off-board sw0 R0 C0
#define SW1_BUTTON PORTC,5//off-board sw1 R0 c1
#define SW2_BUTTON PORTC,6//off-board sw2 R0 C2
#define SW3_BUTTON PORTC,7//off-board sw3 R1 C0
#define SW4_BUTTON PORTD,6//off-board sw4 R1 C2
#define SW5_BUTTON PORTD,7//off-board sw5 R1 C3

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
// REQUIRED: Add initialization for blue, orange, red, green, and yellow LEDs
//           Add initialization for 6 pushbuttons

void INTPeriodTime(){
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R0;
        _delay_cycles(3);

    // Configure Timer 0 as the time base for distance
        TIMER0_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
        TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
        TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
        TIMER0_TAILR_R = 40000000;                       // set load value to 3.6e6 for 90 ms check
        TIMER0_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts for timeout in timer module
        TIMER0_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
        NVIC_EN0_R = 1 << (INT_TIMER0A-16);              // turn-on interrupt 37 (TIMER1A) in NVIC

}
void initHw(void)
{
    // Setup LEDs and pushbuttons
    enablePort(PORTA);
    enablePort(PORTC);
    enablePort(PORTD);
    enablePort(PORTE);
    enablePort(PORTF);

    setPinCommitControl(SW5_BUTTON);

    selectPinPushPullOutput(BLUE_LED);
    selectPinPushPullOutput(RED_LED);
    selectPinPushPullOutput(ORANGE_LED);
    selectPinPushPullOutput(YELLOW_LED);
    selectPinPushPullOutput(GREEN_LED);
    selectPinPushPullOutput(GREENT_LED);



    selectPinDigitalInput(SW0_BUTTON);
    selectPinDigitalInput(SW1_BUTTON);
    selectPinDigitalInput(SW2_BUTTON);
    selectPinDigitalInput(SW3_BUTTON);
    selectPinDigitalInput(SW4_BUTTON);
    selectPinDigitalInput(SW5_BUTTON);

    enablePinPullup(SW0_BUTTON);
    enablePinPullup(SW1_BUTTON);
    enablePinPullup(SW2_BUTTON);
    enablePinPullup(SW3_BUTTON);
    enablePinPullup(SW4_BUTTON);
    enablePinPullup(SW5_BUTTON);




    // Power-up flash
    setPinValue(GREEN_LED, 1);
    waitMicrosecond(250000);
    setPinValue(GREEN_LED, 0);
    waitMicrosecond(250000);
    INTPeriodTime();
}




void Timer0ISR(){
    setPinValue(GREENT_LED, !getPinValue(GREENT_LED));

    TIMER0_ICR_R |= TIMER_ICR_TATOCINT;
    //TIMER0_CTL_R &= ~TIMER_CTL_TAEN;
}

// REQUIRED: add code to return a value from 0-63 indicating which of 6 PBs are pressed
uint8_t readPbs(void)
{
    uint8_t val=0;
    if(!getPinValue(SW0_BUTTON))val =1;
    if(!getPinValue(SW1_BUTTON))val |= (1<<1);
    if(!getPinValue(SW2_BUTTON))val |= (1<<2);
    if(!getPinValue(SW3_BUTTON))val |= (1<<3);
    if(!getPinValue(SW4_BUTTON))val |= (1<<4);
    if(!getPinValue(SW5_BUTTON)) val |= (1<<5);
    return val;
}

// one task must be ready at all times or the scheduler will fail
// the idle task is implemented for this purpose
void idle(void)
{
    while(true)
    {
        setPinValue(ORANGE_LED, 1);
        waitMicrosecond(1000);
        setPinValue(ORANGE_LED, 0);
        yield();
    }
}

/*
void idle2(void)
{
    while(true)
    {
        setPinValue(RED_LED, 1);
        waitMicrosecond(1000);
        setPinValue(RED_LED, 0);
        yield();
    }
}*/


void flash4Hz(void)
{
    while(true)
    {
        setPinValue(GREEN_LED, !getPinValue(GREEN_LED));
        sleep(125);
    }
}

void oneshot(void)
{
    while(true)
    {
        wait(flashReq);
        setPinValue(YELLOW_LED, 1);
        sleep(1000);
        setPinValue(YELLOW_LED, 0);
    }
}

void* Malloc2(uint32_t val){

    __asm(" SVC #13");
    return ;
}

void partOfLengthyFn(void)
{
    // represent some lengthy operation
    waitMicrosecond(990);
    // give another process a chance to run
    yield();
}

void lengthyFn(void)
{
    uint16_t i;
    uint8_t *mem;



   mem = Malloc2(5000 * sizeof(uint8_t));

    while(true)
    {
        lock(resource);
        for (i = 0; i < 5000; i++)
        {
            partOfLengthyFn();
            mem[i] = i % 256;
        }
        setPinValue(RED_LED, !getPinValue(RED_LED));
        unlock(resource);
    }
}

void readKeys(void)
{
    uint8_t buttons;
    while(true)
    {
        wait(keyReleased);
        buttons = 0;
        while (buttons == 0)
        {
            buttons = readPbs();
            yield();
        }
        post(keyPressed);
        if ((buttons & 1) != 0)
        {
            setPinValue(YELLOW_LED, !getPinValue(YELLOW_LED));
            setPinValue(RED_LED, 1);
        }
        if ((buttons & 2) != 0)
        {
            post(flashReq);
            setPinValue(RED_LED, 0);
        }
        if ((buttons & 4) != 0)
        {
           restartThread(flash4Hz);
        }
        if ((buttons & 8) != 0)
        {
           stopThread(flash4Hz);
        }
        if ((buttons & 16) != 0)
        {
            setThreadPriority(lengthyFn, 4);
        }
        yield();
    }
}

void debounce(void)
{
    uint8_t count;
    while(true)
    {
        wait(keyPressed);
        count = 10;
        while (count != 0)
        {
            sleep(10);
            if (readPbs() == 0)
                count--;
            else
                count = 10;
        }
        post(keyReleased);
    }
}

void uncooperative(void)
{
    while(true)
    {
        while (readPbs() == 8)
        {
        }
        yield();
    }
}

void errant(void)
{
    uint32_t* p = (uint32_t*)0x20000000;
    while(true)
    {
        while (readPbs() == 32)
        {
            *p = 0;
        }
        yield();
    }
}

void important(void)
{
    while(true)
    {
        lock(resource);
        setPinValue(BLUE_LED, 1);
        sleep(1000);
        setPinValue(BLUE_LED, 0);
        unlock(resource);
    }
}
