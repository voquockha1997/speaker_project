#include "ComDefinition.h"
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"

#include "ComRedis.h"
#include "ComQueue.h"
#include "ComState.h"
#include "ComProcessInfo.h"
#include "ComCmdRunner.h"
#include "ComManagerClient.h"
#include "ComProcess.h"
#include "ComConfig.h"
#include "ComManagerComInterface.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

RegistrationInfo_T  COM_Queue::RegInfo;
MqttBrokerConfig_T  COM_Queue::MqttBroker;


bool COM_Queue::PushDataBus(const COMStr& deviceLog)
{
    return ComRedis::Local.LPush(REDIS_CH_MCAST_DATA, deviceLog).empty();
}

bool COM_Queue::PushCtrlBus(const COMStr& deviceLog)
{
    return ComRedis::Local.LPush(REDIS_CH_MCAST_CTRL, deviceLog).empty();
}

bool COM_Queue::PushEvent(const COMStr& devID, const COMStr& event, const COMStr& level, const COMStr& msg, COMStrMap params)
{
    if (devID.empty()) {
        MyLogWarn("COM_Queue::PushEvent() device id is empty, ignore the message");
        return true;
    }
    // to do push event to mqtt queue
    try {
        Poco::JSON::Object objEvent(true);
        Poco::JSON::Object objNotify(true);
        Poco::JSON::Object objData(true);

        objNotify.set("master_device_id", devID);
        objNotify.set("code", event);
        objNotify.set("level", level);
        objNotify.set("message", msg);
        objNotify.set("time_stamp", ComUtil::UtcNowString());
        if (params.size() > 0) {
            for (auto p : params) {
                objNotify.set(p.first, p.second);
            }
        }
        objNotify.set("data", objData);
        objEvent.set("event_log", objNotify);

        std::ostringstream os;
        objEvent.stringify(os, 0);
        COMStr ostr = os.str();

        //MyLogDebug("COM_Queue::PushEvent() %s", ostr.c_str());
        return ComRedis::Local.LPush(MQTT_EVENT_QUEUE, ostr).empty();
    } catch (...) {
        MyLogErr("COM_Queue::PushEvent() Exception: %s-%s-%s-%s", devID.c_str(),
                event.c_str(), level.c_str(), msg.c_str());
    }

    return true;

}

bool COM_Queue::PushAudit(const COMStr& audit, const COMStr& level, const COMStr& msg, COMStrMap params)
{
    return COM_Queue::PushAudit(RegInfo.DevID, audit, level, msg, params);
}

bool COM_Queue::PushEvent(const COMStr& event, const COMStr& level, const COMStr& msg, COMStrMap params)
{
    return COM_Queue::PushEvent(RegInfo.DevID, event, level, msg, params);
}

bool COM_Queue::PushAudit(const COMStr& devID, const COMStr& audit, const COMStr& level, const COMStr& msg, COMStrMap params)
{
    if (devID.empty()) {
        MyLogWarn("COM_Queue::PushAudit() device id is empty, ignore the message");
        return true;
    }

    try {
        Poco::JSON::Object objNotify(true);
        Poco::JSON::Object objDataSystem(true);
        Poco::JSON::Object objDataExtension(true);
        Poco::JSON::Array  arrApps;
        std::ostringstream os;

        objNotify.set("master_device_id", RegInfo.DevID);

        objNotify.set("code", audit);
        objNotify.set("level", level);

        objNotify.set("message", msg);
        COMStr now = ComUtil::UtcNowString();
        objNotify.set("time_stamp", ComUtil::UtcTimeStampString(now));

        if (params.size() > 0) {
            for (auto p : params) {
                objNotify.set(p.first, p.second);
            }
        }

        objDataSystem.set("ssh", ComProcessInfo::Ssh);
        objDataSystem.set("user", ComProcessInfo::User);
        objDataSystem.set("user_type", ComProcessInfo::UserType);
        objDataSystem.set("privilege", ComProcessInfo::Privilege);
        objDataSystem.set("last_cmd", ComProcessInfo::LastCmd);

        objNotify.set("data_system", objDataSystem);

        objDataExtension.set("outdoor_temp", ComProcessInfo::Ssh);
        objDataExtension.set("longitude", ComProcessInfo::User);
        objDataExtension.set("latitude", ComProcessInfo::UserType);
        objDataExtension.set("status_speaker", ComProcessInfo::Privilege);
        objDataExtension.set("3g_wavelength", ComProcessInfo::LastCmd);
        objDataExtension.set("data_in_the_sim", ComProcessInfo::LastCmd);

        objNotify.set("data_extension", objDataExtension);

        Poco::JSON::Object objEvent(true);

        objEvent.set("audit_log", objNotify);
        objEvent.stringify(os, 0);

        if (RegInfo.Protocol == PROTOCOL_COMMUNICATE_CLOUD_MQTT) {
            return ComRedis::Local.LPush(MQTT_AUDIT_QUEUE, os.str()).empty();
        } else {
            MyLogErr("COM_Queue::PushAudit() Protocol not support %s", RegInfo.Protocol.c_str());
        }

    } catch (...) {
        MyLogErr("COM_Queue::PushAudit() Exception: %s-%s-%s-%s", devID.c_str(),
                audit.c_str(), level.c_str(), msg.c_str());
    }

    return false;
}

