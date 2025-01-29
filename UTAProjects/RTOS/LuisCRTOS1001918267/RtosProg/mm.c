// Memory manager functions
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
#include <stdbool.h>
//#include "mmHelperFUn.h"
#include "mm.h"
#include "kernel.h"

#define REG0 0x20001000
#define REG1 0x20002000
#define REG2 0x20004000
#define REG3 0x20005000
#define REG4 0x20006000

#define REG5 0x00000000
#define REG6 0x40000000
#define SIZE4k 11
#define SIZE8k  12
#define SIZEFLASH 17
#define SIZEPERIPHERAL 28



static int EntryCount;
static uint64_t AddReff;
static MPUEntry dataArray[14];

MPUEntry* meminfoCall(uint8_t *count){
    *count=EntryCount;
    return &dataArray;
}
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// REQUIRED: add your malloc code here and update the SRD bits for the current thread

uint64_t Adresret(int region, int bit){//returns adress of meme allo given region and num of bits
    uint64_t val=0;
    if(region==0){
        val=0x20001000;
        val+=(bit*512);
    }
    else if(region==1){
        val=0x20002000;
        val+=(bit*1024);
    }
    else if(region ==2){
        val=0x20004000;
         val+=(bit*512);
    }
    else if(region ==3) {
        val=0x20005000;
        val+=(bit*512);
    }
    else if(region ==4){
        val=0x20006000;
        val+=(bit*1024);
    }
    return val;

}

/*HEAP  value |  of 8sub regions|64 bits only look at the frist 40 bits of the unitn 64 bits
     * (5 regions)

     *
     *      1024 b   0x20007C00  39         549755813888
     *      1024b   0x20007800  38         274877906944
     *      1024b   0x20007400  37         137438953472
     *      1024b   0x20007000  36         68719476736
     *      1024b   0x20006C00  35         34359738368
     *      1024b   0x20006800  34         17179869184
     *       1024b   0x20006400  33         8589934592
     *      1024 b  0x20006000  32         4294967296
     *
     * 8kb
     * 4)
     *
     *      512 k  0x20005E00  31         2147483648
     *      512 k  0x20005C00  30         1073741824
     *      512 k  0x20005A00  29         536870912
     *      512 k  0x20005800  28         268435456
     *      512 k  0x20005600  27         134217728
     *      512 k  0x20005400  26         67108864
     *      512 b  0x20005200  25         33554432
     *      512 b   0x20005000  24        16777216
     *
     * 4 kb
     * 3)
     *
     *      512 b   0x20004e00  23         8388608
     *      512 b   0x20004c00  22         4194304
     *      512 b   0x20004a00  21         2097152
     *      512 b   0x20004800  20         1048576
     *      512 b   0x20004600  19         524288
     *      512 b   0x20004400  18         262144
     *      512 b   0x20004200  17         131072
     *      512 b  0x20004000 16         65536
     * 4kb
     * 2)
     *      1024 b  0x20003C00 15         32768
     *      1024 b  0x20003800 14         16384
     *      1024 b  0x20003400 13         8192
     *      1024 b  0x20003000 12         4096
     *      1024 b  0x20002C00 11         2048
     *      1024 b  0x20002800  10          1024
     *      1024 b  0x20002400  9          512
     *      1024 b   0x20002000  8          256
     * 8 kb
     * 1)
     *      512 b   0x20001E00  7          128
     *      512 b   0x20001C00  6          64
     *      512 b   0x20001A00  5          32
     *      512 b   0x20001800  4          16
     *      512 b   0x20001600  3          8
     *      512 b   0x20001400  2          4
     *      512 b   0x20001200  1          2
     *      512b    0x20001000  0          1
     * 4KB
     *0)*/

