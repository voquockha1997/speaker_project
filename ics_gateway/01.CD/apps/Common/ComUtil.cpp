#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <regex>

#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <dirent.h>

#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CipherKey.h"
#include "Poco/Crypto/CryptoStream.h"

//#include "Poco/Logger.h"
#include "Poco/Util/Application.h"

#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Query.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Net/NetworkInterface.h"

#include "ComProcess.h"
#include "ComCmdRunner.h"
#include "ComUtil.h"
#include "ComDefinition.h"

#define COMMON_LOG_GROUP LOG_G_UTIL

std::map<char, char> numeric_map =
{
    {'0' ,'A'},
    {'1' ,'B'},
    {'2' ,'C'},
    {'3' ,'D'},
    {'4' ,'E'},
    {'5' ,'F'},
    {'6' ,'G'},
    {'7' ,'H'},
    {'8' ,'I'},
    {'9' ,'J'},
};

using Poco::Net::NetworkInterface;

COMStr ComUtil::UtcNowString()
{
    timeval curTime;
    gettimeofday(&curTime, NULL);

    char buf[80];
    char fmt[100] = "%Y-%m-%d %X";
    strftime(buf, sizeof buf, fmt, gmtime(&curTime.tv_sec));
    sprintf(buf, "%s.%ld", buf, curTime.tv_usec);

    return COMStr(buf);
}

COMStr ComUtil::UtcNowTimeString()
{
    timeval curTime;
    gettimeofday(&curTime, NULL);

    char buf[80];
    char fmt[100] = "%X";
    strftime(buf, sizeof buf, fmt, gmtime(&curTime.tv_sec));
    //sprintf(buf, "%s.%ld", buf, curTime.tv_usec);

    return COMStr(buf);
}

COMStr ComUtil::UtcString(const time_t *t)
{
    tm* now_tm= gmtime(t);
    char buf[80];
    char fmt[100] = "%Y-%m-%d %X";
    strftime(buf, sizeof buf, fmt, now_tm);
    //sprintf(buf, "%s.%ld", buf, now_tmtv_usec);
    return COMStr(buf);
}

COMStr ComUtil::UtcString(const timeval *t)
{
    char buf[80] = {0,};
    char fmt[100] = "%Y-%m-%d %X";
    strftime(buf, sizeof(buf), fmt, gmtime(&t->tv_sec));
    sprintf(buf, "%s.%ld", buf, t->tv_usec);
    return COMStr(buf);
}

COMStr ComUtil::UtcTimeStampString(const COMStr& input)
{
    char buf[80]= {0,};
    uint32_t usec = 0;
    sscanf (input.c_str(),"%19[^\n].%d",buf,&usec);

    struct tm tm;
    strptime(buf, "%Y-%m-%d %X", &tm);

    memset(buf, 0x00, sizeof(buf));
    char fmt[100] = "%Y-%m-%dT%X";
    strftime(buf, sizeof(buf), fmt, &tm);
    sprintf(buf, "%s.%03d+0000", buf, usec != 0 ? usec/1000 : usec );

    return COMStr(buf);
}
COMStr ComUtil::TimestampString()
{
    time_t now= time(0);
    tm* now_tm= gmtime(&now);
    char buf[50];
    char fmt[100] = "%Y%m%d_%H%M%S";
    strftime(buf, sizeof buf, fmt, now_tm);
    return COMStr(buf);
}

COMStr ComUtil::trim(const COMStr& s)
{
   auto  wsfront = std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   return COMStr(wsfront, std::find_if_not(s.rbegin(), COMStr::const_reverse_iterator(wsfront),[](int c) {return std::isspace(c);}).base());
}

COMStrVect ComUtil::split(const COMStr& str, char de)
{
    std::string s;
    std::istringstream f(str);
    COMStrVect ret;

    while (getline(f, s, de)) {
        ret.push_back(s);
    }

    return ret;
}

COMStrVect ComUtil::split(const COMStr& str, const COMStr &des)
{
    char* c = (char*)str.c_str();
    char token[100];
    char* ptok = token;
    COMStrVect toks;

    while(1) {
        if (des.find(*c) != COMStr::npos) {
            *ptok = '\0';

            if (ptok != token) {
                toks.push_back(COMStr(token));
                ptok = token;
            }
        } else {
            *ptok++ = *c;
        }

        if (*c == '\0')
            break;

        c++;
    }

    return toks;
}

