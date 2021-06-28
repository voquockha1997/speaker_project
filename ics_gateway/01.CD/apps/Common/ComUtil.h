#ifndef COMMON_UTIL_H_
#define COMMON_UTIL_H_

#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <queue>
#include <map>
#include <stdarg.h>
#include <sstream>
#include <sys/stat.h>
#include <pthread.h>
#include <limits>
#include <time.h>

#define COM_STRING_INVALID      "COM_INVALID_STRING"
#define COM_INT_MAX             std::numeric_limits<int>::max()
#define COM_INT_MIN             std::numeric_limits<int>::min()
#define COM_FLOAT_MAX           std::numeric_limits<float>::max()
#define COM_FLOAT_MIN           std::numeric_limits<float>::min()
#define COM_DOUBLE_MAX          std::numeric_limits<double>::max()
#define COM_DOUBLE_MIN          std::numeric_limits<double>::min()
#define COM_NUMBER_INVALID      COM_INT_MAX
#define COM_NUMBER_EMPTY        COM_INT_MIN
#define COM_FLOAT_INVALID       COM_FLOAT_MAX
#define COM_FLOAT_EMPTY         COM_FLOAT_MIN
#define COM_DOUBLE_INVALID      COM_DOUBLE_MAX
#define COM_DOUBLE_EMPTY        COM_DOUBLE_MIN
#define COM_OBJ_INVALID(T)      *(T*)nullptr
#define TOSTR(X)             std::to_string(X)
#define COM_SUPPORTED_SERIAL_PORT   "/dev/ttyS0 /dev/ttyS1 /dev/ttyS2 /dev/ttyS3 /dev/ttyS4 /dev/ttyS5 /dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyUSB2 /dev/ttyUSB3 /dev/ttyUSB4 /dev/ttyUSB5 /dev/ttyUSB6 /dev/ttyXR0 /dev/ttyXR1 /dev/ttyXR2 /dev/ttyXR3 /dev/ttyXR4 /dev/ttymxc1 /dev/ttymxc6"

/**
 * @brief: Bit mask macros
 */
#define BVAL(n)   (1L<<(unsigned int)n)           // number value when enable bit index
#define CBIT(p,n) (p=p&~(1L<<(unsigned int)n))    // Set bit
#define SBIT(p,n) (p=p|(1L<<(unsigned int)n))     // Clear bit
#define GBIT(p,n) ((p)&(1L<<(unsigned int)n))     // Get bit
#define LOG_MAX_LEN 10000

/**
 * @brief: POCO JSON helper
 */
#define JGET(v,j,k,t) \
        if (j->has(k)) {\
            Poco::Nullable<t> nav = j->getNullableValue<t>(k);\
            if (!nav.isNull()) v = nav.value();\
        } else {\
            MyLogDebug("Not existing key %s", k);\
        }

#define JGET_OBJ(v, j, k) \
        if (j->has(k)) {\
            v = j->getObject(k);\
        } else {\
            MyLogDebug("JGET_OBJ: Not existing key %s", k);\
            v = new Poco::JSON::Object(true);\
            j->set(k, *v);\
        }

#define JGET_ARR(v, j, k) \
        if (j.isNull()) {\
            MyLogDebug("JGET ARR null", k);\
        } else if (j->has(k)) {\
            v = j->getArray(k);\
        } else {\
            v = new Poco::JSON::Array();\
            j->set(k, *v);\
            MyLogDebug("JGET_ARR: Not existing key %s %d %d", k, v.referenceCount(), j.referenceCount());\
        }

/**
 * @brief: Common types
 */
typedef std::string                         COMStr;
typedef std::vector<COMStr>                 COMStrVect;
typedef std::pair<COMStr, COMStr>           COMStrPair;
typedef std::vector<COMStrPair>             COMStrPairVect;
typedef std::map<COMStr, COMStr>            COMStrMap;

typedef bool (*ComMessageCallbackFunc)(void* obj, const COMStr& msg);
struct ComMessageCallback
{
    COMStr key;
    ComMessageCallbackFunc func;
    void   *obj;
};

typedef bool (*ComResCallbackFunc)(void* obj, const COMStr& msg);
typedef bool (*ComConsumeCallbackFunc)(void* obj, const COMStr& msg, ComResCallbackFunc res);

