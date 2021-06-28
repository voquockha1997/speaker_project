#ifndef COMMON_CONFIG_H_
#define COMMON_CONFIG_H_

#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Query.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/UUIDGenerator.h"

#include "ComRedis.h"
#include "ComDefinition.h"

#include "ComUtil.h"
#include "BaseConfig.h"
#define COMMON_LOG_GROUP LOG_G_CONFIG

struct CFGKeys
{
    STR_MACRO_T Manager         = "MANAGER_INFO";
    STR_MACRO_T RegInfo         = "REGISTRATION_INFO";
    STR_MACRO_T NTP             = "NTP";
    STR_MACRO_T Params          = "COMMON_PARAMS";
    STR_MACRO_T HTTPS           = "HTTPS";
    STR_MACRO_T MQTT            = "MQTT_BROKER";
    STR_MACRO_T LOGS            = "LOGS";
    STR_MACRO_T Net             = "NETWORK";
    STR_MACRO_T Collector       = "COLLECTOR_CONFIG";
    STR_MACRO_T SensorGroup     = "SENSOR_GROUP";
    STR_MACRO_T DFRelayState    = "DEFAULT_RELAY";
};

// Manager Configuration
struct ProcessConfig
{
    COMStr  Proto;
    COMStr  Command;
    bool    IsRespawn;
    uint    MaxSpawnTimes;
    int     Priority = 100;
    uint    RespawnDelay = 30; // 30 seconds
};

typedef std::vector<ProcessConfig> ProcessesConfig;

class ManagerConfig_T : public ComConfig
{
public:
    ManagerConfig_T(bool readOnly = true) { Init(CFGKeys::Manager, readOnly); }
    COMStr  Host = "127.0.0.1";
    COMStr  SoftVer = PRODUCT_VERSION;
    uint    Port = 6789;
    uint    WatchdogInterval    = 60;
    uint    SysReportInterval   = 60;

    ProcessesConfig Children;

    inline ProcessConfig& Child(const COMStr& Proto)
    {
        for (auto &p : Children) {
            if (p.Proto == Proto) {
                return p;
            }
        }

        return COM_OBJ_INVALID(ProcessConfig);
    }

    void Clear() override;
    bool Load(bool draft = false) override;
    bool JSave() override;
};
////////////////////////////////////////////////////////

// Collector config
struct AlarmConfig
{
    COMStr  Desc;
    COMStr  Tag;
    int     State; // 0 is off , 1 is on
    int     High;
    int     Low;
    COMStr  Level;
};
typedef std::vector<AlarmConfig> AlarmsConfig;

// Device Configuration
struct DeviceConfig
{
    COMStr  DeviceType;
    COMStr  SubID;
    COMStr  Protocol;
    uint    Interval = 10; // 10 seconds
    AlarmsConfig Alarms;
};

typedef std::vector<DeviceConfig> DevicesConfig;

class CollectorConfig_T : public ComConfig
{
public:
    CollectorConfig_T(bool readOnly = true) { Init(CFGKeys::Collector, readOnly); }
    
    uint    DataReportInterval  = 60;

    DevicesConfig Devices;

    inline DeviceConfig& FindBySubId(const COMStr& SubId)
    {
        for (auto &d : Devices) {
            if (d.SubID == SubId) {
                return d;
            }
        }

        return COM_OBJ_INVALID(DeviceConfig);
    }

    void Clear() override;
    bool Load(bool draft = false) override;
    bool JSave() override;
};

//==================================================
// Registration definition                         =
//==================================================

/**
 * @brief: Device Registration info structure
 */
class RegistrationInfo_T : public ComConfig
{
public:
    RegistrationInfo_T(bool readOnly = true) { Init(CFGKeys::RegInfo, readOnly); }

    COMStr Name = "";
    COMStr RestAPI = "";
    COMStr RestAPIVer = "v2";
    COMStr ProductID = PRODUCT_NAME;
    COMStr Model = DEFAULT_RUNNING_MODEL;
    COMStr Protocol = DEFAULT_PROTOCOL_COMMUNICATE_CLOUD;
    COMStr DevID = "";
    COMStr RegisterState = ComState::None;
    COMStr Token = "";
    COMStr OTPSecret = "";
    COMStr PublicKey = "";
    COMStr PrivateKey = "";
    bool   IsLocked = false;

    void Clear() override;
    bool Load(bool draft = false) override;
    bool JSave() override;
};

//==================================================
// MQTT definition                                 =
//==================================================

/**
 * @brief: MQTT Brocker configuration structure
 */
class MqttBrokerConfig_T : public ComConfig
{
public:
    MqttBrokerConfig_T(bool readOnly = true) { Init(CFGKeys::MQTT, readOnly); }

    COMStr Host;
    int    Port;

    bool IsSec = false;

    COMStr TopicData     = MQTT_SEND_DATA_TOPIC;
    uint   TopicDataQoS  = MQTT_QOS_1;

    COMStr TopicCtrl     = MQTT_SEND_READ_EXEC_CMD;
    uint   TopicCtrlQoS  = MQTT_QOS_1;

    uint   SubQoS        = MQTT_QOS_1;  // qos of listen cmd from server DevID,

    COMStr Queue         = REDIS_CH_FWD_DATA;
    COMStr QueueCtrl     = REDIS_CH_FWD_CTRL;

    COMStr PrivateKey    = "";
    COMStr PublicKey     = "";

    int    State;

    inline COMStr StateString() {
        return (State == 0 ? "Deactivated" : "Activated");
    }

