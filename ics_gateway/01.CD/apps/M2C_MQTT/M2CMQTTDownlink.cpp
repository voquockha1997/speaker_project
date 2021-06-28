#include "EventBase.h"
#include "Utils.hpp"
#include "ComCmdRunner.h"
#include "ComUtil.h"
#include "ComCloudInterface.h"
#include "ComManagerClient.h"
#include "ComManagerDownlinkInterface.h"
#include "ComConfig.h"

#include "M2CMQTTDownlink.h"
#include "M2CMQTTConfig.hpp"
#define COMMON_LOG_GROUP LOG_G_COMMON

bool M2CDownlink::ProcessDownlinkConsume(void *obj, const COMStr& msg, ComResCallbackFunc res)
{
    MyLogDebug("M2CDownlink::ProcessDownlinkConsume() %s", msg.c_str());
    COMStr uuid, devId, data, cmd;

    try {
        Poco::JSON::Parser parse;
        Poco::Dynamic::Var elements = parse.parse(msg);
        Poco::JSON::Object::Ptr pObj = elements.extract<Poco::JSON::Object::Ptr>();
        
        uuid = pObj->getValue<COMStr>("ban_tin_id");
        data = pObj->getValue<COMStr>("che_do_phat");

        if (pObj->has("cum_loa_id")) {
            devId = pObj->getValue<COMStr>("cum_loa_id");
        }

        if (MQTTConfig::RegInfo().DevID == devId) {
            // belongs this device
            MyLogDebug("belongs this device");
        } else {
            MyLogDebug("Wrong device ID %s", devId.c_str());
            return true;
        }

        COMStrVect cmds = ComUtil::split(data, ';');
        cmd = cmds[0];

        COMStr respRead = "{\"ban_tin_id\":\"" + uuid + "\"," \
         + "\"che_do_phat\":\"" + cmd + "\"," \
         + "\"cum_loa_id\":\"" + devId + "\"," \
         + "\"thoi_gian_doc\":\"" + ComUtil::UtcNowString() + "\"," \
         + "\"trang_thai\":\"san_sang\"," \
         + "\"data\": [{}]}";

        if (res != NULL) {
            MyLogDebug("respRead=%s\n", respRead.c_str());
            res(obj,respRead);
        }

        // special case reboot and restart service
        if (cmd == M2CCommand::Reboot || cmd == M2CCommand::Restart) {
            COMStr respExec = "{\"ban_tin_id\":\"" + uuid + "\"," \
             + "\"che_do_phat\":\"" + cmd + "\"," \
             + "\"cum_loa_id\":\"" + devId + "\"," \
             + "\"thoi_gian_xu_ly\":\"" + ComUtil::UtcNowString() + "\"," \
             + "\"ket_qua\":true," \
             + "\"thong_bao\":\"successful\"," \
             + "\"du_lieu\": [{}]}";
            if (res != NULL) {
                MyLogDebug("respExec=%s\n", respExec.c_str());
                res(obj,respExec);
                sleep(1); // waiting sent mqtt
            }

            if (cmd == M2CCommand::Reboot) {
                ComReboot(data);
            } else if (cmd == M2CCommand::Restart) {
                ComRestart(data);
            }

            return true;
        }

    } catch (std::exception ex) {
        MyLogErr("M2CDownlink::ProcessDownlinkConsume(): %s unexpected consume message %s", ex.what(), msg.c_str());
        COMStr respRead = "{\"ban_tin_id\":\"" + uuid + "\"," \
         + "\"che_do_phat\":\"" + cmd + "\"," \
         + "\"cum_loa_id\":\"" + devId + "\"," \
         + "\"thoi_gian_doc\":\"" + ComUtil::UtcNowString() + "\"," \
         + "\"trang_thai\":\"error\"," \
         + "\"du_lieu\": [{}]}";
        if (res != NULL) {
            MyLogDebug("respRead=%s\n", respRead.c_str());
            res(obj,respRead);
        }
        return false;
    }

    IPCSEND(MGRDownlinkItf::CloudReq, msg);

    return true;
}

// handle reboot command
void M2CDownlink::ComReboot(const COMStr& data)
{
    MyLogDebug("todo later M2CDownlink::ComReboot");
    COMStr cmd = "reboot";
    CommandRunner::Exec(cmd);
}

// handle Restart command
void M2CDownlink::ComRestart(const COMStr& data)
{
    MyLogDebug("todo later M2CDownlink::ComRestart");
    COMStr cmd = "systemctl restart it5-manager.service";
    CommandRunner::Exec(cmd, true);
}

// 
void M2CDownlink::ComSysInfo(const COMStr& data)
{
    MyLogDebug("todo later M2CDownlink::ComSysInfo");
}


// handle command sent from cloud
void M2CDownlink::ComSysCmd(const COMStr& data)
{
    try {
        COMStrVect params = ComUtil::split(data, ';');
        COMStr svr = params[1];
        uint port = ComUtil::StrTo<uint>(params[2], 0);
        COMStr user = params[3];
        COMStr pwd = params[4];
        COMStr cmd = params[5];
        COMStr body = cmd + "\n";
        body += CommandRunner::ExecRead(cmd);

    } catch (std::exception ex) {
        MyLogErr("M2CDownlink::ComSysCmd() exception: %s", data);
    }
}

