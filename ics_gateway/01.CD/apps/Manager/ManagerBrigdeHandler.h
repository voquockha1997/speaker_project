#ifndef APP_MANAGER_BRIDGE_HANDLER_H_ 
#define APP_MANAGER_BRIDGE_HANDLER_H_

#include "ComProcess.h"
#include "ComManagerBridgeInterface.h"
#include "ComUtil.h"

class ManagerBridgeHandler: ComNoCopyAndInstantiable
{
public:
    static bool ProcessBridgeMsg(const void* sender, ComIPCMsg& packet);
};

#endif /* APP_MANAGER_BRIDGE_HANDLER_H_ */
