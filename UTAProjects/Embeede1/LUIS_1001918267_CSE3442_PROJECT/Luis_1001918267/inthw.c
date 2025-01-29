# include "inthw.h"

void initINTER(){

    // Enable clocks for the port c
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);
    //gpio resitor enable
    GPIO_PORTB_DEN_R |= TRIG_0_MASK ;
     GPIO_PORTB_DIR_R |= TRIG_0_MASK ;

    GPIO_PORTC_DEN_R |= SPEEDONE_MASK | SPEEDTWO_MASK|SYSCTL_RCGCGPIO_R1|REMOTE_MASK ;
    //GPIO_PORTC_DIR_R &= ~(SPEEDONE_MASK | SPEEDTWO_MASK);
    GPIO_PORTC_PDR_R |= SPEEDONE_MASK | SPEEDTWO_MASK;
    GPIO_PORTC_PUR_R|=REMOTE_MASK;

    GPIO_PORTC_IS_R &= ~(SPEEDONE_MASK | SPEEDTWO_MASK);
    GPIO_PORTC_IBE_R &= ~(SPEEDONE_MASK | SPEEDTWO_MASK );
    GPIO_PORTC_IEV_R &= ~(SPEEDONE_MASK | SPEEDTWO_MASK);
    GPIO_PORTC_ICR_R |= SPEEDONE_MASK | SPEEDTWO_MASK|REMOTE_MASK;;
    //GPIO_PORTC_IM_R |= SPEEDONE_MASK | SPEEDTWO_MASK;
    GPIO_PORTC_IM_R |= REMOTE_MASK;

    NVIC_EN0_R = 1 << (INT_GPIOC-16);                // turn-on interrupt 16 (GPIOA)

    // Configure LED pin
    GPIO_PORTF_DEN_R |= RED_LED_MASK |BLUE_LED_MASK|GREEN_LED_MASK ;
    GPIO_PORTF_DIR_R |= RED_LED_MASK |BLUE_LED_MASK |GREEN_LED_MASK;
}



void Remote_wideTimer(){//PC5 wt0ccp1
    // Enable clocks
       SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;
       SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;

       _delay_cycles(16);



       // Configure SIGNAL_IN for frequency and time measurements
       //GPIO_PORTC_DIR_R &= (~REMOTE_MASK);
       GPIO_PORTC_DEN_R |= REMOTE_MASK;
       GPIO_PORTC_AFSEL_R |= REMOTE_MASK;              // select alternative functions for SIGNAL_IN pin
       GPIO_PORTC_PCTL_R &= ~GPIO_PCTL_PC5_M;           // map alt fns to SIGNAL_IN
       GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC5_WT0CCP1;

       //enable Wtimer
       WTIMER0_CTL_R &= ~TIMER_CTL_TBEN;                // turn-off counter before reconfiguring
       WTIMER0_CTL_R = TIMER_CTL_TBEVENT_NEG;           // measure time from negtive edge to negtive edge
       WTIMER0_CFG_R = 4;                               // configure as 32-bit counter (A only)
       WTIMER0_TBV_R = 0;                               // zero counter for first period
       WTIMER0_TBMR_R = TIMER_TBMR_TBCMR  | TIMER_TBMR_TBCDIR;// configure for edge time mode, count up
      // WTIMER0_IMR_R = TIMER_IMR_CBEIM;                 // turn-on interrupts
       WTIMER0_CTL_R |= TIMER_CTL_TBEN;                // turn-on counter
       //NVIC_EN2_R = 1 << (INT_WTIMER0B - 16 - 32 - 32);          // turn-on interrupt 111 (WTIMER0B)
               // turn-on counter
}

void sonar_init(){//Sonar 1: Echo:PD2  WT3CCP0 / Trig PB3

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3 ;//port D
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R5;//eanble clocks

    _delay_cycles(16);


    GPIO_PORTD_DIR_R &= ~ECHO_0_MASK;                     //Input
    GPIO_PORTD_DEN_R |= ECHO_0_MASK;          //Digital enable
        GPIO_PORTD_AFSEL_R |= ECHO_0_MASK;
        GPIO_PORTD_PCTL_R &= ~(GPIO_PCTL_PD6_M);
        GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD6_WT5CCP0;


        WTIMER5_CTL_R &= ~TIMER_CTL_TAEN;           // turn-off counter before reconfiguring
        WTIMER5_CFG_R = 0x4;                          // configure as 32-bit counter (A only)
                                                    // configure for capture mode, count up, edge time mode
        WTIMER5_TAMR_R = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR;
        WTIMER5_CTL_R = TIMER_CTL_TAEVENT_BOTH;     // count both edges
        WTIMER5_IMR_R = TIMER_IMR_CAEIM;            // turn on interrupts for capture //CAEIM Counter A Event Interrupt Mask
        WTIMER5_TAV_R = 0;                          // zero counter for first period
        WTIMER5_ICR_R |= TIMER_ICR_CAECINT;
        NVIC_EN3_R = 1 << (INT_WTIMER5A-16-96);     // turn-on interrupt 112 (WTIMER5A)
        WTIMER5_CTL_R |= TIMER_CTL_TAEN;            // turn-on counter

}




void PIR_init(){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;
    _delay_cycles(16);

    GPIO_PORTA_DEN_R |= MOTION_MASK;
    GPIO_PORTA_DIR_R &= ~(MOTION_MASK);
}




