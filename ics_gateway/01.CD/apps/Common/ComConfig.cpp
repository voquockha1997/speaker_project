#include <ComConfig.h>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ComDefinition.h"
#include "ComCmdRunner.h"
#include "ComRedis.h"
#include "ComSysNetwork.h"
#include <algorithm>

#include "ComUtil.h"
#include "BaseConfig.h"

#define COMMON_LOG_GROUP LOG_G_CONFIG

using namespace Poco;

// Manager Configuration
void ManagerConfig_T::Clear()
{
    Host = "127.0.0.1";
    Port = 6789;
    SoftVer = PRODUCT_VERSION;
    WatchdogInterval = 60;
    SysReportInterval = 60;
    Children.clear();
}

bool ManagerConfig_T::Load(bool draft)
{
    MyLogDebug("ManagerConfig_T::Load(%s)", draft ? "true" : "false");
    Poco::JSON::Object::Ptr jc = JConf;
    if (jc.isNull()) {
        MyLogDebug("ManagerConfig_T::Load() null");
        return false;
    }

    Clear();
    JGET(Host, jc, "Host", COMStr);
    JGET(SoftVer, jc, "SoftVer", COMStr);
    JGET(Port, jc, "Port", uint);
    JGET(WatchdogInterval, jc, "WatchdogInterval", uint);
    JGET(SysReportInterval, jc, "SysReportInterval", uint);

    JSON::Array::Ptr jch; JGET_ARR(jch, jc, "Children");
    if (jch.isNull()) {
        MyLogWarn("ManagerConfig_T::Load() Children is null");
        return false;
    }

    ProcessConfig pc;
    for (uint i = 0; i < jch->size(); i++) {
        JSON::Object::Ptr pobj = jch->getObject(i);
        JGET(pc.Proto, pobj, "Protocol", COMStr);
        JGET(pc.Command, pobj, "Command", COMStr);
        JGET(pc.Priority, pobj, "Priority", int);
        JGET(pc.MaxSpawnTimes, pobj, "MaxSpawnTimes", uint);
        JGET(pc.IsRespawn, pobj, "IsRespawn", bool);
        JGET(pc.RespawnDelay, pobj, "RespawnDelay", uint);
        Children.push_back(pc);
    }
   
}

bool ManagerConfig_T::JSave()
{
    MyLogDebug("ManagerConfig_T::JSave()");

    if (JConf.isNull()) {
        MyLogDebug("ManagerConfig_T::JSave() Null Pointer");
        return false;
    }

    JConf->set("Host", Host);
    JConf->set("SoftVer", SoftVer);
    JConf->set("Port", Port);
    JConf->set("WatchdogInterval", WatchdogInterval);
    JConf->set("SysReportInterval", SysReportInterval);

    IsDraft = false;
    JSON::Array::Ptr jch; JGET_ARR(jch, JConf, "Children");
    if (!jch.isNull()) {
        if (jch->size() > 0) {
            jch->clear();
        }

        for (auto c : Children) {
            JSON::Object obj(true);
            obj.set("Protocol", c.Proto);
            obj.set("Command", c.Command);
            obj.set("MaxSpawnTimes", c.MaxSpawnTimes);
            obj.set("IsRespawn", c.IsRespawn);
            obj.set("Priority", c.Priority);
            obj.set("RespawnDelay", c.RespawnDelay);
            jch->add(obj);
        }
    }

    return true;
}

////////////////////////////////
// Collector Configuration
void CollectorConfig_T::Clear()
{
    DataReportInterval = 60;
    for (auto &d : Devices) {
        d.Alarms.clear();
    }

    Devices.clear();
}

