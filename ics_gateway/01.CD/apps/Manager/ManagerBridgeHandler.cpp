#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Query.h"
#include "Poco/Dynamic/Var.h"
#include "ManagerBrigdeHandler.h"
#include "ManagerProxy.h"
#include "ManagerConfig.h"
#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_GENERAL

bool ManagerBridgeHandler::ProcessBridgeMsg(const void* sender, ComIPCMsg& packet)
{
    // MyLogDebug("ManagerBridgeHandler::ProcessBridgeMsg()");

    COMStr ret = MGRItfStatus::OK;
    COMStr info;
    bool   isSuccess = false;

    try {
        Poco::JSON::Parser parse;
        Poco::Dynamic::Var result = parse.parse(packet.Content);
        Poco::JSON::Object::Ptr pdat = result.extract<Poco::JSON::Object::Ptr>();
        COMStr type = packet.Type;

        if (type == MGRBRGItf::StatusInd) {
            ManagerIPCSend(MGRItf::StatusInd, packet.Content, {NodeKeys::CLI});
        }
    } catch (std::exception ex) {
        MyLogErr("ManagerBridgeHandler::ProcessBridgeMsg() parse json exception %s", ex.what());
        ret = MGRItfStatus::FORMAT_ERROR;
        info = "parse json exception";
    } catch (...) {
        MyLogErr("ManagerBridgeHandler::ProcessBridgeMsg() parse json unknown exception");
        ret = MGRItfStatus::FORMAT_ERROR;
        info = "parse json unknown exception";
    }

ProcessBridgeMsg_Exit:
    return (ret == MGRItfStatus::OK);
}
