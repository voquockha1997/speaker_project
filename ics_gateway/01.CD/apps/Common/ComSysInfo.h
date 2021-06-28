#ifndef COMMON_SYSTEM_INFO_H_
#define COMMON_SYSTEM_INFO_H_

#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMAND_INTERFACE

#define NUM_CPU_STATES  10
#define CPU_USER        0
#define CPU_NICE        1
#define CPU_SYS         2
#define CPU_IDLE        3
#define CPU_IOW         4
#define CPU_IRQ         5
#define CPU_SIRQ        6
#define CPU_STEAL       7
#define CPU_GUEST       8
#define CPU_GNICE       9

struct CPUEntry
{
    uint   times[NUM_CPU_STATES];
    uint   TotalTime;
    uint   ActiveTime;
    uint   IdleTime;
};

class SystemInfo : public ComNoCopyAndInstantiable
{
public:
    static float    UpTime;
    static float    IdleTime;

    static float    MemTotal;
    static float    MemAvail;
    static float    MemUsed;
    static float    MemLoad;

    static float    VMemTotal;
    static float    VMemAvail;
    static float    VMemUsed;
    static float    VMemLoad;

    static float    DiskTotal;
    static float    DiskAvail;
    static float    DiskUsed;
    static float    DiskLoad;

    static CPUEntry CPU;
    static float    CPULoad;
    static uint     CPUNum;
    static uint     CPUCores;
    static COMStr   CPUModel;

    static bool LoadMem();
    static bool LoadDisk();
    static bool LoadCPU();
    static bool LoadCPUInfo();
    static bool Load();
};

#endif /* COMMON_SYSTEM_INFO_H_ */