//========================================================================
// Default return type
enum COM_RET_T
{
    COM_RET_OK,
    COM_RET_FAIL,
    COM_RET_ERROR
};

//========================================================================
/// @brief The class disallows to copy the instances of classes which are derived
/// from this class
class ComNoCopiable
{
protected:
    //========================================================================
    // Default constructor. Does nothing.
    // Needed here to avoid automatic generation and to hide.
    ComNoCopiable()
    {
    }
    //========================================================================

private:
    //========================================================================
    // Copy constructor and assignment operator.
    // Needed here to avoid automatic generation and to hide.
    // Do not implement to generate extra linker error just in case.
    ComNoCopiable(ComNoCopiable const &);
    ComNoCopiable &operator=(ComNoCopiable const &);
    //========================================================================
};

/// @brief The class disallows to create a instance of classes which derived from
/// this class
class ComNoInstantiable
{
private:
    //========================================================================
    // Default constructor.
    // Needed here to avoid automatic generation and to hide.
    // Do not implement to generate extra linker error just in case.
    ComNoInstantiable();
    //========================================================================
};

/// @brief The class disallows to create and copying the instances of classes
/// which are derived from this class.
class ComNoCopyAndInstantiable: ComNoCopiable, ComNoInstantiable
{
};

//==========================================================================

/**
 * @brief: ComUtil provides general functions for common types
 */
class ComUtil : ComNoCopyAndInstantiable
{
public:
    // String utilities
    static COMStr UtcNowString();
    static COMStr UtcNowTimeString();
    static COMStr UtcString(const time_t *t);
    static COMStr UtcString(const timeval *t);
    static COMStr UtcTimeStampString(const COMStr& input);
    static COMStr UtcString(const tm *t);
    static COMStr TimestampString();
    static COMStr trim(const COMStr& str);
    static COMStrVect split(const COMStr& str, char de);
    static COMStrVect split(const COMStr& str, const COMStr &des);

    template< typename T >
    static inline T StrTo(const COMStr& str, T def)
    {
        std::istringstream iss(str);
        T obj;

        iss >> std::ws >> obj >> std::ws;

        if(!iss.eof())
            return def;

        return (obj);
    }

    template< typename T >
    static inline T StrTo(const COMStr& str)
    {
        std::istringstream iss(str);
        T obj, def;

        iss >> std::ws >> obj >> std::ws;

        if(!iss.eof())
            return def;

        return (obj);
    }

    // File utilities
    static inline bool FileExist(const char* fname)
    {
        struct stat buffer;
        return (stat (fname, &buffer) == 0);
    }

    static inline bool LinkExist(const char* path)
    {
        struct stat buffer;
        return (lstat (path, &buffer) == 0);
    }


    static COMStr FileMD5Sum(const COMStr& path);

    static COMStrVect ListDir(const char* dir);

    // String Pair Utilities
    static inline COMStr PairVal2(const COMStrPairVect& vec, const COMStr& first)
    {
        for (auto p : vec) {
            if (p.first == first) {
                return p.second;
            }
        }

        return "";
    }

    static inline COMStr PairVal1(const COMStrPairVect& vec, const COMStr& second)
    {
        for (auto p : vec) {
            if (p.second == second) {
                return p.first;
            }
        }

        return "";
    }

    static COMStr StrMakeUp(const COMStr& tmp, const COMStrMap& map);
    static COMStr StrFileMakeUp(const COMStr& fTmp, const COMStrMap& map);
    static COMStr StrFile(const COMStr& fTmp);
    static bool   StrToFile(const COMStr& content, const COMStr& fpath);
    static COMStr StrRemove(const COMStr& str, char ch);

    static int getNumber(COMStr val);
    static bool getType(const COMStr& val, COMStr &type, int &size);
    static int getNumberInRange(COMStr val, int min, int max);
    static COMStr getIP(COMStr val);
    static COMStr getHost(COMStr val);
    static COMStr getMAC(COMStr val);
    static COMStr getUrl(COMStr val, COMStr proto = "");
    static COMStr getEnv(COMStr key);
    static bool   setEnv(COMStr key, COMStr val);

