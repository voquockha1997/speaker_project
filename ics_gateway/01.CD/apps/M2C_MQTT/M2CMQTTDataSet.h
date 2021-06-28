#ifndef M2C_MQTT_DATA_SET_H_
#define M2C_MQTT_DATA_SET_H_

#include <mqtt/async_client.h>

#include "ComDefinition.h"
#include "ComDataModel.h"
#include "ComRedis.h"
#include "ComConfig.h"

#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

class M2CMQTTDataSet :
        public ComDataSet,
        public virtual mqtt::callback,
        public virtual mqtt::iaction_listener
{
private:
    static RegistrationInfo_T  RegInfo;
    static MqttBrokerConfig_T  MqttCfg;

    // Main configuration is used for cloud connection only
    MqttBrokerConfig_T  *Config = NULL;

    // Security file path
    bool    IsSSL = false;
    COMStr  Cert;
    COMStr  PubKey;
    COMStr  PrvKey;

    mqtt::async_client  *Client = NULL;
    mqtt::connect_options opt_connect;
    mqtt::ssl_options   opt_ssl;

    const long          TIMEOUT = 1000;
    const int           QOS = 1;
    const int           N_RETRY_ATTEMPTS = 5;
    pthread_t           thread_consume;
    pthread_mutex_t     mutex_consume;

    ComConsumeCallbackFunc consume_callback = NULL;
    ComResCallbackFunc res_callback = NULL;

    bool                consuming = false;

public:
    COMStr              SubscribeTopics;
    COMStr              TopicData;
    COMStr              url;
    COMStr              name;
    bool                IsFirstConnect = true;

public:
    static void RefreshCfg() {
        RegInfo.Init();
        MqttCfg.Init();
        ComUtil::StrToFile(RegInfo.PublicKey, MQTT_PUB_KEY_FILE);
        ComUtil::StrToFile(RegInfo.PrivateKey, MQTT_PRV_KEY_FILE);
    }

    M2CMQTTDataSet(MqttBrokerConfig_T* cfg, const COMStr& subcribes = "") :
        Config(cfg), ComDataSet(DataSetMQTT, cfg->Host, cfg->Port, cfg->Name)
    {
        MyLogDebug("Info %s %s %s %d",subcribes.c_str(), cfg->Name.c_str(), cfg->Host.c_str(),cfg->Port);
        SubscribeTopics = subcribes;
        MyLogDebug("Debug %s",Config->TopicData.c_str());
        TopicData = Config->TopicData;
        IsSSL = Config->IsSec;
        Cert = MQTT_CER_KEY_FILE;
        PubKey = MQTT_PUB_KEY_FILE;
        PrvKey = MQTT_PRV_KEY_FILE;
    }

    virtual ~M2CMQTTDataSet()
    {
        if (State() == DataSetConnected) {
            Disconnect();
        }
    }

    virtual bool     Connect(const COMStr& name);
    virtual bool     ConnectSimple(const COMStr& name);

    virtual bool     Disconnect();
    virtual bool     Publish(const COMStr& msg, const COMStr& topic = "");
    virtual bool     Consume();

    /**
     * @brief: Callback functions
     */
    void connected(const COMStr& cause) override
    {
        if (mState != DataSetConnected) {
            mState = DataSetConnected;
            mStateTime = time(0);
        }
    }

    void connection_lost(const COMStr& cause) override
    {
        MyLogDebug("%s Connection lost: %s", name.c_str(), cause.c_str());
        if (mState != DataSetInit) {
            mState = DataSetInit;
            mStateTime = time(0);
        }
    }

    void delivery_complete(mqtt::delivery_token_ptr tok) override
    {
        //MyLogDebug("%s Delivered: [%d]", name.c_str(), tok ? tok->get_message_id() : -1);
    }

    void message_arrived(mqtt::const_message_ptr msg) override
    {
        //MyLogDebug("%s message_arrived: %s", name.c_str(), msg->get_payload_str().c_str());
    }

    void on_failure(const mqtt::token& tok) override
    {
        MyLogDebug("%s on_failure %d", name.c_str(), tok.get_message_id());
    }

    void on_success(const mqtt::token& tok) override
    {
        MyLogDebug("%s on_success", name.c_str());
    }

    /**
     * @brief: The subscribe callback class
     */
    class SubscribeCallback : public virtual mqtt::iaction_listener
    {
        M2CMQTTDataSet* DataSet;

    public:
        SubscribeCallback(M2CMQTTDataSet* ds) : DataSet(ds) {}
        void on_failure(const mqtt::token& tok) override
        {
            MyLogDebug("%s SubscribeCallback::on_failure %d", DataSet->name.c_str(), tok.get_message_id());
        }

        void on_success(const mqtt::token& tok) override
        {
            MyLogDebug("%s SubscribeCallback::on_success", DataSet->name.c_str());
            auto top = tok.get_topics();
            if (top && !top->empty()) {
                for (int i = 0; i < (*top).size(); i++) {
                    MyLogDebug("    %s", (*top)[i].c_str());
                }
            }
        }
    } *SubscribeCallbackPtr = NULL;

    // static M2CMQTTDataSet* INNO()
    // {
    //     if (inno == NULL) {
    //         inno = new M2CMQTTDataSet(&MqttCfg, RegInfo.DevID);
    //         sleep(1); // Waiting for subscribe
    //     }

    //     return inno;
    // }

    static M2CMQTTDataSet* PUB()
    {
        if (pub == NULL) {
            pub = new M2CMQTTDataSet(&MqttCfg, RegInfo.DevID);
            sleep(1); // Waiting for subscribe
        }

        return pub;
    }

private:
    static M2CMQTTDataSet* inno;
    static M2CMQTTDataSet* pub;

    static void* ExecConsume(void* obj)
    {
        M2CMQTTDataSet* ds = (M2CMQTTDataSet*) obj;

        while(ds->consuming) {
            ds->Consume();
            usleep(10000);
        }

        return obj;
    }

public:
    void StartConsume(ComConsumeCallbackFunc callback, ComResCallbackFunc res)
    {
        if (consuming) return;
        mutex_consume = PTHREAD_MUTEX_INITIALIZER;
        consuming = true;
        consume_callback = callback;
        res_callback = res;

        if (pthread_create(&(thread_consume), NULL, ExecConsume, (void*)this)) {
            StopConsume();
        }
    }

    void StopConsume()
    {
        consuming = false;
    }
};

#endif /* M2C_MQTT_DATA_SET_H_ */
