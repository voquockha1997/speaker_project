#ifndef COMMON_MANAGER_MODBUS_INTERFACE_H_
#define COMMON_MANAGER_MODBUS_INTERFACE_H_

#include "ComManagerComInterface.h"

struct MGRMODBUSItf : public MGRItf
{
    STR_MACRO_T DiscoverReq     = "DiscoverReq";
    STR_MACRO_T DiscoverResp    = "DiscoverResp";

    STR_MACRO_T ExecReq         = "ExecReq";
    STR_MACRO_T ExecResp        = "ExecResp";
};


#endif /* COMMON_MANAGER_MODBUS_INTERFACE_H_ */
