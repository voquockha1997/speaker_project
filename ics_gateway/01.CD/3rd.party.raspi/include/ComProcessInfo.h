#ifndef COMMON_PROCESS_INFO_H_
#define COMMON_PROCESS_INFO_H_

#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

class ComProcessInfo : ComNoCopyAndInstantiable
{
public:
    static COMStr Proto;
    static COMStr User;
    static COMStr UserType;
    static uint   Privilege;
    static COMStr Ssh;
    static COMStr State;
    static COMStr LastCmd;

    static COMStr Str() {
        return (Ssh + " (user:" + User + " auth:" + UserType + " State:" + State + ")");
    }

};

#endif /* COMMON_PROCESS_INFO_H_ */