    static bool parseIP(const COMStr& addr, COMStr& ip, int& port);
    static bool parseIPWithMask(const COMStr& addr, COMStr& ip, int& mask);
    static bool parseHost(const COMStr& addr, COMStr& host, int& port);
    static bool isSerialPort(const COMStr& addr);

    static void substituteStr(COMStr& orig, COMStr const& search, COMStr const& substitution);

    static COMStr getMacAddress();
};

/**
 * @brief: simple timer class definition
 */
typedef void (*UtilTimerFunc)(void* obj);
struct UtilTimerCallback
{
    void* obj;
    UtilTimerFunc func;
};

typedef std::vector<UtilTimerCallback> UtilTimerCallbacks;

enum ComTimerState { TimerStop, TimerRunning };

class ComTimer
{
private:
    pthread_t       thread_timer;
    ComTimerState  state;
    uint            interval;

    static void* Run(void* runner)
    {
        ComTimer* r = (ComTimer*) runner;
        static time_t t1 = time(0);

        do
        {
            t1 = time(0);
            for (auto c : r->Callbacks) {
                c.func(c.obj);
            }

            uint span = (uint)(difftime(time(0), t1) * 1000000);
            uint remain = r->interval - span;
            if (r->interval >= remain) {
                usleep(remain);
            }
        } while (r->State() == TimerRunning);

        return runner;
    }

    /*
    static void* RunEvt(void* runner)
    {

    }*/

public:
    UtilTimerCallbacks Callbacks;

    ComTimer() : state(TimerStop)
    {
        interval = 0;
    }

    // State setter
    inline void State(ComTimerState st)
    {
        state = st;
    }

    // State getter
    ComTimerState State()
    {
        return state;
    }

    ~ComTimer()
    {
        Stop();
    }

    void Start(uint interval)
    {
        if (State() != ComTimerState::TimerRunning) {
            this->interval = interval;

            State(TimerRunning);
            if (pthread_create(&thread_timer, NULL, Run, (void*)this)) {
                this->Stop();
            }
        }
    }

    /*
    void StartEvt(uint interval)
    {
        if (State() != ComTimerState::TimerRunning) {
            this->interval = interval;

        }
    }*/

    void Stop()
    {
        State(TimerStop);
    }
};

//==========================================================================

class WatchDogParam_T
{
public:
    COMStr name;
    bool   state = true;
    time_t change_time = time(0);
    time_t prev_change_time = time(0);
    COMStr info;

    bool Change(bool st, const COMStr& msg = "");

    inline double ChangeSecs()
    {
        return difftime(time(0), change_time);
    }

    inline void ResetTime() {
        prev_change_time = change_time = time(0);
    }
    //WatchDogParam_T(COMStr _name) { name = _name; }
};

class WatchDogSection : public std::map<COMStr, WatchDogParam_T>
{
public:
    COMStr Name;
    COMStr Info;

    WatchDogSection(const COMStr& name) : std::map<COMStr, WatchDogParam_T>()
    {
        Name = name;
    }

    COMStr ToJson();
    bool   FromJson(const COMStr& json);
};

//==========================================================================

/**
 * @brief: define logging group
 */
enum LOG_GROUP_T
{
    LOG_G_COMMON,
    LOG_G_UTIL,
    LOG_G_NOTIFICATION,
    LOG_G_EIPCLIENT,
    LOG_G_IFCONFIG,
    LOG_G_DEVICE_MANAGER,
    LOG_G_COMMAND_RUNNER,
    LOG_G_COMMAND,
    LOG_G_COMMAND_INTERFACE,
    LOG_G_M2MINTERFACE,
    LOG_G_M2MINTERFACE_CONFIG,
    LOG_G_CONFIG,
    LOG_G_MODBUS_DEVICE,
    LOG_G_CLOUD_CLIENT,
    LOG_G_MAX_NUM
};

#define LOG_G_GENERAL LOG_G_MAX_NUM

static COMStrVect ComLogGroups = {
    "Common",
    "Util",
    "Notification",
    "EIPClient",
    "IfConfig",
    "DeviceManager",
    "CommandRunner",
    "CLI_Command",
    "CLI_Interface",
    "M2MInterface",
    "M2MInterfaceConfig",
    "Configuration"
    "ModbusDevice",
    "CloudClient",
    "General"
};

