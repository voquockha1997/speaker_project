#include "ManagerConfig.h"
#include "ComConfig.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

ComSysNetwork      ManagerConfig::SysNetworkIf = *(ComSysNetwork::Inst());
ComNetHost         ManagerConfig::SysNetHosts;

bool ManagerConfig::Initialize()
{
    Entries = {
            new ManagerConfig_T(false),
            new CollectorConfig_T(false),
            new RegistrationInfo_T(false),
            new LogConfig_T(false),
            new NTPConfig(false),
            new MqttBrokerConfig_T(false),
            new NetConfig(false),
            new defaultRelayState_T(false),
    };

    return true;
}
