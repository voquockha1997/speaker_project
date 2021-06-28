#ifndef APP_MANAGER_CONFIG_H_
#define APP_MANAGER_CONFIG_H_

#include "ComDefinition.h"
#include "ComConfig.h"
#include "ComSysNetwork.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

class ManagerConfig : public COMCFG
{
public:
    static ComSysNetwork      SysNetworkIf;
    static ComNetHost         SysNetHosts;
    static bool Initialize();
};

#endif /* APP_MANAGER_CONFIG_H_ */
