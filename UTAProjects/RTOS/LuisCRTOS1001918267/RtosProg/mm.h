// Memory manager functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef MM_H_
#define MM_H_

#define NUM_SRAM_REGIONS 4

typedef struct  ___MPUEntry{
    uint8_t region;
    uint32_t HeadAdd;
    uint32_t FreeAdd;//curSp+stackBytes/4;
    uint32_t pid;
    uint32_t size;
    uint16_t bitVal;
    void* PID;
    int bitshift;
}MPUEntry;
static int EntryCount;
static uint64_t AddReff;
static  MPUEntry dataArray[];
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void * mallocFromHeap(uint32_t size_in_bytes);
void freeToHeap(void *pMemory);
MPUEntry* meminfoCall();


void allowFlashAccess(void);
void allowPeripheralAccess(void);
void MPUON();
void setupSramAccess(void);
uint64_t createNoSramAccessMask(void);
void addSramAccessWindow(uint64_t *srdBitMask, uint32_t baseAdd, uint32_t size_in_bytes);
void applySramAccessMask(uint64_t srdBitMask);

#endif