bool CollectorConfig_T::Load(bool draft)
{
    MyLogDebug("CollectorConfig_T::Load(%s)", draft ? "true" : "false");
    Poco::JSON::Object::Ptr jc = JConf;
    if (jc.isNull()) {
        MyLogDebug("CollectorConfig_T::Load() null");
        return false;
    }

    Clear();
    
    JGET(DataReportInterval, jc, "data_report_interval", uint);

    JSON::Array::Ptr jdev; JGET_ARR(jdev, jc, "devices");
    if (jdev.isNull()) {
        MyLogWarn("CollectorConfig_T::Load() Devices is null");
        return false;
    }

    DeviceConfig dev;
    for (uint i = 0; i < jdev->size(); i++) {
        JSON::Object::Ptr pdev = jdev->getObject(i);
        JGET(dev.DeviceType, pdev, "device_type", COMStr);
        JGET(dev.SubID, pdev, "sub_id", COMStr);
        JGET(dev.Protocol, pdev, "protocol", COMStr);
        JGET(dev.Interval, pdev, "interval", uint);

        Poco::JSON::Array::Ptr alarms; JGET_ARR(alarms, pdev, "alarms");
        if (alarms.isNull()) {
            continue;
        }
        dev.Alarms.clear();
        for (uint j = 0; j < alarms->size(); j++) {
            Poco::JSON::Object::Ptr alarm = alarms->getObject(j);
            if (alarm.isNull()) {
                MyLogWarn("alarms index %d is null", j);
                continue;
            }
            AlarmConfig alarmNew;
            JGET(alarmNew.Desc, alarm, "description", COMStr);
            JGET(alarmNew.Tag, alarm, "tag", COMStr);
            JGET(alarmNew.State, alarm, "state", int);
            JGET(alarmNew.High, alarm, "high", int);
            JGET(alarmNew.Low, alarm, "low", int);
            JGET(alarmNew.Level, alarm, "alarm_level", COMStr);
            dev.Alarms.push_back(alarmNew);
        }
        Devices.push_back(dev);
    } 
}

bool CollectorConfig_T::JSave()
{
    MyLogDebug("CollectorConfig_T::JSave()");

    if (JConf.isNull()) {
        MyLogDebug("CollectorConfig_T::JSave() Null Pointer");
        return false;
    }

    JConf->set("data_report_interval", DataReportInterval);

    IsDraft = false;
    JSON::Array::Ptr jdev; JGET_ARR(jdev, JConf, "devices");
    if (!jdev.isNull()) {
        if (jdev->size() > 0) {
            jdev->clear();
        }

        for (auto dev : Devices) {
            JSON::Object obj(true);
            obj.set("device_type", dev.DeviceType);
            obj.set("sub_id", dev.SubID);
            obj.set("protocol", dev.Protocol);
            obj.set("interval", dev.Interval);

            Poco::JSON::Array alarms;
            for (auto a : dev.Alarms) {
               Poco::JSON::Object jalarm(true);
               jalarm.set("description", a.Desc); 
               jalarm.set("tag", a.Tag);
               jalarm.set("state", a.State);
               jalarm.set("high", a.High);
               jalarm.set("low", a.Low);
               jalarm.set("alarm_level", a.Level);
               alarms.add(jalarm);
            }
            obj.set("alarms", alarms);
            jdev->add(obj);
        }
    }

    return true;
}

//==================================================
// Registration definition                         =
//==================================================

void RegistrationInfo_T::Clear()
{
    Name = "";
    ProductID = PRODUCT_NAME;
    DevID = "";
    RestAPI = "";
    RestAPIVer = "v2";
    RegisterState = ComState::None;
    Model = "";
    Token = "";
    PublicKey = "";
    PrivateKey = "";
    OTPSecret = "";
    IsLocked = false;
}

bool RegistrationInfo_T::Load(bool draft)
{
    MyLogDebug("RegistrationInfo_T::Load(%s)", draft ? "true" : "false");
    Poco::JSON::Object::Ptr jc = JConf;
    if (jc.isNull()) {
        MyLogDebug("RegistrationInfo_T::Load() null");
        return false;
    }

    Clear();
    JGET(Name, jc, "name", COMStr);
    JGET(Model, jc, "Model", COMStr);
    JGET(RestAPI, jc, "RestAPI", COMStr);
    RestAPI = ComUtil::StrRemove(RestAPI, ' ');
    JGET(RestAPIVer, jc, "RestAPIVer", COMStr);
    JGET(ProductID, jc, "ProductID", COMStr);
    JGET(DevID, jc, "DevID", COMStr);
    JGET(Protocol, jc, "Protocol", COMStr);
    JGET(RegisterState, jc, "RegisterState", COMStr);
    JGET(Token, jc, "Token", COMStr);
    JGET(PublicKey, jc, "PublicKey", COMStr);
    JGET(PrivateKey, jc, "PrivateKey", COMStr);
    JGET(OTPSecret, jc, "OTPSecret", COMStr);
    JGET(IsLocked, jc, "IsLocked", bool);
    MyLogDebug("DevID=%s", DevID.c_str());
    return true;
}