COMStrVect ComUtil::ListDir(const char* dir)
{
    struct dirent *entry;
    DIR *dp;
    COMStrVect ret;

    dp = opendir(dir);
    if (dp == NULL) {
        return ret;
    }

    while ((entry = readdir(dp))) {
        if (0 != strcmp(entry->d_name, ".") &&
            0 != strcmp(entry->d_name, ".."))
        {
            ret.push_back(entry->d_name);
        }
    }

    closedir(dp);
    return ret;
}

COMStr ComUtil::FileMD5Sum(const COMStr& path)
{
    if (ComUtil::FileExist(path.c_str())) {
        COMStr cmd = "md5sum " + path;
        COMStr out = CommandRunner::ExecRead(cmd);
        if (!out.empty()) {
            return ComUtil::split(out, ' ')[0];
        }
    }

    return "";
}

// Get string from template string and list of parameters
COMStr ComUtil::StrMakeUp(const COMStr& tmp, const COMStrMap& params)
{
    if (tmp.empty()) {
        return "";
    }

    if (params.size() == 0) {
        return tmp;
    }

    COMStr ret = tmp;
    COMStr key, val;
    size_t pos;

    for (auto p : params) {
        key = "[" + p.first + "]";
        while (std::string::npos != (pos = ret.find(key))) {
            //printf("%s [%s:%d]\n", ret.c_str(), key.c_str(), pos);
            ret = ret.replace(pos, key.length(), p.second);
        }
    }

    return ret;
}

COMStr ComUtil::StrFile(const COMStr& fTmp)
{
    std::ifstream ifsTmp(fTmp);
    COMStr tmp((std::istreambuf_iterator<char>(ifsTmp)), (std::istreambuf_iterator<char>()));
    ifsTmp.close();
    return tmp;
}

// Get string from template file and list of parameters
COMStr ComUtil::StrFileMakeUp(const COMStr& fTmp, const COMStrMap& params)
{
    std::ifstream ifsTmp(fTmp);
    COMStr tmp((std::istreambuf_iterator<char>(ifsTmp)), (std::istreambuf_iterator<char>()));
    ifsTmp.close();

    return StrMakeUp(tmp, params);
}

bool ComUtil::StrToFile(const COMStr& content, const COMStr& fpath)
{
    std::fstream fs;
    fs.open(fpath.c_str(), std::fstream::out);
    if (fs.is_open()) {
        //fs << content.c_str();
        fs << content;
        fs.flush();
        fs.close();
        return true;
    }

    return false;
}

COMStr ComUtil::StrRemove(const COMStr& str, char ch)
{
    COMStr result;
    char c;

    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] != c) result += str[i];
    }

    return result;
}

/*
 * Get number from string
 *
 * @param:
 *      val:    the string to convert
 *
 * @return: the converted number
 *          COM_NUMBER_EMPTY:   string is empty
 *          COM_NUMBER_INVALID: string is not number
 */
int ComUtil::getNumber(COMStr val)
{
    if (val == "") {
        return COM_NUMBER_EMPTY;
    }

    int ret = COM_NUMBER_INVALID;

    try {
        char* end;
        if(val.size() > 2 && val[0] == '0' && (val[1] == 'x' || val[1] == 'X'))
            ret = std::strtoul(val.c_str(), &end, 16);
        else
            ret = std::strtoul(val.c_str(), &end, 10);

        if(end == val.c_str()) {
            ret = COM_NUMBER_INVALID;
        }
    } catch (std::exception e) {
        ret = COM_NUMBER_INVALID;
        std::cerr <<"Exception: " <<e.what() <<std::endl;
    }

    return ret;
}

/*
 * Get customized data type
 *
 * @param:
 *    val:      the string to convert
 *    type(out) base customized type
 *    size(out) size if input is array type
 *
 * @return: success or not
 */
bool ComUtil::getType(const COMStr& val, COMStr &type, int &size)
{
    COMStrVect toks = split(val, "()");
    if (toks.size() <= 0) {
        return false;
    }

    type = toks[0];
    size = 1;

    if (toks.size() > 1) {
        size = getNumber(toks[1]);
    }

    return true;
}

/*
 * Get number in range from string
 *
 * @param:
 *      val: the string to convert
 *      min: minium value
 *      max: maximum value
 *
 * @return: the converted number
 *      COM_NUMBER_EMPTY:   string is empty
 *      COM_NUMBER_INVALID: string is not number or out of range
 */
