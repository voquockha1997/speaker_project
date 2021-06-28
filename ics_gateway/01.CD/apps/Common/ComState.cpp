#include "ComDefinition.h"
#include "ComRedis.h"

#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"

#include "ComState.h"

bool ComGlobalState::SetGlobalState(const COMStr state)
{
    COMStr ret = ComRedis::Local.Set(REDIS_KEY_GATEWAY_STATE, state);
    ComRedis::Local.Save();
    return (!ret.empty());
}

COMStr ComGlobalState::GlobalState()
{
    return ComRedis::Local.Get(REDIS_KEY_GATEWAY_STATE);
}