bool RegistrationInfo_T::JSave()
{
    MyLogDebug("RegistrationInfo_T::JSave()");
    Poco::JSON::Object::Ptr jc = JConf;

    if (JConf.isNull()) {
        MyLogDebug("RegistrationInfo_T::JSave() Null Pointer");
        return false;
    }

    JConf->set("name", Name);
    JConf->set("Model", Model);
    JConf->set("RestAPI", RestAPI);
    JConf->set("RestAPIVer", RestAPIVer);
    JConf->set("ProductID", ProductID);
    JConf->set("DevID", DevID);
    JConf->set("Protocol", Protocol);
    JConf->set("RegisterState", RegisterState);
    JConf->set("Token", Token);
    JConf->set("PublicKey", PublicKey);
    JConf->set("PrivateKey", PrivateKey);
    JConf->set("IsLocked", IsLocked);
    JConf->set("OTPSecret", OTPSecret);

    return true;
}

//==================================================
// MQTT definition                                 =
//==================================================

void MqttBrokerConfig_T::Clear()
{
    Host         = "";
    Port         = 0;
    
    TopicData    = MQTT_SEND_DATA_TOPIC;
    TopicDataQoS = MQTT_QOS_1;
    
    TopicCtrl    = MQTT_SEND_READ_EXEC_CMD;
    TopicCtrlQoS = MQTT_QOS_1;

    SubQoS       = MQTT_QOS_1;
    
    Queue        = REDIS_CH_FWD_DATA;
    QueueCtrl    = REDIS_CH_FWD_CTRL;
    
    State        = 0;
}

bool MqttBrokerConfig_T::Load(bool draft)
{
    MyLogDebug("MqttBrokerConfig_T::Load(draft=%s)", draft ? "true" : "false");
    Poco::JSON::Object::Ptr jc = draft ? JConfDraft : JConf;
    if (jc.isNull()) {
        return false;
    }

    Clear();

    JGET(Host, jc, "host", COMStr);
    Host = ComUtil::StrRemove(Host, ' ');
    JGET(Port, jc, "port", int);

    JGET(TopicData, jc, "topic_data", COMStr);
    JGET(TopicDataQoS, jc, "topic_qos", uint);
    JGET(TopicCtrl, jc, "topic_ctrl", COMStr);
    JGET(TopicCtrlQoS, jc, "topic_ctrl_qos", uint);
    JGET(SubQoS, jc, "sub_qos", uint);
    JGET(Queue, jc, "queue", COMStr);
    JGET(QueueCtrl, jc, "queue_ctrl", COMStr);
    JGET(State, jc, "state", int);

    return true;
}

bool MqttBrokerConfig_T::JSave()
{
    if (JConf.isNull()) {
        MyLogDebug("MqttBrokerConfig_T::JSave() Null Pointer");
        return false;
    }

    IsDraft = false;

    JConf->set("host", Host);
    JConf->set("port", Port);
    JConf->set("topic_data", TopicData);
    JConf->set("topic_qos", TopicDataQoS);
    JConf->set("topic_ctrl", TopicCtrl);
    JConf->set("topic_ctrl_qos", TopicCtrlQoS);
    JConf->set("sub_qos", SubQoS);
    JConf->set("queue", Queue);
    JConf->set("queue_ctrl", QueueCtrl);
    JConf->set("state", State);

    return true;
}

bool MqttBrokerConfig_T::IsReady()
{
    if (State == 0) {
        return false;
    }

    if (Host.empty() || Port == 0 || TopicData.empty() || Queue.empty()) {
        return false;
    }

    return true;
}

