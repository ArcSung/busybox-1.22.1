#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "common.h"

void Help()
{
    printf("\n  __________ Register Tool Usage ___________ \n");
    printf("/--------------------------------------------\\\n");  
    printf("|      sr    [value]      get/set sr.        |\n");
    printf("|      srb   [value]      get/set srb.       |\n");
    printf("|      cr    [value]      get/set cr.        |\n");
    printf("|      crb   [value]      get/set crb.       |\n");
    printf("|      pmu   [value]      get/set pmu.       |\n");
    printf("|      mmio  [value]      get/set mmio.      |\n");
    printf("|      gpio  [value]      get/set gpio.      |\n");
    printf("|      dump  fileName     dump regs to file. |\n");
    printf("|      ld    fileName     load dumped file.  |\n");
    printf("|      diff  f-A f-B      diff 2 dumped file.|\n");
    printf("|      ldd   a/b[l/r]     [ldda/lddb]        |\n");
    printf("|            load diff a/b or left/right.    |\n");
    printf("|      q                  quit.              |\n");
    printf("\\--------------------------------------------/\n\n"); 
    printf("Note: if backspace Not work well, please use Del.\n"); 
}

//int reg_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int reg_main(int argc, char **argv)
{
    int g_fd = -1;
    void* mmioVirtBase = 0;
    void* pmuVirtBase = 0;
    void* gpioVirtBase = 0;
    unsigned long mmioPhysicalAddress = 0xd80a0000;
    unsigned long pmuPhysicalAddress  = 0xd8390000;
    unsigned long gpioPhysicalAddress  = 0xd8110000;
    
    char str[128] = {0};
    char cmd[16];
    char fileNameA[128] = {'\0'};
    char fileNameB[128] = {'\0'};
    
    Reg_Tool_Para reg = {0};
    VirtBase_Para virtualBase = {0};

//#ifdef ISARM
    // add map code here
    if ( !virtualBase.mmio && !virtualBase.pmu)
    {
        g_fd = open("/dev/mem", O_RDWR, 0);
        if (g_fd < 0)
        {
            printf("error: can't open device: /dev/mem\n");
            return -1;
        }
        
        virtualBase.mmio = mmap(0, MMIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, mmioPhysicalAddress);
        virtualBase.pmu = mmap(0, PMU_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, pmuPhysicalAddress);
        virtualBase.gpio = mmap(0, GPIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, gpioPhysicalAddress);
        
        printf("MMIO info:  physical: 0x%lx    Virtual: %p\n", mmioPhysicalAddress, virtualBase.mmio);
        printf("PMU  info:  physical: 0x%lx    Virtual: %p\n", pmuPhysicalAddress, virtualBase.pmu);   
        printf("GPIO  info:  physical: 0x%lx    Virtual: %p\n", gpioPhysicalAddress, virtualBase.gpio);
	printf("virtualBase: %p\n", virtualBase.virtualBase); 	
        close(g_fd);
    }
//#endif

    if(argc < 2)
    {
        printf("Welcome:\n");
        Help(); 
        while(1)
        {
            printf("reg>");
            memset(str, 0, sizeof(str));
            memset(cmd, 0, sizeof(cmd));
            memset(fileNameA, '\0',sizeof(fileNameA));
            memset(fileNameB, '\0',sizeof(fileNameB));
            gets(str);
            sscanf(str, "%s ", cmd);
            if(!strcmp(cmd, "q"))
            {
                break;
            }
            else if(!strcmp(cmd, "h"))
            {
                Help();
            }
            else if(!strcmp(cmd, "dump"))
            {
                sscanf(str, "%s %s", cmd,fileNameA);
                DumpRegToFile((void *)fileNameA, &virtualBase);
            }
            else if(!strcmp(cmd, "diff"))
            {
                sscanf(str, "%s %s %s", cmd, fileNameA, fileNameB);
                Diff2DumpFiles(fileNameA, fileNameB);
            }
            else if(!strcmp(cmd, "ldda"))
            {
                LoadDiff("ldda", &virtualBase);
            }
            else if(!strcmp(cmd, "lddb"))
            {
                LoadDiff("lddb", &virtualBase);
            }
            else if(!strcmp(cmd, "ld"))
            {
                sscanf(str, "%s %s", cmd, fileNameA);
                LoadDumpedFile(fileNameA, &virtualBase);
            }
            else if(!strcmp(cmd, "ldd"))
            {
                sscanf(str, "%s %s", cmd, fileNameA);
                if((!strcmp(fileNameA, "a")) || (!strcmp(fileNameA, "l")) )
                {
                    LoadDiff("ldda", &virtualBase);
                }
                else if((!strcmp(fileNameA, "b")) || (!strcmp(fileNameA, "r")) )
                {
                    LoadDiff("lddb", &virtualBase);
                }
                else
                {
                    printf("ldd command error!\n");
                }
                
            }
            else if(!strcmp(cmd, ""))
            {
                continue;
            }
            else
            {
                if(ParseCommand(&str, &reg))
                {
                    PreReadWrite(&virtualBase, &reg);

                    if(reg.isRead)
                    {
                        ReadRegister((void *)virtualBase.virtualBase, &reg);
                        PrintResult(&reg);
                    }
                    else
                    {
                        WriteRegister((void *)virtualBase.virtualBase, &reg);
                        ReadRegister((void *)virtualBase.virtualBase, &reg);
                        PrintResult(&reg);
                    }
                }
                else
                {
                    Help();
                }
                
            }
        }
    }
    else
    {
        if(argc > 4)
        {
            printf("Invalid parameters!\n");
            Help();
            return 0;
        }
        sscanf(argv[1], "%s", cmd);
        PreParseArgvs(cmd, &reg);
        
        if(reg.isRegFile)
        {
            if(!strcmp(cmd, "dump"))
            {
                sscanf(argv[2], "%s",fileNameA);
                DumpRegToFile((void *)fileNameA, &virtualBase);
            }
            else if(!strcmp(cmd, "diff"))
            {
                sscanf(argv[2], "%s",fileNameA);
                sscanf(argv[3], "%s",fileNameB);
                Diff2DumpFiles(fileNameA, fileNameB);
            }
            else if(!strcmp(cmd, "ldda"))
            {
                LoadDiff("ldda", &virtualBase);
            }
            else if(!strcmp(cmd, "lddb"))
            {
                LoadDiff("lddb", &virtualBase);
            }
            else if(!strcmp(cmd, "ld"))
            {
                sscanf(argv[2], "%s",fileNameA);
                LoadDumpedFile(fileNameA, &virtualBase);
            }
            else if(!strcmp(cmd, "ldd"))
            {
                sscanf(argv[2], "%s",fileNameA);
                if((!strcmp(fileNameA, "a")) || (!strcmp(fileNameA, "l")) )
                {
                    LoadDiff("ldda", &virtualBase);
                }
                else if((!strcmp(fileNameA, "b")) || (!strcmp(fileNameA, "r")) )
                {
                    LoadDiff("lddb", &virtualBase);
                }
                else
                {
                    printf("ldd command error!\n");
                }
            }
            else
            {
                printf("Command error!\n");
            }
        }
        else
        {
            if(argc < 3)
            {
                return 0;
            }
            sscanf(argv[1], "%s", &(reg.regTypeName));
            sscanf(argv[2], "%x", &(reg.regIndex));
            ParseArgvsCommand(&reg);
            PreReadWrite(&virtualBase, &reg);
           
            if(3 == argc) //read
            {
                reg.isRead = TRUE;
                ReadRegister((void *)virtualBase.virtualBase, &reg);
                PrintResult(&reg);
            }
            else // write
            {
                reg.isRead = FALSE;
                sscanf(argv[3], "%x", &(reg.regValue));
                WriteRegister((void *)virtualBase.virtualBase, &reg);
                ReadRegister((void *)virtualBase.virtualBase, &reg);
                PrintResult(&reg);
            }
        }  

    }
    
    return 0;
}
