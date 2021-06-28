#include "ManagerWatchDog.h"
#include "ManagerDefine.h"
#include "ComProcess.h"
#include "ManagerProxy.h"
#include "ManagerConfig.h"
#include "ComState.h"
#include "ComQueue.h"
#include "ComSysInfo.h"
#include "ComSysNetwork.h"
#include "ComCmdRunner.h"
#include "ComCloudInterface.h"

#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_GENERAL

ComTimer       ManagerWatchDog::wdtimer;
CommandRunner*  ManagerWatchDog::dockerEventRunner = NULL;

WatchDogSection ManagerWatchDog::MANAGER(NodeKeys::Manager);
WatchDogSection ManagerWatchDog::M2CMQTT(NodeKeys::MQTT);


bool ManagerWatchDog::Start()
{
    MyLogDebug("ManagerWatchDog::Start()");

    if (wdtimer.State() == ComTimerState::TimerRunning) {
        return false;
    }

    wdtimer.Callbacks.push_back({NULL, ManagerWatchDog::watchdog});
    wdtimer.Start(ManagerConfig::Manager().WatchdogInterval * 1000000);

    return true;
}

void ManagerWatchDog::watchdog(void* obj)
{
    MyLogDebug("ManagerWatchDog::watchdog()");

    static uint beat_cnt = 0;
    if (beat_cnt == 1000) beat_cnt = 0;

    beat_cnt++;

    // Observe and set device's global status
    // Note: make sure the state change don't conflict with update SW/OS procedure
    COMStr globalState = GET_GSTATE();
    bool state_need_change = false;

    COMStr info;
    while (!(info = ComRedis::Local.LPop("SystemInfor")).empty()) {
        MyLogWarn("%s", info.c_str());
    }

    if (globalState == DevGlobalState::InstallSuccess) {
        MyLogWarn("DEV INSTALL SUCCESS");
        COM_AUDIT(M2CAudit::DevState, M2CAuditLevel::Info, "DEV INSTALL SUCCESS");
        state_need_change = true;
    } else if (globalState == DevGlobalState::InstallFail) {
        MyLogWarn("DEV INSTALL FAILED");
        COM_AUDIT(M2CAudit::DevState, M2CAuditLevel::Critical, "DEV INSTALL FAILED");
        state_need_change = true;
    }

    COMStr cmd;
    // Wireless modem status
    //bool cnn = (ComSysNetwork::TestConnection("8.8.8.8") != 0);
    //network.Change(cnn);

    bool cnn;
    if (ManagerConfig::MQTT().Host.empty()) {
        WD_MANAGER_CHANGE("MQTT_REACHABLE", false, "Host is is not defined");
    } else {
        cnn = ComSysNetwork::TestTcpConnection(ManagerConfig::MQTT().Host, ManagerConfig::MQTT().Port);
        WD_MANAGER_CHANGE("MQTT_REACHABLE", cnn, cnn ? "Primary MQTT cloud is reachable" : "Primary MQTT is unreachable");
    }

    // Send System information
    static time_t sysreport_time = time(0);
    time_t sysreport_time_now = time(0);
    uint diff_sysreport_time = difftime(sysreport_time_now, sysreport_time);

    if (diff_sysreport_time >= ManagerConfig::Manager().SysReportInterval) {
        sysreport_time = sysreport_time_now;

        SystemInfo::Load();
        COM_AUDIT(M2CAudit::SysInfo, M2CAuditLevel::Info, "System Information", COMStrMap({
            {"AppStorageSizeUsed", std::to_string(SystemInfo::DiskUsed)},
            {"AppStorageSizeAvailable", std::to_string(SystemInfo::DiskAvail)},
            {"NumOfCPU", std::to_string(SystemInfo::CPUNum)},
            {"CPUCoresPerSocket", std::to_string(SystemInfo::CPUCores)},
            {"CPUModel", SystemInfo::CPUModel},
            {"CPUUsed", std::to_string(SystemInfo::CPULoad)},
            {"CPUAvailable", std::to_string(100 - SystemInfo::CPULoad)},
            {"MemoryUsed", std::to_string(SystemInfo::MemUsed)},
            {"MemoryAvailable", std::to_string(SystemInfo::MemAvail)},
        }));

    }
}

bool ManagerWatchDog::MQTTReachable()
{
    if (ManagerConfig::MQTT().Host.empty()) {
        return false;
    }

    return ComSysNetwork::TestTcpConnection(ManagerConfig::MQTT().Host, ManagerConfig::MQTT().Port);
}
