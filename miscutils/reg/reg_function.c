#include "common.h"

static REG_FILE_PAR reg_par[] = {
INVALID,"",     0x0,
SR,   "SR",     SRBASE,
SR_B, "SR_B",   SR_BBASE,
CR,   "CR",     CRBASE, 
CR_B, "CR_B",   CR_BBASE 
};

UINT32 pmu_reg_group[] = 
{
    0x0250,
    0x3200,
    0x320c,
    0x3210,
    0xc340,0xc348,
    0xc6c0,0xc6c4,
    0xc6c8,0xc6cc,
    0xc6f4,
    0xd2e0,
    0xe080, 
};

void DumpRegToFile(char* pFileName, pVirtBase_Para pVirtBase)
{
    unsigned int num;
    unsigned char i, j, m, n, ch;
    unsigned int mmio, mmioValue, pmuValue;

    FILE *pFile;
    static int iWriteTimes = 0;

    if( (pFile = fopen( pFileName, "wt" )) == NULL)
    {
        printf("Cannot open the file. Write to register file failed!\n");
        return;
    }
    
    fprintf(pFile, "::Start::\n");
    fprintf(pFile, " This file show the registers dumped by Register Tool. \n");

    //dump sr,srb,cr,crb
    fprintf(pFile, "\n[SRCR]\n=========================================================\n"); 
    for(i = SR; i <= CR_B; i++)
    {                
        j = 0;
        num = 256;
        fprintf(pFile, "\n%6s | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n", reg_par[i].name);
        fprintf(pFile, " ------|-------------------------------------------------\n");
        for(m=0; m<16; m++)
        {
            fprintf(pFile, "    %X0 | ",m);
            for(n=0; n<16; n++)
            {
                if(j < num)
                {
                    ch = MmioUtil_Read8(pVirtBase->mmio, (UINT32)reg_par[i].addr+16*m+n );
                    fprintf(pFile, "%02x ", ch);                   
                    j++;
                }
            }
            fprintf(pFile, "\n");
        }      
    }
    printf("dump SR, SR_B, CR, CR_B success!\n");
    
    //dump mmio
    fprintf(pFile, "\n[MMIO]\n========================================================="); 
    i =0;
    for(mmio = 0x8180; mmio <= 0x83ac;mmio += 4)
    {
        if((i++)%4 == 0)
        {
            fprintf(pFile, "\n  %X | ",mmio); 
        }
        mmioValue = MmioUtil_Read32(pVirtBase->mmio, mmio);
        fprintf(pFile, "%08x   ", mmioValue);
    }
    for(mmio = 0x3000; mmio <= 0x300c;mmio += 4)
    {
        if((i++)%4 == 0)
        {
            fprintf(pFile, "\n  %X | ",mmio); 
        }
        mmioValue = MmioUtil_Read32(pVirtBase->mmio, mmio);
        fprintf(pFile, "%08x   ", mmioValue);
    }
    for(mmio = 0x3100; mmio <= 0x3288;mmio += 4)
    {
        if((i++)%4 == 0)
        {
            fprintf(pFile, "\n  %X | ",mmio); 
        }
        mmioValue = MmioUtil_Read32(pVirtBase->mmio, mmio);
        fprintf(pFile, "%08x   ", mmioValue);
    }
    fprintf(pFile, "\n"); 
    printf("dump MMIO success!\n");  

    //dump pmu
    fprintf(pFile, "\n[PMU]\n=========================================================\n");
    j = sizeof(pmu_reg_group)/sizeof(UINT32);
    for(i=0; i<j; i++)
    {
        pmuValue = MmioUtil_Read32(pVirtBase->pmu, pmu_reg_group[i]);
        fprintf(pFile, "  %04X | %08x\n", pmu_reg_group[i], pmuValue);
    }
    printf("dump PMU  success!\n");  
   
    fprintf(pFile, "\n::End::\n"); 
    fclose(pFile);
    
    printf("Write all registers to file success!\n");
    return;
}


