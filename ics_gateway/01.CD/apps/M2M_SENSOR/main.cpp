#include <iostream>
#include <vector>
#include <unistd.h>
#include <sstream>

#include "ComIPCClient.h"
#include "ComProcess.h"
#include "M2MSensorConfig.h"
#include "ComUtil.h"
#include "M2MSensor.hpp"
#define COMMON_LOG_GROUP LOG_G_M2MINTERFACE

bool AtExit(void* obj)
{
    MyLogDebug("AtExit() stopping logger");
    MyLogger::stop();

    Net::EventBase::getInstance().StopSuccess();
    return true;
}

int main(int argc, char **argv)
{
    MyLogger::init();
    COMStr logPath = DEFAULT_LOG_FILE_DIR + COMStr(NodeKeys::Sensor) + ".log";
    MyLogger::setFile(logPath.c_str());
    ComProcess::Initialize(NodeKeys::Sensor, AtExit);
    M2MSensorConfig::Initialize();

    IPCClient client(ComProcess::Proto, M2MSensorConfig::Manager().Host, M2MSensorConfig::Manager().Port, NULL);
    auto &eventBase = Net::EventBase::getInstance();
    M2MSensor M2M;
    if (M2M.Initialize()) {
        eventBase.doLoop();
    }

    return 0;
}
