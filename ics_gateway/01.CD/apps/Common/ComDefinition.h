#ifndef COM_DEFINITION_H_
#define COM_DEFINITION_H_
#include <iostream>
#include <vector>
#include <map>
#include "ComUtil.h"
#include "Product.h"

// Define user permision 
#define COM_USR_PRIV_ADMIN          BVAL(0)
#define COM_USR_PRIV_USER           BVAL(1)
#define COM_USR_PRIV_POLICY         BVAL(2)
#define COM_USR_PRIV_ADMIN_POLICY   (COM_USR_PRIV_ADMIN | COM_USR_PRIV_POLICY)
#define COM_USR_PRIV_USER_POLICY    (COM_USR_PRIV_USER | COM_USR_PRIV_POLICY)
#define COM_USR_PRIV_ALL_POLICY     (COM_USR_PRIV_USER | COM_USR_PRIV_ADMIN | COM_USR_PRIV_POLICY)
#define COM_USR_PRIV_ALL            (COM_USR_PRIV_ADMIN | COM_USR_PRIV_USER)

// Define string 
#define STR_MACRO_T static constexpr const char*

// Define running mode
#define COM_STANDARD_MODE           0
#define COM_CLOUD_MODE              1
#define RUNNING_MODE                COM_STANDARD_MODE

// Define compilation process
#define APP_INDEX_APP        0
#define APP_BRIDGE_INDEX     1
#define APP_MQTT_INDEX       2
#define APP_MODBUS_INDEX     3
#define APP_SNMP_INDEX       5
#define APP_SERIAL_INDEX     6

#define APP_CLI_INDEX        99
#define APP_UNIT_TEST        100

/* Nodes' keys */
struct NodeKeys
{
    STR_MACRO_T None       = "None";
    STR_MACRO_T All        = "All";
    STR_MACRO_T MasterCLI  = "MCLI";
    STR_MACRO_T CLI        = "CLI";
    STR_MACRO_T Manager    = "MANAGER";
    STR_MACRO_T MQTT       = "MQTT";
    STR_MACRO_T Bridge     = "BRIDGE";
    STR_MACRO_T Sensor     = "SENSOR";

    static COMStr Current;
};

// Define data structure used to exchange between Manager and child processes
typedef struct
{
    COMStr      Instance;                   // Sample data: "A", "B", ...
    COMStr      Source;                     // Source of message. Sample data: "Manager", "MODBUS", ...
    COMStrVect  Destination;                // Destination of message. Sample data: "ALL", "Manager", ...
    time_t      Time;
    COMStr      Type;                       // Command type
    COMStr      Content;                    // Message's content. It was JSON string
} ComIPCMsg;

/**
 * @brief: Configuration files
 */
#define WORKSPACE_PATH                         "/data/it5"
#define COM_BIN_DIR                            WORKSPACE_PATH "/bin"
#define COM_CFG_DIR                            WORKSPACE_PATH "/config"
#define COM_SCR_DIR                            WORKSPACE_PATH "/script"
#define DEFAULT_RUNNING_MODEL                  "DEV-001"

#define PROTOCOL_COMMUNICATE_CLOUD_MQTT        "MQTT"
#define DEFAULT_PROTOCOL_COMMUNICATE_CLOUD     PROTOCOL_COMMUNICATE_CLOUD_MQTT

struct ComState
{
    STR_MACRO_T None           = "None";
    STR_MACRO_T NotAuthorized  = "Not Authorized";
    STR_MACRO_T Authorized     = "Authorized";
    STR_MACRO_T Certificated   = "Got Certification";
};

/**
 * @brief: MQTT communication
 */
#define MQTT_PUB_KEY_FILE           COM_CFG_DIR "/pubkey.pem"
#define MQTT_PRV_KEY_FILE           COM_CFG_DIR "/prvkey.pem"
#define MQTT_CER_KEY_FILE           COM_CFG_DIR "/ca.pem"

#define MQTT_SEND_DATA_TOPIC        "data_channel"
#define MQTT_SEND_READ_EXEC_CMD     "message_delivery"
#define MQTT_QOS_0                  0
#define MQTT_QOS_1                  1
#define MQTT_QOS_2                  2
/**
 * @brief: Redis database configuration
 */
#define DEFAULT_REDIS_HOST          "127.0.0.1"
#define DEFAULT_REDIS_PORT          6379
#define REDIS_M2C_DATA_QUEUE        "M2C_DATA_"
#define REDIS_LIVE_DATA_KEY         "live_data"
#define REDIS_SENSOR_DATA_KEY       "sensor_data"