int ComUtil::getNumberInRange(COMStr val, int min, int max)
{
    int ret = getNumber(val);

    if (ret == COM_NUMBER_EMPTY || ret == COM_NUMBER_INVALID) {
        return ret;
    }

    if (ret < min || ret > max) {
        return COM_NUMBER_INVALID;
    }

    return ret;
}

/*
 * Get the IP address from string
 *
 * @param: given string
 * @return: IP address
 *      COM_STRING_INVALID: Invalid IP format
 */
COMStr ComUtil::getIP(COMStr val)
{
    COMStrVect toks = ComUtil::split(val, '.');
    if (toks.size() != 4) {
        return COM_STRING_INVALID;
    }

    int num;
    for (auto tok = toks.begin(); tok != toks.end(); tok++) {
        num = getNumberInRange(*tok, 0, 255);
        if (num == COM_NUMBER_INVALID || num == COM_NUMBER_EMPTY) {
            return COM_STRING_INVALID;
        }
    }

    return val;
}

/*
 * @brief: Get the host name from url
 *
 * @param: given string
 * @return: host name
 *      COM_STRING_INVALID: Invalid host format
 */
COMStr ComUtil::getHost(COMStr val)
{
    try {
        COMStr exp = "(.*://)?([\\w_\\-]+(?:(?:\\.[\\w_\\-]+)+))([\\w.,@?^=%&:/~\\+#\\-]*[\\w@?^=%&/~\\+#\\-])?(:[0-9]+)?$";
        std::regex rgx(exp);
        std::smatch match;

        if (std::regex_match(val, match, rgx)) {
#ifdef ENABLE_COM_UTIL_TEST
            for (uint i = 0; i < match.size(); i++) {
                printf("%d-%s ", i, match[i].str().c_str());
            }
#endif
            return match[2].str();
        } else {
            MyLogErr("getHost() not match:  %s", val.c_str());
        }
    } catch (std::exception e) {
        // do nothing
        MyLogErr("exception: %s", e.what());
    }

    return COM_STRING_INVALID;
}

/*
 * @brief: Get MAC address from given string
 *
 * @param: given string
 * @return: MAC Address
 *      COM_STRING_INVALID: Invalid host format
 */
COMStr ComUtil::getMAC(COMStr val)
{
    try {
        COMStr exp = "(([0-9a-f]{2}[:-]){5}[0-9a-f]{2})";
        std::regex rgx(exp);
        std::smatch match;

        if (std::regex_match(val, match, rgx)) {
#ifdef ENABLE_COM_UTIL_TEST
            for (uint i = 0; i < match.size(); i++) {
                printf("%d-%s ", i, match[i].str().c_str());
            }
#endif
            return match[1].str();
        } else {
            MyLogErr("getMAC() not match:  %s", val.c_str());
        }
    } catch (std::exception e) {
        // do nothing
        MyLogErr("getMAC() %s exception: %s", val.c_str(), e.what());
    }

    return COM_STRING_INVALID;
}

/*
 * Get the URL string from string
 *
 * @param: given string
 * @return: validated URL
 *      COM_STRING_INVALID: Invalid URL format
 */
COMStr ComUtil::getUrl(COMStr val, COMStr proto)
{
    try {
        COMStr exp = "^" + (proto.empty() ? ".*" : proto + "://") + "([\\w_\\-]+(?:(?:\\.[\\w_\\-]+)+))([\\w.,@?^=%&:/~\\+#\\-]*[\\w@?^=%&/~\\+#\\-])?(:[0-9]+)?$";
        std::regex rgx(exp);
        std::smatch match;

        if (std::regex_match(val, match, rgx)) {
            //printf("match:      %s\n", match[0].str().c_str());
            return match[0].str();
        } else {
            MyLogDebug("getUrl() not match:  %s\n", val.c_str());
        }
    } catch (std::exception e) {
        // do nothing
        MyLogErr("getUrl() %s exception: %s", val.c_str(), e.what());
    }

    return COM_STRING_INVALID;
}

/*
 * Get system environment variable
 *
 * @param:  key string
 * @return: value string
 */
COMStr ComUtil::getEnv(COMStr key)
{
    if (key.empty()) {
        return "";
    }

    const char* env = std::getenv(key.c_str());
    return (env == NULL ? "" : COMStr(env));
}

bool ComUtil::setEnv(COMStr key, COMStr val)
{
    if (key.empty()) {
        return false;
    }

    return (0 != setenv(key.c_str(), val.c_str(), true));
}