void Diff2DumpFiles(char* pFileNameA, char* pFileNameB)
{
    FILE *pFileA, *pFileB, *pFileDiff;
    char regStrA[11],regStrB[11];
    UINT32 reg, valueA, valueB;
    char regTemp[256];
    BOOL isFirst = TRUE;
    char *diff2File = "diff_a_b.txt";
    UINT32 i,j,mmio;
    pFileA = fopen(pFileNameA,"rt");
    pFileB = fopen(pFileNameB,"rt");
    pFileDiff = fopen(diff2File,"wt");

    if((pFileA == NULL)||(pFileB == NULL) || pFileDiff == NULL)
    {
        printf("open file error! please check the  two file names!\n");
        return; 
    }

    fprintf(pFileDiff,"::Start::\nThis file is auto created by Register tool,\nPlease do not modify it's style. Thanks!\n");
    fprintf(pFileDiff,"\n=======================================\n");
    fprintf(pFileDiff,"   Reg    File-%s       File-%s\n", pFileNameA, pFileNameB);
    printf("   Reg    File-%s       File-%s\n", pFileNameA, pFileNameB);
    fprintf(pFileDiff,"=======================================\n\n");
    while(TRUE)
    {
        if(isFirst)
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileA);
            fgets(regTemp,sizeof(regTemp)-1,pFileB);
            isFirst = FALSE;
            if(0 !=(strcmp("::Start::\n",regTemp)) )
            {
                printf("dump file format error,please check it.\n");
                break;
            }
            for(i=0;i<5;i++)
            {
                fgets(regTemp,sizeof(regTemp)-1,pFileA);
                fgets(regTemp,sizeof(regTemp)-1,pFileB);
            }
            continue;
        }  
        memset(regStrA,'\0',sizeof(regStrA));
        memset(regStrB,'\0',sizeof(regStrB));

        if((NULL == fgets(regStrA,10,pFileA)) ||(NULL == fgets(regStrB,10,pFileB)) ) 
        {
            break;
        }
        if(0 ==(strcmp("::End::\n",regStrA)) || 0 ==(strcmp("::End::\n",regStrB)) )
        {
            printf("\nDiff all register finished! The result is saved in the file 'diff_a_b.txt'.\n");
            fprintf(pFileDiff,"::End::\n");
            break;
        }
        if(strcmp(regStrA,regStrB))
        {
            printf("Format different, cannot diff them.\n");
            break;
        }
        
        sscanf(regStrB,"%s",regStrA);
        //printf("regStrA: %s , regStrB: %s\n",regStrA, regStrB);

        if(0 == strcmp(regStrA,"SR"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileA);
            fgets(regTemp,sizeof(regTemp)-1,pFileB);
            fgets(regTemp,sizeof(regTemp)-1,pFileA);
            fgets(regTemp,sizeof(regTemp)-1,pFileB);

            printf("SR diff:-------------------------------\n");
            fprintf(pFileDiff,"SR diff:-------------------------------\n");
            for(j=0; j<16; j++)
            {
                for(i=0; i<16; i++)
                {
                    if(i == 0)
                    {
                        fscanf(pFileA, "%02X |", &valueA);
                        fscanf(pFileB, "%02X |", &valueB);
                    }
                    fscanf(pFileA, "%02x", &valueA);
                    fscanf(pFileB, "%02x", &valueB);
                    if(valueA != valueB)
                    {
                        printf("  sr %01X%01X     %02x     %02x\n",j,i, valueA, valueB);
                        fprintf(pFileDiff,"  sr %01X%01X      %02x         %02x\n",j,i, valueA, valueB);
                    }
                }
            }
            fprintf(pFileDiff,"\n");
            printf("\n");
            
            fscanf(pFileA, "\n");
            fscanf(pFileB, "\n"); 
        }
        else if (0 == strcmp(regStrA,"SR_B"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileA);
            fgets(regTemp,sizeof(regTemp)-1,pFileB);
            fgets(regTemp,sizeof(regTemp)-1,pFileA);
            fgets(regTemp,sizeof(regTemp)-1,pFileB);
            printf("SR_B diff:-----------------------------\n");
            fprintf(pFileDiff,"SR_B diff:-----------------------------\n");

            for(j=0; j<16; j++)
            {
                for(i=0; i<16; i++)
                {
                    if(i == 0)
                    {
                        fscanf(pFileA, "%02X |", &valueA);
                        fscanf(pFileB, "%02X |", &valueB);
                    }
                    fscanf(pFileA, "%02x", &valueA);
                    fscanf(pFileB, "%02x", &valueB);
                    if(valueA != valueB)
                    {
                        printf(" srb %01X%01X     %02x     %02x\n",j,i, valueA, valueB);
                        fprintf(pFileDiff," srb %01X%01X      %02x        %02x\n",j,i, valueA, valueB);
                    }
                }
            }
            fprintf(pFileDiff,"\n");
            printf("\n");

            fscanf(pFileA, "\n");
            fscanf(pFileB, "\n");
        }
        else if (0 == strcmp(regStrA,"CR"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileA);
            fgets(regTemp,sizeof(regTemp)-1,pFileB);
            fgets(regTemp,sizeof(regTemp)-1,pFileA);
            fgets(regTemp,sizeof(regTemp)-1,pFileB);
            printf("CR diff:-------------------------------\n");
            fprintf(pFileDiff,"CR diff:-------------------------------\n");

            for(j=0; j<16; j++)
            {
                for(i=0; i<16; i++)
                {
                    if(i == 0)
                    {
                        fscanf(pFileA, "%02X |", &valueA);
                        fscanf(pFileB, "%02X |", &valueB);
                    }
                    fscanf(pFileA, "%02x", &valueA);
                    fscanf(pFileB, "%02x", &valueB);
                    if(valueA != valueB)
                    {
                        printf("  cr %01X%01X     %02x     %02x\n",j,i, valueA, valueB);
                        fprintf(pFileDiff,"  cr %01X%01X      %02x        %02x\n",j,i, valueA, valueB);
                    }
                }
            }
            fprintf(pFileDiff,"\n");
            printf("\n");

            fscanf(pFileA, "\n");
            fscanf(pFileB, "\n");
        }
        else if (0 == strcmp(regStrA,"CR_B"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileA);
            fgets(regTemp,sizeof(regTemp)-1,pFileB);
            fgets(regTemp,sizeof(regTemp)-1,pFileA);
            fgets(regTemp,sizeof(regTemp)-1,pFileB);
            printf("CR_B diff:-----------------------------\n");
            fprintf(pFileDiff,"CR_B diff:-----------------------------\n");

            for(j=0; j<16; j++)
            {
                for(i=0; i<16; i++)
                {
                    if(i == 0)
                    {
                        fscanf(pFileA, "%02X |", &valueA);
                        fscanf(pFileB, "%02X |", &valueB);
                    }
                    fscanf(pFileA, "%02x", &valueA);
                    fscanf(pFileB, "%02x", &valueB);
                    if(valueA != valueB)
                    {
                        printf(" crb %01X%01X     %02x     %02x\n",j,i, valueA, valueB);
                        fprintf(pFileDiff," crb %01X%01X      %02x        %02x\n",j,i, valueA, valueB);
                    }
                }
            }
            fprintf(pFileDiff,"\n");
            printf("\n");

            fscanf(pFileA, "\n");
            fscanf(pFileB, "\n");
        }
        else if (0 == strcmp(regStrA,"[MMIO]"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileA);
            fgets(regTemp,sizeof(regTemp)-1,pFileB);
            printf("MMIO diff:-----------------------------\n");
            fprintf(pFileDiff,"MMIO diff:-----------------------------\n");

            i =0;
            for(mmio = 0x8180; mmio <= 0x83ac;mmio += 4)
            {
                if((i++)%4 == 0)
                {
                    fscanf(pFileA, "%04x |",&valueA); 
                    fscanf(pFileB, "%04x |",&valueB); 
                }
                fscanf(pFileA, "%08x", &valueA);
                fscanf(pFileB, "%08x", &valueB);
                if(valueA != valueB)
                {
                    printf("  %04X    %08x     %08x\n",mmio, valueA, valueB);
                    fprintf(pFileDiff," %04X     %08x     %08x\n",mmio, valueA, valueB);
                }
            }

            for(mmio = 0x3000; mmio <= 0x300c;mmio += 4)
            {
                if((i++)%4 == 0)
                {
                    fscanf(pFileA, "%04x |",&valueA); 
                    fscanf(pFileB, "%04x |",&valueB); 
                }
                fscanf(pFileA, "%08x", &valueA);
                fscanf(pFileB, "%08x", &valueB);
                if(valueA != valueB)
                {
                    printf("  %04X    %08x     %08x\n",mmio, valueA, valueB);
                    fprintf(pFileDiff," %04X     %08x     %08x\n",mmio, valueA, valueB);
                }
            }

            for(mmio = 0x3100; mmio <= 0x3288;mmio += 4)
            {
                if((i++)%4 == 0)
                {
                    fscanf(pFileA, "%04x |",&valueA); 
                    fscanf(pFileB, "%04x |",&valueB); 
                }
                fscanf(pFileA, "%08x", &valueA);
                fscanf(pFileB, "%08x", &valueB);
                if(valueA != valueB)
                {
                    printf("  %04X    %08x     %08x\n",mmio, valueA, valueB);
                    fprintf(pFileDiff," %04X     %08x     %08x\n",mmio, valueA, valueB);
                }
            }

            fprintf(pFileDiff,"\n");
            printf("\n");

            fscanf(pFileA, "\n");
            fscanf(pFileB, "\n");
        }
        else if (0 == strcmp(regStrA,"[PMU]"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileA);
            fgets(regTemp,sizeof(regTemp)-1,pFileB);
            printf("PMU diff:------------------------------\n");
            fprintf(pFileDiff,"PMU diff:-----------------------------\n");
            
            while(TRUE)
            {
                i = fscanf(pFileA, "%04x | %08x", &reg, &valueA);
                j = fscanf(pFileB, "%04x | %08x", &reg, &valueB);
                if((i != j) || (i != 2))
                {
                    break;
                }
                if(valueA != valueB)
                {
                    printf("  %04X    %08x     %08x\n",reg, valueA, valueB);
                    fprintf(pFileDiff," %04X     %08x     %08x\n",reg, valueA, valueB);
                }
            }

            fprintf(pFileDiff,"\n");

            fscanf(pFileA, "\n\n");
            fscanf(pFileB, "\n\n");
        }
        else
        {
            printf("command error! The file style cannot match the Register tool!\n");
            break;
        }
    }

    fclose(pFileA);
    fclose(pFileB);
    fclose(pFileDiff);    
}

