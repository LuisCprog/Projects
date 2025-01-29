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
#include "faults.h"
#include "kernel.h"

#include "uart0.h"//maybe remove
#include "AssemblyHeader.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------


void intTostring(uint32_t num,char*ret){
    char temp[12];
    int i=0;
    if(num==0){
        ret[0]='0';
        ret[1]='\0';
        return;
    }
    while(num>0){
        temp[i++]=(num%10)+'0';
        num /=10;
    }
    int run=0;
    for(run;run<i;run++){
        ret[run]=temp[i-run-1];
    }
    ret[i]='\0';
}
void intToHexStr(uint32_t val,char *str){
    const char hexDig[] = "0123456789ABCDEF";
    int i = 7;
      // Start from the end of the string
      str[8] = '\0';  // Null-terminate the string
      for (i; i >= 0; --i) {
          str[i] = hexDig[val & 0x0F];  // Get the last 4 bits
          val >>= 4;  // Shift right by 4 bits
      }
}

void DisStackDump(uint32_t *sp){
    char strtemp[12];

    intToHexStr(*sp,strtemp);
    putsUart0("R0: 0x");
    putsUart0(strtemp);
    putsUart0("\n");


    intToHexStr(*(sp+1),strtemp);
    putsUart0("R1: 0x");
    putsUart0(strtemp);
    putsUart0("\n");

    intToHexStr(*(sp+2),strtemp);
    putsUart0("R2: 0x");
    putsUart0(strtemp);
    putsUart0("\n");

    intToHexStr(*(sp+3),strtemp);
    putsUart0("R3: 0x");
    putsUart0(strtemp);
    putsUart0("\n");

    intToHexStr(*(sp+4),strtemp);
    putsUart0("R12: 0x");
    putsUart0(strtemp);
    putsUart0("\n");

    intToHexStr(*(sp+5),strtemp);
    putsUart0("LR: 0x");
    putsUart0(strtemp);
    putsUart0("\n");

    intToHexStr(*(sp+6),strtemp);
    putsUart0("PC: 0x");
    putsUart0(strtemp);
    putsUart0("\n");

    intToHexStr(*(sp+7),strtemp);
        putsUart0("PSRx: 0x");
        putsUart0(strtemp);
        putsUart0("\n");


}
// REQUIRED: If these were written in assembly
//           omit this file and add a faults.s file

// REQUIRED: code this function



void mpuFaultIsr(void)
{
    char strtemp[12];
        uint32_t *psp=GetPSP();
        uint32_t *msp=GetMSP();

        putsUart0("MPU FAULT in ");
        intTostring(1,strtemp);
        putsUart0(strtemp);
        putsUart0("\n");

        intToHexStr(psp,strtemp);
        putsUart0("PSP: 0x");
        putsUart0(strtemp);
        putsUart0("\n");

        intToHexStr(msp,strtemp);
        putsUart0("MSP: 0x");
        putsUart0(strtemp);
        putsUart0("\n");

        intToHexStr(*(psp+6),strtemp);
        putsUart0("offending Instruction ");
        putsUart0(strtemp);
        putsUart0("\n");

        intToHexStr(NVIC_FAULT_STAT_R,strtemp);
        putsUart0("MFault Flags: ");
        putsUart0(strtemp);
        putsUart0("\n");

        intToHexStr(NVIC_MM_ADDR_R,strtemp);
        putsUart0("MMFAULTADDR: ");
        putsUart0(strtemp);
        putsUart0("\n");
        DisStackDump(psp);



        //Clear the MPU fault pending bit and trigger a pendsv ISR call.
        killThread(getPID());
        NVIC_SYS_HND_CTRL_R &= ~(NVIC_SYS_HND_CTRL_MEMP| NVIC_SYS_HND_CTRL_MEMA);



        NVIC_INT_CTRL_R|=NVIC_INT_CTRL_PEND_SV;
        //NVIC_INT_CTRL_VEC_PEN_PNDSV|=1;

        //data adress offdeing instruction and
}

// REQUIRED: code this function
void hardFaultIsr(void)
{
    char strtemp[12];
       uint32_t *psp=GetPSP();
       uint32_t *msp=GetMSP();



       putsUart0("HARD FAULT in ");
       intTostring(1,strtemp);
       putsUart0(strtemp);
       putsUart0("\n");

       intToHexStr(psp,strtemp);
       putsUart0("PSP: 0x");
       putsUart0(strtemp);
       putsUart0("\n");

       intToHexStr(msp,strtemp);
       putsUart0("MSP: 0x");
       putsUart0(strtemp);
       putsUart0("\n");

       intToHexStr(*(psp+6),strtemp);
       putsUart0("offending Instruction ");
       putsUart0(strtemp);
       putsUart0("\n");

       intToHexStr(NVIC_HFAULT_STAT_R,strtemp);
       putsUart0("HFault Flags: ");
       putsUart0(strtemp);
       putsUart0("\n");




       DisStackDump(psp);

   //flags and ofdenein instrucion

       //
       // Enter an infinite loop.
       //
       while(1)
       {
       }
}

// REQUIRED: code this function
void busFaultIsr(void)
{
    char strtemp[4];
    intTostring(0,strtemp);
    putsUart0("BUS FAULT in ");
    putsUart0(strtemp);
    while(1);
}

// REQUIRED: code this function
void usageFaultIsr(void)
{
    char strtemp[4];
    intTostring(0,strtemp);
    putsUart0("USAGE FAULT in ");
    putsUart0(strtemp);
    while(1);
}




