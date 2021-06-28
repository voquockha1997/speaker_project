#ifndef COMMON_PROCESS_H_
#define COMMON_PROCESS_H_

#include "ComDefinition.h"
#include "pthread.h"
#include "ComUtil.h"

#define MAX_RESPAWN_TIMES 500
// Process's status
enum class ProcessStatus {
    Initial,
    Running,
    Terminated,
    Starting
};

typedef bool (*ComProcessCallback)(void* obj);

// Process's information
struct ProcessInfo {
    COMStr  Command;
    COMStr  Proto;
    uint    PID;
    bool    IsRespawn;
    bool    IsOnly1Process = false;
    ComProcessCallback RespawnCallback;
    ProcessStatus  Status = ProcessStatus::Initial;
    uint    MaxRespawnTimes = MAX_RESPAWN_TIMES;
    uint    RespawnTimes = 0;
    uint    RespawnDelay = 30; // 30 seconds
};

typedef std::vector<ProcessInfo> ProcessInfos;

// class to store current process information and manage child processes
class ComProcess : ComNoCopyAndInstantiable
{
private:
    static bool             Initialized;
    static bool             Exiting;
    static pthread_mutex_t  ChildsMutex;
    static ComTimer        Observer;

    static ComProcessCallback ExitCallback;

    static bool Stop(ProcessInfo& child, bool force = false);
    static bool Stop(const COMStr& proto = NodeKeys::All, bool force = true);
    static bool Respawn(ProcessInfo& child);
    static void HandleSigchild(int signum);
    static void HandleExit(int signum);
    static void HandleCore(int signum);
    static void AtExit();

    static void UpdateChildStatus();
    static void MonitorChild(void* ps);

public:
#if APP_INDEX == APP_MANAGER_INDEX
    static bool StopAll();
    static bool RestartAll();
#endif

    static void UpdateChildStatus(ProcessInfo& child);
    static ProcessInfos Childs;
    static COMStr       Proto;
    static uint         PID;

    /**
     * @brief: Initialize current process
     */
    static void Initialize(const COMStr proto = "", ComProcessCallback exitCallback = NULL);

    /**
     * @brief: Spawn child process
     */
    static bool Spawn(const COMStr& cmd, const COMStr& proto, uint maxRespawnTimes, uint respawnDelay, bool respawn = true, ComProcessCallback respawnCallback = NULL);

    /**
     * @brief: Respawn protocol
     */
    static bool Respawn(const COMStr& proto = NodeKeys::All, bool force = false);

    static bool Respawn(uint maxRespawnTimes, uint respawnDelay, const COMStr& proto = NodeKeys::All, bool force = false);

    static void ShowChilds();
};

#endif /* COMMON_PROCESS_H_ */