void LoadDiff(char *pLoadWhat, pVirtBase_Para virtBase_Para)
{
    FILE *pFileDiff;
    char regStrA[16],regStrB[16];
    UINT32 reg,valueA,valueB,regValue;
    UINT32 i,j,mmio;
    BOOL isFirst = TRUE;
    BOOL isLoadA = TRUE;
    char regTemp[256];
    char *diff2File = "diff_a_b.txt";
    
    if (0 == strcmp(pLoadWhat,"ldda"))
    {
        isLoadA = TRUE;
    }
    else if(0 == strcmp(pLoadWhat,"lddb"))
    {
        isLoadA = FALSE;
    }
    else
    {
        printf("LoadDiff: command error, don't know what to load.\n");
        return;
    }
    
    if(NULL == (pFileDiff = fopen(diff2File,"rt")))
    {
        printf("cannot open diff file.\n");
        return; 
    }

    printf("\n");
    printf("-------------------\n");
    printf("Load:\n  Reg      Value\n");
    printf("-------------------\n");
    
    while(TRUE)
    {
        if(isFirst)
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileDiff);
            isFirst = FALSE;
            if(0 !=(strcmp("::Start::\n",regTemp)) )
            {
                printf("Diff file format error,please check it.\n");
                break;
            }
            for(i=0;i<7;i++)
            {
                fgets(regTemp,sizeof(regTemp)-1,pFileDiff);
            }
            continue;
        }  
        fscanf(pFileDiff, "\n");
        
        memset(regStrA,'\0',sizeof(regStrA));
        memset(regStrB,'\0',sizeof(regStrB));

        if(NULL == fgets(regStrB,15,pFileDiff)) 
        {
            break;
        }
        if(0 ==(strcmp("::End::\n",regStrB)))
        {
            printf("\nLoad all diffs finished!\n");
            break;
        }
       
        sscanf(regStrB,"%s",regStrA);
        //printf("regStrA: %s    regStrB: %s\n",regStrA,regStrB);
        if(0 == strcmp(regStrA,"SR"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileDiff);
            fscanf(pFileDiff, "\n");
            while(TRUE)
            {
                j = fscanf(pFileDiff, "sr %02x  %02x  %02x \n", &reg, &valueA, &valueB);
                if(3 != j)
                {
                    break;
                }
                regValue = isLoadA ? valueA : valueB;
                printf(" sr %02x      %02x\n",reg,regValue);
                MmioUtil_Write8(virtBase_Para->mmio, SRBASE+reg, regValue);
            }
        }
        else if(0 == strcmp(regStrA,"SR_B"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileDiff);
            fscanf(pFileDiff, "\n");
            while(TRUE)
            {
                j = fscanf(pFileDiff, "srb %02x  %02x  %02x \n", &reg, &valueA, &valueB);
                if(3 != j)
                {
                    break;
                }
                regValue = isLoadA ? valueA : valueB;
                printf(" srb %02x     %02x\n",reg,regValue);
                MmioUtil_Write8(virtBase_Para->mmio, SR_BBASE+reg, regValue);
            }
        }
        else if(0 == strcmp(regStrA,"CR"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileDiff);  
            fscanf(pFileDiff, "\n");
            while(TRUE)
            {
                j = fscanf(pFileDiff, "cr %02x  %02x  %02x \n", &reg, &valueA, &valueB);
                if(3 != j)
                {
                    break;
                }
                regValue = isLoadA ? valueA : valueB;
                printf(" cr %02x      %02x\n",reg,regValue);
                MmioUtil_Write8(virtBase_Para->mmio, CRBASE+reg, regValue);
            }
        }
        else if(0 == strcmp(regStrA,"CR_B"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileDiff);
            fscanf(pFileDiff, "\n");
            while(TRUE)
            {
                j = fscanf(pFileDiff, "crb %02x  %02x  %02x \n", &reg, &valueA, &valueB);
                if(3 != j)
                {
                    break;
                }
                regValue = isLoadA ? valueA : valueB;
                printf(" crb %02x     %02x\n",reg,regValue);
                MmioUtil_Write8(virtBase_Para->mmio, CR_BBASE+reg, regValue);
            }
             fscanf(pFileDiff, "\n");
        }
        else if(0 == strcmp(regStrA,"MMIO"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileDiff);
            printf("\nMMIO:\n");
            while(TRUE)
            {
                j = fscanf(pFileDiff, "%04x  %08x  %08x", &reg, &valueA, &valueB);
                if(3 != j)
                {
                    break;
                }
                regValue = isLoadA ? valueA : valueB;
                printf(" %04x     %08x\n",reg,regValue);
                MmioUtil_Write32(virtBase_Para->mmio, reg, regValue);
            }
        }
        else if(0 == strcmp(regStrA,"PMU"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFileDiff);
            printf("\nPMU:\n");
            while(TRUE)
            {
                j = fscanf(pFileDiff, "%04x  %08x  %08x", &reg, &valueA, &valueB);
                if(3 != j)
                {
                    break;
                }
                regValue = isLoadA ? valueA : valueB;
                printf(" %04x     %08x\n",reg,regValue);
                MmioUtil_Write32(virtBase_Para->pmu, reg, regValue);
            }
            printf("-------------------\n");
        }
        else
        {
            printf("command error! The file style cannot match the Register tool!\n");
            break;
        }
    }
    
    fclose(pFileDiff); 
}

