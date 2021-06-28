#ifndef APP_M2C_MQTT_H_
#define APP_M2C_MQTT_H_

#include <pthread.h>
#include "ComRedis.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

class M2C_MQTT : ComNoCopyAndInstantiable
{
    static pthread_t thread_init;
    static void* initialize(void* obj);

public:
    static bool Initialize();

    static void ProcessIPC(const void* sender, ComIPCMsg& packet);

    static bool ProcessDev2Cloud_DATA(void* obj, const COMStr& msg);
    static bool ProcessDev2Cloud_CTRL(void* obj, const COMStr& msg);
    static bool ProcessEventQueue(void* obj, const COMStr& msg);
    static bool ProcessAuditQueue(void* obj, const COMStr& msg);
};

#endif /* APP_M2C_MQTT_H_ */