    void Clear() override;
    bool Load(bool draft = false) override;
    bool JSave() override;
    bool IsReady();
};

//==================================================
// NTP Client definition                           =
//==================================================

class NTPConfig : public ComConfig
{
public:
    NTPConfig(bool readOnly = true) { Init(CFGKeys::NTP, readOnly); }
    COMStrVect Servers;
    COMStrMap Restrict;

    void Clear() override;
    bool Load(bool draft = false) override;
    bool JSave() override;
    bool Apply();
};

//==================================================
// Logging definition                              =
//==================================================

/**
 * @brief: Logger configuration structure
 */
class LogConfig_T : public ComConfig
{
public:
    LogConfig_T(bool readOnly = true) { Init(CFGKeys::LOGS, readOnly); }

    COMStr      logDir = "";
    int         logLevel = 0;
    int         pocoLogLevel = 8;
    int         logMbSize = 5;

    unsigned int compMask;
    unsigned int prioMask;
    unsigned int outMask;
    COMStr       filePath;

    void Clear() override;
    bool Load(bool draft = false) override;
    bool JSave() override;
};





//==================================================
// Network Interfaces definition                   =
//==================================================

struct NetItf
{
    COMStr Name;
    COMStr Type;
    COMStr IP;
    COMStr Subnet;
    COMStr Gateway;
    COMStr Mode;
    bool   IsDefault;
};

typedef std::vector<NetItf> NetItfs;
class NetConfig : public ComConfig
{
public:
    NetConfig(bool readOnly = true) { Init(CFGKeys::Net, readOnly); }
    NetItfs Itfs;
    COMStrVect DNS;

#ifdef DEVICE_AUTO_GATEWAY
    COMStrVect Gateways;
#endif

    void Clear() override;
    bool Load(bool draft = false) override;
    bool JSave() override;
    bool Commit();
    bool CommitDNS();

    NetItf& ItfByName(const COMStr& name);
};



//////////////////////////////////////

// Sensor group config
struct SensorAlarmConfig
{
    COMStr  Desc;
    COMStr  Tag;
    int     State; // 0 is off , 1 is on
    int     High;
    int     Low;
    COMStr  Level;
    COMStr  Address;
};
typedef std::vector<SensorAlarmConfig> SensorsAlarmConfig;

//==================================================
// Group OF Sensor definition                      =
//==================================================

/**
 * @brief: Group OF Sensor configuration
 */
class SensorGroup_T : public ComConfig
{
public:
    SensorGroup_T(bool readOnly = true) { Init(CFGKeys::SensorGroup, readOnly); }
    SensorsAlarmConfig SensorAlarm;

    inline SensorAlarmConfig& Sensor(const COMStr& Tag)
    {
        for (auto &s : SensorAlarm) {
            if (s.Tag == Tag) {
                return s;
            }
        }

        return COM_OBJ_INVALID(SensorAlarmConfig);
    }
    void Clear() override;
    bool Load(bool draft = false) override;
    bool JSave() override;
};


// Defaul relay state config
struct defaultRelayState
{
    COMStr  Desc;
    COMStr  Tag;
    int     State; // 0 is off , 1 is on
    int     Gpio;
    int     Index;
};
typedef std::vector<defaultRelayState> defaultRelaysState;


//==================================================
// Group OF Sensor definition                      =
//==================================================

/**
 * @brief: Group OF Sensor configuration
 */
class defaultRelayState_T : public ComConfig
{
public:
    defaultRelayState_T(bool readOnly = true) { Init(CFGKeys::DFRelayState, readOnly); }
    int     State = 0;
    int     Relay1 = 23; //Set Relay 1 GPIO is 23
    int     Relay2 = 24; //Set Relay 2 GPIO is 24
    defaultRelaysState DFRelay;

    inline defaultRelayState& Relay(const COMStr& Tag)
    {
        for (auto &r : DFRelay) {
            if (r.Tag == Tag) {
                return r;
            }
        }

        return COM_OBJ_INVALID(defaultRelayState);
    }
    void Clear() override;
    bool Load(bool draft = false) override;
    bool JSave() override;
};

/**
 * @brief: Handle list of configurations
 */
#define COM_CFG_GET(type, key)              \
static type &key() {                       \
    ComConfig* cfg = At(CFGKeys::key);     \
    if (cfg == NULL) {                     \
        return COM_OBJ_INVALID(type);      \
    }                                      \
    return *((type*)cfg);                  \
}

class COMCFG : public ComNoCopyAndInstantiable
{
public:
    static std::vector<ComConfig*> Entries;

    static ComConfig* At(const COMStr& key);
    static bool Refresh(const COMStr& key = "");
    static bool Reset(const COMStr& key = "");
    static bool Save(const COMStr& key = "");
    static bool Release();

    COM_CFG_GET(ManagerConfig_T,     Manager)
    COM_CFG_GET(RegistrationInfo_T,  RegInfo)
    COM_CFG_GET(LogConfig_T,         LOGS)
    COM_CFG_GET(NTPConfig,           NTP)
    COM_CFG_GET(MqttBrokerConfig_T,  MQTT)
    COM_CFG_GET(NetConfig,           Net)
    COM_CFG_GET(CollectorConfig_T,   Collector)
    COM_CFG_GET(SensorGroup_T,       SensorGroup)
    COM_CFG_GET(defaultRelayState_T, DFRelayState)

};

#endif /* COMMON_CONFIG_H_ */

