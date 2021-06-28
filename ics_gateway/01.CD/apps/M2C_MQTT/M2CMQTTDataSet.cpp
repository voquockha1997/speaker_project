#include <mqtt/async_client.h>
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Object.h"
#include "ComRedis.h"
#include "M2CMQTTDataSet.h"
#include "ComQueue.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_COMMON

RegistrationInfo_T  M2CMQTTDataSet::RegInfo;
MqttBrokerConfig_T  M2CMQTTDataSet::MqttCfg;

M2CMQTTDataSet* M2CMQTTDataSet::pub = NULL;

bool M2CMQTTDataSet::Connect(const COMStr& name)
{
    this->name = name;
    url = (IsSSL ? "ssl://" : "tcp://") + mHost + ":" + std::to_string(mPort);
    MyLogDebug("******* M2CMQTTDataSet::Connect() %s %s", url.c_str(), name.c_str());

    // Ignore mqtt connection if device have not authorized yet
    if (Config != NULL && !Config->IsReady() && this != pub) {
        MyLogDebug("M2CMQTTDataSet::Connect(): configuration is not ready!");
        return false;
    }

    MyLogDebug("M2CMQTTDataSet::Connect %s:%s", url.c_str(), name.c_str());

    Client = new mqtt::async_client(url, name);
    Client->set_callback(*this);
    SubscribeCallbackPtr = new SubscribeCallback(this);

    /**
     * Init security MQTT connection
     */
    if (IsSSL) {
        opt_ssl.set_trust_store(Cert);

        if (!PubKey.empty() && this != pub) {
            opt_ssl.set_key_store(PubKey);
        } else {
            MyLogErr("%s PublicKey is empty", name.c_str());
        }

        if (!PrvKey.empty() && this != pub) {
            opt_ssl.set_private_key(PrvKey);
        } else {
            MyLogErr("%s PrivateKey is empty", name.c_str());
        }

        opt_connect.set_ssl(opt_ssl);
    }

    opt_connect.set_automatic_reconnect(true);
    opt_connect.set_connect_timeout(5);
    opt_connect.set_clean_session(true);
    opt_connect.set_keep_alive_interval(60000000);
    opt_connect.set_mqtt_version(3);

    try {
        mqtt::token_ptr tok_connect = Client->connect(opt_connect);
        MyLogDebug("%s Waiting for connection", url.c_str());
        if (!tok_connect->wait_for(10000)) {
            delete SubscribeCallbackPtr;
            SubscribeCallbackPtr = NULL;

            delete Client;
            Client = NULL;

            mState = DataSetInit;
            return false;
        }
    } catch (const mqtt::exception& exc) {
        MyLogDebug("M2CMQTTDataSet::Connect exception: %s %s", url.c_str(), exc.what());

        delete SubscribeCallbackPtr;
        SubscribeCallbackPtr = NULL;

        delete Client;
        Client = NULL;

        mState = DataSetInit;
        return false;
    }

    // Wait for client event update state
    usleep(500000);

    IsFirstConnect = false;
    return true;
}

bool M2CMQTTDataSet::ConnectSimple(const COMStr& name)
{
    url = "tcp://" + mHost + ":" + std::to_string(mPort);
    MyLogDebug("M2CMQTTDataSet::ConnectSimple %s:%s", url.c_str(), Config->TopicData.c_str());

    this->name = name;
    Client = new mqtt::async_client(url, name.c_str());
    Client->set_callback(*this);
    SubscribeCallbackPtr = new SubscribeCallback(this);
    opt_connect.set_automatic_reconnect(true);
    opt_connect.set_connect_timeout(5);
    opt_connect.set_clean_session(true);
    opt_connect.set_keep_alive_interval(60000000);
    opt_connect.set_mqtt_version(3);

    try {
        mqtt::token_ptr tok_connect = Client->connect(opt_connect);
        MyLogDebug("M2CMQTTDataSet::ConnectSimple %s Waiting for connection", url.c_str());
        if (!tok_connect->wait_for(10000)) {
            delete SubscribeCallbackPtr;
            SubscribeCallbackPtr = NULL;

            delete Client;
            Client = NULL;

            mState = DataSetInit;
            return false;
        }
    } catch (const mqtt::exception& exc) {
        MyLogDebug("M2CMQTTDataSet::ConnectSimple: %s %s", url.c_str(), exc.what());

        delete SubscribeCallbackPtr;
        SubscribeCallbackPtr = NULL;

        delete Client;
        Client = NULL;

        mState = DataSetInit;

        COM_AUDIT(M2CAudit::CompFail, M2CAuditLevel::Critical, "MQTT connection exception " + COMStr(exc.what()));
        return false;
    }

    IsFirstConnect = false;
    return true;
}