bool ComUtil::parseIP(const COMStr& addr, COMStr& ip, int& port)
{
    try {
        COMStr exp = "^([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)(:[0-9]+)?$";
        std::regex rgx(exp);
        std::smatch match;

        if (std::regex_match(addr, match, rgx)) {
            ip = match[1].str();
            if (match[2].str().empty()) {
                port = 0;
            } else {
                port = ComUtil::StrTo<int>(match[2].str().substr(1, match[2].str().size() - 1), 0);
            }
            return true;
        } else {
            MyLogDebug("parseIP() not match %s", addr.c_str());
            return false;
        }
    } catch (std::exception e) {
        MyLogErr("parseIP() %s exception: %s", addr.c_str(), e.what());
    }

    return false;
}

bool ComUtil::parseIPWithMask(const COMStr& addr, COMStr& ip, int& mask)
{
    try {
        COMStr exp = "^([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)(/[0-9]+)?$";
        std::regex rgx(exp);
        std::smatch match;

        if (std::regex_match(addr, match, rgx)) {
            ip = match[1].str();
            if (match[2].str().empty()) {
                mask = 0;
            } else {
                mask = ComUtil::StrTo<int>(match[2].str().substr(1, match[2].str().size() - 1), 0);
            }
            return true;
        } else {
            MyLogDebug("parseIPWithMask() not match %s", addr.c_str());
            return false;
        }
    } catch (std::exception e) {
        MyLogErr("parseIPWithMask() %s exception: %s", addr.c_str(), e.what());
    }

    return false;
}

bool ComUtil::parseHost(const COMStr& addr, COMStr& host, int& port)
{
    try {
        COMStr exp = "^([\\w_\\-]+(?:(?:\\.[\\w_\\-]+)+))([\\w.,@?^=%&:/~\\+#\\-]*[\\w@?^=%&/~\\+#\\-])?(:[0-9]+)?$";
        std::regex rgx(exp);
        std::smatch match;

        if (std::regex_match(addr, match, rgx)) {
            host = match[1].str();
            if (match[2].str().empty()) {
                port = 0;
            } else {
                port = ComUtil::StrTo<int>(match[2].str().substr(1, match[2].str().size() - 1), 0);
            }
            return true;
        } else {
            return false;
        }
    } catch (std::exception e) {
        // do nothing
        MyLogErr("parseHost() %s exception: %s", addr.c_str(), e.what());
    }

    return false;
}

bool ComUtil::isSerialPort(const COMStr& addr)
{
    COMStrVect ports = ComUtil::split(COMStr(COM_SUPPORTED_SERIAL_PORT), ' ');
    if (std::find(ports.begin(), ports.end(), addr) != ports.end()) {
        return true;
    }

    return false;
}

/*
 * @brief: Get MAC address from interface
 *
 * @param: N/A
 * @return: MAC Address
 */
COMStr ComUtil::getMacAddress()
{
    char macAddr[30] = "\0";
    char macAddrFirst[30] = "\0";
    bool is_zero_mac = true;

    const NetworkInterface::Map map = NetworkInterface::map(false, false);
    for(NetworkInterface::Map::const_iterator it = map.begin(); it != map.end(); ++it) {
        if (strlen(macAddrFirst) == 0 || is_zero_mac) {
            NetworkInterface::MACAddress mac(it->second.macAddress());
            macAddrFirst[0] = '\0';

            for (auto k : mac) {
                if (k != 0) is_zero_mac = false;
                sprintf(macAddrFirst, "%s:%02x", macAddrFirst, k);
            }
        }

        if ( it->second.name() == DEV_LAN1_ITF) {
            NetworkInterface::MACAddress mac(it->second.macAddress());

            for (auto k : mac) {
                sprintf(macAddr, "%s:%02x", macAddr, k);
            }

            break;
        }
    }

    COMStr mac = (strlen(macAddr) == 0 ? macAddrFirst : macAddr);
    // remove ":"
    mac.erase(0, 1);
    MyLogDebug("GetFirstMACAddress(): %s", mac.c_str());
    return mac;
}

bool WatchDogParam_T::Change(bool st, const COMStr& msg)
{
    // MyLogDebug("WatchDogParam_T::Change() %s %s => %s", name.c_str(), state ? "true" : "false", st ? "true" : "false");
    info = msg;
    if (st == state) return false;
    state = st;
    prev_change_time = change_time;
    change_time = time(0);
    return true;
}


