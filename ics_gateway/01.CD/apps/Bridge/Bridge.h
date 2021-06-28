#ifndef APP_BRIDGE_H_
#define APP_BRIDGE_H_

#include "ComDefinition.h"
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"

#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

class ComRedis;

class Bridge : public ComNoCopyAndInstantiable
{
private:
    static bool ProcessDataQueue(void* obj, const COMStr& msg);
    static bool ProcessCollectData(void* obj, const COMStr& msg);
    static ComRedis DataClient;
    static ComTimer dptimer;
    static void DataPointProcessor(void* obj);
    static bool StartTimer();
public:
    static bool Initialize();
};

#endif /* APP_BRIDGE_H_ */