//==================================================
// NTP Client definition                           =
//==================================================
void NTPConfig::Clear()
{
    Servers.clear();
    Restrict.clear();
}

bool NTPConfig::Load(bool draft)
{
    MyLogDebug("NTPConfig::Load(%s)", draft ? "true" : "false");
    JSON::Object::Ptr jc = draft ? JConfDraft : JConf;
    if (jc.isNull()) {
        return false;
    }

    IsDraft = draft;

    this->Clear();
    JSON::Array::Ptr jsvr; JGET_ARR(jsvr, jc, "Servers");
    if (jsvr.isNull()) {
        return false;
    }

    for (uint i = 0; i < jsvr->size(); i++) {
        Servers.push_back(jsvr->getElement<COMStr>(i));
    }

    JSON::Array::Ptr jres; JGET_ARR(jres, jc, "Restrict");
    if (jres.isNull()) {
        return false;
    }

    COMStr ip, mask;
    for (uint i = 0; i < jres->size(); i++) {
        JSON::Object::Ptr pobj = jres->getObject(i);
        JGET(ip, pobj, "ip", COMStr);
        JGET(mask, pobj, "mask", COMStr);
        Restrict[ip] = mask;
    }

    return true;
}

bool NTPConfig::JSave()
{
    MyLogDebug("NTPConfig::JSave()");
    JSON::Object::Ptr jc = JConf;

    IsDraft = false;

    if (jc.isNull()) {
        MyLogDebug("NTPConfig::JSave() Null Pointer");
        return false;
    }

    JSON::Array::Ptr jsvr; JGET_ARR(jsvr, JConf, "Servers");
    MyLogDebug("NTPConfig::JSave() %d %d", jsvr.referenceCount(), JConf.referenceCount());

    if (jsvr->size() > 0) {
        MyLogDebug("Clear NTP %d %d", jsvr.referenceCount(), JConf.referenceCount());
        jsvr->clear();
    }

    for (uint i = 0; i < Servers.size(); i++) {
        MyLogDebug("Add server %s %d %d", Servers[i].c_str(), jsvr.referenceCount(), JConf.referenceCount());
        jsvr->add(Servers[i]);
    }

    MyLogDebug("Get restrict array");
    JSON::Array::Ptr jres; JGET_ARR(jres, JConf, "Restrict");
    if (jres->size() > 0) {
        MyLogDebug("Clear restrict");
        jres->clear();
    }

    MyLogDebug("Add restrict rule");
    for (auto r : Restrict) {
        JSON::Object::Ptr pobj = new JSON::Object(true);
        pobj->set("ip", r.first);
        pobj->set("mask", r.second);
        jres->add(*pobj);
    }

    MyLogDebug("Added, start saving");
    return Apply();
}

bool NTPConfig::Apply()
{
    MyLogDebug("NTPConfig::Apply()");
    COMStr strSvr;
    for (auto s : Servers) {
        strSvr += "pool " + s + " iburst\n";
    }
    COMStr strRes;
    for (auto s : Restrict) {
        strRes += "restrict " + s.first + " mask " + s.second;
    }

    COMStr ntp = ComUtil::StrFileMakeUp(COM_CFG_DIR"/ntp.conf", {{"Servers", strSvr}, {"Restrict", strRes}});
    ComUtil::StrToFile(ntp, "/etc/ntp.conf");
    COMStr cmd = "systemctl restart ntpd.service";
    CommandRunner::Exec(cmd);
    return true;
}

//================================================================================
// LogConfig_T

void LogConfig_T::Clear()
{
    logDir = "";
    logLevel = 0;
    pocoLogLevel = 8;
    logMbSize = 5;
}

bool LogConfig_T::Load(bool draft)
{
    MyLogDebug("LogConfig_T::Load(%s)", draft ? "true" : "false");
    Poco::JSON::Object::Ptr jc =  JConf;
    if (jc.isNull()) {
        return false;
    }

    this->Clear();

    JGET(logDir, jc, "logDir", COMStr);
    JGET(logMbSize, jc, "logMbSize", int);
    JGET(logLevel, jc, "logLevel", int);
    JGET(pocoLogLevel, jc, "pocoLogLevel", int);
    JGET(prioMask, jc, "PrioMask", uint);
    JGET(compMask, jc, "CompMask", uint);
    JGET(outMask, jc, "OutMask", uint);
    JGET(filePath, jc, "LogOutFile", COMStr);

    return true;
}