void LoadDumpedFile(char* pFileName, pVirtBase_Para pVirtBase)
{
    FILE *pFile;
    char regStrA[11],regStrB[11];
    UINT32 reg, valueA,valueB;
    UINT32 i,j,num,mmio;
    char regTemp[256];
    BOOL isFirst = TRUE; 

    if(NULL == (pFile = fopen(pFileName,"rt")))
    {
        printf("cannot open the file.\n");
        return; 
    }

    while(TRUE)
    {
        if(isFirst)
        {
            fgets(regTemp,sizeof(regTemp)-1,pFile);
            isFirst = FALSE;
            if(0 !=(strcmp("::Start::\n",regTemp)) )
            {
                printf("dump file format error,please check it.\n");
                break;
            }
            for(i=0;i<5;i++)
            {
                fgets(regTemp,sizeof(regTemp)-1,pFile);
            }
            continue;
        }  

        memset(regStrA,'\0',sizeof(regStrA));
        memset(regStrB,'\0',sizeof(regStrB));
        if(NULL == fgets(regStrB,10,pFile)) 
        {
            break;
        }
        if(0 ==(strcmp("::End::\n",regStrB)))
        {
            printf("\nLoad dump file finished!\n");
            break;
        }
        sscanf(regStrB,"%s",regStrA);
        //printf("regStrA: %s    regStrB: %s\n",regStrA,regStrB);

        if(0 == strcmp(regStrA,"SR"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFile);
            fgets(regTemp,sizeof(regTemp)-1,pFile);
            for(j=0; j<16; j++)
            {
                for(i=0; i<16; i++)
                {
                    if(i == 0)
                    {
                        fscanf(pFile, "%02X |", &valueA);
                    }
                    num = fscanf(pFile, "%02x", &valueA);
                    if(1 != num)
                    {
                        break;
                    }
                    //printf("  %02X  %02x  |",j*16+i, valueA);
                    reg = j*16+i;
                    MmioUtil_Write8(pVirtBase->mmio, SRBASE+reg, valueA);
                }
            }
            printf("load SR done.\n");
            fscanf(pFile, "\n");
        }
        else if(0 == strcmp(regStrA,"SR_B"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFile);
            fgets(regTemp,sizeof(regTemp)-1,pFile);
            for(j=0; j<16; j++)
            {
                for(i=0; i<16; i++)
                {
                    if(i == 0)
                    {
                        fscanf(pFile, "%02X |", &valueA);
                    }
                    num = fscanf(pFile, "%02x", &valueA);
                    if(1 != num)
                    {
                        break;
                    }
                    //printf("  %02X  %02x  |",j*16+i, valueA);
                    reg = j*16+i;
                    MmioUtil_Write8(pVirtBase->mmio, SR_BBASE+reg, valueA);
                }
            }
            printf("load SR_B done.\n");
            fscanf(pFile, "\n");            
        }
        else if(0 == strcmp(regStrA,"CR"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFile);
            fgets(regTemp,sizeof(regTemp)-1,pFile);
            for(j=0; j<16; j++)
            {
                for(i=0; i<16; i++)
                {
                    if(i == 0)
                    {
                        fscanf(pFile, "%02X |", &valueA);
                    }
                    num = fscanf(pFile, "%02x", &valueA);
                    if(1 != num)
                    {
                        break;
                    }
                    //printf("  %02X  %02x  |",j*16+i, valueA);
                    reg = j*16+i;
                    MmioUtil_Write8(pVirtBase->mmio, CRBASE+reg, valueA);
                }
            }
            printf("load CR done.\n");
            fscanf(pFile, "\n");
        }
        else if(0 == strcmp(regStrA,"CR_B"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFile);
            fgets(regTemp,sizeof(regTemp)-1,pFile);
            for(j=0; j<16; j++)
            {
                for(i=0; i<16; i++)
                {
                    if(i == 0)
                    {
                        fscanf(pFile, "%02X |", &valueA);
                    }
                    num = fscanf(pFile, "%02x", &valueA);
                    if(1 != num)
                    {
                        break;
                    }
                    //printf("  %02X  %02x  |",j*16+i, valueA);
                    reg = j*16+i;
                    MmioUtil_Write8(pVirtBase->mmio, CR_BBASE+reg, valueA);
                }
            }
            printf("load CR_B done.\n");
            fscanf(pFile, "\n");
        }
        else if (0 == strcmp(regStrA,"[MMIO]"))
        {
            i = 0;
            fgets(regTemp,sizeof(regTemp)-1,pFile);
            for(mmio = 0x8180; mmio <= 0x83ac;mmio += 4)
            {
                if((i++)%4 == 0)
                {
                    fscanf(pFile, "%04x |",&valueA); 
                }
                fscanf(pFile, "%08x", &valueA);
                MmioUtil_Write32(pVirtBase->mmio, mmio, valueA);
                //printf("  %04X ->  %08x\n",mmio, valueA);
            }
            for(mmio = 0x3000; mmio <= 0x300c;mmio += 4)
            {
                if((i++)%4 == 0)
                {
                    fscanf(pFile, "%04x |",&valueA); 
                }
                fscanf(pFile, "%08x", &valueA);
                MmioUtil_Write32(pVirtBase->mmio, mmio, valueA);
                //printf("  %04X ->  %08x\n",mmio, valueA);
            }
            for(mmio = 0x3100; mmio <= 0x3288;mmio += 4)
            {
                if((i++)%4 == 0)
                {
                     fscanf(pFile, "%04x |",&valueA);
                }
                fscanf(pFile, "%08x", &valueA);
                MmioUtil_Write32(pVirtBase->mmio, mmio, valueA);
                //printf("  %04X ->  %08x\n",mmio, valueA);
            }
            printf("load MMIO done.\n");
            fscanf(pFile, "\n");
        }
        else if (0 == strcmp(regStrA,"[PMU]"))
        {
            fgets(regTemp,sizeof(regTemp)-1,pFile);
            while(TRUE)
            {
                j = fscanf(pFile, "%04x | %08x", &reg, &valueA);
                if(2 != j)
                {
                    break;
                }
                MmioUtil_Write32(pVirtBase->pmu, reg, valueA);
                //printf("%04x     %08x\n",reg,valueA);
            }
            printf("load PMU done.\n");
            fscanf(pFile, "\n\n");
        }
        else
        {
            printf("test command error.\n");
            break;
        }      
    }
    
}

