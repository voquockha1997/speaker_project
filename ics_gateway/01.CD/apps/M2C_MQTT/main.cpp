#include <iostream>
#include "EventBase.h"
#include "Utils.hpp"

#include "ComProcess.h"
#include "M2CMQTTDataSet.h"
#include <event2/event.h>

#include "M2CMQTT.h"
#include "M2CMQTTConfig.hpp"
#include "M2CMQTTDefine.h"
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
    //==================================================
    // Init logging
    MyLogger::init();
    COMStr logPath = DEFAULT_LOG_FILE_DIR + COMStr(NodeKeys::MQTT) + ".log";
    MyLogger::setFile(logPath.c_str());

    //==================================================
    // Loading MQTT configuration
    try
    {
        // Initialize process
        ComProcess::Initialize(NodeKeys::MQTT, AtExit);

        //start event base
        auto &eventBase = Net::EventBase::getInstance();

        MQTTConfig::Initialize();
        IPCClient client(ComProcess::Proto, MQTTConfig::Manager().Host, MQTTConfig::Manager().Port, M2C_MQTT::ProcessIPC);
        M2C_MQTT::Initialize();

        eventBase.doLoop();

        MyLogDebug("Exit MQTT");
    } catch (const std::exception &e) {
        MyLogErr("std exception: %s", e.what());
        exit(EXIT_SUCCESS + 2);
    }
    catch (...)
    {
        MyLogErr("Unknown exception");
        exit(EXIT_SUCCESS + 3);
    }

    return 0;
}

