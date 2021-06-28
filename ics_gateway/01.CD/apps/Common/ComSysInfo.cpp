#include "ComSysInfo.h"
#include <cmath>
#include <sstream>
#include <regex>
#include <fstream>
#include <sys/statvfs.h>

#include "ComCmdRunner.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMAND_INTERFACE

float    SystemInfo::UpTime;
float    SystemInfo::IdleTime;

float    SystemInfo::MemTotal;
float    SystemInfo::MemAvail;
float    SystemInfo::MemUsed;
float    SystemInfo::MemLoad;

float    SystemInfo::VMemTotal;
float    SystemInfo::VMemAvail;
float    SystemInfo::VMemUsed;
float    SystemInfo::VMemLoad;

float    SystemInfo::DiskTotal;
float    SystemInfo::DiskAvail;
float    SystemInfo::DiskUsed;
float    SystemInfo::DiskLoad;
uint     SystemInfo::CPUNum;
uint     SystemInfo::CPUCores;
COMStr   SystemInfo::CPUModel;

CPUEntry SystemInfo::CPU;
float    SystemInfo::CPULoad;

#define MEGABYTE (1024*1024)

/*
 * /proc/meminfo
 * MemFree: The amount of physical RAM, in kilobytes, left unused by the system.
 * MemAvailable: An estimate of how much memory is available for starting new applications, without swapping.
 */
bool SystemInfo::LoadMem()
{
    //MyLogDebug("SystemInfo::LoadMem()");
    std::ifstream fmem("/proc/meminfo");
    COMStr line, key, unit;
    uint val;

    while (std::getline(fmem, line)) {
        std::istringstream ss(line);
        ss >> key >> val >> unit;
        key.erase(key.size() - 1, 1);

        if (key == "MemTotal") {
            MemTotal = val/1000;
        } else if (key == "MemAvailable") {
            MemAvail = val/1000;
        }
    }

    MemUsed = MemTotal - MemAvail;
    MemLoad = (MemUsed * 100) / MemTotal;
    return true;
}

bool SystemInfo::LoadDisk()
{
    //MyLogDebug("SystemInfo::LoadDisk()");

    struct statvfs vfs;
    if (0 != statvfs("/data", &vfs)) {
        MyLogErr("Failed to get file system information");
        return false;
    }

    DiskTotal = (float)(vfs.f_blocks * vfs.f_frsize)/MEGABYTE;
    DiskAvail = (float)(vfs.f_bfree * vfs.f_frsize)/MEGABYTE;
    DiskUsed = DiskTotal - DiskAvail;
    DiskLoad = (DiskUsed *100) / DiskTotal;

    return true;
}

bool SystemInfo::LoadCPU()
{
    //MyLogDebug("SystemInfo::LoadCPU()");
    LoadCPUInfo();

    std::ifstream fileStat("/proc/stat");

    COMStr line;
    COMStr cpu;
    while (std::getline(fileStat, line)) {
        std::istringstream ss(line);
        ss >> cpu;

        if (cpu == "cpu") {
            for(int i = 0; i < NUM_CPU_STATES; ++i)
                ss >> CPU.times[i];

            CPU.ActiveTime = CPU.times[CPU_USER] +
                             CPU.times[CPU_NICE] +
                             CPU.times[CPU_SYS] +
                             CPU.times[CPU_IRQ] +
                             CPU.times[CPU_SIRQ] +
                             CPU.times[CPU_STEAL] +
                             CPU.times[CPU_GUEST] +
                             CPU.times[CPU_GNICE];

            CPU.IdleTime = CPU.times[CPU_IDLE] + CPU.times[CPU_IOW];
            CPU.TotalTime = CPU.ActiveTime + CPU.IdleTime;
            CPULoad = (float)(CPU.ActiveTime * 100)/CPU.TotalTime;
            return true;
        }
    }

    return false;
}

bool SystemInfo::LoadCPUInfo()
{
    if (!CPUModel.empty() && CPUCores > 0 && CPUNum > 0) {
        return false;
    }

    COMStr cmd = "/usr/bin/lscpu";
    COMStr txt = CommandRunner::ExecRead(cmd, false);
    std::istringstream f(txt);
    COMStr ln;

    while (getline(f, ln)) {
        COMStrVect vec = ComUtil::split(ln, ':');
        if (vec.size() == 2) {
            if (vec[0].find("Model name") != COMStr::npos) {
                CPUModel = ComUtil::trim(vec[1]);
            } else if (vec[0].find("per socket") != COMStr::npos) {
                CPUCores = ComUtil::StrTo<uint>(ComUtil::trim(vec[1]));
            } else if (vec[0] == "CPU(s)") {
                CPUNum = ComUtil::StrTo<uint>(ComUtil::trim(vec[1]));
            }
        }
    }

    return true;
}

bool SystemInfo::Load()
{
    //MyLogDebug("SystemInfo::Load()");
    std::ifstream ftime("/proc/uptime");
    COMStr line;
    if (std::getline(ftime, line)) {
        std::istringstream ss(line);
        ss >> UpTime >> IdleTime;
    }
    ftime.close();

    bool ret = LoadMem();
    ret &= LoadDisk();
    ret &= LoadCPU();
    return ret;
}


//g++ -DENABLE_SYSTEM_INFO_TEST --std=c++11 -DComUtil_DISABLE_POCO -o sysinfo ./CommandRunner.cpp ./ComUtil.cpp ./SystemInfo.cpp -I"../3rd/include" -I"../Core/Net" -L"../3rd/lib" -levent -lpthread
#ifdef ENABLE_SYSTEM_INFO_TEST
int main(int argc, char** argv)
{
    MyLogger::init();
    MyLogger::setMask(65535);
    MyLogger::setPrio(255);
    MyLogger::setOut(3);

    SystemInfo::Load();

    MyLogDebug("Mem : %6.2f %6.2f %6.2f %2.2f", SystemInfo::MemTotal, SystemInfo::MemAvail, SystemInfo::MemUsed, SystemInfo::MemLoad);
    //MyLogDebug("VMem: %6u %6u %6u %2.2f", SystemInfo::VMemTotal, SystemInfo::VMemAvail, SystemInfo::VMemUsed, SystemInfo::VMemLoad);
    MyLogDebug("Disk: %6.2f %6.2f %6.2f %2.2f", SystemInfo::DiskTotal, SystemInfo::DiskAvail, SystemInfo::DiskUsed, SystemInfo::DiskLoad);
    MyLogDebug("CPU : %6u %6u %6u %2.2f", SystemInfo::CPU.TotalTime, SystemInfo::CPU.IdleTime, SystemInfo::CPU.ActiveTime, SystemInfo::CPULoad);
    return 0;
}
#endif
