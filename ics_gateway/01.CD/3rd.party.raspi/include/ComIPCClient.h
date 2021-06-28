#ifndef COMON_IPC_CLIENT_H
#define COMON_IPC_CLIENT_H

#include "ComDefinition.h"
#include "ComManagerComInterface.h"
#include "ComManagerClient.h"
#include "ComUtil.h"

typedef void (*IPCClientCallback)(const void* pSender, ComIPCMsg& packet);

class IPCClient
{
public:

    IPCClient(const COMStr& proto, const COMStr& host, uint port, IPCClientCallback callback);
    virtual ~IPCClient();

    void OnMessage(const void* pSender, ComIPCMsg& packet);

private:
    // The communication object makes connection to Manager
    ComMGRClient& m_ClientInstance;
    IPCClientCallback m_Callback;
};

#endif /* COMON_IPC_CLIENT_H */
