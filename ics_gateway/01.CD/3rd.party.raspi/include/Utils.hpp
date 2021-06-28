#pragma once
#ifdef WIN32
#include<Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <functional>
#include <event.h>
#include <string>
#include <vector>
#include <signal.h>

#include "../Common/ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_GENERAL

namespace Utility
{
    inline std::string convertTime(std::string hex) 
    {
        // copy to tab_reg
        // 14 04 05 01 00 35 12
        char buff[5] = {0,};
        COMStr dev_time;
        // get 14 
        COMStr tmp = hex.substr(0,2);
        int value = (int)strtol(tmp.c_str(), NULL, 16);
        sprintf(buff, "%2d", value);
        dev_time = dev_time + buff;

        tmp = hex.substr(2,2);
        value = (int)strtol(tmp.c_str(), NULL, 16);
        sprintf(buff, "%02d", value);
        dev_time = dev_time + buff;

        dev_time.append("-");

        tmp = hex.substr(4,2);
        value = (int)strtol(tmp.c_str(), NULL, 16);
        sprintf(buff, "%02d", value);
        dev_time = dev_time + buff;

        dev_time.append("-");

        tmp = hex.substr(6,2);
        value = (int)strtol(tmp.c_str(), NULL, 16);
        sprintf(buff, "%02d", value);
        dev_time = dev_time + buff;

        dev_time.append(" ");

        tmp = hex.substr(8,2);
        value = (int)strtol(tmp.c_str(), NULL, 16);
        sprintf(buff, "%02d", value);
        dev_time = dev_time + buff;

        dev_time.append("-");

        tmp = hex.substr(10,2);
        value = (int)strtol(tmp.c_str(), NULL, 16);
        sprintf(buff, "%02d", value);
        dev_time = dev_time + buff;

        dev_time.append("-");

        tmp = hex.substr(12,2);
        value = (int)strtol(tmp.c_str(), NULL, 16);
        sprintf(buff, "%02d", value);
        dev_time = dev_time + buff;

        return dev_time;
    }
    inline std::string convertASCII(std::string hex)
    {
        // remove all 0x and " "
        // remove 0x 
        hex.erase(0,2);
        // remove all "0x "
        while (1) {
            std::string::size_type foundpos = hex.find(COMStr(" 0x")); // sub len
            if ( foundpos != std::string::npos )
                hex.erase(hex.begin() + foundpos, hex.begin() + foundpos + 3);// 3 is sub len
            else {
                break;
            }
        }

        std::string ascii = "";
        for (size_t i = 0; i < hex.length(); i += 2){
            //taking two characters from hex string
            std::string part = hex.substr(i, 2);
            //changing it into base 16
            char ch = stoul(part, nullptr, 16);
            //putting it into the ASCII string
            ascii += ch;
        }
        return ascii;
    }

    // convert address to hex
    inline std::string Int2Hex(const int input)
    {
        char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        
        std::string str;
        uint8_t ch;
        
        // High byte
        ch = (input >> 8) & 0xFF;
        str.append(&hex[(ch & 0xF0) >> 4], 1);
        str.append(&hex[ch & 0xF], 1);

        // Low byte
        ch = input & 0xFF;
        str.append(&hex[(ch & 0xF0) >> 4], 1);
        str.append(&hex[ch & 0xF], 1);
            
        return str;
    } 

    inline std::string ConvertToHex(const uint16_t* pWords, int size)
    {
        char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        
        std::string str;
        uint8_t ch;
        for (int i = 0; i < size; ++i) 
        {
            // High byte
            ch = (pWords[i] >> 8) & 0xFF;
            str.append("0x");
            str.append(&hex[(ch & 0xF0) >> 4], 1);
            str.append(&hex[ch & 0xF], 1);
            // Low byte
            ch = pWords[i] & 0xFF;
            str.append(&hex[(ch & 0xF0) >> 4], 1);
            str.append(&hex[ch & 0xF], 1);
            
            if( i < size - 1)
                str.append(" ");
        }
        return str;
    } 

    inline std::string ConvertToHex(const uint8_t* pBytes, int size)
    {
        char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        
        std::string str;
        uint8_t ch;
        for (int i = 0; i < size; ++i) 
        {            
            ch = pBytes[i];
            str.append(&hex[(ch & 0xF0) >> 4], 1);
            str.append(&hex[ch & 0xF], 1);
            
            if( i < size - 1)
                str.append(" ");
        }
        return str;
    }     
    
    inline std::string MakeFixedLength(std::string str, int len)
    {
        std::string result;
        int strlen, i;
        
        if(str.empty())
            return str;
            
        strlen = (int)str.length();
        if(strlen > len)
            return str.substr(0, len);
        else if(strlen == len)
            return str;
        else
        {                            
            result = str;
            for (i = strlen; i < len; i++)
            {
                result = result.append(" ");
            }
            return result;
        }   
    }
    
