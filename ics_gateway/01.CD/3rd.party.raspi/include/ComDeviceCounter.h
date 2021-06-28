#ifndef COMMON_DEVICE_COUNTER_H_
#define COMMON_DEVICE_COUNTER_H_
#include <time.h>

#include "ComDefinition.h"
#include "ComUtil.h"

/**
 * @brief: Define device counter entry structure
 */
struct DeviceCounterEntry
{
    COMStr       tag;
    unsigned int ack;
    unsigned int nack;
};

/**
 * @brief: Device Counter class
 */
class DeviceCounter
{
public:
    COMStr Name, DevID;
    unsigned int Interval;
    DeviceState  State;

    DeviceCounter();
    virtual ~DeviceCounter();

    void Ack(const COMStr& tag);
    void Nack(const COMStr& tag);
    COMStr ReportStr();
    void dump();
    void Reset();
    void Analysis();

private:
    std::map<COMStr, DeviceCounterEntry> Entries;
    timeval start;
};

/**
 * @brief: Device counter manager
 */
class DeviceCounterManager : public ComNoCopyAndInstantiable
{
public:
    static std::vector<DeviceCounter*> Counters;
    static COMStr GetReport(const COMStr& devId = "");
    static void Reset();
};


#endif /* COMMON_DEVICE_COUNTER_H_ */
