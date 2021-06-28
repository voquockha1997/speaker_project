#ifndef COMMON_COMMAND_RUNNER_H_
#define COMMON_COMMAND_RUNNER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <vector>
#include <wait.h>

#include "ComUtil.h"

#define COM_OUTPUT_BUFF_SZ  5000
#define MAX_ABOVE_BLOCKS    10
#define MAX_BELOW_BLOCKS    10

// Callback type to execute matched output
typedef void (*OutputFilterCallback)(const COMStr& match);
struct ComOutputFilter
{
    COMStr             key;
    COMStr             match;
    OutputFilterCallback    func;
};

// Define list of output filters type
typedef std::vector<ComOutputFilter> ComOutputFilters;

// Runner's status enum
enum CommandRunnerState {
    Init, Running, Suspended, Stopped, Failed
};

// Runner type enum
enum CommandRunnerType {
    Exec, FileMonitor
};

// Internal PIPE type to handle process IO
typedef struct {
  FILE  *read_fd;
  FILE  *write_fd;
  pid_t child_pid;
} COM_PIPE;

// CommandRunner class
// Provides utilities to execute external application
// And handle specific process output or file contents
class CommandRunner
{
private:
    pthread_t           thread; // Runner thread
    pthread_mutex_t     mutex;  // Protect runner status and output content
    COMStr              cmd;    // Process command or file path to monitor
    bool                show;   // Show content to console or running silently
    CommandRunnerState  state;  // Runner private state
    CommandRunnerType   type;   // Runner's type

    // State setter
    void State(CommandRunnerState st) {
        pthread_mutex_lock(&mutex);
        state = st;
        pthread_mutex_unlock(&mutex);
    }

    static void SetCmdEnv();

public:
    ComOutputFilters Filters;  // List of output filter
    static COMStrMap CmdEnv;

    // State getter
    CommandRunnerState State() {
        return state;
    }

    // Constructor
    CommandRunner(const COMStr& cmd, bool show = false, CommandRunnerType type = CommandRunnerType::Exec);

    // Destructor
    virtual ~CommandRunner();

    // Thread handling functions
    void Start();
    void Stop();
    void Restart();

    // Environment path handling functions
    static void SetSecureEnv();
    static bool IsSecureEnv();

    // External application's executing utilities
    static COMStr ExecRead(COMStrVect &cmds, bool show = false);
    static COMStr ExecRead(COMStr &cmd, bool show = true);
    static COMStr ExecRead(const char* argv[], bool show = true);

    static int Exec(COMStrVect &cmds, bool show = false);
    static int Exec(COMStr &cmd, bool show = false, uint timeout = 0, bool wait = true, int* pid_ptr = NULL);
    static int Exec(const char* args[], bool show = false, uint timeout = 0, bool wait = true, int* pid_ptr = NULL);

private:
    // Process running listening function
    static void* Run(void* runner);
    // File content listening function
    static void* FileMonitor(void* runner);
    // Execute a command and return r/w IO handler
    static COM_PIPE* POpen(const char* argv[]);
    // Stop command and close r/w IO
    static int PClose(COM_PIPE* p);

    //---------------------
    // List of runners
    std::vector<CommandRunner> Runners;
};

#endif /* COMMON_COMMAND_RUNNER_H_ */