bool LogConfig_T::JSave()
{
    Poco::JSON::Object::Ptr jc =  JConf;
    if (jc.isNull()) {
        MyLogDebug("LogConfig_T::JSave() Null Pointer");
        return false;
    }

    jc->set("logDir", logDir);
    jc->set("logMbSize", logMbSize);
    jc->set("logLevel", logLevel);
    jc->set("pocoLogLevel", pocoLogLevel);
    jc->set("PrioMask", prioMask);
    jc->set("CompMask", compMask);
    jc->set("OutMask", outMask);
    jc->set("LogOutFile", filePath);

    return true;
}



//==================================================
// Network Interfaces definition                   =
//==================================================

void NetConfig::Clear()
{
    Itfs.clear();
    DNS.clear();

#ifdef DEVICE_AUTO_GATEWAY
    Gateways.clear();
#endif
}

bool NetConfig::Load(bool draft)
{
    MyLogDebug("NetConfig::Load()");

    Poco::JSON::Object::Ptr jc = draft ? JConfDraft : JConf;
    if (jc.isNull()) {
        return false;
    }

    IsDraft = draft;
    Clear();

    MyLogDebug("NetConfig::Load() DNS");
    Poco::JSON::Array::Ptr dns; JGET_ARR(dns, jc, "DNS");
    if (dns.isNull()) {
        MyLogErr("NetConfig::Load() DNS is null");
        return false;
    }

    MyLogDebug("NetConfig::Load() Itfs");
    Poco::JSON::Array::Ptr conns; JGET_ARR(conns, jc, "Itfs");
    if (conns.isNull()) {
        MyLogErr("NetConfig::Load() Itfs is null");
        return false;
    }

#ifdef DEVICE_AUTO_GATEWAY
    MyLogDebug("NetConfig::Load() Gateways");
    Poco::JSON::Array::Ptr gws; JGET_ARR(dns, jc, "Gateways");
    if (gws.isNull()) {
        MyLogErr("NetConfig::Load() Gateways is null");
    } else if (gws->size() > 0) {
        COMStr g;
        for (uint i = 0; i < gws->size(); i++) {
            g = gws->getElement<COMStr>(i);
            Gateways.push_back(g);
        }
    }
#endif /*DEVICE_AUTO_GATEWAY*/

    COMStr d;
    for (uint i = 0; i < dns->size(); i++) {
         d = dns->getElement<COMStr>(i);
         DNS.push_back(d);
    }

    for (uint i = 0; i < conns->size(); i++) {
      auto obj = conns->getObject(i);
      NetItf itf;
      JGET(itf.Name, obj, "Name", COMStr);
      JGET(itf.IP, obj, "IP", COMStr);
      JGET(itf.Subnet, obj, "Subnet", COMStr);
      JGET(itf.Gateway, obj, "Gateway", COMStr);
      JGET(itf.Type, obj, "Type", COMStr);
      JGET(itf.Mode, obj, "Mode", COMStr);
      JGET(itf.IsDefault, obj, "IsDefault", bool);
      Itfs.push_back(itf);
    }

    MyLogDebug("NetConfig::Load() done");
    return true;
}

