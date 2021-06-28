#pragma once

#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "Utility/Singleton.hpp"
#include "EventBase.h"
#include "Logger.hpp"

#include "ManagerIPCConnection.h"
#include "ManagerProxy.h"

#include <event2/listener.h>
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

class ManagerServer : public Singleton<ManagerServer>
{
public:
    ManagerServer(int port);

    virtual ~ManagerServer();
        
private:
    std::unique_ptr<evconnlistener, void(*)(evconnlistener*)> listener_ = {nullptr, evconnlistener_free};
};
