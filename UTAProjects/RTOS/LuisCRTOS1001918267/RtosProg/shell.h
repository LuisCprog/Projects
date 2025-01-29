// Shell functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef SHELL_H_
#define SHELL_H_
#include <stdbool.h>

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

typedef struct  ___MEMINFO{
    uint32_t pid;
    uint32_t taskname;
    uint8_t region;
    uint32_t FreeAdd;//curSp+stackBytes/4;
    uint32_t size;
}meminfo;


typedef struct __infoPS
{
    char name[16];
        void *pid;                     // used to uniquely identify thread (add of task fn)
        uint64_t CPUtime;
        uint8_t state;
        uint8_t mutex;                 // index of the mutex in use or blocking the thread
       uint8_t semaphore;
}PS;

typedef struct _infoIPSC
{//mux info
    bool lock;
    uint8_t MUXqueueSize;
    char processQueue[2][16];
    char lockedBy[16];//semaphor inof
    uint8_t SEMcount[3];
    uint8_t SEMqueueSize[3];
    char SEMprocessQueue[3][2][16];
} IPSC;





void shell(void);

#endif