void MakeEntry(int region,uint32_t HeadAdd,uint32_t size,uint16_t bitVal ,int bitshift ){//creates entery of meme for the meme table
    if(EntryCount<=14){
    dataArray[EntryCount].PID=getPID();
    dataArray[EntryCount].region=region;
    dataArray[EntryCount].HeadAdd=HeadAdd;
    dataArray[EntryCount].pid=HeadAdd;
    dataArray[EntryCount].size=size;
    dataArray[EntryCount].FreeAdd=HeadAdd+size;
    dataArray[EntryCount].bitVal=bitVal;
    dataArray[EntryCount].bitshift=bitshift;

    EntryCount++;
    }

}
bool findCBits(int region, int num_bits){//finds the bit for countio given the region and num of bits
    int regval[5]={512,1024,512,512,1024};
    uint64_t temp=0;
    uint64_t addtemo=0;
    int shift=region * 8;
    int i=0;

    if(num_bits==2){
        temp=3;
        temp=temp<<shift;
        for(i=0;i<7;i++){
             if(!(AddReff&temp)){
                 AddReff|=temp;
                addtemo=Adresret(region, i);
                MakeEntry(region,addtemo,(regval[region]*num_bits),3,(shift+i));
                return true;
            }
            temp=temp<<1;
        }


      return false;
    }
    else if(num_bits==3){
        temp=7;
        temp=temp<<shift;
         for(i=0;i<6;i++){
             if(!(AddReff&temp)){
                 AddReff|=temp;
               addtemo=Adresret(region, i);
                MakeEntry(region,addtemo,(regval[region]*num_bits),7,(shift+i));
                return true;
            }
            temp=temp<<1;
        }

       return false;
    }
    else if(num_bits==4){
        temp=15;
        temp=temp<<shift;
         for(i=0;i<5;i++){
             if(!(AddReff&temp)){
                 AddReff|=temp;
                addtemo=Adresret(region, i);
                MakeEntry(region,addtemo,(regval[region]*num_bits),15,(shift+i));
                return true;
            }
            temp=temp<<1;
        }
        return false;

    }
    else if(num_bits ==5 ){
        temp=31;
        temp=temp<<shift;
         for(i=0;i<4;i++){
             if(!(AddReff&temp)){
                 AddReff|=temp;
                addtemo=Adresret(region, i);
                MakeEntry(region,addtemo,(regval[region]*num_bits),31,(shift+i));
                return true;
            }
            temp=temp<<1;
        }

    return false;
    }
    else if(num_bits ==6){
        temp=63;
        temp=temp<<shift;
         for(i=0;i<3;i++){
             if(!(AddReff&temp)){
                 AddReff|=temp;
                addtemo=Adresret(region, i);
                MakeEntry(region,addtemo,(regval[region]*num_bits),63,(shift+i));
                return true;
            }
            temp=temp<<1;
        }
        return false;

    }
    else if(num_bits ==7){
        temp=127;
        temp=temp<<shift;
         for(i=0;i<2;i++){
             if(!(AddReff&temp)){
                 AddReff|=temp;
                addtemo=Adresret(region, i);
                MakeEntry(region,addtemo,(regval[region]*num_bits),127,(shift+i));
                return true;
            }
            temp=temp<<1;
        }
        return false;
    }
    else if(num_bits ==8){
        temp=255;
        temp=temp<<shift;
             if(!(AddReff&temp)){
                 AddReff|=temp;
                addtemo=Adresret(region, i);
                MakeEntry(region,addtemo,(regval[region]*num_bits),255,(shift+i));
                return true;
            }
            return false;

    }
    if (num_bits==10 && region ==2){
        int run=0;
        temp=1023;
        temp=temp<<shift;
        for(run=0;run<6;run++){
            if(!(AddReff&temp)){
                                   AddReff|=temp;
                                  addtemo=Adresret(region, i);
                                  MakeEntry(region,addtemo,(regval[region]*num_bits),0x3FF,(shift+run));
                                  return true;
                              }
            temp=temp<<1;
        }


        //add the test for the 10 bits on loop by six to get all posible combinations and that will chekc if a whole block is nred for region 2 and 3
    }
    return false;

}