COMStr WatchDogSection::ToJson()
{
    // MyLogDebug("WatchDogSection::ToJson() %s", Name.c_str());
    //printf("WatchDogSection::ToJson()\n");
    Poco::JSON::Object obj(true);
    obj.set("Name", Name);
    obj.set("Info", Info);

    for (auto &it : *this) {
        WatchDogParam_T p = it.second;

        Poco::JSON::Object jit(true);
        jit.set("Name", p.name);
        jit.set("Info", p.info);
        jit.set("State", p.state);
        jit.set("ChangeTime", p.ChangeSecs());
        obj.set(it.first, jit);
    }

    std::ostringstream os;
    obj.stringify(os, 0);

    //MyLogDebug("%s", os.str().c_str());
    return os.str();
}

bool WatchDogSection::FromJson(const COMStr& json)
{
    // MyLogDebug("WatchDogSection::FromJson() %s", Name.c_str());
    // MyLogDebug("%s", json.c_str());

    Poco::JSON::Parser parse;
    Poco::Dynamic::Var result = parse.parse(json);
    Poco::JSON::Object::Ptr pobj = result.extract<Poco::JSON::Object::Ptr>();
    JGET(Info, pobj, "Info", COMStr);

    Poco::JSON::Object::Iterator it;
    COMStr key, info;
    bool   state, changed;

    for (it = pobj->begin(); it != pobj->end(); it++) {
        key = it->first;
        if (key == "Info" || key == "Name")
            continue;

        //MyLogDebug("WatchDogSection::FromJson() %s key=%s", Name.c_str(), key.c_str());
        Poco::JSON::Object::Ptr pit = pobj->getObject(key);
        JGET(state, pit, "State", bool);
        JGET(info, pit, "Info", COMStr);

        if (this->count(key) > 0) {
            JGET((*this)[key].name, pit, "Name", COMStr);
            JGET((*this)[key].info, pit, "Info", COMStr);
            changed = (*this)[key].Change(state, info);
        } else {
            WatchDogParam_T wd;
            JGET(wd.name, pit, "Name", COMStr);
            JGET(wd.info, pit, "Info", COMStr);
            wd.Change(state, info);
            (*this)[wd.name] = wd;
            changed = true;
        }

#if APP_INDEX == APP_MANAGER_INDEX
        if (ComProcess::Proto == NodeKeys::Manager && changed) {
            MyLogWarn("%s", info.c_str());
        }
#endif
    }

    return true;
}

//=============================================================================
unsigned int MyLogger::mask =  0;
unsigned int MyLogger::prio =  0;
unsigned int MyLogger::out  =  BVAL(LOG_O_FILE) | BVAL(LOG_O_STD);
unsigned int MyLogger::maxIndex    = 10;
unsigned int MyLogger::maxFileSize = 5242880; // 5MB
unsigned int MyLogger::obsCount    = 0;

COMStr              MyLogger::filePath   =  "./initilize.log";
pthread_mutex_t     MyLogger::lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t           MyLogger::log_thread;
std::queue<char*>   MyLogger::log_queue;
bool                MyLogger::stopped = true;
FILE*               MyLogger::fdOut = NULL;

void* MyLogger::doLog(void* obj)
{
    while (!stopped) {
        pthread_mutex_lock(&lock);
        if (log_queue.size() <= 0) {
            pthread_mutex_unlock(&lock);
            usleep(200000);
            continue;
        }

        char* entry = log_queue.front();
        log_queue.pop();
        pthread_mutex_unlock(&lock);

        if (GBIT(out, LOG_O_STD)) {
            printf("%s\n", entry);
        }

        if (GBIT(out, LOG_O_FILE)) {
            if (obsCount >= 50000) {
                obsCount = 0;
                checkMaxSize();
            }

            obsCount++;

            if (!fdOut) {
                fdOut = fopen(filePath.c_str(), "a");
            }

            if (fdOut) {
                fprintf(fdOut, "%s\n", entry);
                fclose(fdOut);
                fdOut = NULL;
            } else {
                printf("MyLogger::print() fail to open file %s\n", filePath.c_str());
            }
        }

        free(entry);
        usleep(10000);
    }

    return NULL;
}

COM_RET_T MyLogger::init()
{
    MyLogger::setMask(65535);
    MyLogger::setPrio(65535);
    MyLogger::setOut(2);

    stopped = false;
    if (pthread_create(&log_thread, NULL, MyLogger::doLog, NULL)) {
        return COM_RET_FAIL;
    }

    return COM_RET_OK;
}

