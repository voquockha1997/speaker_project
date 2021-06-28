#ifndef APP_BRIDGE_CONFIG_H_
#define APP_BRIDGE_CONFIG_H_

#include "ComConfig.h"
#include "ComUtil.h"
#include "BridgeDefine.h"

class BridgeConfig: public COMCFG
{
public:
    static void Initialize();
};

#endif /* APP_BRIDGE_CONFIG_H_ */
