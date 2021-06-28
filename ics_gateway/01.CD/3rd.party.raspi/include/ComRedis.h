#ifndef COMMON_REDIS_H_
#define COMMON_REDIS_H_

#include <pthread.h>
#include <hiredis/hiredis.h>

#if (APP_INDEX == APP_MQTT_INDEX || APP_INDEX == APP_SNMP_INDEX)
#define ENABLE_REDIS_LISTENER
#else
#ifdef ENABLE_REDIS_LISTENER
#undef ENABLE_REDIS_LISTENER
#endif
#endif

#ifdef ENABLE_REDIS_LISTENER
#include "ComRedisListener.h"
#endif

#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

typedef bool (*ComRedisCallbackFunc)(void* obj, const COMStr& msg);
struct ComRedisCallback
{
    COMStr                key;
    ComRedisCallbackFunc func;
    void                  *data;
    void                  *owner;
    bool                  running;
    pthread_t             thread;
    pthread_mutex_t       mutex;
    uint                  interval;
};

class ComRedis
{
protected:
    std::map<COMStr, ComRedisCallback*> RListenCallbacks;
#ifdef ENABLE_REDIS_LISTENER
    ComRedisListener*  subcriber = NULL;
#endif

    COMStr  host;
    uint    port;
    redisContext*   context;
    pthread_mutex_t lock_context = PTHREAD_MUTEX_INITIALIZER;
    COMStr  err;

    /*
     * Get Redis context base on redis configuration
     * Note: caller should free the context pointer after using
     */
    redisContext* GetRedisContext();

public:
    ComRedis(const COMStr& host, uint port);
    virtual ~ComRedis()
    {
        for (auto cb : RListenCallbacks) {
            if (cb.second->running) {
                cb.second->running = false;
            }

            delete cb.second;
        }

        RListenCallbacks.clear();

        if (context != NULL) {
            redisFree(context);
        }
    }

    COMStr Get(const COMStr& key);
    COMStr Set(const COMStr& key, const COMStr& val);
    COMStr Publish(const COMStr& key, const COMStr& val);

    COMStr LPush(const COMStr& key, const COMStr& val);
    COMStr RPush(const COMStr& key, const COMStr& val);
    COMStr LPop(const COMStr& key);
    COMStr RPop(const COMStr& key);
    COMStr Left(const COMStr& key);
    uint   LLen(const COMStr& key);
    COMStr Save();

    bool   StartRListen(ComRedisCallback* callback, uint interval = 20000);
    void   StopRListen(const COMStr& key);

    bool   StartRListenEvt(ComRedisCallback* callback, uint interval = 20000);
    void   StopRListenEvt(const COMStr& key);

#ifdef ENABLE_REDIS_LISTENER
    bool   StartSubcribe(ComMessageCallback callback);
    bool   StopSubcribe();
#endif

    static ComRedis Local;

private:
    static void* ExecRListen(void* obj);

    static void* ExecRListenEvt(void* obj);
};

#endif /* COMMON_REDIS_H_ */
