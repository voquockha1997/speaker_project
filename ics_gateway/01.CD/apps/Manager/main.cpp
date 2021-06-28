#include "EventBase.h"
#include "ComProcess.h"
#include "Manager.h"
#include "ComUtil.h"
#include "ManagerDefine.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

bool AtExit(void* obj)
{
    MyLogger::stop();
    Net::EventBase::getInstance().StopSuccess();
    return true;
}

int main(int argc, char** argv)
{
    MyLogger::init();
    COMStr logPath = DEFAULT_LOG_FILE_DIR + COMStr(NodeKeys::Manager) + ".log";
    MyLogger::setFile(logPath.c_str());

    ComProcess::Initialize(NodeKeys::Manager, AtExit);

    auto &eventBase = Net::EventBase::getInstance();
    if (Manager::Initialize()) {
        eventBase.doLoop();
    } else {
        MyLogDebug("Failed to start manager");
        return 1;
    }

    return 0;
}

