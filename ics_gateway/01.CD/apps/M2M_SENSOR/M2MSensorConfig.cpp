#include "M2MSensorConfig.h"
#include "ComQueue.h"
#include "ComState.h"
#include "ComRedis.h"
#include "ComConfig.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_M2MINTERFACE_CONFIG

bool M2MSensorConfig::Initialize()
{
    Entries = {
		new ManagerConfig_T(),
    	new SensorGroup_T(),
    	new CollectorConfig_T(),
    	new defaultRelayState_T(),
    	new RegistrationInfo_T(),
    };

    return true;
}
