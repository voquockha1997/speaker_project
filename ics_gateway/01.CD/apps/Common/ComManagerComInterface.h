#ifndef COMMON_MANAGER_BASE_INTERFACE_H_
#define COMMON_MANAGER_BASE_INTERFACE_H_

#include "ComDefinition.h"

enum class CLIOType {
    Info, OK, Fail, Warn, Error, Process, Print
};

struct MGRItf
{
    STR_MACRO_T ConnectReq          = "connect";
    STR_MACRO_T ConnectResp         = "connected";

    STR_MACRO_T StatusInd           = "StatusInd";
    STR_MACRO_T SubDeviceStatusInd  = "SubDevStatusInd";
    STR_MACRO_T M2MData             = "M2MData";
    STR_MACRO_T M2CData             = "M2CData";
    STR_MACRO_T ConsoleInd          = "console_indication";
};

struct MGRItfStatus
{
    STR_MACRO_T OK             = "OK";
    STR_MACRO_T FAIL           = "FAIL";
    STR_MACRO_T ERROR          = "ERROR";
    STR_MACRO_T TIMEOUT        = "TIMEOUT";
    STR_MACRO_T BUSY           = "BUSY";
    STR_MACRO_T FORMAT_ERROR   = "FORMAT ERROR";
};

#endif /* COMMON_MANAGER_BASE_INTERFACE_H_ */