#define REDIS_COLLECTOR_PREFIX      "collector_"

#define REDIS_CH_MCAST_DATA         "local_data_bus"
#define REDIS_CH_MCAST_CTRL         "local_ctrl_bus"
#define REDIS_CH_MCAST_AUDIT        "local_audit_bus"

#define REDIS_CH_FWD_DATA           "device_log"        // dev --- data ---> BRD
#define REDIS_CH_FWD_CTRL           "receive_cmd"       // dev --- ctrl ---> BRD

#define REDIS_CH_FWD2DEV_CTRL       "send_cmd"          // BRD --- ctrl ---> dev
#define REDIS_CH_INNER_COMM         "dev_inner_notify"  // @
#define REDIS_KEY_GATEWAY_STATE     "SystemState"

struct DevGlobalState
{
    STR_MACRO_T Install            = "Install";
    STR_MACRO_T InstallSuccess     = "InstallSuccess";
    STR_MACRO_T InstallFail        = "InstallFail";
    STR_MACRO_T Running            = "Running";
};

// logging 
#define DEFAULT_LOG_PATH                   "/var/log"
#define DEFAULT_LOG_FILE_DIR               "/var/log/it5/"

// Network interface definition
#define DEV_LAN1_ITF "enp2s0"
#define DEV_LAN2_ITF "enp3s0"


// State of device's sub devices
#define SUB_DEV_NOT_AUTHORIZED  0
#define SUB_DEV_AUTHORIZED      1
#define SUB_DEV_ACTIVATED       2
#define SUB_DEV_DEACTIVATED     3

// Action of device's sub devices
#define SUB_DEV_ACT_NONE        0
#define SUB_DEV_ACT_DELETE      1

enum class DeviceState
{
    None            = 0,
    Error           = 1,
    Unreachable     = 2,
    Reachable       = 3,
    Unstable        = 4
};

#define MAX_TAG_INPUT_DATA       32

/**
 * Define tag item class
 */
struct DataTag_T
{
    // Type of Serial USR_02
    int GrpAddr = 0;
    
    // Register address inside RS-485 device
    int RegAddr = 0;

    // Function code used to send to RS-485 device
    int FuncCode = 0;

    int InfoLen = 0; // len of comand info RS232

    // Number of words/bytes were return from RS-485 device
    int WordNum = 0;

    // Low threshold
    double LThreshold = COM_NUMBER_INVALID;

    // High threshold
    double HThreshold = COM_NUMBER_INVALID;

    // Data type of measurement value (string, int, uint , double, bool, ... )
    COMStr DataType = "";

    // DataType Length. When output is array of 'DataType'. DataTypeLen = 0 implied that the output is scalar
    int DataTypeLen = 0;

    // Number of words/bytes were sent to RS-485 device
    int InputWordNum = 0;

    // Array holds input data that should be sent to RS-485 device
    uint16_t Inputs[MAX_TAG_INPUT_DATA];

    // Expression to calculate scalar measurement value
    COMStr Expression = "";

    // C Function to calculate measurement value
    COMStr Function = "";

    COMStr OID;

    COMStr Result = "";
};

typedef std::map<COMStr, DataTag_T> Tags_T;

/**
 * @brief: define data model
 */
struct DataModel_T
{
    COMStr name;   // Name of device type
    Tags_T tags;   // List of tags
};

typedef std::map<COMStr, DataModel_T> Models_T;

/**
 * @brief: define data field struct
 */
typedef struct
{
    COMStr TagName;
    COMStr Value;
    COMStr DataType;
    double Threshold;
} TagData;

/**
 * @brief: Data record definition
 */
typedef std::map<COMStr, TagData> Record_T;

typedef struct
{
    COMStr Time;
    timeval RealTime;
    COMStr Name;
    COMStr Address;
    COMStr DeviceID;
    COMStr Serial;
    bool   IsGenSerial;
    bool   ReportUnauthEvent;
    bool   ReportUnauthData;

    COMStr DataSource;
    COMStr DeviceType;
    COMStr Status;
    COMStr DataTarget;
    uint   AuthState;

    std::vector<TagData> Tags;
    std::vector<TagData> ErrorTags;

    COMStr From;
} DeviceData;




#define DEV_USER_PRIVILEGE              "User Admin"
#define DEV_PRIVILEGE_ADMIN             "Admin"
#define DEV_DEFAULT_ADMIN_ACCOUNT       "admin"
#define DEV_KEY_ENCRYPT_USER            "encrypt_passphrase"

enum UserPrivilege
{
    User            = 2,
    Admin           = 3,
};

#endif /* COM_DEFINITION_H_ */
