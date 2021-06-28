#ifndef COMMON_QUEUE_H_
#define COMMON_QUEUE_H_

#if APP_INDEX == APP_MODBUS_INDEX | APP_INDEX == APP_SNMP_INDEX | APP_INDEX == APP_SERIAL_INDEX
#include "ThresholdRule.h"
#endif /*M2M*/

#include "ComConfig.h"
#include "ComCloudInterface.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON


#define MQTT_EVENT_QUEUE         "mqtt_events"

#define MQTT_AUDIT_QUEUE         "mqtt_audits"

class COM_Queue : public ComNoCopyAndInstantiable
{
private:
    static bool PushEvent(const COMStr& devID, const COMStr& event, const COMStr& level, const COMStr& msg, COMStrMap params = {});

    static bool PushAudit(const COMStr& devID, const COMStr& audit, const COMStr& level, const COMStr& msg, COMStrMap params = {});

public:
    static RegistrationInfo_T  RegInfo;
    static MqttBrokerConfig_T  MqttBroker;


    static bool PushDataBus(const COMStr& deviceLog);
    static bool PushCtrlBus(const COMStr& deviceLog);

    static bool PushAudit(const COMStr& audit, const COMStr& level, const COMStr& msg, COMStrMap params = {});

    static bool PushEvent(const COMStr& event, const COMStr& level, const COMStr& msg, COMStrMap params = {});

};

#define COM_AUDIT(...)      COM_Queue::PushAudit(__VA_ARGS__)
#define COM_EVENT(...)      COM_Queue::PushEvent(__VA_ARGS__)

#endif /* COMMON_QUEUE_H_ */
