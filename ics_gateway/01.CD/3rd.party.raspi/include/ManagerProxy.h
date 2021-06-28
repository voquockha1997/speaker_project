#pragma once

#include <map>
#include <list>
#include <vector>
#include <utility>
#include "../3rd/Utility/Singleton.hpp"
#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "Poco/BasicEvent.h"
#include "Poco/ActiveMethod.h"
#include "ManagerIPCConnection.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON
#define MANAGER_QUEUE_SLEEP_TIME 300

class ManagerProxy : public Singleton<ManagerProxy>
{
public:
    ManagerProxy();
    virtual ~ManagerProxy();

    // Event signalled when getting new data from MODBUS process
    Poco::BasicEvent<ComIPCMsg> MODBUSDataEvent;
    // Event signalled when getting new data from SERIAL process
    Poco::BasicEvent<ComIPCMsg> SERIALDataEvent;
    // Event signalled when getting new data from MQTT process
    Poco::BasicEvent<ComIPCMsg> MQTTDataEvent;
    // Event signalled when getting new data from SNMP process
    Poco::BasicEvent<ComIPCMsg> SNMPDataEvent;
    // Event signalled when getting new data from Bridge process
    Poco::BasicEvent<ComIPCMsg> BridgeDataEvent;
    
    // Add new connection to the collection
    void AddConnection(ComIPCConnection* connection);
    
    // Remove connection from the collection
    void RemoveConnection(ComIPCConnection* connection);
    
    // Get connection in the collection based on its name
    ComIPCConnection* GetConnection(const COMStr& name);
    
    // Send message to the remote end-points. This function call be called from any thread
    void SendMessageToPeers(const COMStr& msgCmd, const COMStr& msgData, const COMStrVect& destinations, COMStr source = NodeKeys::Manager);
    
    // Push packet to queue
    void PushToQueue(ComIPCMsg& packet);
    
private:
    // Scan RX queue and push data to consume class for each destination
    void PushReceivedData(void);
    // Process MODBUS data
    void ProcessModbusData(void);
    // Process MODBUS data
    void ProcessSerialData(void);
    // Process MQTT data
    void ProcessMqttData(void);
    // Process SNMP data
    void ProcessSnmpData(void);
    // Process Bridge data
    void ProcessBridgeData(void);

    // Signalled to exit all threads
    Poco::Event m_quit;
    
    // The collection stores connections from remote processes
    std::vector<ComIPCConnection*> m_connections;
   
    // Queue used to hold raw data from child process
    std::queue<ComIPCMsg> m_RxQueue;

    // Lock used to protect RX queue because RX queue can be accessed from different threads
    std::mutex m_RxLock;
    
    // The object used to execute a function by a thread in ThreadPool. 
    // This function distributes messages to specified queue for each destination
    Poco::ActiveMethod<void, void, ManagerProxy> m_ScanRXQueueMethod;
    
    // Queue used to hold MODBUS messages
    std::queue<ComIPCMsg> m_ModbusQueue;
    std::mutex m_ModbusLock;
    // The object used to execute a function by a thread in ThreadPool. This function executes master MODBUS data
    Poco::ActiveMethod<void, void, ManagerProxy> m_ModbusMethod;

    // Queue used to hold SERIAL messages
    std::queue<ComIPCMsg> m_SerialQueue;
    std::mutex m_SerialLock;
    // The object used to execute a function by a thread in ThreadPool. This function executes master SERIAL data
    Poco::ActiveMethod<void, void, ManagerProxy> m_SerialMethod;

    // Queue used to hold MQTT messages
    std::queue<ComIPCMsg> m_MqttQueue;
    std::mutex m_MqttLock;
    // The object used to execute a function by a thread in ThreadPool. This function executes MQTT data
    Poco::ActiveMethod<void, void, ManagerProxy> m_MqttMethod;

    // Queue used to hold SNMP messages
    std::queue<ComIPCMsg> m_SnmpQueue;
    std::mutex m_SnmpLock;
    // The object used to execute a function by a thread in ThreadPool. This function executes SNMP data
    Poco::ActiveMethod<void, void, ManagerProxy> m_SnmpMethod;

    // Queue used to hold Bridge messages
    std::queue<ComIPCMsg> m_BridgeQueue;
    std::mutex m_BridgeLock;
    // The object used to execute a function by a thread in ThreadPool. This function executes Bridge data
    Poco::ActiveMethod<void, void, ManagerProxy> m_BridgeMethod;
};

#define     ManagerIPCSend  ManagerProxy::getInstance().SendMessageToPeers

