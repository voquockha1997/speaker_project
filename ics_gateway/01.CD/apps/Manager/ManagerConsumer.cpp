#include "ManagerConsumer.h"

ManagerConsumer::ManagerConsumer(ManagerProxy& proxy) : m_ManagerProxy(proxy)
{
    // Setup 1s timer for sending test data
    m_testTimer.setPeriodicInterval(1000);
    m_testTimer.setStartInterval(1000);
    
    // Start the timer
    Poco::TimerCallback<ManagerConsumer> callback(*this, &ManagerConsumer::OnTestTimer);
    m_testTimer.start(callback);
}
    
ManagerConsumer::~ManagerConsumer()
{
    m_testTimer.stop();
}


// Callback used to process MQTT packet
void ManagerConsumer::OnMqttMessage(const void* pSender, ComIPCMsg& packet)
{
    ManagerDownlinkHandler::ProcessMGRDownlinkMsg(pSender, packet);
}

// Callback used to process Bridge packet
void ManagerConsumer::OnBridgeMessage(const void* pSender, ComIPCMsg& packet)
{
    ManagerBridgeHandler::ProcessBridgeMsg(this, packet);
}


// Register data from remote processes
void ManagerConsumer::RegisterData()
{
    m_ManagerProxy.MQTTDataEvent += Poco::Delegate<ManagerConsumer, ComIPCMsg>(this, &ManagerConsumer::OnMqttMessage);
    m_ManagerProxy.BridgeDataEvent += Poco::Delegate<ManagerConsumer, ComIPCMsg>(this, &ManagerConsumer::OnBridgeMessage);

}

void ManagerConsumer::OnTestTimer(Poco::Timer& timer)
{
    std::vector<std::string> des;
    des.push_back(NodeKeys::MQTT);
}