void MyLogger::print(LOG_PRIO_T type, LOG_GROUP_T group, const char* fmt, ...)
{
    if (stopped)
        return;

    if (!GBIT(mask, group)|| !GBIT(prio, type))
        return;

    char msg[LOG_MAX_LEN];
    char hdr[100];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, LOG_MAX_LEN, fmt, args);
    va_end(args);

    // The implementation wraps Poco::Logger
    // It is easy to change to other library or self defined file print
    const char* priostr;
    switch (type) {
    case LOG_PRIO_T::PRIO_DEBUG:
        priostr = "[D]";
#ifndef ComUtil_DISABLE_POCO
        if (GBIT(out, LOG_O_POCO))
            Poco::Util::Application::instance().logger().debug(msg);
#endif
        break;

    case LOG_PRIO_T::PRIO_ERROR:
        priostr = "[E]";
#ifndef ComUtil_DISABLE_POCO
        if (GBIT(out, LOG_O_POCO))
            Poco::Util::Application::instance().logger().error(msg);
#endif
        break;

    case LOG_PRIO_T::PRIO_WARNING:
        priostr = "[W]";
#ifndef ComUtil_DISABLE_POCO
        if (GBIT(out, LOG_O_POCO))
            Poco::Util::Application::instance().logger().warning(msg);
#endif
        break;

    case LOG_PRIO_T::PRIO_INFORM:
    default:
        priostr = "[I]";
#ifndef ComUtil_DISABLE_POCO
        if (GBIT(out, LOG_O_POCO))
            Poco::Util::Application::instance().logger().information(msg);
#endif
        break;
    }

    msg[LOG_MAX_LEN-1] = '\0';
    snprintf(hdr, 100, "%-30s:%-20s%5s ", ComUtil::UtcNowString().c_str(), ComLogGroups[group].c_str(), priostr);
    uint len = strlen(msg) + strlen(hdr);

    pthread_mutex_lock(&lock);
    char* omsg = (char*)malloc(len + 1);
    snprintf(omsg, len + 1, "%s%s", hdr, msg);
    omsg[len] = '\0';
    log_queue.push(omsg);
    pthread_mutex_unlock(&lock);
}

//===============================================================

bool ComOut::Enable = true;

COMStrMap CoutMakup = {
        /* Format makeup */
        {"b",       CSL_BOLD },
        {"i",       CSL_ITALIC },
        {"u",       CSL_ULINE},
        {"s",       CSL_STRIKE},
        {"-f",      CSL_FMT_RESET },
        /* Color makeup */
        {"rd",      CSL_RED },
        {"bl",      CSL_BLUE },
        {"gr",      CSL_GREEN },
        {"yl",      CSL_YELLOW},
        {"mg",      CSL_MAGENTA},
        {"cy",      CSL_CYAN},
        {"-c",      CSL_CL_RESET},
};

COMStr ComOut::makeup(const char* fmt, ...)
{
    char msg[CSL_MAX_LEN];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, LOG_MAX_LEN, fmt, args);
    va_end(args);

    return ComUtil::StrMakeUp(msg, CoutMakup);
}

void ComOut::info(const char* fmt, ...)
{
    if (!Enable) return;

    char msg[CSL_MAX_LEN];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, LOG_MAX_LEN, fmt, args);
    va_end(args);

    COMStr msgstr = ComUtil::StrMakeUp(msg, CoutMakup);
    printf("%s\n", msgstr.c_str());
}

void ComOut::ok(const char* fmt, ...)
{
    if (!Enable) return;

    char msg[CSL_MAX_LEN];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, LOG_MAX_LEN, fmt, args);
    va_end(args);

    COMStr msgstr = ComUtil::StrMakeUp(msg, CoutMakup);
    printf(CSL_BOLD CSL_GREEN "%s\n" CSL_CL_RESET CSL_FMT_RESET, msgstr.c_str());
}

void ComOut::fail(const char* fmt, ...)
{
    if (!Enable) return;

    char msg[CSL_MAX_LEN];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, LOG_MAX_LEN, fmt, args);
    va_end(args);

    COMStr msgstr = ComUtil::StrMakeUp(msg, CoutMakup);
    printf(CSL_BOLD CSL_RED "%s\n" CSL_CL_RESET CSL_FMT_RESET, msgstr.c_str());
}

void ComOut::process(const char* fmt, ...)
{
    if (!Enable) return;

    char msg[CSL_MAX_LEN];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, LOG_MAX_LEN, fmt, args);
    va_end(args);

    COMStr msgstr = ComUtil::StrMakeUp(msg, CoutMakup);
    printf(CSL_YELLOW "%s\n" CSL_CL_RESET, msgstr.c_str());
}