enum LOG_PRIO_T
{
    PRIO_DEBUG,
    PRIO_ERROR,
    PRIO_WARNING,
    PRIO_INFORM,
    PRIO_MAX_NUM
};

enum LOG_OUT_T
{
    LOG_O_STD,
    LOG_O_FILE,
    LOG_O_POCO,
    LOG_O_MAX_NUM
};

//==========================================================================

/**
 * @brief: Global logger which wraps various logger
 */
class MyLogger : ComNoCopyAndInstantiable
{
    static unsigned int mask;
    static unsigned int prio;
    static unsigned int out;
    static unsigned int maxIndex;
    static unsigned int maxFileSize;
    static unsigned int obsCount;
    static COMStr       filePath;
    static FILE*        fdOut;

    static bool                 stopped;
    static pthread_mutex_t      lock;
    static pthread_t            log_thread;
    static std::queue<char*>    log_queue;

    static void* doLog(void* obj);
public:

    /**
     * @brief: init logger package
     */
    static COM_RET_T init();
    static COM_RET_T stop() {
        stopped = true;
        return COM_RET_OK;
    }

    /**
     * @brief: set file path
     */
    inline static void setFile(const char* file)
    {
        filePath = COMStr(file);
        obsCount = 1000;
    }

    inline static void checkMaxSize()
    {
        if (fdOut) {
            fclose(fdOut);
            fdOut = NULL;
        }

        if (!fdOut) {
            fdOut = fopen(filePath.c_str(), "r");
        }

        if (fdOut) {                                // File is existing
            fseek(fdOut,0, SEEK_END);
            uint size = (uint)ftell(fdOut);

            fclose(fdOut);
            fdOut = NULL;

            if (size >= maxFileSize) {              // File is upper max size
                FILE* fd;
                uint i = 0;

                for (i = 0; i < maxIndex; i++) {
                    fd = fopen((filePath + std::to_string(i)).c_str(), "r");
                    if (fd) {
                        fclose(fd);
                    } else {
                        break;
                    }
                }

                if (i < maxIndex) {
                    rename(filePath.c_str(), (filePath + std::to_string(i)).c_str());
                } else {
                    for (i = 1; i < maxIndex; i++) {
                        rename((filePath.c_str() + std::to_string(i)).c_str(), (filePath.c_str() + std::to_string(i-1)).c_str());
                    }

                    rename(filePath.c_str(), (filePath + std::to_string(maxIndex - 1)).c_str());
                }
            }

            fdOut = fopen(filePath.c_str(), "a");
        }
    }

    /**
     * @brief: get logger group mask
     */
    inline static unsigned int getMask()
    {
        return (mask);
    }

    /**
     * @brief: set logging group mask
     */
    inline static void setMask(unsigned int _mask)
    {
        mask = _mask;
    }

    /**
     * @brief: get logger group mask
     */
    inline static unsigned int getPrio()
    {
        return (prio);
    }

    /**
     * @brief: set logging group mask
     */
    inline static void setPrio(unsigned int _prio)
    {
        prio = _prio;
    }

    /**
     * @brief: get logger group mask
     */
    inline static unsigned int getOut()
    {
        return (out);
    }

    /**
     * @brief: set logging group mask
     */
    inline static void setOut(unsigned int _out)
    {
        out = _out;
    }

    /**
     * @brief: enable/disable logging for group
     */
    inline static void setGroup(LOG_GROUP_T group, bool enable)
    {
        if (enable) {
            SBIT(mask, group);
        } else {
            CBIT(mask, group);
        }
    }

    /**
     * @brief: get logging state of a logging group
     */
    inline static bool getGroup(LOG_GROUP_T group)
    {
        return (GBIT(mask, group));
    }

    static void print(LOG_PRIO_T type, LOG_GROUP_T group, const char* fmt, ...);
};

/**
 * @brief: macros for logging
 */
#define MyLogInf(...)  \
        MyLogger::print(LOG_PRIO_T::PRIO_INFORM, COMMON_LOG_GROUP, __VA_ARGS__)

#define MyLogWarn(...)  \
        MyLogger::print(LOG_PRIO_T::PRIO_WARNING, COMMON_LOG_GROUP, __VA_ARGS__)