BOOL ParseCommand(char *buffer, pReg_Tool_Para pRegToolPara)
{
    UINT32 i, j,numofstrings;
    REGISTER_TYPE  command;
    char str[NUMBEROFSTRING][LENGTHPERSTRING];
    char *token = NULL;
    char filter[] = " ,\t\n";

    //printf("enter: parseCommand\n");

    token = strtok(buffer, filter);
    strcpy(str[0], token);
    i = 1;
    while(token != NULL)
    {
        token = strtok(NULL, filter);
        if(token != NULL)
        {
            if(i < NUMBEROFSTRING)//to avoid no enough lines buffer
            {
                if(strlen(token) < LENGTHPERSTRING)//to avoid string too long
                {
                    strcpy(str[i], token);
                    i++;
                }
            }
            else
                break;
        }
    }
    numofstrings = i;

    //command judgment.
    i = 0;
    
    if(stricmp(str[i], "sr") == 0)
    {
        command = SR;
    }
    else if(stricmp(str[i], "srb") == 0)
    {
        command = SR_B;
    }
    else if(stricmp(str[i], "cr") == 0)
    {
        command = CR;
    }
    else if(stricmp(str[i], "crb") == 0)
    {
        command = CR_B;
    }
    else if(stricmp(str[i], "mmio") == 0)
    {
        command = MMIO;
    }
    else if(stricmp(str[i], "pmu") == 0)
    {
        command = PMU;
    }
    else if(stricmp(str[i], "gpio") == 0)
    {
        command = GPIO;
    }
    else
    {
        command = INVALID;
    }

    pRegToolPara->regType = command;
    strcpy(pRegToolPara->regTypeName, str[i]); 

    if(numofstrings < 2 || numofstrings > 3 || (pRegToolPara->regType == INVALID))
    {
        printf("ParseCommand: Command error!\n");
        return FALSE;
    }

    i++; //regIndex
    sscanf(str[i], "%x", &(pRegToolPara->regIndex));
    
    if(2 == numofstrings)
    {
        pRegToolPara->isRead = TRUE;
    }
    else
    {
        pRegToolPara->isRead = FALSE;
        i++; //regValue
        sscanf(str[i], "%x", &(pRegToolPara->regValue));
    }
 
    //printf("out: parseCommand\n");
    return TRUE;
}

