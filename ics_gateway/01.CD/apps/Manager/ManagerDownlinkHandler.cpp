#include "ManagerDownlinkHandler.h"

#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Query.h"
#include "Poco/Dynamic/Var.h"
#include "ComCloudInterface.h"
#include "ManagerProxy.h"
#include "ManagerConfig.h"
#include "ComQueue.h"
#include "ComState.h"
#include "ComCmdRunner.h"
#include "ComSysInfo.h"
#include "ManagerWatchDog.h"
#include "ComManagerModbusInterface.h"
#include "ComUtil.h"

#include <wiringPi.h>

#define COMMON_LOG_GROUP LOG_G_GENERAL

/*
# get version
pIn
{
    "uuid": "b990ea58-98e2-11eb-a8b3-0242ac130003", 
    "command": "get_version", 
    "master_device_id": "d378e0b5-d2f5-3864-9dec-32a135687fca",
    "create_at": "2021-04-08T02:54:55.125+0000", 
    "data" : [{}]
}

aOut: [{"version":"01.01.01"}]

# get_led_status
{
    "uuid": "b990ea58-98e2-11eb-a8b3-0242ac130003", 
    "command": "get_led_status", 
    "master_device_id": "d378e0b5-d2f5-3864-9dec-32a135687fca",
    "create_at": "2021-04-08T02:54:55.125+0000", 
    "data" : [{}]
}

aOut: 
    {
      "index" : 1,
      "state": 0
    },
    {
      "index" : 2,
      "state": 0
    },

# set_air_conditional_status
{
    "uuid":"b990ea58-98e2-11eb-a8b3-0242ac130003",
    "command":"set_air_conditional_status",
    "master_device_id":"cf086f63-7b94-3969-9a61-ef39fe8df480",
    "create_at":"2021-04-08T02:54:55.125+0000",
    "data":[
    {
        "index":1, //device index
        "state":0 // off
    }
    ]
}

*/

bool ManagerDownlinkHandler::ProcessMGRDownlinkMsg(const void* sender, ComIPCMsg& packet)
{
    MyLogDebug("ManagerDownlinkHandler::ProcessMGRDownlinkMsg() %s", packet.Content.c_str());

    COMStr ret = MGRItfStatus::OK;
    COMStr info;
    bool   isSuccess = false;

    try {
        Poco::JSON::Parser parse;
        Poco::Dynamic::Var result = parse.parse(packet.Content);
        Poco::JSON::Object::Ptr pIn = result.extract<Poco::JSON::Object::Ptr>();
        COMStr type = packet.Type;
        if (type == MGRDownlinkItf::CloudReq) {
            COMStr uuid, command, broadcastType;
            Poco::JSON::Array aOut;
            
            JGET(uuid, pIn, "ban_tin_id", COMStr);
            JGET(command, pIn, "che_do_phat", COMStr);
            JGET(broadcastType, pIn, "kieu_phat", COMStr);

            // main process
            isSuccess = ProcessCloudReqest(command, pIn, aOut);

            Poco::JSON::Object response(true);
            response.set("ban_tin_id", uuid);
            response.set("cum_loa_id", ManagerConfig::RegInfo().DevID);
            response.set("che_do_phat", command);
            response.set("thoi_gian_xu_ly", ComUtil::UtcNowString());
            response.set("ket_qua", isSuccess);
            response.set("thong_bao", isSuccess ? "thanh_cong" : "that_bai");
            response.set("du_lieu", aOut);
            std::ostringstream os;
            response.stringify(os, 0);
            COMStr strrsp = os.str();

            if (ManagerConfig::RegInfo().Protocol == PROTOCOL_COMMUNICATE_CLOUD_MQTT) {
                ManagerIPCSend(MGRDownlinkItf::CloudResp, strrsp, {NodeKeys::MQTT});
            } else
            {
                MyLogErr("ManagerDownlinkHandler::ProcessMGRDownlinkMsg CloudReq Protocol not support %s", ManagerConfig::RegInfo().Protocol.c_str());
            }
            return isSuccess;
        } else if (type == MGRDownlinkItf::StatusInd) {
            if (ManagerConfig::RegInfo().Protocol == PROTOCOL_COMMUNICATE_CLOUD_MQTT) {
                ManagerWatchDog::M2CMQTT.FromJson(packet.Content);
                ManagerIPCSend(MGRItf::StatusInd, packet.Content, {NodeKeys::CLI});
            } else {
                MyLogErr("ManagerDownlinkHandler::ProcessMGRDownlinkMsg StatusInd Protocol not support %s", ManagerConfig::RegInfo().Protocol.c_str());
            }
        }else {
            ret = MGRItfStatus::FORMAT_ERROR;
            info = "Unknown message type " + type;
        }
    } catch (std::exception ex) {
        MyLogErr("ManagerDownlinkHandler::ProcessMGRDownlinkMsg() parse json exception %s", ex.what());
        ret = MGRItfStatus::FORMAT_ERROR;
        info = "parse json exception";
    } catch (...) {
        MyLogErr("ManagerDownlinkHandler::ProcessMGRDownlinkMsg() parse json unknown exception");
        ret = MGRItfStatus::FORMAT_ERROR;
        info = "parse json unknown exception";
    }

ProcessMGRDownlinkMsg_Exit:
    // TODO: Send response here
    
    return (ret == MGRItfStatus::OK);
}