void ComOut::warning(const char* fmt, ...)
{
    if (!Enable) return;

    char msg[CSL_MAX_LEN];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, LOG_MAX_LEN, fmt, args);
    va_end(args);

    COMStr msgstr = ComUtil::StrMakeUp(msg, CoutMakup);
    printf(CSL_BOLD CSL_YELLOW "%s\n" CSL_CL_RESET CSL_FMT_RESET, msgstr.c_str());
}

void ComOut::error(const char* fmt, ...)
{
    char msg[CSL_MAX_LEN];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, LOG_MAX_LEN, fmt, args);
    va_end(args);

    COMStr msgstr = ComUtil::StrMakeUp(msg, CoutMakup);
    printf(CSL_BOLD CSL_RED "%s\n" CSL_CL_RESET CSL_FMT_RESET, msgstr.c_str());
}

void ComOut::print(const char* fmt, ...)
{
    if (!Enable) return;

    char msg[CSL_MAX_LEN];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, LOG_MAX_LEN, fmt, args);
    va_end(args);

    COMStr msgstr = ComUtil::StrMakeUp(msg, CoutMakup);
    printf("%s", msgstr.c_str());
}

void ComOut::beep()
{
    int ms = 5000;
    int freq = 440;

    int fd = open("/dev/console", O_WRONLY);
    ioctl(fd, KIOCSOUND, (ms<<16 | 1193180/freq));
    //ioctl(fd, KIOCSOUND, 0);
    close(fd);
}

/*
 * @brief:
 *        Substitute all of "serach" string within the "orig" string with
 *        "substitution" string
 *
 * @param:
 *      orig        : input and output string after substitution
 *      search      : token to be replaced
 *      substitution: replacement string
 *
 */
void
ComUtil::substituteStr(COMStr& orig, COMStr const& search, COMStr const& substitution)
{
  std::size_t pos = 0;
  while((pos = orig.find(search, pos)) != std::string::npos) {
    orig.replace(pos, search.length(), substitution);
    pos += substitution.length();
  }
}

//=============================================================================
COMStr UtilCrypt::RandomPassphase()
{
    COMStr pass;
    char key;
    std::srand(time(0) ^ getpid());

    for (int i = 0; i < 64; ++i) {
        int randomChar = rand() % (26+26+10);

        if (randomChar < 26)
            key = 'a' + randomChar;
        else if (randomChar < 26+26)
            key = 'A' + randomChar - 26;
        else
            key = '0' + randomChar - 26 - 26;

        pass += key;
        wait(0);
    }

    return pass;
}

