#include "MmioUtil.h"

#if defined(__BIG_ENDIAN__)

unsigned char MmioUtil_Read8(void *base, const unsigned long offset)        
{                                                                                                                                   
    register unsigned char val;                                               
    unsigned long alignedoffset = (offset /4) *4;                             
    unsigned long mask = offset - alignedoffset;                       

#if defined(__powerpc__)
    __asm__ __volatile__("lbz%U1%X1 %0,%1; twi 0,%0,0; isync"
                       : "=r" (val) : "m" (*(volatile unsigned char*)(void*)((char*)(base) + (alignedoffset+(3-mask)))));
#else
     val = *(volatile unsigned char*)(void*)((char*)(base) + (alignedoffset+(3-mask)));
#endif
    return val;                                                                       
}                                                                                     
                                                                                      
unsigned short MmioUtil_Read16(void *base, const unsigned long offset)               
{                                                                                     
   register unsigned short val;                                                       
   unsigned long alignedoffset = (offset /4)*4;                                       
   unsigned long mask = offset - alignedoffset;                                       
                                                                                      
   val = *(volatile unsigned short*)(void*)((char*)(base) + (alignedoffset+(2-mask)));
   return val;                                                                        
                                                                                      
}                                                                                     
                                                                                      
unsigned int MmioUtil_Read32(void *base, const unsigned long offset)                                                            {                                                                             
    register unsigned int val;
    val = *(volatile unsigned int *)(void *)((char *)(base) + (offset));
    return val;
}                                                                             

void MmioUtil_Write8(void *base, const unsigned long offset, const unsigned char val)           
{                                                                                     
    unsigned long alignedoffset = (offset /4) *4;                                     
    unsigned long mask = offset - alignedoffset;   
    
#if  defined( __powerpc__)
    __asm__ __volatile__("stb%U0%X0 %1,%0; sync"
                         : "=m" (*(volatile unsigned char *)(void *)((char *)(base) + (alignedoffset+(3-mask)))) : "r" (val));  
#else
    *(volatile unsigned char *)(void *)((char *)(base) + (alignedoffset+(3-mask))) = (val);
#endif                                                                                           
}                                                                                          
                                                                                           
void MmioUtil_Write16(void *base, const unsigned long offset, const unsigned short val)               
{                                                                                          
    unsigned long alignedoffset = (offset /4) *4;                                          
    unsigned long mask = offset - alignedoffset;                                           
    *(volatile unsigned short *)(void *)((char *)(base) + (alignedoffset+(2-mask))) = (val);
}                    

void MmioUtil_Write32(void *base, const unsigned long offset, const unsigned int val)         
{                                                                                     
    *(volatile unsigned int *)(void *)((char *)(base) + (offset)) = (val);            
}                                                                                     



#else


unsigned char MmioUtil_Read8(void *base, const unsigned long offset)
{
    return *(volatile unsigned char *)(void *)((char *)(base) + (offset));
}

unsigned short MmioUtil_Read16(void *base, const unsigned long offset)
{
    return *(volatile unsigned short *)(void *)((char *)(base) + (offset));
}

unsigned int MmioUtil_Read32(void *base, const unsigned long offset)
{
    return *(volatile unsigned int *)(void *)((char *)(base) + (offset));
}

void MmioUtil_Write8(void *base, const unsigned long offset, const unsigned char val)
{
    *(volatile unsigned char *)(void *)((char *)(base) + (offset)) = (val);
}

void MmioUtil_Write16(void *base, const unsigned long offset, const unsigned short val)
{
    *(volatile unsigned short *)(void *)((char *)(base) + (offset)) = (val);
}

void MmioUtil_Write32(void *base, const unsigned long offset, const unsigned int val)
{
    *(volatile unsigned int *)(void *)((char *)(base) + (offset)) = (val);
}

#endif //#if defined(__BIG_ENDIAN__)
