#pragma once

#include <queue>
#include <mutex>
#include "Client.h"
#include "../3rd/Utility/Singleton.hpp"
#include "ComDefinition.h"

#include "Poco/Thread.h"
#include "Poco/BasicEvent.h"
#include "Poco/ActiveMethod.h"

#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"

#include "ComManagerComInterface.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

class ComMGRClient : public Net::TCP::Client
{
    static ComMGRClient* inst;

public:
    static WatchDogSection WDI;

    static ComMGRClient& getInstance(const char *pName, const char *pHost, int port) {
        if (inst == NULL) {
            inst = new ComMGRClient(pName, pHost, port);
        }

        return (*inst);
    }

    static ComMGRClient& getInstance() {
        return (*inst);
    }

    static void Exit() {
        if (inst) {
            inst->m_quit.set();
            delete inst;
            inst = NULL;
        }
    }

    ComMGRClient(const char *pName, const char *pHost, int port);

    virtual ~ComMGRClient();
    bool decodeFrame();
    void onConnect();
    
    // Catch 'CLose" event from parent class
    void onClose();
    
    // Send message to the remote end-point. This function call be called from any thread
    void SendMessageToRemote(const COMStr& msgCmd, const COMStr& msgData, COMStr destination = NodeKeys::Manager);
    void SendNotification(const COMStr& info);
    void CLI_CPrint(CLIOType type, const char* fmt, ...);

    // Event signed when getting new data from Manager
    Poco::BasicEvent<ComIPCMsg> DataEvent;
    
private:
    bool sendData(const COMStr & data) noexcept;
    void onPacket(const uint32_t &sz, struct evbuffer* evbuf);
    
    COMStr SerializeMessageToJson(const ComIPCMsg& packet);      // Convert structure to JSON
    ComIPCMsg DeserializeJsonToMessage(const COMStr& jsonData);  // Convert JSON to structure
    
    void PushReceivedData(void);    // Scan RX queue and push data to consume class
    
private:
    // Timer used to scan TX queue and make sending operations was executed on the LibEvent's thread
    std::unique_ptr<struct event, void(*)(struct event*)> m_TxTimer;
    
    // The object used to execute a function by a thread in ThreadPool
    Poco::ActiveMethod<void, void, ComMGRClient> m_PushMethod;
    
    // Queue used to hold messages need to sent to Manager
    std::queue<ComIPCMsg> m_TxQueue;
    // Lock used to protect TX queue because TX queue can be accessed from different threads
    std::mutex m_TxLock;
    
    // Queue used to hold raw data from Manager
    std::queue<COMStr> m_RxQueue;

    // Lock used to protect RX queue because RX queue can be accessed from different threads
    std::mutex m_RxLock;
    
    COMStr m_name; // Name of this client
    COMStr m_host; // Host to connect to
    int m_port;         // Port to connect to
    
    bool m_Connected = false;   // The flag was set when this client connected to Manager
    
    // Signalled to exit all threads
    Poco::Event m_quit;
};

#define WDCLIENT ComMGRClient::WDI

#define IPCSEND ComMGRClient::getInstance().SendMessageToRemote

#define IPCNOTI ComMGRClient::getInstance().SendNotification
#define WDCLIENT_CHANGE(key, state, info) ({                                            \
    bool change = false, first = false;                                                 \
    if (ComMGRClient::WDI.count(key) == 0) {                                           \
        ComMGRClient::WDI[key] = WatchDogParam_T();                                    \
        ComMGRClient::WDI[key].name = key;                                             \
        first = true;                                                                   \
    }                                                                                   \
    change = ComMGRClient::WDI[key].Change(state, info);                               \
    if (change || first || (uint)(ComMGRClient::WDI[key].ChangeSecs()) % 30 == 0) {    \
        ComMGRClient::WDI.Info = info;                                                 \
        ComMGRClient::getInstance().SendNotification(info);                            \
    }                                                                                   \
    (change || first);                                                                  \
})

#define CLI_CI(...) ComMGRClient::getInstance().CLI_CPrint(CLIOType::Info,   __VA_ARGS__)
#define CLI_CO(...) ComMGRClient::getInstance().CLI_CPrint(CLIOType::OK,     __VA_ARGS__)
#define CLI_CF(...) ComMGRClient::getInstance().CLI_CPrint(CLIOType::Fail,   __VA_ARGS__)
#define CLI_CW(...) ComMGRClient::getInstance().CLI_CPrint(CLIOType::Warn,   __VA_ARGS__)
#define CLI_CE(...) ComMGRClient::getInstance().CLI_CPrint(CLIOType::Error,  __VA_ARGS__)
#define CLI_CG(...) ComMGRClient::getInstance().CLI_CPrint(CLIOType::Process, __VA_ARGS__)
#define CLI_CP(...) ComMGRClient::getInstance().CLI_CPrint(CLIOType::Print,  __VA_ARGS__)
