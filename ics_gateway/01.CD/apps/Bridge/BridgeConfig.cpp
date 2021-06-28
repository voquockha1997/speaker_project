#include "BridgeConfig.h"
#include "ComConfig.h"
#include "ComQueue.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

void BridgeConfig::Initialize()
{
    Entries = {
            new RegistrationInfo_T(),
            new ManagerConfig_T(),
            new CollectorConfig_T(),
    };
}
