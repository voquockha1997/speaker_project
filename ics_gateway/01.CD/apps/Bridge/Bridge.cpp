#include "Bridge.h"
#include "ComRedis.h"
#include "ComQueue.h"
#include "ComState.h"
#include "BridgeConfig.h"
#include "ComCmdRunner.h"
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

ComRedis Bridge::DataClient(DEFAULT_REDIS_HOST, DEFAULT_REDIS_PORT);
ComTimer Bridge::dptimer;

bool Bridge::Initialize()
{
    MyLogDebug("Bridge::Initialize()");
    DataClient.StartRListenEvt(new ComRedisCallback({REDIS_CH_MCAST_DATA, Bridge::ProcessCollectData, NULL}), 100000);
    StartTimer();

    return true;
}

bool Bridge::StartTimer()
{
    MyLogDebug("Bridge::StartTimer()");

    if (dptimer.State() == ComTimerState::TimerRunning) {
        return false;
    }

    dptimer.Callbacks.push_back({NULL, DataPointProcessor});
    dptimer.Start(BridgeConfig::Collector().DataReportInterval * 1000000);

    return true;
}

void Bridge::DataPointProcessor(void* obj)
{
    std::ostringstream msg;
    // // Lock scope context
    // Poco::Mutex::ScopedLock lock(m_dpLock);
    Poco::JSON::Object dataPoint(true);
    COMStr now = ComUtil::UtcNowString();
    dataPoint.set("time_stamp", ComUtil::UtcTimeStampString(now));
    dataPoint.set("name", BridgeConfig::RegInfo().Name);
    dataPoint.set("master_device_id", BridgeConfig::RegInfo().DevID);
    for (auto dev : BridgeConfig::Collector().Devices) {
        COMStr data = ComRedis::Local.Get(COMStr(REDIS_COLLECTOR_PREFIX) + dev.DeviceType);
        if (data.empty()) {
            // not set yet
            MyLogDebug("Data of %s not set",dev.DeviceType.c_str());
            data = "{\"good_tags\":[],\"error_tags\":[],\"low_alarm\":[],\"high_alarm\":[]}";
        } else {
            // clear 
            ComRedis::Local.Set(COMStr(REDIS_COLLECTOR_PREFIX) + dev.DeviceType, "");
        }
        Poco::JSON::Parser parse;
        Poco::Dynamic::Var result = parse.parse(data);
        Poco::JSON::Object::Ptr jData = result.extract<Poco::JSON::Object::Ptr>();
        dataPoint.set(dev.DeviceType,jData);
    }

    dataPoint.stringify(msg, 0);
    MyLogDebug("DataPoint: %s", msg.str().c_str());

    if (BridgeConfig::RegInfo().DevID.empty()) {
        MyLogWarn("DataPointProcessor device id is empty, ignore the message");
        return;
    }
    
    // go to server via mqtt
    ComRedis::Local.LPush(COMStr(REDIS_M2C_DATA_QUEUE) + NodeKeys::MQTT, msg.str());

    // live local web
    ComRedis::Local.Set(REDIS_LIVE_DATA_KEY, msg.str());
}

bool Bridge::ProcessCollectData(void* obj, const COMStr& msg)
{
    Poco::JSON::Object::Ptr jmsg;

    MyLogDebug("Bridge::ProcessCollectData()");

    try {
        Poco::Dynamic::Var jvar = Poco::JSON::Parser().parse(msg);
        jmsg = jvar.extract<Poco::JSON::Object::Ptr>();
    } catch (std::exception e) {
        MyLogErr("Bridge::ProcessCollectData(): %s Failed to parse json format %s", e.what(), msg.c_str());
        return true; // Pop message from queue
    } catch (...) {
        MyLogErr("Bridge::ProcessCollectData(): Failed to parse json format %s", msg.c_str());
        return true; // Pop message from queue
    }

    COMStr subId;  JGET(subId, jmsg, "sub_device_id", COMStr);

    DeviceConfig &d = BridgeConfig::Collector().FindBySubId(subId);
     if (!&d) {
        MyLogDebug("No need to collect data of device: %s", subId.c_str());
        return true; // Pop message from queue
    }
    // MyLogDebug("%s %s", d.DeviceType.c_str(), d.Id.c_str());

    Poco::JSON::Object dataPoint(true);

    Poco::JSON::Array::Ptr jGoods; JGET_ARR(jGoods, jmsg, "good_tags");
    if (jGoods.isNull()) {
        MyLogWarn("tags is null");
    }else {
        dataPoint.set("good_tags", jGoods);
    }

    Poco::JSON::Array::Ptr jErrs; JGET_ARR(jErrs, jmsg, "error_tags");
    if (jErrs.isNull()) {
        MyLogWarn("error_tags is null");
    }else {
        dataPoint.set("error_tags", jErrs);
    }

    Poco::JSON::Array::Ptr jLows; JGET_ARR(jLows, jmsg, "low_alarm");
    if (jLows.isNull()) {
        MyLogWarn("low_alarm is null");
    }else {
        dataPoint.set("low_alarm", jLows);
    }

    Poco::JSON::Array::Ptr jHighs; JGET_ARR(jHighs, jmsg, "high_alarm");
    if (jHighs.isNull()) {
        MyLogWarn("high_alarm is null");
    }else {
        dataPoint.set("high_alarm", jHighs);
    }

    // check exist or not
    std::ostringstream os; 
    dataPoint.stringify(os, 0);
    MyLogDebug("%s: %s",d.DeviceType.c_str(), os.str().c_str());

    // store
    ComRedis::Local.Set(COMStr(REDIS_COLLECTOR_PREFIX) + d.DeviceType, os.str());

    return true;
}