bool sizeof1536allo(int region,int sizeB){//makes special case of 1536 allo geivn the 3 special cross sections
    uint64_t check=3;
    if(region==0){
        check=check<<7;
        if(!(AddReff&check)){
            AddReff|=check;
            uint64_t val=0x20001E00;
            MakeEntry(0,val,sizeB,3,7);
            return true;
        }
        return false;
    }
    else if(region==1){
        check=check<<15;
        if(!(AddReff&check)){
            AddReff|=check;
            uint64_t val=0x20003c00;
            MakeEntry(1,val,sizeB,3,15);
            return true;
        }
        return false;
    }
    else if(region==3){
        check=check<<31;
        if(!(AddReff&check)){
            AddReff|=check;
            uint64_t val=0x20005E00;
            MakeEntry(3,val,sizeB,3,31);
            return true;
        }
        return false;
    }
    return false;

}
bool S_open_bit(int region,int sizeB){//find if bits in that cross region are able given the region
    uint64_t check=1;
    if(region==0){
        check=check<<7;

        if(!(AddReff&check)){
            AddReff|=check;
            uint64_t val=0x20001E00;
            MakeEntry(0,val,sizeB,1,7);
            return true;
        }
        return false;
    }
    else if(region==1){
        check=check<<8;

        if(!(AddReff&check)){
                AddReff|=check;
                uint64_t val=0x20002000;
                MakeEntry(1,val,sizeB,1,8);
                return true;
        }
        check=check<<7;

        if(!(AddReff&check)){
            AddReff|=check;
            uint64_t val=0x20003C00;
            MakeEntry(1,val,sizeB,1,15);
            return true;
        }
        return false;
    }
    else if(region ==2){
        check=check<<16;

        if(!(AddReff&check)){
                AddReff|=check;
                uint64_t val=0x20004000;
                MakeEntry(2,val,sizeB,1,16);
                return true;
        }
       return false;
    }
    else if(region==3){
         check=check<<31;

        if(!(AddReff&check)){
                AddReff|=check;
                uint64_t val=0x20005E00;
                MakeEntry(3,val,sizeB,1,31);
                return true;
        }
        return false;
    }
    else if(region==4){
        check=check<<32;
        if(!(AddReff&check)){
                AddReff|=check;
                uint64_t val=0x20006000;
                MakeEntry(4,val,sizeB,1,32);
                return true;
        }
        return false;
    }
    return false;

}

bool find_open_bit( int region,int sizeB){ //find the bit in a spefice region
    int i=0;
    int start_bit =region*8;
    uint64_t check=1;
    check=check<<start_bit;
    if(region==0 || region==2 || region == 3){
        if(region==0){
            for(i=0;i<7;i++){
                if(!(AddReff&check)){
                    AddReff|=check;
                    uint64_t val=0x20001000+(i*512);
                    MakeEntry(0,val,sizeB,1,i);
                    return true;
                }
            check=check<<1;
            }
            return false;
        }
        else if (region==2){
            check=check<<1;
            for(i=1;i<7;i++){
                if(!(AddReff&check)){
                    AddReff|=check;
                    uint64_t val=0x20004000+(i*512);
                    MakeEntry(2,val,sizeB,1,((region*8)+i));

                    return true;
                }
                check=check<<1;
            }
            return false;
        }
        else if (region==3){
            uint64_t val=0x20005000;
             check=check<<1;
                 for(i=1;i<8;i++){
                    if(!(AddReff&check)){
                        AddReff|=check;
                        val+=(i*512);
                        MakeEntry(4,val,sizeB,1,((region*8)+i));


                        return true;
                    }
                check=check<<1;
            }
            return false;
        }
    }
    else if (region==1 ||region ==4){
         if (region==4){
            uint64_t val=0x20006000;
             check=check<<1;
                 for( i=1;i<8;i++){
                    if(!(AddReff&check)){
                        AddReff|=check;
                        val+=(i*1024);
                        MakeEntry(4,val,sizeB,1,((region*8)+i));


                        return true;
                    }
                check=check<<1;
            }
            return false;
        }
        else if(region==1 ){
            check=check<<1;
        uint64_t val=0x20002000;
        for(i=1;i<8;i++){
            if(!(AddReff&check)){
                AddReff|=check;
                    val+=(i*1024);
                    MakeEntry(1,val,sizeB,1,((region*8)+(i)));

                return true;
            }
            check=check<<1;
        }
        return false;
    }
        }

    return false;

}




int roundUpToMultipleOf512(int size) {
   if (size % 512 == 0) {
        return size; // Already a multiple of 512
    }
    return ((size / 512) + 1) * 512; // Round up to the next multiple
}

