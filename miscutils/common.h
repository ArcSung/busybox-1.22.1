#include <stdio.h>
#include<strings.h>
#include "MmioUtil.h"

#define stricmp strcasecmp

#define MMIO_SIZE 0x200000
#define PMU_SIZE  0x80000
#define GPIO_SIZE 0x80000

#define LENGTHPERSTRING 20
#define NUMBEROFSTRING   5

#define TRUE        1
#define FALSE       0

typedef unsigned char   UCHAR,    *PUCHAR;
typedef unsigned char   UINT8,    *PUINT8;
typedef unsigned short  UINT16,   *PUINT16;
typedef unsigned int    UINT32,   *PUINT32;
typedef unsigned long long UINT64, *PUINT64;
typedef int BOOL;

#define SRBASE          0x8600
#define SR_BBASE        0x8700
#define CRBASE          0x8800
#define CR_BBASE        0x8900

typedef enum _REGISTER_TYPE
{
    INVALID,
    SR,
    SR_B,
    CR,
    CR_B,
    MMIO,
    PMU,
    GPIO
}REGISTER_TYPE;

typedef struct _REG_FILE_PAR
{
    REGISTER_TYPE regType;
    char* name;
    int addr;
} REG_FILE_PAR;

typedef struct _Reg_Tool_Para
{
    REGISTER_TYPE regType;
    UCHAR regTypeName[LENGTHPERSTRING];
    UINT32 regIndex;
    UINT32 regValue;
    UINT32 isRead;
    UINT32 isRegFile;
}Reg_Tool_Para, *pReg_Tool_Para;

typedef struct _VirtBase_Para
{
    void *mmio;
    void *pmu;
    void *gpio;
    void *virtualBase;
}VirtBase_Para,*pVirtBase_Para;


//---------------------------------------------
void PrintResult(pReg_Tool_Para pRegToolPara);
void WriteRegister(void *virtualBase, pReg_Tool_Para pRegToolPara);
void ReadRegister(void *virtualBase, pReg_Tool_Para pRegToolPara);
void PreReadWrite(pVirtBase_Para virtBase_Para, pReg_Tool_Para pRegToolPara);
void ParseArgvsCommand(pReg_Tool_Para pRegToolPara);
BOOL ParseCommand(char *buffer, pReg_Tool_Para pRegToolPara);
