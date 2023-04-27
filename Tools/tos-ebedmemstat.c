/**
 * @file tos-ebedmemstat.c
* 【tos-ebedmemstat】：自制内存状态监测工具，运行时（总览 + 进程 + 内核），用量和分布
* 【默认：无参数】=【-S：统计模式，数据源自统计或汇总数据，典型如/proc/[meminfo,slabinfo]】
    * 【Overview】：Total/Used/Free/Shared/Buffer/Cached/Available（RefTool: free）
    * 【USpace】：
        * eachProcess（USS/PSS/RSS/VMZ=Program/Process          + Heap/MMAP/Stack + SharedObject/FileMMAP）
            * Kernel           = Task Stack + Socket Buffer + File Buffer/Cache
    * 【KSpace】：
        * Kernel = Static/Image +          Slab/<un>Reclaimable + Stack + PageTables
        * Module = Static/File +          kmalloc
* 【显式：传参数】
    * -P：哪个进程， 多个‘-P’表示哪几个进程
    * -T：跟踪模式（vs -S：统计模式），部分数据原本来自统计数据，切换为来自实时跟踪（Trace），典型如perf_events，以此提高监测精度
        * -TT/-TTT：实时跟踪数据的程度（比例），程度越深，精度越高，消耗越多、干扰增大
    * -Dn：细节程度（n=1/2/3）
    * -Cn：持续模式，间隔（n=1、2、…seconds）输出信息
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>

typedef struct 
{
    //Read from /proc/meminfo
    unsigned long MemTotal;
    unsigned long MemFree;
    unsigned long MemAvailable;
    unsigned long Buffers;
    unsigned long Cached;
    unsigned long Shmem;
} TOS_EMS_Overview_T, *TOS_EMS_Overview_pT;

typedef struct 
{
    pid_t PID;

    //read from /proc/[pid]/smaps
    unsigned long VMZ;//Current process's virtual memory size
    unsigned long RSS;//Current process's resident set size
    unsigned long PSS;//Current process's proportional set size
    unsigned long USS;//Current process's unique set size
    
    #if 0
    unsigned long Heap;//Current process's heap size
    unsigned long Anonymous;//Current process's anonymous size
    unsigned long Stack;//Current process's stack size

    unsigned long SharedObject;//Current process's shared object size
    unsigned long FileMMAP;//Current process's file mmap size
    
    unsigned long SocketBuffer;//Current process's socket buffer size
    unsigned long FileBuffer;//Current process's file buffer size
    #endif
} TOS_EMS_USpaceProcess_T, *TOS_EMS_USpaceProcess_pT;

//readUspaceProcess from /proc/[pid]/smaps and /proc/[pid]/maps,
//  then calculate VMZ/RSS/PSS/USS
void TOS_EMS_ReadUSpaceProcess(TOS_EMS_USpaceProcess_pT pProcess)
{
    
 
}

typedef struct 
{

} TOS_EMS_USpace_T, *TOS_EMS_USpace_pT;

typedef struct 
{

} TOS_EMS_KSpace_T, *TOS_EMS_KSpace_pT;

typedef enum 
{
    //UseStatData: /proc/meminfo, /proc/slabinfo
        // /proc/[pid]/statm, /proc/[pid]/smaps, /proc/[pid]/maps
    TOS_EMS_StatDataSource_Statistic,//Default

    //UseTraceData: perf_events...
    TOS_EMS_StatDataSource_Trace_1  = 1UL<<0,//RefArgs: -T/-TT/-TTT

    //TODO: TOS_EMS_StatDataSource_Trace_2  = 1UL<<1,
    //TODO: TOS_EMS_StatDataSource_Trace_3  = 1UL<<2,
} TOS_EMS_StatDataSource_T;

typedef enum 
{
    TOS_EMS_DetailLevel_0,//Default
    TOS_EMS_DetailLevel_1,//RefArgs: -D1
    TOS_EMS_DetailLevel_2,//RefArgs: -D2
    TOS_EMS_DetailLevel_3,//RefArgs: -D3
} TOS_EMS_DetailLevel_T;

typedef struct 
{
    TOS_EMS_StatDataSource_T StatDataSource;//RefArgs: -S/-T/-TT/-TTT
    TOS_EMS_DetailLevel_T DetailLevel;//RefArgs: -Dn

    unsigned long StatCount;//RefArgs: -Cn, default is 1

    TOS_EMS_Overview_T  Overview;

} TOS_EMS_ToolContext_T, *TOS_EMS_ToolContext_pT;

//parseArgs
static void TOS_EMS_ParseArgs(TOS_EMS_ToolContext_pT pContext, int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "SPTTD:C:")) != -1)
    {
        switch (opt)
        {
            case 'S':
                pContext->StatDataSource = TOS_EMS_StatDataSource_Statistic;
                break;
            case 'P':
                break;
            case 'T':
                pContext->StatDataSource = TOS_EMS_StatDataSource_Trace_1;
                break;
            case 'D':
                pContext->DetailLevel = atoi(optarg);
                break;
            case 'C':
                pContext->StatCount = atoi(optarg);
                break;
            default:
                break;
        }
    }
}

//readOverview
void TOS_EMS_ReadOverview(TOS_EMS_ToolContext_pT pContext)
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if( NULL == fp )
    {
        return;
    }

    char line[256];
    while( NULL != fgets(line, sizeof(line), fp) )
    {
        if( 0 == strncmp(line, "MemTotal:", 9) )
        {
            sscanf(line, "MemTotal: %lu kB", &pContext->Overview.MemTotal);
        }
        else if( 0 == strncmp(line, "MemFree:", 8) )
        {
            sscanf(line, "MemFree: %lu kB", &pContext->Overview.MemFree);
        }
        else if( 0 == strncmp(line, "MemAvailable:", 13) )
        {
            sscanf(line, "MemAvailable: %lu kB", &pContext->Overview.MemAvailable);
        }
        else if( 0 == strncmp(line, "Buffers:", 8) )
        {
            sscanf(line, "Buffers: %lu kB", &pContext->Overview.Buffers);
        }
        else if( 0 == strncmp(line, "Cached:", 7) )
        {
            sscanf(line, "Cached: %lu kB", &pContext->Overview.Cached);
        }
        else if( 0 == strncmp(line, "Shmem:", 6) )
        {
            sscanf(line, "Shmem: %lu kB", &pContext->Overview.Shmem);
        }
    }

    fclose(fp);
}

//printOverview
void TOS_EMS_PrintOverview(TOS_EMS_ToolContext_pT pContext)
{
    printf("MemTotal: %lu kB, MemFree: %lu kB, MemAvailable: %lu kB, Buffers: %lu kB, Cached: %lu kB, Shmem: %lu kB\n",
        pContext->Overview.MemTotal,
        pContext->Overview.MemFree,
        pContext->Overview.MemAvailable,
        pContext->Overview.Buffers,
        pContext->Overview.Cached,
        pContext->Overview.Shmem);
}

int main(int argc, char *argv[])
{
    TOS_EMS_ToolContext_T Context =
    {
        .StatDataSource = TOS_EMS_StatDataSource_Statistic,
        .DetailLevel = TOS_EMS_DetailLevel_0,
        .StatCount = 1,
    };

    TOS_EMS_ParseArgs(&Context, argc, argv);

    do 
    {
        //readOverview
        TOS_EMS_ReadOverview(&Context);

        //printOverview
        TOS_EMS_PrintOverview(&Context);

        //---------------------------------------------------------------------
        //readUSpace
        //printUSpace

        //readKSpace
        //printKSpace

        //---------------------------------------------------------------------
        Context.StatCount--;
        if( 0 == Context.StatCount )
        {
            break;
        }
    } while( 0x20230419 );


    return 0;
}
