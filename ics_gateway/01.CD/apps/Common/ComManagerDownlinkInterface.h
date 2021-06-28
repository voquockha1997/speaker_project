#ifndef COMMON_MANAGER_DOWNLINK_INTERFACE_H_
#define COMMON_MANAGER_DOWNLINK_INTERFACE_H_

#include "ComManagerComInterface.h"

struct MGRDownlinkItf : public MGRItf
{
    STR_MACRO_T CloudReq      = "cloud_req";
    STR_MACRO_T CloudResp     = "cloud_resp";
};

#endif /* COMMON_MANAGER_DOWNLINK_INTERFACE_H_ */