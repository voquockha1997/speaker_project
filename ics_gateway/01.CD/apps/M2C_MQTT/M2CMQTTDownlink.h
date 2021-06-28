#ifndef MQTT_DOWNLINK_H_
#define MQTT_DOWNLINK_H_

#include "ComUtil.h"
#include "ComConfig.h"

class M2CDownlink: public ComNoCopyAndInstantiable {
private:
    static void ComReboot(const COMStr& data);
    static void ComRestart(const COMStr& data);
    static void ComSysInfo(const COMStr& data);
    static void ComSysCmd(const COMStr& data);

public:
    static bool ProcessPubDownlinkConsume(void *obj, const COMStr& msg, ComResCallbackFunc res);
    static bool ProcessDownlinkConsume(void *obj, const COMStr& msg, ComResCallbackFunc res);
    static bool ProcessDownlinkCommonConsume(void *obj, const COMStr& msg, ComResCallbackFunc res);

};

#endif /* MQTT_DOWNLINK_H_ */