// Handle Cloud request
bool ManagerDownlinkHandler::ProcessCloudReqest(const COMStr& command, Poco::JSON::Object::Ptr pInput, Poco::JSON::Array &aOutput)
{
    COMStrVect cmds = ComUtil::split(command, ';');
    COMStr cmd = cmds[0];

    // get pDataInput
    // Poco::JSON::Array::Ptr pDataIn; JGET_ARR(pDataIn, pInput, "data");
    // std::ostringstream os;
    // pDataIn->stringify(os, 0);
    // MyLogDebug("Data Input %s", os.str().c_str());

    // Handling command
    if (cmd == M2CCommand::GetVersion) {
        return CloudGetVersion(pInput, aOutput);
    } 
    else if (cmd == M2CCommand::SetAirCStatus)
    {
        return CloudSetAirCState(pInput, aOutput);
    } 
    else if (cmd == M2CCommand::GetAlarmConfig)
    {
        return CloudGetAlarmConfig(pInput, aOutput);
    } 
    // else if (cmd == M2CCommand::SetAlarmConfig)
    // {
    //     return CloudSetAlarmConfig(pInput, aOutput);
    // }

    return false;

}

bool ManagerDownlinkHandler::CloudGetVersion(Poco::JSON::Object::Ptr pInput, Poco::JSON::Array &aOutput)
{
    MyLogDebug("ManagerDownlinkHandler::CloudGetVersion()");

    Poco::JSON::Object data(true);
    data.set("version", ManagerConfig::Manager().SoftVer);
    aOutput.add(data);
    // get array output   
    std::ostringstream os2;
    aOutput.stringify(os2, 0);
    MyLogDebug("Array Ouput %s", os2.str().c_str());

    return true;
}

bool ManagerDownlinkHandler::CloudSetAirCState(Poco::JSON::Object::Ptr pInput, Poco::JSON::Array &aOutput)
{
    wiringPiSetupGpio();
    pinMode(ManagerConfig::DFRelayState().Relay1, OUTPUT);
    pinMode(ManagerConfig::DFRelayState().Relay2, OUTPUT);

    MyLogDebug("ManagerDownlinkHandler::CloudSetAirCState()");
    Poco::JSON::Object data(true);
    int index = -1, state = -1;
    Poco::JSON::Array::Ptr pDataIn; JGET_ARR(pDataIn, pInput, "data");
    if (pDataIn->size() == 0) {
        MyLogDebug("Data Input is null");
    }

    for (uint j = 0; j < pDataIn->size(); j++) {
        Poco::JSON::Object::Ptr jdata = pDataIn->getObject(j);

        /*not data*/
        if (jdata->size() == 0) {
            MyLogWarn("data index %d is null", j);
            return false;
        }

        JGET(state, jdata, "state", int);
        JGET(index, jdata, "index", int);
    }

    /*state = 0 or state =1 if state != 0 && 1 => false*/
    if (state != 1 && state != 0)
    {
        // MyLogWarn("Wrong format set command");
        MyLogWarn("Could not set state : %d", state);
        return false;
    } 
    if (index <= 0 || index >2){
        // MyLogWarn("Wrong format set command");
        MyLogWarn("Not found index [%d] in device", index);
        return false;
    }

    for (auto &r : ManagerConfig::DFRelayState().DFRelay) {
        if (index == r.Index){ //check device index
            if (state == r.State){ // if state current = state cmd from cloud => not exec
                MyLogWarn("State current of device = state command");
                data.set("info", "State cmd equal state current");
            }else{
                MyLogDebug("Exec command"); //Exec cmmd
                r.State = state;
                digitalWrite(r.Gpio, state);//Set relay = state
                ManagerConfig::DFRelayState().Save();
                ComProcess::Respawn(NodeKeys::Sensor,true); //restart SENSOR
            }
        }    
    }

    // data.set("state", ManagerConfig::DFRelayState().State);
    aOutput.add(data);
    // get array output   
    std::ostringstream os2;
    aOutput.stringify(os2, 0);
    MyLogDebug("Array Ouput %s", os2.str().c_str());

    return true;
}

bool ManagerDownlinkHandler::CloudGetAlarmConfig(Poco::JSON::Object::Ptr pInput, Poco::JSON::Array &aOutput)
{
    MyLogDebug("ManagerDownlinkHandler::CloudGetAlarmConfig()");
    Poco::JSON::Parser parse;
    COMStr alarmConfig = ComRedis::Local.Get("COLLECTOR_CONFIG");
    Poco::Dynamic::Var result = parse.parse(alarmConfig);
    Poco::JSON::Object::Ptr jData = result.extract<Poco::JSON::Object::Ptr>();

    aOutput.add(jData);
    // get array output   
    std::ostringstream os2;
    aOutput.stringify(os2, 0);
    MyLogDebug("Array Ouput %s", os2.str().c_str());

    return true;
}

/*bool ManagerDownlinkHandler::CloudSetAlarmConfig(Poco::JSON::Object::Ptr pInput, Poco::JSON::Array &aOutput)
{
    MyLogDebug("ManagerDownlinkHandler::CloudSetAlarmConfig()");
    Poco::JSON::Object data(true);
    int alarmConfig = -1;
    Poco::JSON::Array::Ptr pDataIn; JGET_ARR(pDataIn, pInput, "data");
    std::ostringstream os; 
    
    if (pDataIn->size() == 0) {
        MyLogDebug("Data Input is null");
    }

    for (uint j = 0; j < pDataIn->size(); j++) {
        Poco::JSON::Object::Ptr jdata = pDataIn->getObject(j);

        //not data
        if (jdata->size() == 0) {
            MyLogWarn("data index %d is null", j);
            return false;
        }
        
        jdata->stringify(os, 0);
        ComRedis::Local.Set("COLLECTOR_CONFIG", os.str());

    }

    

    // data.set("state", ManagerConfig::DFRelayState().State);
    aOutput.add(data);
    // get array output   
    std::ostringstream os2;
    aOutput.stringify(os2, 0);
    MyLogDebug("Array Ouput %s", os2.str().c_str());

    return true;
}*/