#ifndef APP_MANAGER_WATCHDOG_H
#define APP_MANAGER_WATCHDOG_H

#include "ComUtil.h"
#include "ComCmdRunner.h"

class ManagerWatchDog : ComNoCopyAndInstantiable
{
    static ComTimer wdtimer;
    static void watchdog(void* obj);
    static bool WLModemIsOn();
    static bool DLCReachable();
    static bool MQTTReachable();
    static bool EnableCellularNetwork();
    static CommandRunner* dockerEventRunner;

public:
    static WatchDogSection MANAGER;
    static WatchDogSection M2MModbus;
    static WatchDogSection M2MSNMP;
    static WatchDogSection M2CMQTT;

    static bool Start();
    static void CheckLicense(bool changed = true);
    static void DockerEvent(const COMStr& content);
    static void AutoLinkNetwork();
};

#define WD_MANAGER_CHANGE(key, state, info) {                      \
    if (ManagerWatchDog::MANAGER.count(key) == 0) {                   \
        ManagerWatchDog::MANAGER[key] = WatchDogParam_T();            \
        ManagerWatchDog::MANAGER[key].name = key;                     \
    }                                                         \
    if (ManagerWatchDog::MANAGER[key].Change(state, info)) {          \
    }                                                         \
}

#endif /* APP_MANAGER_WATCHDOG_H */
