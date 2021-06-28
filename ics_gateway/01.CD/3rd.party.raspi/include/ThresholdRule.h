#ifndef CORE_THRESHOLD_RULE_H_
#define CORE_THRESHOLD_RULE_H_
#include <iostream>
#include "ComUtil.h"
#include "Poco/Logger.h"
#include "Poco/Util/Application.h"
#define COMMON_LOG_GROUP LOG_G_M2MINTERFACE_CONFIG
namespace M2MEngine
{
    namespace Rules
    {
        typedef enum
        {
            Good =       0,
            LowAlarm =   1,
            HighAlarm =  (1 << 1)
        } AlarmState;

        typedef struct
        {
            // Name of tag
            std::string TagName;
            // Low threshold
            double LowThreshold;
            // High threshold
            double HigThreshold;
            
            // Data source ("Modbus", "SNMP", ...)
            std::string DataSource;
            // Device type belong to specific data source
            std::string DeviceType;
            
            /*
            // The numbers of minutes from 00:00 that thresholds should be applied
            int StartMinutes;
            // The numbers of minutes from 00:00 that thresholds shouldn't be applied
            int StopMinutes;
            //////////////////////////////////////////////////////////////////////////////
            //  00:00                                                              23:59
            //    ----------------| Thresholds were applied |-------------------------
            //                    ^                         ^
            //                    |                         |
            //
            //                StartMinutes              StopMinutes
            //////////////////////////////////////////////////////////////////////////////
            */
            
        } TagAttribute;
                
        //                  Tag      Tag's attribute
        typedef std::map<std::string, TagAttribute> TagsAttribute;
        //               Device type  Tags's attribute
        typedef std::map<std::string, TagsAttribute> DevTypesAttribute;
                
        class ThresholdRule
        {
        public:
            ThresholdRule();
            ThresholdRule(std::map<std::string, DevTypesAttribute>& tagAtt);
            ~ThresholdRule();
        
            AlarmState VerifyTag(double &outthreshold, const std::string& dataSource, const std::string& devType, const std::string& tagName, double value);
        
            void AddTagAtt(const std::string& dataSource, const std::string& devType, const std::string& tagName, TagAttribute tagAtt);
            void RemoveTagAtt(const std::string& dataSource, const std::string& devType, const std::string& tagName);
            void RemoveAllTagAtts();
        
        private:
            //       Data Source  Tags's attribute
            std::map<std::string, DevTypesAttribute> TagAttributes;
        
        };   // class ThresholdRule
        
    }   // namespace Rules
    
}   // namespace M2MEngine

#endif  // CORE_THRESHOLD_RULE_H_
