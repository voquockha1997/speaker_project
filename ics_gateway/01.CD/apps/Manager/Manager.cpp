#include "Manager.h"
#include "ManagerConfig.h"
#include "ComProcess.h"
#include "ComRedis.h"
#include "ManagerServer.h"
#include "ManagerProxy.h"
#include "ManagerConsumer.h"
#include "ManagerWatchDog.h"
#include "ComQueue.h"
#include "Poco/Net/NetworkInterface.h"
#include "ManagerDefine.h"
#include "ComUtil.h"

#include <wiringPi.h>

#define COMMON_LOG_GROUP LOG_G_COMMON

ComRedis Manager::CtrlClient(DEFAULT_REDIS_HOST, DEFAULT_REDIS_PORT);

using Poco::Net::NetworkInterface;

bool Manager::Initialize()
{
    ManagerConfig::Initialize();

    // generate serial number if empty
    if (ManagerConfig::RegInfo().DevID.empty()) {
        char macAddrFirst[30] = "\0";
        bool is_zero_mac = true;
        const NetworkInterface::Map map = NetworkInterface::map(false, false);
        for(NetworkInterface::Map::const_iterator it = map.begin(); it != map.end(); ++it) {
            if (strlen(macAddrFirst) == 0 || is_zero_mac) {
                MyLogDebug("Manager::GetFirstMACAddress() First interface %s", it->second.name().c_str());
                NetworkInterface::MACAddress mac(it->second.macAddress());
                macAddrFirst[0] = '\0';

                for (auto k : mac) {
                    if (k != 0) is_zero_mac = false;
                    sprintf(macAddrFirst, "%s:%02x", macAddrFirst, k);
                }
            }
        }
        //
        COMStr mac = COMStr(macAddrFirst);
        mac.erase(0,1);
        COMStr tmp = Poco::UUIDGenerator::defaultGenerator().createFromName(Poco::UUID::x500(), mac).toString();
        MyLogDebug("Final %s Device ID %s",mac.c_str(), tmp.c_str());
        ManagerConfig::RegInfo().DevID = tmp;
        ManagerConfig::RegInfo().Save();
        COM_Queue::RegInfo.DevID = ManagerConfig::RegInfo().DevID;
    }

    
    ManagerWatchDog::Start();
    wiringPiSetupGpio();
    pinMode(ManagerConfig::DFRelayState().Relay1, OUTPUT);
    pinMode(ManagerConfig::DFRelayState().Relay2, OUTPUT);
    MyLogDebug("Get state of relay"); //Exec cmmd
    for (auto &r : ManagerConfig::DFRelayState().DFRelay) {
        digitalWrite(r.Gpio, r.State);//Set relay = state
        MyLogDebug("State of rlay luc init %d", r.State); //Exec cmmd
    }

    // Listen connection requests from child process
    ManagerServer::getInstance(6789);

    auto &proxy = ManagerProxy::getInstance();
    ManagerConsumer consumer(proxy);
    consumer.RegisterData();

    for (auto p : ManagerConfig::Manager().Children) {
        MyLogDebug("%s %s", p.Proto.c_str(), p.Command.c_str());
        ComProcess::Spawn(p.Command, p.Proto, p.MaxSpawnTimes, p.RespawnDelay,true);
    }
    ManagerConfig::Manager().Save();
    return true;
}

bool Manager::FactoryReset(void* obj)
{
    MyLogDebug("Manager::FactoryReset()");
    ManagerConfig::Reset();
}