    inline void createDirectory(const std::string &dir)
    {
        #ifndef WIN32
        struct stat st = {0};
        if(dir.empty()) return;
        if (stat(dir.c_str(), &st) == -1) 
        {
            mkdir(dir.c_str(), 0700);
        }
        #endif
    }
    template<typename T = int>
    inline void setupLibEventLog(T t = 0)
    {
#if 1
        event_set_log_callback([](int severity, const char *msg)
        {
            switch (severity)
            {
            case EVENT_LOG_DEBUG:
                MyLogDebug("[libevent] %s", msg);
                break;
            case EVENT_LOG_MSG:
                MyLogDebug("[libevent] %s", msg);
                break;
            case EVENT_LOG_WARN:
                MyLogWarn("[libevent] {}", msg);
                break;
            case EVENT_LOG_ERR:
            default:
                MyLogErr("[libevent] {}", msg);
                break;
        }
        });
#endif
    }
    inline void setupSignalHandlers()
    {
#ifndef WIN32
        signal(SIGHUP, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        struct sigaction act;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        act.sa_handler = [](const int sig)
        {
            //MyLogInf("Signal handled: %s", strsignal(sig));
            exit(EXIT_SUCCESS);
        };
        sigaction(SIGTERM, &act, NULL);
        sigaction(SIGINT, &act, NULL);
#endif
    }

    template<typename T = std::string, typename Container = std::vector<T>>
    inline Container split(const T &text, char sep = ',') {
        Container tokens;
        std::size_t start = 0, end = 0;
        while ((end = text.find(sep, start)) != T::npos) {
            T temp = text.substr(start, end - start);
            if (temp != "") tokens.push_back(temp);
            start = end + 1;
        }
        T temp = text.substr(start);
        if (temp != "") tokens.push_back(temp);
        return tokens;
    }
}
namespace RedisParser
{
    static inline const char* findclr(const char *buff, const char *end)
    {
        for (--end; buff < end; ++buff)
        {
            if (buff[0] == '\r' && buff[1] == '\n') return buff;
        }
        return nullptr;
    }
    static inline std::pair<std::string, const char*> parseSimpleString(const char *buff, const char *end)
    {
        auto p = findclr(buff, end);
        if (!p || p > end) return std::make_pair(std::string(), nullptr);
        else return std::make_pair(std::string(buff, p), p + 2);
    }
    static inline std::pair<int, const char*> parseLength(const char *buff, const char *end)
    {
        auto p = findclr(buff, end);
        if (!p || p > end) return std::make_pair(-1, nullptr);
        int len = 0;
        bool negative = false;
        if (buff < p && *buff == '-')
        {
            ++buff;
            negative = true;
        }
        for (; buff < p; ++buff)
        {
            int val = *buff - '0';
            if (val < 0 || val > 9) throw std::logic_error("bulk string length error format");
            len = len * 10 + val;
        }
        if (negative) len = -len;
        return std::make_pair(len, p + 2);
    }
    static inline std::pair<std::string, const char*> parseBulkString(const char *buff, const char *end)
    {
        auto res = parseLength(buff, end);
        auto p = res.second;
        auto len = res.first;
        if ((p == nullptr) || (p + (len + 2) > end)) return std::make_pair(std::string(), nullptr);
        else if(len < 0 )
        { 
            return std::make_pair(std::string(""), p);
        }
        else
            return std::make_pair(std::string(p, len), p + (len + 2));
    }
    static inline std::pair<std::vector<std::string>, const char*> parseArrayReply(const char *buff, const char *end)
    {
        std::pair<std::vector<std::string>, const char*> result;
        result.second = nullptr;
        auto res = parseLength(buff, end);
        auto p = res.second;
        auto len = res.first;
        if (len < 0) throw std::logic_error("redis invalid array size " + std::to_string(len));
        if (p == nullptr || p > end) return result;
        for (int i = 0; i < len; ++i)
        {
            if (p >= end || p == nullptr) return result;
            switch (*p)
            {
            case '+':
            case '-':
            case ':':
            {
                auto item = parseSimpleString(p + 1, end);
                result.first.push_back(std::move(item.first));
                p = item.second;
            }
            break;
            case '$':
            {
                auto item = parseBulkString(p + 1, end);
                result.first.push_back(std::move(item.first));
                p = item.second;
            }
            break;
            default:
                throw std::logic_error("invalid redis type");
                break;
            }
        }
        result.second = p;
        return result;
    }
    static inline size_t parseReply(const std::string &reply, const std::function<void(char, const std::string&)> &onStringCB, const std::function<void(const std::vector<std::string>&)> &onArrayStringCB)
    {
        auto p = reply.c_str(), end = p + reply.size();
        size_t len = 0;
        while (p < end && p != nullptr)
        {
            auto type = *p;
            switch (*p)
            {
            case '+':
            case '-':
            case ':':
            {
                auto result = parseSimpleString(p + 1, end);
                p = result.second;
                if (p) onStringCB(type, result.first);
            }
            break;
            case '$':
            {
                auto result = parseBulkString(p + 1, end);
                p = result.second;
                if (p) onStringCB(type, result.first);
            }
            break;
            case '*':
            {
                auto result = parseArrayReply(p + 1, end);
                p = result.second;
                if (p) onArrayStringCB(result.first);
            }
            break;
            default:
                throw std::logic_error("invalid redis type");
                break;
            }
            if (p) len = p - reply.c_str();
        }
        return len;
    }
};
