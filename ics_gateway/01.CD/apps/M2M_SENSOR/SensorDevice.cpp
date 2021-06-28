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
#include "DS18B20.h"
#include "SensorDevice.h"

#include <vector>
#include <dirent.h>

#include <unistd.h>
#define COMMON_LOG_GROUP LOG_G_M2MINTERFACE

namespace M2M
{
    namespace Sensor
    {
        SensorDevice::SensorDevice(){
            MyLogDebug("SensorDevice::SensorDevice()");
        }

        SensorDevice::~SensorDevice(){
            MyLogDebug("SensorDevice::~SensorDevice()");
        }

        void SensorDevice::StartCollecting() {

            MyLogDebug("Start Collect sensor");
            
            float romTemp;
            float outDoorTemp;
            int flag_d = 0;
            int interval = -1;
            const char* ds18b20_1;
            const char* ds18b20_2;
            
            Poco::JSON::Object dataPoint(true);
            
            Poco::JSON::Array low_alarm;
            Poco::JSON::Array high_alarm;


            /*Scan folder of DS18B20*/
            /*DIR *dir; struct dirent *diread;
            std::vector<const char *> files;

            if ((dir = opendir(BUS)) != nullptr) {
                while ((diread = readdir(dir)) != nullptr) {
                    if (strncmp(diread->d_name, "28-", 3)==0){
                        files.push_back(diread->d_name);
                        flag_d = 0;
                    } else{
                        flag_d = -1;
                    }
                }
                closedir (dir);
            } else {
                MyLogErr("Could not open path %s", BUS);
                
            }

            if (flag_d == -1 || files.size() < 2){
                for (auto &s : M2MSensorConfig::SensorGroup().SensorAlarm) {
                    // MyLogErr("address ds18b20_1 %s",s.Address.c_str());
                    if (strncmp(s.Tag.c_str(), "RoomTemperature",15) == 0){
                        ds18b20_1 = s.Address.c_str();
                    } else if (strncmp(s.Tag.c_str(), "OutdoorTemperature",18) == 0)
                    {
                        ds18b20_2 = s.Address.c_str();
                    }
                }
            } else{
                ds18b20_1 = files[0]; 
                ds18b20_2 = files[1];    
            }*/

            /*****************/
            for (auto &s : M2MSensorConfig::SensorGroup().SensorAlarm) {
                // MyLogErr("address ds18b20_1 %s",s.Address.c_str());
                if (strncmp(s.Tag.c_str(), "RoomTemperature",15) == 0){
                    ds18b20_1 = s.Address.c_str();
                } else if (strncmp(s.Tag.c_str(), "OutdoorTemperature",18) == 0)
                {
                    ds18b20_2 = s.Address.c_str();
                }
            }
            DS18B20 w1Device1 (ds18b20_1); // get folder of DS18B20 
            DS18B20 w1Device2 (ds18b20_2);

            /*Get interval*/
            COMStr data = ComRedis::Local.Get("SENSOR_GROUP");
            Poco::JSON::Parser parse;
            Poco::Dynamic::Var result = parse.parse(data);
            Poco::JSON::Object::Ptr jData = result.extract<Poco::JSON::Object::Ptr>();
            JGET(interval, jData, "interval", int);
            
            if (interval == -1)
            {
                MyLogErr("Wrong format of SENSOR_GROUP");
            }
            /*****************/

            while(1){
                std::ostringstream msg;
                Poco::JSON::Array error_tags;
                Poco::JSON::Array good_tags;
                COMStr now = ComUtil::UtcNowString();
                
                romTemp = w1Device1.getTemp(); // get rom temp
                outDoorTemp = w1Device2.getTemp(); // get outdoor temp

                dataPoint.set("time_stamp", ComUtil::UtcTimeStampString(now));
                dataPoint.set("name", "sensor");
                dataPoint.set("sub_device_id", "c78ferf4-c5an-8983-1e2d-kio3nhenbwec");
                dataPoint.set("master_device_id", M2MSensorConfig::RegInfo().DevID);

                if (romTemp == -2 || romTemp == -1){
                    MyLogErr("Read rom temp from DS18B20 is error");
                    for (auto &s : M2MSensorConfig::SensorGroup().SensorAlarm) {
                        Poco::JSON::Object error_tagObj(true);
                        // MyLogErr("erorrrrrrrrrrrr %s",s.Tag.c_str());
                        if (strncmp(s.Tag.c_str(), "RoomTemperature",15) == 0){
                            error_tagObj.set(s.Tag.c_str(), "Not read data"); 
                            error_tags.add(error_tagObj); 
                        }
                    }
                }
                else{
                    MyLogDebug("Temp of Rom is: %.3f ",romTemp);
                    for (auto &s : M2MSensorConfig::SensorGroup().SensorAlarm) {
                        Poco::JSON::Object tagObj(true);
                        if (strncmp(s.Tag.c_str(), "RoomTemperature",15) == 0){
                            tagObj.set(s.Tag.c_str(), romTemp);
                            good_tags.add(tagObj);
                        }
                    }
                }
                if (outDoorTemp == -2 || outDoorTemp == -1){
                    MyLogErr("Read Out door temp from DS18B20 is error");
                    for (auto &s : M2MSensorConfig::SensorGroup().SensorAlarm) {
                        Poco::JSON::Object error_tagObj(true);
                        // MyLogErr("erorrrrrrrrrrrr %s",s.Tag.c_str());
                        if (strncmp(s.Tag.c_str(), "OutdoorTemperature",18) == 0){
                            error_tagObj.set(s.Tag.c_str(), "Not read data");  
                            error_tags.add(error_tagObj);
                        }
                    }
                }
                else{
                    MyLogDebug("Temp of Out Door is: %.3f ",outDoorTemp);
                    for (auto &s : M2MSensorConfig::SensorGroup().SensorAlarm) {
                        Poco::JSON::Object tagObj(true);
                        if(strncmp(s.Tag.c_str(), "OutdoorTemperature",18) == 0){
                            tagObj.set(s.Tag.c_str(), outDoorTemp);
                            good_tags.add(tagObj);
                        }
                        
                    }
                }

                for (auto &r : M2MSensorConfig::DFRelayState().DFRelay) {
                    Poco::JSON::Object tagObj(true);
                    if (r.Index == 1){
                        tagObj.set(r.Tag.c_str(), r.State);
                    } else if(r.Index == 2){
                        tagObj.set(r.Tag.c_str(), r.State);
                    } 
                    good_tags.add(tagObj);
                }
                dataPoint.set("error_tags", error_tags);
                dataPoint.set("good_tags", good_tags);
                dataPoint.set("high_alarm", high_alarm);
                dataPoint.set("low_alarm", low_alarm);
                dataPoint.stringify(msg, 0);
                MyLogDebug("DataPoint: %s", msg.str().c_str());

                ComRedis::Local.LPush(REDIS_CH_MCAST_DATA, msg.str());
                
                sleep(interval);
            }
        }
    }
}