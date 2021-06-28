#include "ComDefinition.h"
#include "ComRedis.h"
#include "ComQueue.h"
#include "M2CMQTTDataSet.h"
#include "ComCmdRunner.h"
#include "M2CMQTT.h"
#include "ComQueue.h"
#include "ComManagerClient.h"
#include "ComCloudInterface.h"
#include "ComProcess.h"
#include "ComManagerMQTTInterface.h"
#include "ComUtil.h"

#include "M2CMQTTConfig.hpp"
#include "M2CMQTTDownlink.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

pthread_t M2C_MQTT::thread_init;

void* M2C_MQTT::initialize(void* obj)
{
    uint try_cnt = 0;
    bool all_cnn;

    while (true) {
        M2CMQTTDataSet::RefreshCfg();
        all_cnn = true;

        /*****
         * Trying to establish public channel connection
         */
        if (M2CMQTTDataSet::PUB()->State() == DataSetInit && M2CMQTTDataSet::PUB()->IsFirstConnect) {
            if (!MQTTConfig::MQTT().Host.empty() && !MQTTConfig::RegInfo().DevID.empty()) {
                if (!M2CMQTTDataSet::PUB()->Connect(MQTTConfig::RegInfo().DevID + "_PUB")) {
                    WDCLIENT_CHANGE("MQTT_PUB", false, "MQTT public channel connection failed");
                    all_cnn = false;
                } else {
                    WDCLIENT_CHANGE("MQTT_PUB", true, "MQTT public channel is connected");
                    
                    COM_AUDIT(M2CAudit::DevState, M2CAuditLevel::Info, "MQTT primary cloud is connected");

                    M2CMQTTDataSet::PUB()->StartConsume(M2CDownlink::ProcessDownlinkConsume, M2C_MQTT::ProcessDev2Cloud_CTRL);

                    // forward data
                    ComRedis::Local.StartRListenEvt(new ComRedisCallback({REDIS_M2C_DATA_QUEUE + ComProcess::Proto,
                                                                            M2C_MQTT::ProcessDev2Cloud_DATA,
                                                                            (void*)M2CMQTTDataSet::PUB()}), 100000);

                    // forward event
                    ComRedis::Local.StartRListenEvt(new ComRedisCallback({MQTT_EVENT_QUEUE, M2C_MQTT::ProcessEventQueue, NULL}), 200000);
 
                    // forward audit
                    ComRedis::Local.StartRListenEvt(new ComRedisCallback({MQTT_AUDIT_QUEUE, M2C_MQTT::ProcessAuditQueue, NULL}), 200000);

                }
            } else {
                WDCLIENT_CHANGE("MQTT_PUB", false, "MQTT public channel host is not defined");
                all_cnn = false;
            }
        }

        if (all_cnn) {
            break;
        }

        usleep(3000000);
    }

    return NULL;
}

/*
 * @brief: Initialize MQTT handlers and establish connections to target servers
 */
bool M2C_MQTT::Initialize()
{
    if (pthread_create(&thread_init, NULL, M2C_MQTT::initialize, NULL)) {
        return false;
    }

    return true;
}

/*
 * @brief: Redis data queue message callback function
 * @input:
 *      obj: Callback Object pointer
 *      msg: message content (json)
 *
 * @return:
 *      false: handling failed
 */
bool M2C_MQTT::ProcessDev2Cloud_DATA(void* obj, const COMStr& msg)
{
    //MyLogDebug("M2C_MQTT::ProcessDev2Cloud_DATA() %s %p", msg.c_str(), obj);
    M2CMQTTDataSet* ds = (M2CMQTTDataSet*) obj;
    if (ds != NULL) {
        return ds->Publish(msg);
    } else {
        MyLogWarn("M2C_MQTT::ProcessDev2Cloud_DATA() unknown target");
    }

    return false;
}

/*
 * @brief: Redis control queue message callback function
 * @input:
 *      obj: Callback Object pointer
 *      msg: message content (json)
 *
 * @return:
 *      false: handling failed
 */
bool M2C_MQTT::ProcessDev2Cloud_CTRL(void* obj, const COMStr& msg)
{
    if (!MQTTConfig::MQTT().IsReady()) {
        return false;
    }

    return M2CMQTTDataSet::PUB()->Publish(msg, MQTTConfig::MQTT().TopicCtrl);
}

// push mqtt event queue
bool M2C_MQTT::ProcessEventQueue(void* obj, const COMStr& msg)
{
    if (!MQTTConfig::MQTT().IsReady()) {
        return false;
    }

    //MyLogDebug("M2C_MQTT::ProcessAuditQueue() %s", msg.c_str());
    return M2CMQTTDataSet::PUB()->Publish(msg, "events");
}

bool M2C_MQTT::ProcessAuditQueue(void* obj, const COMStr& msg)
{
    if (!MQTTConfig::MQTT().IsReady()) {
        return false;
    }

    //MyLogDebug("M2C_MQTT::ProcessAuditQueue() %s", msg.c_str());
    return M2CMQTTDataSet::PUB()->Publish(msg, "audit_log");
}

//============================================================================

/*
 * @brief: Handle internal messages
 */
void M2C_MQTT::ProcessIPC(const void* sender, ComIPCMsg& packet)
{
    MyLogDebug("M2C_MQTT::ProcessIPC %s", packet.Type.c_str());

    try {
        Poco::JSON::Parser parse;
        Poco::Dynamic::Var result = parse.parse(packet.Content);
        Poco::JSON::Object::Ptr pdat = result.extract<Poco::JSON::Object::Ptr>();
        COMStr type = packet.Type;

        if (type == MGRMQTTItf::CloudResp) {
            MyLogDebug("M2C_MQTT::ProcessIPC() %s %s", type.c_str(), packet.Content.c_str());

            COMStr ret; JGET(ret, pdat, "Result", COMStr);
            COMStr info; JGET(info, pdat, "Info", COMStr);
            std::ostringstream os;
            pdat->stringify(os, 0);
            COMStr strrsp = os.str();

            M2CMQTTDataSet::PUB()->Publish(strrsp, MQTTConfig::MQTT().TopicCtrl);
        } else {
            MyLogWarn("CLICommand::ProcessIPC() Unknown command %s", type.c_str());
        }
    } catch (std::exception ex) {
        MyLogErr("ProcessSPVCLI() parse json exception %s", ex.what());
    } catch (...) {
        MyLogErr("ProcessSPVCLI() parse json unknown exception");
    }
}
