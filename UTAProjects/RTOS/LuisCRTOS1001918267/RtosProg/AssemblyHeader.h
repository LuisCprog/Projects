#ifndef ASSEMBLYHEADER_H_
#define ASSEMBLYHEADER_H_

extern void SetTMPL(int i);
extern void SetPSP(uint32_t psp_stack);
extern void SetASP(void);
extern uint32_t GetPSP();
extern uint32_t GetMSP();
//extern void SVCsetOne();
//extern void launchFirstTask();
extern void Pop();
extern void Push();
//extern void callFunction(void *pid );


#endif
