#ifndef COMMON_DATA_MODEL_H_
#define COMMON_DATA_MODEL_H_

#include "ComUtil.h"
#include "ComDefinition.h"

//=======================================================================================
class ComDataSet;

/**
 * @brief: ComDataController class is a storage which stores datasets and
 *         database model. And ComDataController provides functions to execute data
 *         transference.
 */
class ComDataController : ComNoCopyAndInstantiable
{
public:
    static std::vector<ComDataSet*> DataSetPtrs;
    static Models_T Models;
    static void Initialize();
    static bool GetToSendRaw(ComDataSet* dsin, ComDataSet* dsout, const DataModel_T& model);
    static bool GetToSend(ComDataSet* dsin, ComDataSet* dsout, const DataModel_T& model);
};

//=======================================================================================

enum ComDataSetState { DataSetInit, DataSetConnected, DataSetError };
enum ComDataSetType { DataSetMQTT, DataSetHTTPS, DataSetRTIDDS, DataSetOSPLDDS, DataSetModbus, DataSetEIP, DataSetRedisQueue };

/**
 * @brief: ComDataSet class define a flexible data storage interface
 */
class ComDataSet
{
protected:
    ComDataSetState mState;
    time_t           mStateTime = time(0);
    ComDataSetType  mType;
    COMStr           mHost, mName;
    uint             mPort;

public:
    ComDataSet()
    {
    }

    ComDataSet(ComDataSetType type, const COMStr& host, uint port, const COMStr& name)
    {
        ComDataController::DataSetPtrs.push_back(this);

        mState = DataSetInit;
        mType = type; mHost = host, mName = name;
        mPort = port;
    }

    virtual ~ComDataSet() {}
    inline ComDataSetState State() { return mState; }
    inline double StateSecs() { return difftime(time(0), mStateTime); }
    inline ComDataSetType  Type()   { return mType; }

    virtual bool     Connect()      { return false; };
    virtual bool     Disconnect()   { return false; };

    virtual COMStr   GetJsonRecord(const DataModel_T& model) { return ""; };
    virtual Record_T GetRecord(const DataModel_T& model)     { return Record_T(); };

    virtual bool     PushJsonRecord(const DataModel_T& model, const COMStr& json) { return false; };
    virtual bool     PushRecord(const DataModel_T& model, const Record_T& record) { return false; };
};

#endif /* COMMON_DATA_MODEL_H_ */