void PreParseArgvs(char* cmd, pReg_Tool_Para pRegToolPara)
{
    if( !(stricmp(cmd,"sr"))  || !(stricmp(cmd,"srb")) || !(stricmp(cmd,"cr")) || !(stricmp(cmd,"crb")) ||!(stricmp(cmd,"mmio")) ||!(stricmp(cmd,"gpio"))||!(stricmp(cmd,"pmu")) )
    {
        pRegToolPara->isRegFile = FALSE;
    }
    else if( !(stricmp(cmd,"dump")) || !(stricmp(cmd,"ld")) || !(stricmp(cmd,"diff")) ||!(stricmp(cmd,"ldda")) ||!(stricmp(cmd,"lddb")) ||!(stricmp(cmd,"ldd")) )
    {
        pRegToolPara->isRegFile = TRUE;
    }
    else
    {
        printf("command error!\n");
        return 0;
    }
}

void ParseArgvsCommand(pReg_Tool_Para pRegToolPara)
{
    char* regName = pRegToolPara->regTypeName;
    REGISTER_TYPE  command;
    if(stricmp(regName, "sr") == 0)
    {
        command = SR;
    }
    else if(stricmp(regName, "srb") == 0)
    {
        command = SR_B;
    }
    else if(stricmp(regName, "cr") == 0)
    {
        command = CR;
    }
    else if(stricmp(regName, "crb") == 0)
    {
        command = CR_B;
    }
    else if(stricmp(regName, "mmio") == 0)
    {
        command = MMIO;
    }
    else if(stricmp(regName, "pmu") == 0)
    {
        command = PMU;
    }
    else if(stricmp(regName, "gpio") == 0)
    {
        command = GPIO;
    }
    else
    {
        command = INVALID;
    }
    
    pRegToolPara->regType = command;
}

