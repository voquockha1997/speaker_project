#ifndef COMMON_SYSTEM_NETWORK_H_
#define COMMON_SYSTEM_NETWORK_H_

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <pthread.h>
#include "ComUtil.h"

struct DevNetIf
{
    COMStr name;
    COMStr ip;
    COMStr gateway;
    COMStr subnet;
    uint tx_packets;
    uint rx_packets;
    uint tx_bytes;
    uint rx_bytes;
    int  state;

    void dump() {
        if (state == 1) printf(CSL_BOLD CSL_GREEN);
        printf("[%s]\n" CSL_CL_RESET CSL_FMT_RESET, name.c_str());
        printf("    IP:         %s\n", ip.c_str());
        printf("    Subnet:     %s\n", subnet.c_str());
        printf("    Gateway:    %s\n", gateway.c_str());
        printf("    tx_packets: %-10u | rx_packets: %-8u\n", tx_packets, rx_packets);
        printf("    tx_bytes:   %-10u | rx_bytes:   %-8u\n\n", tx_bytes, rx_bytes);
    }
};

class ComSysNetwork
{
    static ComSysNetwork* inst;

    // Default constructor
    ComSysNetwork();

    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

public:
    static ComSysNetwork* Inst() {
        if (inst != NULL) {
            return inst;
        }

        return (inst = new ComSysNetwork());
    }

    // Destructor
    virtual ~ComSysNetwork();

    // Initialize network interface configuration
    void Load();

    // Get default network interface name
    COMStr IfName();

    // IPv4 address getter/setter
    COMStr Ip();

    // subnet mask getter/setter
    COMStr Subnet();

    // gateway getter/setter
    COMStr Gateway();

    DevNetIf& GetItf(const COMStr& itf)
    {
        if (Interfaces.count(itf) == 0) {
            return COM_OBJ_INVALID(DevNetIf);
        }

        return Interfaces[itf];
    }

    void printIf(bool reload = false);
    std::map<COMStr, DevNetIf> Interfaces;

    static double TestConnection(const COMStr& addr);
    static COMStr HostToIp(const COMStr addr);
    static bool   TestTcpConnection(const COMStr& addr, int portNum, int timeout_microseconds = 1000000);
    static bool   TestUriConnection(const COMStr& uri, int timeout_microseconds = 1000000);


    COMStr IfDefault;
};

class ComNetHost
{
public:
    // Default constructor
    ComNetHost();

    // Destructor
    virtual ~ComNetHost();

    // Load hosts configuration
    void Initialize();

    // Save hosts configuration
    void Save();

    // Add new host
    void Add(const COMStr& host, const COMStr& ip);

    // Remove existing host
    void Remove(const COMStr& host);

    // Show hosts information
    void Show();

    COMStrMap   Hosts;

private:
    int         state = 0; // -1:error 0:init - 1:changed
    bool isAcceptIp(const COMStr& ip);
};

class ComPinger
{
private:
    static int32_t checksum(uint16_t *buf, int32_t len)
    {
        int32_t nleft = len;
        int32_t sum = 0;
        uint16_t *w = buf;
        uint16_t answer = 0;

        while(nleft > 1)
        {
            sum += *w++;
            nleft -= 2;
        }

        if(nleft == 1)
        {
            *(uint16_t *)(&answer) = *(uint8_t *)w;
            sum += answer;
        }

        sum = (sum >> 16) + (sum & 0xFFFF);
        sum += (sum >> 16);
        answer = ~sum;
        return answer;
    }

public:
    static double Ping(const COMStr& ip, int pingTimes = 1, int timeout = 500000);
    static double SysPing(const COMStr& ip, int pingTimes = 1, int timeout = 500000);
};

class ComUrl
{
    COMStr          _host;
    unsigned int    _port;

    void            _simple_parse(const COMStr &url);
public:
    ComUrl(const COMStr &url);
    COMStr          getHost() { return _host;}
    unsigned int    getPort() { return _port;}
};

#endif /* COMMON_SYSTEM_NETWORK_H_ */
