#include "EventBase.h"
#include "BridgeDefine.h"
#include "BridgeConfig.h"
#include "ComProcess.h"
#include "Bridge.h"
#include "ComIPCClient.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

bool AtExit(void* obj)
{
    MyLogger::stop();
    Net::EventBase::getInstance().StopSuccess();
    return true;
}

int main(int argc, char** argv)
{
    printf("Bridge is starting...\n");

    //==================================================
    // Init logging
    MyLogger::init();
    COMStr logPath = DEFAULT_LOG_FILE_DIR + COMStr(NodeKeys::Bridge) + ".log";
    MyLogger::setFile(logPath.c_str());

    ComProcess::Initialize(NodeKeys::Bridge, AtExit);
    BridgeConfig::Initialize();

    //start event base
    auto &eventBase = Net::EventBase::getInstance();
    IPCClient client(NodeKeys::Bridge, BridgeConfig::Manager().Host, BridgeConfig::Manager().Port, NULL);

    if (Bridge::Initialize()) {
        eventBase.doLoop();
    }

    MyLogDebug("Exit Bridge");
    return 0;
}