void PreReadWrite(pVirtBase_Para virtBase_Para, pReg_Tool_Para pRegToolPara)
{
    UINT32 virtualBase;

    //get virtual base
    switch(pRegToolPara->regType)
    {
        case SR:
        case SR_B:
        case CR:
        case CR_B:
        case MMIO:
            virtBase_Para->virtualBase = virtBase_Para->mmio;
            break;
            
        case PMU:
            virtBase_Para->virtualBase = virtBase_Para->pmu;
            break; 
        case GPIO:
            virtBase_Para->virtualBase = virtBase_Para->gpio;
            break; 
             
        default:
            break;
    }

    //get register index
    switch(pRegToolPara->regType)
    {
        case SR:
            pRegToolPara->regIndex += SRBASE;
            break;
        case SR_B:
            pRegToolPara->regIndex += SR_BBASE;
            break;
        case CR:
            pRegToolPara->regIndex += CRBASE;
            break;
        case CR_B:
            pRegToolPara->regIndex += CR_BBASE;
            break;
            
        default:
            break;
    }

}

void ReadRegister(void *virtualBase, pReg_Tool_Para pRegToolPara)
{

    //read register
    switch(pRegToolPara->regType)
    {
        case SR:
        case SR_B:
        case CR:
        case CR_B:
            pRegToolPara->regValue = MmioUtil_Read8(virtualBase,pRegToolPara->regIndex);
            break;
            
        case PMU:
        case MMIO:
        case GPIO:
            pRegToolPara->regValue = MmioUtil_Read32(virtualBase,pRegToolPara->regIndex);
            break; 
             
        default:
            break;
    }

}

