#pragma once

#include "ComDefinition.h"
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "ManagerProxy.h"

#include "ManagerBrigdeHandler.h"
#include "ManagerDownlinkHandler.h"

class ManagerConsumer
{
public:
    ManagerConsumer(ManagerProxy& proxy);
    
    ~ManagerConsumer();

    // Callback used to process MQTT packet
    void OnMqttMessage(const void* pSender, ComIPCMsg& packet);

    // Callback used to process Bridge packet
    void OnBridgeMessage(const void* pSender, ComIPCMsg& packet);


    // Register data from remote processes
    void RegisterData();  

private:
    // Manage connections from child processes
    ManagerProxy& m_ManagerProxy;
    
    // Timer used to send test messages
    Poco::Timer m_testTimer;
    void OnTestTimer(Poco::Timer& timer); 
};