int roundUpToMultipleOf1024(int size) {
   if (size % 1024 == 0) {
        return size; // Already a multiple of 1024
    }
    return ((size / 1024) + 1) * 1024; // Round up to the next multiple
}

int numof1024Need(int size){
    if (size == 0) {
        return 0; // No multiples of 1024 in 0
    }
    return (size + 1023) / 1024; // This effectively rounds up
}

int numof512need(int size){
    if (size == 0) {
        return 0; // No multiples of 512 in 0
    }
    return (size + 511) / 512; // This effectively rounds up
}

void * mallocFromHeap(uint32_t size_in_bytes)
{
    int i=0;
      void* mallFH=1;
      if(size_in_bytes<=0)return mallFH;
      if(size_in_bytes>1535&&size_in_bytes<1537){//looks for the 1536 spelicas regions frist
          if(sizeof1536allo(0,size_in_bytes)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
          if(sizeof1536allo(1,size_in_bytes)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
          if(sizeof1536allo(3,size_in_bytes)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
      }
      //for singualr bytes of allocation
      else if(size_in_bytes<=512){ //find the immmedete small size but skip over the the intersections line
          if(find_open_bit(0,512)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
          for(i=2;i<=3;i++){
              if(find_open_bit(i,512)){
                  mallFH=dataArray[EntryCount-1].HeadAdd;
                  return mallFH;
               }
          }
          //if not found go intersection line
          if(S_open_bit(0,512)){
                  mallFH=dataArray[EntryCount-1].HeadAdd;
                  return mallFH;
          }
          for(i=2;i<=3;i++){
              if(S_open_bit(i,512)){
                  mallFH=dataArray[EntryCount-1].HeadAdd;
                  return mallFH;
               }
          }
          for (i = 1; i <=4; i+=3)//if out of 512 use 1024
          {
              if(find_open_bit(i,1024)){
                  mallFH=dataArray[EntryCount-1].HeadAdd;
                  return mallFH;
              }
          }
          for (i = 1; i <=4; i+=3)//if out of 1024 use intersection line 1024
          {
              if(S_open_bit(i,1024)){
                  mallFH=dataArray[EntryCount-1].HeadAdd;
                  return mallFH;
              }
          }
          return mallFH;
      }
      else if( size_in_bytes ==1024){
          for (i = 1; i <=4; i+=3)
          {
              if(find_open_bit(i,size_in_bytes)){
                  mallFH=dataArray[EntryCount-1].HeadAdd;
                  return mallFH;
              }
          }
          for (i = 1; i <=4; i+=3)//if out of 1024 use special 102
              {
              if(S_open_bit(i,size_in_bytes)){
                  mallFH=dataArray[EntryCount-1].HeadAdd;
                  return mallFH;
              }
          }
          int num512block=numof512need(size_in_bytes);
          if(findCBits(0,num512block)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
          else if(findCBits(2,num512block)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
          else if(findCBits(3,num512block)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
      }
      else{ //for anthing taht is not part of the ideal memorly allocation like single
          int num512block=numof512need(roundUpToMultipleOf512(size_in_bytes));
          int num1024block=numof1024Need(roundUpToMultipleOf1024(size_in_bytes));
          if((num512block<=4)){
              if(findCBits(0,num512block)){
                  mallFH=dataArray[EntryCount-1].HeadAdd;
                  return mallFH;
                  }
              }
          else if(findCBits(1,num1024block)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
              //region o and 1
              //region 2 and 3
          else if((num512block<=4)){
              if(findCBits(2,num512block)){
                  mallFH=dataArray[EntryCount-1].HeadAdd;
                  return mallFH;
                  }
          }
          else if((num512block<=4)){
              if(findCBits(3,num512block)){
                  mallFH=dataArray[EntryCount-1].HeadAdd;
                  return mallFH;
              }
          }
          else  if(findCBits(4,num1024block)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
          else if(findCBits(0,num512block)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
          else if(findCBits(2,num512block)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
          else if(findCBits(3,num512block)){
              mallFH=dataArray[EntryCount-1].HeadAdd;
              return mallFH;
          }
      }
      return mallFH;
}

// REQUIRED: add your free code here and update the SRD bits for the current thread
void removeElement( int index) {
    int i,j;
    if (index < 0 || index >= EntryCount) {
        return;
    }
    // Create a new array with size reduced by 1
    // Copy elements, skipping the specified index
    for (i = 0, j = 0; i < EntryCount; i++) {
        if (i != index) {
            dataArray[j++] = dataArray[i];
        }
    }//erase the last index
    dataArray[j].pid=0;
    dataArray[j].HeadAdd=0;
    dataArray[j].FreeAdd=0;
    dataArray[j].region=0;
    dataArray[j].size=0;
    dataArray[j].bitshift=0;
    dataArray[j].bitVal=0;
    dataArray[j].PID=0;
    EntryCount--;
}


void freebits( int compVal,int bitShift) {
    uint64_t temp=0;
    temp=compVal;
    temp=temp<<bitShift;
    AddReff^=temp;
}

void freeToHeap(void *pMemory)//make sure this works
{
   // uint32_t addToFree=(uint32_t) pMemory;
    int i=0;
       int refnum=0;
       for(i=0;i<EntryCount;i++){//slower but removes every elemnt with taht pid
           if(dataArray[i].PID==pMemory){
               refnum=i;
               freebits(dataArray[i].bitVal,dataArray[i].bitshift);
               dataArray[i].pid=0;
               dataArray[i].HeadAdd=0;
               dataArray[i].region=0;
               dataArray[i].size=0;
               dataArray[i].bitshift=0;
               dataArray[i].bitVal=0;
               dataArray[i].PID=0;
               dataArray[i].FreeAdd=0;
               removeElement(i);
              // return;for orginal implemtn
           }
       }
       return;
}

// REQUIRED: include your solution from the mini project
void MPUON(){
    NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_PRIVDEFEN|NVIC_MPU_CTRL_ENABLE;
}

void allowFlashAccess(void)
{
    NVIC_MPU_NUMBER_R =0x5;
    NVIC_MPU_BASE_R |=(NVIC_MPU_BASE_ADDR_M & REG5);//192 and 128 193
                    //  AP        S C B      SRD         SIZE            ENABLE
    NVIC_MPU_ATTR_R|= (0x3<<24) |(0x2<<16) |(0x00<< 8) | (SIZEFLASH<<1)|NVIC_MPU_ATTR_ENABLE;
}

void allowPeripheralAccess(void)
{
    NVIC_MPU_NUMBER_R =0x6;
    NVIC_MPU_BASE_R |=(NVIC_MPU_BASE_ADDR_M & REG6);//192 and 128 193
                             //  AP        S C B      SRD         SIZE            ENABLE
    NVIC_MPU_ATTR_R|= NVIC_MPU_ATTR_XN  |(0x3<<24) |(0x0<<16) |(0x00<< 8) | (SIZEPERIPHERAL<<1)|NVIC_MPU_ATTR_ENABLE;

}

void setupSramAccess(void)
{
    //SRAM
      NVIC_MPU_NUMBER_R =0x0;
      NVIC_MPU_BASE_R |=(NVIC_MPU_BASE_ADDR_M & REG0);//192 and 128 193
                      //  AP        S C B      SRD         SIZE            ENABLE
      NVIC_MPU_ATTR_R|= (0x3<<24) |(0x6<<16) |(0x00<< 8) | (SIZE4k<<1)|NVIC_MPU_ATTR_ENABLE;


      NVIC_MPU_NUMBER_R =0x1;
      NVIC_MPU_BASE_R |=(NVIC_MPU_BASE_ADDR_M & REG1);//192 and 128 193
                          //  AP        S C B      SRD         SIZE            ENABLE
      NVIC_MPU_ATTR_R|= (0x3<<24) |(0x6<<16) |(0x00<< 8) | (SIZE8k<<1)|NVIC_MPU_ATTR_ENABLE;

      NVIC_MPU_NUMBER_R =0x2;
      NVIC_MPU_BASE_R |=(NVIC_MPU_BASE_ADDR_M & REG2);//192 and 128 193
                         //  AP        S C B      SRD         SIZE            ENABLE
      NVIC_MPU_ATTR_R|= (0x3<<24) |(0x6<<16) |(0x00<< 8) | (SIZE4k<<1)|NVIC_MPU_ATTR_ENABLE;


      NVIC_MPU_NUMBER_R =0x3;
      NVIC_MPU_BASE_R |=(NVIC_MPU_BASE_ADDR_M & REG3);//192 and 128 193
                         //  AP        S C B      SRD         SIZE            ENABLE
      NVIC_MPU_ATTR_R|= (0x3<<24) |(0x6<<16) |(0x00<< 8) | (SIZE4k<<1)|NVIC_MPU_ATTR_ENABLE;


      NVIC_MPU_NUMBER_R=0x4;
      NVIC_MPU_BASE_R |=(NVIC_MPU_BASE_ADDR_M & REG4);//192 and 128 193
                      //  AP        S C B      SRD         SIZE            ENABLE
      NVIC_MPU_ATTR_R|= (0x3<<24) |(0x6<<16) |(0x00<< 8) | (SIZE8k<<1)|NVIC_MPU_ATTR_ENABLE;
}

uint64_t createNoSramAccessMask(void)//1 menas no acces 0 mean full acess
{
    return 0xFFFFFFFFFFFFFFFF;
}

int SubAdd(uint32_t baseAdd){//for
    int ret=0;
        int i;
        if(0x20001000<=baseAdd &&baseAdd<=0x20001E00){
            baseAdd=baseAdd-0x20001000;
            for(i=0;i<8;i++){
                if(baseAdd==0){
                    return i;
                }
                baseAdd-=512;
            }
        }
        else if(0x20002000<=baseAdd &&baseAdd<=0x20003C00){
            ret=8;
            baseAdd=baseAdd-0x20002000;
            for(i=0;i<8;i++){
                if(baseAdd==0){
                    return (ret+i);
                }
                baseAdd-=1024;
            }
        }
        else if(0x20004000<=baseAdd &&baseAdd<=0x20004e00){
            ret=16;
            baseAdd=baseAdd-0x20004000;
            for(i=0;i<8;i++){
                if(baseAdd==0){
                    return (ret+i);
                }
                baseAdd-=512;
            }
        }
        else if(0x20005000<=baseAdd &&baseAdd<=0x20005E00){
            ret=24;
            baseAdd=baseAdd-0x20005000;
            for(i=0;i<8;i++){
                if(baseAdd==0){
                    return (ret+i);
                }
                baseAdd-=512;
            }
        }
        else if(0x20006000<=baseAdd &&baseAdd<=0x20007C00){
            ret=32;
            baseAdd=baseAdd-0x20006000;
            for(i=0;i<8;i++){
                if(baseAdd==0){
                    return (ret+i);
                }
                baseAdd-=1024;
            }
        }
        return ret;//if 0x2000
}


void addSramAccessWindow(uint64_t *srdBitMask, uint32_t baseAdd, uint32_t size_in_bytes) {
    uint32_t subregionBit = SubAdd(baseAdd);

    if (subregionBit == -1) {

        return; // Exit if the base address is invalid
    }
    int Reg = 0;
    uint32_t sumOFBits = 0, TempOfSize = 0;

    do {
        Reg = subregionBit / 8;  // Determine TempOfSize based on Reg
        if (Reg == 0 || Reg == 2 || Reg == 3) {
            TempOfSize = 512;
        } else {
            TempOfSize = 1024;
        }
        *srdBitMask &= ~(1ULL << subregionBit);// Clear the specific bit to disable access
        sumOFBits += TempOfSize;
        subregionBit++; // Increment the subregion bit for the next iteration
    } while (sumOFBits < size_in_bytes);
}

void applySramAccessMask(uint64_t srdBitMask)
{
    uint8_t REG;
         for (REG = 0; REG < 5; REG++) { // Looping from 0 to 4 (5 iterations)
             uint8_t SRDBIT = (uint8_t)(srdBitMask >> (REG * 8)); // Shift and mask to get the correct byte
             NVIC_MPU_NUMBER_R = REG;
             NVIC_MPU_ATTR_R &= ~(NVIC_MPU_ATTR_SRD_M); // Clear the previous SRD bits
             NVIC_MPU_ATTR_R |= (SRDBIT<<8); // Set the new SRD bits change to or if not work
         }
}