void WriteRegister(void *virtualBase, pReg_Tool_Para pRegToolPara)
{
    //write register
    switch(pRegToolPara->regType)
    {
        case SR:
        case SR_B:
        case CR:
        case CR_B:
            MmioUtil_Write8(virtualBase, pRegToolPara->regIndex,  (UINT8)pRegToolPara->regValue);
            break;
            
        case PMU:
        case MMIO:
        case GPIO:
            MmioUtil_Write32(virtualBase, pRegToolPara->regIndex,  pRegToolPara->regValue);
            break; 
             
        default:
            break;
    }
}

void PrintResult(pReg_Tool_Para pRegToolPara)
{
    if(pRegToolPara->isRead)
    {
        printf("\nread  ");
    }
    else
    {
        printf("\nwrite ");
    }
    
    switch(pRegToolPara->regType)
    {
        case SR:
        case SR_B:
        case CR:
        case CR_B:
            printf("%s %x = %x\n\n",pRegToolPara->regTypeName, (UINT8)pRegToolPara->regIndex, pRegToolPara->regValue);
            break;
            
        case PMU:
        case MMIO:
        case GPIO:
            printf("%s %x = %x\n\n",pRegToolPara->regTypeName, pRegToolPara->regIndex, pRegToolPara->regValue);
            break; 
             
        default:
            break;
    }
}

