#ifndef BASE_CONFIG_H_
#define BASE_CONFIG_H_
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Query.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/UUIDGenerator.h"
#include "ComDefinition.h"
#include "ComUtil.h"
#include "ComRedis.h"

#ifndef COMMON_LOG_GROUP
#define COMMON_LOG_GROUP LOG_G_CONFIG
#endif

class ComConfig
{
public:
    void Init(const COMStr& name = "", bool readOnly = true);

    virtual ~ComConfig() {}

    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    bool is_lock = false;

    COMStr Name;
    Poco::JSON::Object::Ptr JConf;
    Poco::JSON::Object::Ptr JConfDraft;
    bool IsDraft;
    bool ReadOnly;

    inline bool Lock() {
        pthread_mutex_lock(&lock);
        is_lock = true;
        return is_lock;
    }

    inline bool Unlock()
    {
        pthread_mutex_unlock(&lock);
        is_lock = false;
        return true;
    }

    virtual void Clear() {
        MyLogErr("ComConfig::Clear(): Invalid usage");
    }

    virtual bool Load(bool draft = false) {
        MyLogErr("ComConfig::Load(): Invalid usage");
        return false;
    }

    /**
     * @brief: Load configuration from a json object
     */
    virtual bool JLoad(Poco::JSON::Object::Ptr jc = NULL, bool draft = false, bool force = false);
    virtual bool JLoadList(Poco::JSON::Array::Ptr jc = NULL, bool draft = false, bool force = false);

    /**
     * @brief: Convert Config object to Json object
     */
    virtual bool JSave() {
        MyLogErr("ComConfig::JSave(): Invalid usage");
        return false;
    }

    /**
     * @brief: Save configuration to database
     */
    virtual bool Save();

    /**
     * @brief: Get full Json string from configuration
     */
    virtual COMStr JStr();
};
#endif /* BASE_CONFIG_H_ */