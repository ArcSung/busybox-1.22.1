#ifndef __MMIOUTIL_H__
#define __MMIOUTIL_H__

unsigned char MmioUtil_Read8(void *base, const unsigned long offset);
unsigned short MmioUtil_Read16(void *base, const unsigned long offset);
unsigned int MmioUtil_Read32(void *base, const unsigned long offset);
void MmioUtil_Write8(void *base, const unsigned long offset, const unsigned char val);
void MmioUtil_Write16(void *base, const unsigned long offset, const unsigned short val);
void MmioUtil_Write32(void *base, const unsigned long offset, const unsigned int val);


#endif //#ifndef __MMIOUTIL_H__
