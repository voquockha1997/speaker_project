#include "M2CMQTTConfig.hpp"

#include "ComRedis.h"
#include "ComUtil.h"

#define COMMON_LOG_GROUP LOG_G_COMMON

bool MQTTConfig::Initialize()
{
    Entries = {
            new ManagerConfig_T(),
            new MqttBrokerConfig_T(),
            new RegistrationInfo_T()
    };

    return true;
}

