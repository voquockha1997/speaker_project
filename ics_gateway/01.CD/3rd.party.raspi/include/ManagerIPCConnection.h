#pragma once
#include <queue>
#include <mutex>
#include "Connection.h"
#include "ComDefinition.h"
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"
#include "ComUtil.h"

class ComIPCConnection : public Net::TCP::Connection
{
public:
    ComIPCConnection(int fd);
    virtual ~ComIPCConnection();
    
    void onClose();
    bool decodeFrame();
    
    // Send message to the remote end-point. This function call be called from any thread
    void SendMessageToPeer(const COMStr& msgCmd, const COMStr& msgData, COMStr source = NodeKeys::Manager);
    
private:
    void sendData(const COMStr& data);
    void onPacket(const uint32_t &sz, struct evbuffer* evbuf);
    
    COMStr SerializeMessageToJson(const ComIPCMsg& packet);      // Convert structure to JSON
    ComIPCMsg DeserializeJsonToMessage(const COMStr& jsonData);  // Convert JSON to structure
    
private:
    COMStr name_;
    
    // Timer used to scan TX queue and make sending operations was executed on the LibEvent's thread
    std::unique_ptr<struct event, void(*)(struct event*)> m_TxTimer;
    
    // Queue used to hold messages need to sent to Supervisor
    std::queue<ComIPCMsg> m_TxQueue;

    // Lock used to protect TX queue because TX queue can be accessed from different threads
    std::mutex m_TxLock;
        
public:
    const COMStr &getName() const
    {
        return name_;
    }
};
