#ifndef APP_Manager_H_
#define APP_Manager_H_

#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

class ComRedis;

class Manager : ComNoCopyAndInstantiable
{
public:
    static ComRedis CtrlClient;
    static bool Initialize();
    static bool FactoryReset(void* obj);
};

#endif /* APP_Manager_H_ */
