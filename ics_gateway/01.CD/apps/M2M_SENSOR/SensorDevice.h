#ifndef APP_SERIAL_COMMUNICATE_H_
#define APP_SERIAL_COMMUNICATE_H_

#include <iostream>
#include <memory>
#include <list>
#include <queue>

#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "Poco/Logger.h"
#include "Poco/Util/Application.h"
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Var.h"

#include "ComUtil.h"

namespace M2M
{
    namespace Sensor
    {
                
        class SensorDevice
        {
        public:
            
            SensorDevice();
            // SensorInit(int a);
            ~SensorDevice();
            
        public:
            
            void StartCollecting();
            void StopCollecting();

        };
        
    }   // namespace Sensor
    
}   // namespace M2M

#endif  // APP_SERIAL_COMMUNICATE_H_