void initTimer(){
    // Enable clocks for timer
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R0|SYSCTL_RCGCTIMER_R1 |SYSCTL_RCGCTIMER_R2|SYSCTL_RCGCTIMER_R3|SYSCTL_RCGCTIMER_R4;
    _delay_cycles(16);

    // Configure Timer 1 as the time base
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_1_SHOT;          // configure for one shot (count down)
    TIMER1_TAILR_R = 40000;                       // set load value to 10e6 for 25 ns
    TIMER1_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts for timeout in timer module
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN0_R = 1 << (INT_TIMER1A-16);              // turn-on interrupt 37 (TIMER1A) in NVIC


    // Configure Timer 4 as the time base
    TIMER4_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER4_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER4_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for one shot (count down)
    TIMER4_TAILR_R = 40000000;                       // set load value to 1 sec  load val/40Mhz = timeuwant,
    TIMER4_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts for timeout in timer module
    TIMER4_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN2_R = 1 << (INT_TIMER4A-16-64);              // turn-on interrupt 37 (TIMER1A) in NVIC


    // Configure Timer 2 as the time base
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER2_TAMR_R = TIMER_TAMR_TAMR_1_SHOT;          // configure for one shot (count down)
    TIMER2_TAILR_R = 40000;                       // set load value to 10e6 for 25 ns
    TIMER2_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts for timeout in timer module
    TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN0_R = 1 << (INT_TIMER2A-16);              // turn-on interrupt 37 (TIMER2A) in NVIC

    // Configure Timer 0 as the time base for distance
    TIMER0_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER0_TAILR_R = 3600000;                       // set load value to 3.6e6 for 90 ms check
    TIMER0_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts for timeout in timer module
    TIMER0_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN0_R = 1 << (INT_TIMER0A-16);              // turn-on interrupt 37 (TIMER1A) in NVIC

 // Configure Timer 3 as the time base for distance
    TIMER3_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER3_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER3_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER3_TAILR_R = 3600000;                       // set load value to 3.6e6 for 90 ms check
    TIMER3_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts for timeout in timer module
    TIMER3_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN1_R = 1 << (INT_TIMER3A-16-32);              // turn-on interrupt 37 (TIMER1A) in NVIC
}


void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
    _delay_cycles(16);

    // Configure LED pins
    GPIO_PORTE_DIR_R |=SLEEP_MASK;
    GPIO_PORTE_DEN_R |=SLEEP_MASK;
}
void initPWM(){
          SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
          SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;
          _delay_cycles(16);
         // Configure three LEDs

         GPIO_PORTB_DEN_R |= PWM1_MASK | PWM2_MASK |PWM3_MASK| PWM4_MASK;
         GPIO_PORTB_AFSEL_R |= PWM1_MASK | PWM2_MASK|PWM3_MASK| PWM4_MASK;
         GPIO_PORTB_PCTL_R &= ~(GPIO_PCTL_PB4_M | GPIO_PCTL_PB5_M | GPIO_PCTL_PB6_M | GPIO_PCTL_PB7_M);
         GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB4_M0PWM2 | GPIO_PCTL_PB5_M0PWM3 | GPIO_PCTL_PB6_M0PWM0 | GPIO_PCTL_PB7_M0PWM1;

         // Configure PWM module 1 to drive RGB LED
         // GEN 1 on (PB4), M0PWM2
         // GEN 1 on (PB5), M0PWM3

         // GEN 0 on (PB6), M0PWM0
         // GEN 0 on (PB7), M0PWM1
         SYSCTL_SRPWM_R = SYSCTL_SRPWM_R0;                // reset PWM0 module
         SYSCTL_SRPWM_R = 0;                              // leave reset state
         PWM0_0_CTL_R = 0;                                // turn-off PWM1 generator 2 (drives outs 4 and 5)
         PWM0_1_CTL_R = 0;                                // turn-off PWM1 generator 3 (drives outs 6 and 7)
         PWM0_0_GENA_R = PWM_0_GENA_ACTCMPAD_ONE | PWM_0_GENA_ACTLOAD_ZERO;
         PWM0_0_GENB_R = PWM_0_GENB_ACTCMPBD_ONE | PWM_0_GENB_ACTLOAD_ZERO;
         PWM0_1_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;
         PWM0_1_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;
                                                          // output 7 on PWM1, gen 3b, cmpb

         PWM0_0_LOAD_R = 1024;                            // set frequency to 40 MHz sys clock / 2 / 1024 = 19.53125 kHz
         PWM0_1_LOAD_R = 1024;                            // (internal counter counts down from load value to zero)

         PWM0_0_CMPA_R = 0;// red off (0=always low, 1023=always high)
         PWM0_0_CMPB_R = 0;
         PWM0_1_CMPA_R = 0;
         PWM0_1_CMPB_R = 0;                               // green off

         PWM0_0_CTL_R = PWM_1_CTL_ENABLE;                 // turn-on PWM1 generator 2
         PWM0_1_CTL_R = PWM_1_CTL_ENABLE;                 // turn-on PWM1 generator 3
         PWM0_ENABLE_R = PWM_ENABLE_PWM0EN | PWM_ENABLE_PWM1EN | PWM_ENABLE_PWM2EN|PWM_ENABLE_PWM3EN;// enable outputs
}

