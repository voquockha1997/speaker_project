#pragma once

// System
#include <vector>

// 3rd
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Query.h"
#include "Poco/Dynamic/Var.h"

// Project
#include "../3rd/Utility/Singleton.hpp"
#include "ComConfig.h"
#include "ComUtil.h"

#include "M2CMQTTDefine.h"
#define COMMON_LOG_GROUP LOG_G_COMMON


/**
 * @brief: Main configuration class
 */
class MQTTConfig : public COMCFG
{
public:
    static bool Initialize();
};