bool M2CMQTTDataSet::Consume()
{
    if (Client == NULL || State() != DataSetConnected || SubscribeTopics.empty()) {
        return false;
    }

    pthread_mutex_lock(&mutex_consume);
    try {
        Client->start_consuming();
        Client->subscribe(SubscribeTopics, QOS, nullptr, *SubscribeCallbackPtr)->wait();
        MyLogDebug("M2CMQTTDataSet::Consume Enter: %s %s", url.c_str(), SubscribeTopics.c_str());

        while( 1 ) {
            auto msg = Client->consume_message();
            if (!msg) {
                break;
            }

            if (consume_callback != NULL && res_callback != NULL) {
                consume_callback(this, msg->get_payload_str(), res_callback);
            }
        }

        MyLogDebug("M2CMQTTDataSet::Consume Leave: %s %s", url.c_str(), SubscribeTopics.c_str());
        Client->unsubscribe(SubscribeTopics)->wait();
        Client->stop_consuming();
    } catch (std::exception ex) {
        MyLogErr("M2CMQTTDataSet::Consume: %s", ex.what());
    } catch (...) {
        MyLogErr("M2CMQTTDataSet::Consume: Unknown exception");
    }
    pthread_mutex_unlock(&mutex_consume);
    return true;
}

bool M2CMQTTDataSet::Disconnect()
{
    MyLogDebug("M2CMQTTDataSet::Disconnect() %s", url.c_str());
    if (Client != NULL) {
        if (consuming) {
            StopConsume();
        }

        auto tok_disconnect = Client->disconnect();
        tok_disconnect->wait();

        delete Client;
        Client = NULL;
    }

    mState = DataSetInit;
}

bool M2CMQTTDataSet::Publish(const COMStr& msg, const COMStr& topic)
{
    if (Client == NULL) {
        MyLogWarn("M2CMQTTDataSet::Publish() has not init");
        return false;
    }

    COMStr pmsg = msg;
    bool is_real_dat = false;

    // Wait for connection
    while (State() != DataSetConnected) {
        if (StateSecs() > 300) {
            COM_AUDIT(M2CAudit::CompFail, M2CAuditLevel::Critical,
                      "MQTT lost connection in more than " + std::to_string(StateSecs()) + " seconds");

            exit(1);
        }

        MyLogWarn("M2CMQTTDataSet::Publish() disconnected");
        return false;
    }

    bool wret;
    try {
        mqtt::message_ptr pubmsg;
        if (topic.empty()) {
            //MyLogDebug("%s Topic: %s", name.c_str(), Topic.c_str());
            pubmsg = mqtt::make_message(TopicData, pmsg);
        } else {
            pubmsg = mqtt::make_message(topic, pmsg);
        }
        pubmsg->set_qos(QOS);

        mqtt::delivery_token_ptr tok = Client->publish(pubmsg);
        wret = tok->wait_for(10000); // Wait for action complete in 10s
    } catch (std::exception ex) {
        MyLogErr("Exception %s Topic: %s", name.c_str(), TopicData.c_str());
        return false;
    } catch (...) {
        MyLogErr("Unknown exception %s Topic: %s", name.c_str(), TopicData.c_str());
        return false;
    }

    MyLogDebug("Publish() %s Topic:%s Ret:%s Payload:%s", name.c_str(), topic.empty() ? TopicData.c_str() : topic.c_str(),wret == true ? "Successed" : "Failed", msg.c_str());
    return wret;
}

