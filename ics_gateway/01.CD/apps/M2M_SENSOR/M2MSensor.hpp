#ifndef M2M_INTERFACE_H_
#define M2M_SENSOR_H_

#include <iostream>
#include <string>
#include <sstream>
#include "Poco/DateTimeFormatter.h"

#include "ThresholdRule.h"
#include "ComProcess.h"
#include "ComManagerClient.h"
#include "ComRedis.h"
#include "ComUtil.h"
#include "ComQueue.h"
#include "M2MSensorConfig.h"
#include "SensorDevice.h"

#define COMMON_LOG_GROUP LOG_G_M2MINTERFACE

using namespace M2M::Sensor;

class M2MSensor
{
public:
    M2MSensor() {}

    ~M2MSensor()
    {
    }

    bool Initialize()
    {
        MyLogDebug("M2MSensor::Initialize()");
        SensorDevice SensorInit;
        SensorInit.StartCollecting();
    }
};

#endif // M2M_SENSOR_H_
