#ifndef APP_MANAGER_DOWNLINK_HANDLER_H_
#define APP_MANAGER_DOWNLINK_HANDLER_H_
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Query.h"
#include "Poco/Dynamic/Var.h"

#include "ComManagerDownlinkInterface.h"
#include "ComProcess.h"
#include "ComUtil.h"

class ManagerDownlinkHandler : ComNoCopyAndInstantiable
{
public:

    // Handle CLOUD messages
    static bool ProcessMGRDownlinkMsg(const void* sender, ComIPCMsg& packet);

    // Handle list cloud request
    static bool ProcessCloudReqest(const COMStr& command, Poco::JSON::Object::Ptr pInput, Poco::JSON::Array &aOutput);

    //=====================================
private:
    static bool CloudGetVersion(Poco::JSON::Object::Ptr pInput, Poco::JSON::Array &aOutput);
    static bool CloudSetAirCState(Poco::JSON::Object::Ptr pInput, Poco::JSON::Array &aOutput);
    
    static bool CloudGetAlarmConfig(Poco::JSON::Object::Ptr pInput, Poco::JSON::Array &aOutput);
    static bool CloudSetAlarmConfig(Poco::JSON::Object::Ptr pInput, Poco::JSON::Array &aOutput);
};

#endif /* APP_MANAGER_DOWNLINK_HANDLER_H_ */
