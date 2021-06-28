#ifndef COMMON_DEVICE_STATE_H_
#define COMMON_DEVICE_STATE_H_

#include "ComDefinition.h"

class ComGlobalState : ComNoCopyAndInstantiable
{
public:
    static bool   SetGlobalState(const COMStr state);
    static COMStr GlobalState();
};

#define SET_GSTATE(x)   ComGlobalState::SetGlobalState(x)
#define GET_GSTATE()    ComGlobalState::GlobalState()

#endif /* COMMON_DEVICE_STATE_H_ */