bool NetConfig::JSave()
{
    MyLogDebug("NetConfig::JSave()");
    JSON::Object::Ptr jc = JConf;

    if (jc.isNull()) {
        MyLogDebug("NetConfig::JSave() Null Pointer");
        return false;
    }

    JSON::Array::Ptr dns; JGET_ARR(dns, JConf, "DNS");
    if (!dns.isNull()) {
        if (dns->size() > 0) {
            dns->clear();
        }

        for (auto d : DNS) {
            dns->add(d);
        }
    } else {
        MyLogErr("NetConfig::JSave() DNS is null");
    }

#ifdef DEVICE_AUTO_GATEWAY
    JSON::Array::Ptr gws; JGET_ARR(gws, JConf, "Gateways");
    if (!gws.isNull()) {
        if (gws->size() > 0) {
            gws->clear();
        }

        for (auto g : Gateways) {
            gws->add(g);
        }
    } else {
        MyLogErr("NetConfig::JSave() Gateways is null");
    }
#endif /*DEVICE_AUTO_GATEWAY*/

    JSON::Array::Ptr jitf; JGET_ARR(jitf, JConf, "Itfs");
    if (!jitf.isNull()) {
        if (jitf->size() > 0) {
            jitf->clear();
        }

        for (auto itf : Itfs) {
            JSON::Object obj(true);
            obj.set("Name", itf.Name);
            obj.set("IP", itf.IP);
            obj.set("Subnet", itf.Subnet);
            obj.set("Gateway", itf.Gateway);
            obj.set("IsDefault", itf.IsDefault);
            obj.set("Type", itf.Type);
            obj.set("Mode", itf.Mode);
            jitf->add(obj);
        }
    } else {
        MyLogErr("NetConfig::JSave() Itfs is null");
    }

    return true;
}

NetItf& NetConfig::ItfByName(const COMStr& name)
{
    for (uint i = 0; i < Itfs.size(); i++) {
        if (Itfs[i].Name == name) {
            return Itfs[i];
        }
    }

    return COM_OBJ_INVALID(NetItf);
}

bool NetConfig::CommitDNS()
{
    COMStr resolv = "#This file is generated automatically\n\n"
                    "nameserver 127.0.1.1\n";

    for (auto d : DNS) {
        resolv += "nameserver " + d + "\n";
    }

    ComUtil::StrToFile(resolv, "/etc/resolv.conf");
    return true;
}

bool NetConfig::Commit()
{
    COMStr loTmp =  "# Network interface configuration"
                    "# Following is auto generated content\n\n"
                    "source /etc/network/interfaces.d/*\n\n"
                    "auto lo\n"
                    "    iface lo inet loopback\n";

    COMStr itfTmp = "auto [Name]\n"
                    "    iface [Name] inet static\n"
                    "    address [IP]\n"
                    "    netmask [Subnet]\n"
                    "    dns-nameservers [DNS]\n";

    COMStr itfTmp2 = "auto [Name]\n"
                     "    iface [Name] inet [Mode]";

    COMStr brgTmp = "auto " DEV_LAN1_ITF "\n"
                    "    iface " DEV_LAN1_ITF " inet manual\n\n"
                    "auto " DEV_LAN2_ITF "\n"
                    "    iface " DEV_LAN2_ITF " inet manual\n\n"
                    "auto [Name]\n"
                    "    iface [Name] inet static\n"
                    "    bridge_stp off\n"
                    "    address [IP]\n"
                    "    netmask [Subnet]\n"
                    "    dns-nameservers [DNS]\n"
                    "    bridge_ports [Ports]\n"
                    "    gateway [Gateway]\n";
                    //"    post-up route add default gw [Gateway]\n"
                    //"    pre-down route del default gw [Gateway]\n";

    COMStr itfs = loTmp;
    for (auto itf : Itfs) {
        if (itf.Type == "bridge") {
            COMStrMap map({{"Name", itf.Name},
                          {"IP", itf.IP},
                          {"Subnet", itf.Subnet},
                          {"Gateway", itf.Gateway},
                          {"DNS", "8.8.8.8 8.8.4.4"},
                          {"Ports", DEV_LAN1_ITF " " DEV_LAN2_ITF},
                          {"Mode", itf.Mode}});

            itfs += "\n\n" + ComUtil::StrMakeUp(brgTmp, map);
            ComUtil::StrToFile(itfs, "/etc/network/interfaces");
            DevNetIf& nif = ComSysNetwork::Inst()->GetItf(itf.Name);

            if (!&nif) {
                COM_CW("Create network bridge interface: %s", itf.Name.c_str());
                COMStr cmd = "brctl addbr " + itf.Name;
                CommandRunner::Exec(cmd);
            }

            COM_CW("Device is restarting networking, please wait");
            COMStrVect cmds = {"ifconfig " DEV_LAN1_ITF " 0.0.0.0 up",
                               "ip addr flush " DEV_LAN1_ITF,
                               "ifconfig " DEV_LAN2_ITF " 0.0.0.0 up",
                               "ip addr flush " DEV_LAN2_ITF,
                               "ifconfig " + itf.Name + " 0.0.0.0 up",
                               "ip addr flush " + itf.Name,
                               "systemctl restart networking"};

            return (CommandRunner::Exec(cmds, true) == 0);
        }
    }

    for (auto itf : Itfs) {
        COMStrMap map({{"Name", itf.Name},
                      {"IP", itf.IP},
                      {"Subnet", itf.Subnet},
                      {"DNS", "8.8.8.8 8.8.4.4"},
                      {"Mode", itf.Mode}});

        COMStr tmp = ComUtil::StrMakeUp((itf.Mode == "static" ? itfTmp : itfTmp2), map);
        if (itf.Mode == "static" && itf.Gateway != "0.0.0.0") {
            tmp += "    gateway " + itf.Gateway + "\n";

            if (itf.IsDefault) {
                tmp += "    post-up route add default gw " + itf.Gateway + "\n" +
                       "    pre-down route del default gw " + itf.Gateway + "\n";
            }
        }

        itfs += "\n\n" + tmp;
        COMStrVect cmds = {"ifconfig " + itf.Name + " 0.0.0.0 up",
                           "ip addr flush " + itf.Name};

        CommandRunner::Exec(cmds, true);
    }

    CommitDNS();

    COMStr cmd;
    ComUtil::StrToFile(itfs, "/etc/network/interfaces");
    cmd = "systemctl restart networking";
    CommandRunner::Exec(cmd, true);

    /**
     * Delete undefined interface router
     */
    ComSysNetwork::Inst()->Load();
    for (auto itf : ComSysNetwork::Inst()->Interfaces) {
        if (!itf.second.gateway.empty()) {
            bool found = false;
            for (auto icfg : Itfs) {
                if (icfg.Name == itf.first) {
                    found = true;
                }
            }

            if (!found) {
                cmd = "route del default dev " + itf.first;
                CommandRunner::Exec(cmd, true);
            }
        }
    }

    return true;
}