bool UtilCrypt::Encrypt(const COMStr& file, const COMStr& pass)
{
    MyLogDebug("Enter: Encrypt\n");
#ifdef COM_ENCRYPT_ENABLE
    Poco::Crypto::CipherFactory &factory = Poco::Crypto::CipherFactory::defaultFactory();
    Poco::Crypto::CipherKey cipherKey("bf", pass);
    Poco::Crypto::Cipher* pCipher = factory.createCipher(cipherKey);

    std::ifstream in(file, std::ios::in);
    if(!in.is_open()) {
        MyLogErr("file in cannot save %s", file.c_str());
        return false;
    }

    std::string data;
    data.append(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    in.close();

    const std::string encrypt_string(pCipher->encryptString(data));
    std::ofstream ot(file, std::ios::out);
    if (!ot.is_open()) {
        MyLogErr("file out cannot save %s", file.c_str());
        return false;
    }

    ot <<encrypt_string;
    ot.close();

    delete pCipher;
#endif

    MyLogDebug("Exit: Encrypt\n");
    return true;
}

bool UtilCrypt::Decrypt(const COMStr& file, const COMStr& pass)
{
    MyLogDebug("Enter: Decrypt\n");

#ifdef COM_DECRYPT_ENABLE
    Poco::Crypto::CipherFactory &factory = Poco::Crypto::CipherFactory::defaultFactory();
    Poco::Crypto::CipherKey cipherKey("bf", pass);
    Poco::Crypto::Cipher* pCipher = factory.createCipher(cipherKey);

    std::ifstream in(file, std::ios::in);
    MyLogDebug("Open in file %s\n", file.c_str());
    if(!in.is_open()) {
        MyLogErr("file in cannot save %s", file.c_str());
        return false;
    }

    std::string data;
    data.append(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    in.close();

    const std::string decrypted_string(pCipher->decryptString(data));
    std::ofstream ot(file, std::ios::out);
    MyLogDebug("Open out file %s\n", file.c_str());
    if (!ot.is_open()) {
        MyLogErr("file out cannot save %s", file.c_str());
        return false;
    }

    MyLogDebug("Input string %s\n", decrypted_string.c_str());
    ot <<decrypted_string;

    MyLogDebug("Close out file\n");
    ot.close();

    delete pCipher;
#endif

    MyLogDebug("Exit: Decrypt\n");
    return true;
}

#define ComUtil_CRYPT_IV "0000000000000000"
COMStr UtilCrypt::DecryptString(const COMStr& text, const COMStr& pass, const COMStr& alg)
{
#ifndef ComUtil_DISABLE_POCO
    Poco::Crypto::CipherFactory &factory = Poco::Crypto::CipherFactory::defaultFactory();
    COMStr dstr;

    if (alg == "bf") {
        Poco::Crypto::CipherKey cipherKey(alg, pass);
        Poco::Crypto::Cipher* pCipher = factory.createCipher(cipherKey);
        dstr = pCipher->decryptString(text, Poco::Crypto::Cipher::ENC_BASE64);
        delete pCipher;
    } else {
        COMStr ivStr(ComUtil_CRYPT_IV);
        Poco::Crypto::Cipher::ByteVec iv{ivStr.begin(), ivStr.end()};
        Poco::Crypto::Cipher::ByteVec passwordKey{ pass.begin(), pass.end() };
        Poco::Crypto::CipherKey cipherKey(alg, passwordKey, iv);

        Poco::Crypto::Cipher* pCipher = factory.createCipher(cipherKey);
        dstr = pCipher->decryptString(text, Poco::Crypto::Cipher::ENC_BASE64);
        delete pCipher;
    }

    return dstr;
#else
    return text;
#endif
}

COMStr UtilCrypt::EncryptString(const COMStr& text, const COMStr& pass, const COMStr& alg)
{
#ifndef ComUtil_DISABLE_POCO
    COMStr estr;
    Poco::Crypto::CipherFactory &factory = Poco::Crypto::CipherFactory::defaultFactory();

    if (alg == "bf") {
        Poco::Crypto::CipherKey cipherKey(alg, pass);
        Poco::Crypto::Cipher* pCipher = factory.createCipher(cipherKey);
        estr = pCipher->encryptString(text, Poco::Crypto::Cipher::ENC_BASE64);
        delete pCipher;
    } else {
        COMStr ivStr(ComUtil_CRYPT_IV);
        Poco::Crypto::Cipher::ByteVec iv{ivStr.begin(), ivStr.end()};
        Poco::Crypto::Cipher::ByteVec passwordKey{ pass.begin(), pass.end() };
        Poco::Crypto::CipherKey cipherKey(alg, passwordKey, iv);

        Poco::Crypto::Cipher* pCipher = factory.createCipher(cipherKey);
        estr = pCipher->encryptString(text, Poco::Crypto::Cipher::ENC_BASE64);
        delete pCipher;
    }

    return estr;
#else
    return text;
#endif
}

//=============================================================================


#ifdef ENABLE_COM_UTIL_TEST
//g++ -DENABLE_COM_UTIL_TEST -DComUtil_DISABLE_POCO --std=c++11 -o util ./CommandRunner.cpp ./ComUtil.cpp -lpthread
int main(int argc, char **argv)
{
    MyLogDebug("Something is success");
    MyLogDebug("Something is failed");

    COMStr url = "https://demo-api.com";
    COMStr url1 = "demo-mqtt.com";
    MyLogDebug("%s => %s", url.c_str(), ComUtil::getUrl(url).c_str());
    MyLogDebug("%s => %s", url1.c_str(), ComUtil::getUrl(url1).c_str());

    COMStr mac = "a8:1e:84:76:8f:a0";
    COMStr mac1 = "a8:1e:84:76:8:a0";
    COMStr mac2 = "aa8:1e:84:76:8:a0";
    MyLogDebug("%s => %s", mac.c_str(), ComUtil::getMAC(mac).c_str());
    MyLogDebug("%s => %s", mac1.c_str(), ComUtil::getMAC(mac1).c_str());
    MyLogDebug("%s => %s", mac2.c_str(), ComUtil::getMAC(mac2).c_str());
    return 0;
}

#endif