#define MyLogErr(...)  \
        MyLogger::print(LOG_PRIO_T::PRIO_ERROR, COMMON_LOG_GROUP, __VA_ARGS__)

#define MyLogDebug(...)  \
        MyLogger::print(LOG_PRIO_T::PRIO_DEBUG, COMMON_LOG_GROUP, __VA_ARGS__)

#define MyLogT(name,x) \
        try { x; }catch(const std::exception &e){MyLogErr("%s:%s", name, e.what());}catch(...){MyLogErr("%s: Unknow exception", name);}

#define COM_LOG_G_ENABLE    (MyLogger::getGroup(COMMON_LOG_GROUP)==1)
#define COM_STD_LOG_ENABLE  (MyLogger::getOut()&BVAL(LOG_O_STD))

//==========================================================================

/**
 * @brief: Util for console output
 */

#define CSL_MAX_LEN     10000
#define CSL_CLR         "\033c"
#define CSL_CR          "\n"
#define CSL_RED         "\x1b[31m"
#define CSL_GREEN       "\x1b[32m"
#define CSL_YELLOW      "\x1b[33m"
#define CSL_BLUE        "\x1b[34m"
#define CSL_MAGENTA     "\x1b[35m"
#define CSL_CYAN        "\x1b[36m"
#define CSL_LGRAY       "\e[37m"
#define CSL_DGRAY       "\e[90m"
#define CSL_CL_RESET    "\x1b[0m"

#define CSL_BOLD        "\e[1m"
#define CSL_ITALIC      "\e[3m"
#define CSL_ULINE       "\e[4m"
#define CSL_STRIKE      "\e[1m"
#define CSL_FMT_RESET   "\e[0m"
#define CSL_BEEP        "\007"

class ComOut : ComNoCopyAndInstantiable
{
public:
    static bool Enable;

    static COMStr makeup(const char* fmt, ...);

    /**
     * @brief: print console output with green color
     */
    static void info(const char* fmt, ...);

    /**
     *@brief: print processing info to console
     */
    static void process(const char* fmt, ...);

    /**
     *@brief: print success info to console
     */
    static void ok(const char* fmt, ...);

    /**
     * @brief: print fail info to console
     */
    static void fail(const char* fmt, ...);

    /**
     * @brief: print console output with yellow color
     */
    static void warning(const char* fmt, ...);

    /**
     * @brief: print console output with red color
     */
    static void error(const char* fmt, ...);

    /**
     * @brief: print console output with normal font
     */
    static void print(const char* fmt, ...);

    /**
     * @brief: clear screen
     */
    inline static void clr()
    {
        printf(CSL_CLR);
    }

    inline static void cr()
    {
        printf(CSL_CR);
    }

    inline static void reset()
    {
        printf(CSL_CL_RESET CSL_FMT_RESET);
    }

    static void beep();
};

//==========================================================================

/**
 * @brief: crypto utility class definition
 */
class UtilCrypt : ComNoCopyAndInstantiable
{
public:
    static COMStr RandomPassphase();
    static bool Encrypt(const COMStr& file, const COMStr& pass);
    static bool Decrypt(const COMStr& file, const COMStr& pass);
    static COMStr EncryptString(const COMStr& text, const COMStr& pass, const COMStr& alg = "bf");
    static COMStr DecryptString(const COMStr& text, const COMStr& pass, const COMStr& alg = "bf");
};

//==========================================================================

/**
 * @brief: macros for console output
 */
#define COM_CI ComOut::info
#define COM_CO ComOut::ok
#define COM_CF ComOut::fail
#define COM_CW ComOut::warning
#define COM_CE ComOut::error
#define COM_CG ComOut::process
#define COM_CP ComOut::print
#define COM_CLR ComOut::clr
#define COM_CCR ComOut::cr
#define COM_CRS ComOut::reset
#define COM_BEEP ComOut::beep

#endif /* COMMON_UTIL_H_ */

/**
 * Any file include ComUtil.h, it have to redefine the current logging group
 * COMMON_LOG_GROUP
 */
#ifdef COMMON_LOG_GROUP
#undef COMMON_LOG_GROUP
// use LOG_CRR_GROUP macro to redefine the current logging group!
#endif // LOG_CRR_GROUP