/********************************************************************************
 * @brief: Handle list of configuration entries                                 *
 *******************************************************************************/
std::vector<ComConfig*> COMCFG::Entries;

ComConfig* COMCFG::At(const COMStr& key)
{
    for (uint i = 0; i < Entries.size(); i++) {
        if (Entries[i]->Name == key) {
            return Entries[i];
        }
    }

    return NULL;
}

bool COMCFG::Refresh(const COMStr& key)
{
    MyLogDebug("ComConfigs::Refresh()");
    for (uint i = 0; i < Entries.size(); i++) {
        if (key.empty() || Entries[i]->Name == key) {
            Entries[i]->Init();
        }
    }

    return true;
}

bool COMCFG::Reset(const COMStr& key)
{
    MyLogDebug("ComConfigs::Reset()");
    for (uint i = 0; i < Entries.size(); i++) {
        if (Entries[i]->Name == CFGKeys::Manager ||
            Entries[i]->Name == CFGKeys::Net ||
            Entries[i]->Name == CFGKeys::NTP )
        {
            continue;
        }

        if (key.empty() || Entries[i]->Name == key) {
            Entries[i]->Clear();
            if (!Entries[i]->ReadOnly) {
                Entries[i]->Save();
            }
        }
    }

    return true;
}

bool COMCFG::Save(const COMStr& key)
{
    MyLogDebug("ComConfigs::Save()");
    bool ret = true;
    for (uint i = 0; i < Entries.size(); i++) {
        if (key.empty() || Entries[i]->Name == key) {
            if (!Entries[i]->ReadOnly) {
                ret = (ret && Entries[i]->Save());
            }
        }
    }
    return ret;
}

bool COMCFG::Release()
{
    MyLogDebug("ComConfigs::Release()");
    for (uint i = 0; i < Entries.size(); i++) {
        delete Entries[i];
        Entries[i] = NULL;
    }

    Entries.clear();
    return true;
}


void SensorGroup_T::Clear()
{
    SensorAlarm.clear();
    
}

