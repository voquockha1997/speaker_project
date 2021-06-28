#ifndef COMMON_CLOUD_INTERFACE_H_
#define COMMON_CLOUD_INTERFACE_H_

#include "ComDefinition.h"

struct M2CCommand
{
    STR_MACRO_T GetVersion     = "get_version";
    STR_MACRO_T Reboot         = "reboot";
    STR_MACRO_T Restart        = "restart";
    STR_MACRO_T SetAirCStatus  = "set_air_conditional_status";
    STR_MACRO_T GetAlarmConfig = "get_alarm_config";
    STR_MACRO_T SetAlarmConfig = "set_alarm_config";
};

// cloud audit definition
struct M2CAudit
{
    STR_MACRO_T SysInfo        = "system_info";
    STR_MACRO_T DevState       = "device_state";
    STR_MACRO_T CompFail       = "component_failure";
    STR_MACRO_T DevOn          = "device_on";
    STR_MACRO_T DevOff         = "device_off";
    
};

struct M2CAuditLevel
{
    STR_MACRO_T Info       = "info";
    STR_MACRO_T Warn       = "warning";
    STR_MACRO_T Debug      = "debug";
    STR_MACRO_T Error      = "error";
    STR_MACRO_T Critical   = "critical";
};

struct M2CEvent
{
    STR_MACRO_T Over        = "OverThreshold";
    STR_MACRO_T Under       = "UnderThreshold";
};

struct M2CEventLevel
{
    STR_MACRO_T Observation = "Observation";
    STR_MACRO_T Warning     = "Warning";
    STR_MACRO_T Critical    = "Critical";
};
#endif /* COMMON_CLOUD_INTERFACE_H_ */
