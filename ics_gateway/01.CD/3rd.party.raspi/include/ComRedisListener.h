#pragma once
#include "Client.h"

#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON


class ComRedisListener : public Net::TCP::Client
{
private:
    ComMessageCallback callback;

public:
    ComRedisListener(const COMStr& host, uint port, ComMessageCallback cb);

    virtual ~ComRedisListener();
    bool decodeFrame();
    void onConnect();
        
private:
    bool encodeParam(const char* param);

public:
    void getLogData();
};

