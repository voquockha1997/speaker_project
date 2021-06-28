#pragma once
#include <vector>
#include <algorithm>
#include "Utils.hpp"
#include "ComSysNetwork.h"

// #include "ComQueue.h"
#include "ComConfig.h"
#include "ComUtil.h"

#define COMMON_LOG_GROUP LOG_G_M2MINTERFACE_CONFIG

class M2MSensorConfig : public COMCFG
{
public:
    static bool Initialize();

public:
};