bool SensorGroup_T::Load(bool draft)
{
    MyLogDebug("SensorGroup_T::Load(%s)", draft ? "true" : "false");
    Poco::JSON::Object::Ptr jc = JConf;
    if (jc.isNull()) {
        MyLogDebug("SensorGroup_T::Load() null");
        return false;
    }
    Clear();

    Poco::JSON::Array::Ptr jsen; JGET_ARR(jsen, jc, "Sensors");
    if (jsen.isNull()) {
        MyLogWarn("SensorGroup_T::Load() Sensors is null");
        return false;
    }
    SensorAlarmConfig sac;
    for (uint i = 0; i < jsen->size(); i++) {
        JSON::Object::Ptr sobj = jsen->getObject(i);
        JGET(sac.Desc, sobj, "description", COMStr);
        JGET(sac.Tag, sobj, "tag", COMStr);
        JGET(sac.State, sobj, "state", int);
        JGET(sac.High, sobj, "high", int);
        JGET(sac.Low, sobj, "low", int);
        JGET(sac.Level, sobj, "alarm_level", COMStr);
        JGET(sac.Address, sobj, "mac_addr", COMStr);
        SensorAlarm.push_back(sac);
    }
}

bool SensorGroup_T::JSave()
{
    MyLogDebug("SensorGroup_T::JSave()");

    if (JConf.isNull()) {
        MyLogDebug("SensorGroup_T::JSave() Null Pointer");
        return false;
    }
    IsDraft = false;
    Poco::JSON::Array::Ptr jsen; JGET_ARR(jsen, JConf, "Sensors");
    if (!jsen.isNull()) {
        if (jsen->size() > 0) {
            jsen->clear();
        }

        for (auto s : SensorAlarm) {
            JSON::Object obj(true);
            obj.set("description", s.Desc); 
            obj.set("tag", s.Tag);
            obj.set("state", s.State);
            obj.set("high", s.High);
            obj.set("low", s.Low);
            obj.set("alarm_level", s.Level);
            obj.set("mac_addr", s.Address);
            jsen->add(obj);
        }
    }
    return true;
}

void defaultRelayState_T::Clear()
{
    State = 0;
    DFRelay.clear();
    
}

bool defaultRelayState_T::Load(bool draft)
{
    MyLogDebug("defaultRelayState_T::Load(%s)", draft ? "true" : "false");
    Poco::JSON::Object::Ptr jc = JConf;
    if (jc.isNull()) {
        MyLogDebug("defaultRelayState_T::Load() null");
        return false;
    }
    Clear();

    Poco::JSON::Array::Ptr jrlay; JGET_ARR(jrlay, jc, "Relays");
    if (jrlay.isNull()) {
        MyLogWarn("defaultRelayState_T::Load() Relay is null");
        return false;
    }
    defaultRelayState rl;
    for (uint i = 0; i < jrlay->size(); i++) {
        JSON::Object::Ptr robj = jrlay->getObject(i);
        JGET(rl.Desc, robj, "description", COMStr);
        JGET(rl.Tag, robj, "tag", COMStr);
        JGET(rl.State, robj, "state", int);
        JGET(rl.Gpio, robj, "gpio", int);
        JGET(rl.Index, robj, "index", int);
        DFRelay.push_back(rl);
    }
}

bool defaultRelayState_T::JSave()
{
    MyLogDebug("defaultRelayState_T::JSave()");

    if (JConf.isNull()) {
        MyLogDebug("defaultRelayState_T::JSave() Null Pointer");
        return false;
    }

    IsDraft = false;
    Poco::JSON::Array::Ptr jrlay; JGET_ARR(jrlay, JConf, "Relays");
    if (!jrlay.isNull()) {
        if (jrlay->size() > 0) {
            jrlay->clear();
        }

        for (auto r : DFRelay) {
            JSON::Object obj(true);
            obj.set("description", r.Desc); 
            obj.set("tag", r.Tag);
            obj.set("state", r.State);
            obj.set("gpio", r.Gpio);
            obj.set("index", r.Index);
            jrlay->add(obj);
        }
    }
    return true;
}