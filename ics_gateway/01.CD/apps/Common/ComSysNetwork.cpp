#include <stdexcept>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string.h>
#include <algorithm>
#include <regex>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/route.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex>
#include <linux/if_link.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include "ComCmdRunner.h"
#include "ComSysNetwork.h"

#include "ComUtil.h"
#define COMMON_LOG_GROUP LOG_G_IFCONFIG

ComSysNetwork* ComSysNetwork::inst = NULL;

/*
 * Default constructor
 */
ComSysNetwork::ComSysNetwork()
{
    Load();
}

/*
 * Destructor
 */
ComSysNetwork::~ComSysNetwork()
{
}

/*
 * Initialize network interface configuration
 *
 * Note:    when network change, re-call the function to update
 *          network interface configuration.
 *
 * @param:
 *      updateConfig :allow to update editing info or not
 *
 * @return: None
 */
void ComSysNetwork::Load()
{
    pthread_mutex_lock(&lock);

    struct ifaddrs *ifaddr, *ifa;
    int family, s, n;
    char host[NI_MAXHOST];
    COMStr name;

    Interfaces.clear();

    if (-1 == getifaddrs(&ifaddr)) {
        pthread_mutex_unlock(&lock);
        return;
    }

    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        family = ifa->ifa_addr->sa_family;
        name = ifa->ifa_name;
        if (0 == Interfaces.count(name)) {
            Interfaces[name] = DevNetIf();
            Interfaces[name].name = name;
        }

        if (ifa->ifa_netmask != NULL && ifa->ifa_netmask->sa_family == AF_INET) {
            s = getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            Interfaces[name].subnet = host;
        }

        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

            if (s != 0) {
                continue;
            }

            Interfaces[name].ip = host;
        } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
            struct rtnl_link_stats *stats = (struct rtnl_link_stats *)ifa->ifa_data;
            Interfaces[name].tx_packets = stats->tx_packets;
            Interfaces[name].rx_packets = stats->rx_packets;
            Interfaces[name].tx_bytes = stats->tx_bytes;
            Interfaces[name].rx_bytes = stats->rx_bytes;
        }
    }

    freeifaddrs(ifaddr);

    IfDefault = "";

    // Parse /proc/net/route file
    std::ifstream is;
    is.open("/proc/net/route");
    COMStr ln, iface, gateway;
    int idx = 0;
    int state = 0; // 0-init 1-first found if 2-first found default route

    if (is.is_open()) {
        while (!is.eof()) {
            idx++;
            is >> ln;
            //printf("%d: %s : %d\n", idx, ln.c_str(), state);
            switch (idx)
            {
            case 1: // If name
                if (Interfaces.count(ln) > 0) {
                    iface = ln;
                    if (state == 0) state = 1;
                }
                break;
            case 2: // Dest
                if (ln == "00000000") { // any
                    Interfaces[iface].state = 1;
                    if (state == 1) {
                        state = 2; // First found default route entry
                        IfDefault = iface;
                    } else if (state == 2) { // n times found default route entry
                        state = 3;
                    }
                }
                break;
            case 3: // Gateway
                if (state != 0) {
                    gateway = ln;
                    struct in_addr addr;
                    addr.s_addr = std::strtoul(ln.c_str(), NULL, 16);
                    if (Interfaces[iface].gateway.empty() || Interfaces[iface].gateway == "0.0.0.0") {
                        gateway = inet_ntoa(addr);
                        if (gateway != "0.0.0.0") {
                            Interfaces[iface].gateway = gateway;
                        }
                    }
                }
                break;
            case 11:
                idx = 0;
                break;
            default:
                break;
            }
        }
    }

    is.close();
    pthread_mutex_unlock(&lock);
}

/*
 * Get default network interface name
 *
 * @param:  None
 * @return: network interface name
 */
COMStr ComSysNetwork::IfName()
{
    return IfDefault;
}

/*
 * Get IPv4 address
 *
 * @param:  None
 * @return:
 *      IPv4 address of default network interface
 *      Error message if there is no default network interface
 */
COMStr ComSysNetwork::Ip()
{
    return Interfaces.count(IfDefault) == 0 ? "" : Interfaces[IfDefault].ip;
}

/*
 * Get subnet mask
 *
 * @param:  None
 * @return:
 *      subnet mask string
 *      Error message if there is no default network interface
 */
COMStr ComSysNetwork::Subnet()
{
    return Interfaces.count(IfDefault) == 0 ? "" : Interfaces[IfDefault].subnet;
}

/*
 * Dump the network interface information to stdout
 *
 * @param: None
 * @return: None
 */
void ComSysNetwork::printIf(bool reload)
{
    if (reload) {
        Load();
    }

    for (auto& i : Interfaces) {
        i.second.dump();
    }
}

/*
 * Get IP address of activated gateway
 *
 * @param: None
 * @return:
 *      IP address of activated gateway
 *      Error message if procedure is failed
 */
COMStr ComSysNetwork::Gateway()
{
    return Interfaces.count(IfDefault) == 0 ? "" : Interfaces[IfDefault].gateway;
}

double ComSysNetwork::TestConnection(const COMStr& addr)
{
    COMStr ip = HostToIp(addr);
    if (ip != COM_STRING_INVALID)
        return ComPinger::Ping(ip, 1, 1500000);
    else
        return 0;
}

COMStr ComSysNetwork::HostToIp(const COMStr addr)
{
    if (ComUtil::getIP(addr) != COM_STRING_INVALID) {
        return (addr);
    }

    COMStr domain = addr;
    std::regex rgxaddr(".*://(.*)$");
    std::smatch match;
    if (std::regex_search(domain, match, rgxaddr)) {
        domain = match[1].str();
    }

    struct hostent *he = gethostbyname(domain.c_str());
    if (he == NULL) {
        return COM_STRING_INVALID;
    }

    return COMStr(inet_ntoa (*((struct in_addr *) he->h_addr_list[0])));
}

bool ComSysNetwork::TestTcpConnection(const COMStr& addr, int portNum, int timeout_microseconds)
{
    struct addrinfo * res, *rp;
    char port_str[10];
    bool ret = false;
    struct addrinfo hints = { 0 };

    hints.ai_socktype = SOCK_STREAM;

    snprintf(port_str, sizeof (port_str), "%d", portNum);

    if (getaddrinfo(addr.c_str(), port_str, &hints, &res)) {
        MyLogErr("getaddrinfo fail (%s)", strerror(errno));
        return false;
    }

    for (rp = res; rp && !ret; rp = rp->ai_next) {
        struct timeval timeout;
        timeout.tv_sec  = timeout_microseconds / (1000*1000);
        timeout.tv_usec = timeout_microseconds % (1000*1000);

        int sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1) {
            continue;
        }

        if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            MyLogErr("setsockopt failed");
        }

        if (connect(sock, rp->ai_addr, rp->ai_addrlen) != -1) {
            ret = true;
        }

        close(sock);
    }

    freeaddrinfo(res);

    if (ret) {
        MyLogDebug("TcpPortReachable: %s:%d is reachable", addr.c_str(), portNum);
    } else {
        MyLogDebug("TcpPortReachable: %s:%d is NOT reachable", addr.c_str(), portNum);
    }

    return ret;
}

bool ComSysNetwork::TestUriConnection(const COMStr& uri, int timeout_microseconds)
{
    ComUrl restUrl(uri);

    MyLogDebug("TestUriConnection: uri(%s), host : %s, port %u",
        uri.c_str(), restUrl.getHost().c_str(), restUrl.getPort());

    return ComSysNetwork::TestTcpConnection(restUrl.getHost(), restUrl.getPort(), timeout_microseconds);
}

//=========================================================================

ComNetHost::ComNetHost()
{
    Initialize();
}

ComNetHost::~ComNetHost()
{
}

void ComNetHost::Initialize()
{
    std::ifstream is;
    is.open("/etc/hosts");
    COMStr ln, ip, host;
    Hosts.clear();

    state = 0;
    if (!is.is_open()) {
        state = -1;
        return;
    }

    COMStrVect toks;
    while (!is.eof()) {
        getline(is, ln);
        toks = ComUtil::split(ln, ' ');
        if (toks.size() < 2 || toks[0][0] == '#') {
            continue;
        }

        if (!isAcceptIp(toks[0])) {
            continue;
        }

        ip = toks[0]; host = toks[1];
        Hosts[host] = ip;
    }

    is.close();
}

void ComNetHost::Save()
{
    if (state == 0) {
        return;
    }

    state = 0;
    char host[64];
    gethostname(host, 64);
    host[63] = '\0';
    COMStr hostname(host);

    COMStr hdef =   "# The /etc/hosts file\n"
                    "127.0.0.1  localhost\n"
                    "127.0.1.1  " + hostname + "\n\n"
                    "#The following lines are desirable for IPv6 capable hosts\n"
                    "::1     localhost ip6-localhost ip6-loopback\n"
                    "ff02::1 ip6-allnodes\n"
                    "ff02::2 ip6-allrouters\n";

    std::ofstream fhost;
    fhost.open("/var/log/it5/host.tmp");
    fhost << hdef;
    for (auto h : Hosts) {
        fhost << h.second <<" " <<h.first <<std::endl;
    }

    fhost.close();
    COMStr cmd = "cp -f /var/log/it5/host.tmp /etc/hosts";
    CommandRunner::Exec(cmd, false);
}

void ComNetHost::Add(const COMStr& host, const COMStr& ip)
{
    if (!isAcceptIp(ip)) {
        return;
    }

    Hosts[host] = ip;
    state = 1;
}

void ComNetHost::Remove(const COMStr& host)
{
    if (Hosts.count(host) > 0) {
        COMStr ip = Hosts[host];
        Hosts.erase(host);
        state = 1;
    }
}

void ComNetHost::Show()
{
    Initialize();

    if (Hosts.size() == 0) {
        printf("There is not customized host.\n");
        return;
    }

    for (auto h : Hosts) {
        printf("    %-35s%s\n", h.first.c_str(), h.second.c_str());
    }
}

bool ComNetHost::isAcceptIp(const COMStr& ip)
{
    COMStr i = ComUtil::getIP(ip);
    if (i == COM_STRING_INVALID ||
        i == "127.0.0.1" ||
        i == "127.0.1.1" ||
        i == "::1" ||
        i == "ff02::0" ||
        i == "ff02::1" ||
        i == "ff02::2")
    {
        return false;
    }

    return true;
}

double ComPinger::Ping(const COMStr& ip, int pingTimes, int timeout)
{
    unsigned long daddr;
    int pid = getpid();
    daddr = inet_addr(ip.c_str());

    // Raw socket
    int sockfd = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        MyLogErr("ComPinger::Ping() open socket");
        return 0;
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = daddr;
    int addrlen = sizeof(servaddr);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))) {
        MyLogErr("ComPinger::Ping() set socket timeout");
        return 0;
    }

    char packet[sizeof(icmphdr)];
    char inbuf[192];
    icmphdr *pkt = (icmphdr *)packet;
    int sents = 0, sentsz, rcvs = 0, rcvsz;

    for (int i = 0; i < pingTimes; i++) {
        memset(packet, 0, sizeof(packet));
        pkt->type = ICMP_ECHO;
        pkt->code = 0;
        pkt->checksum = 0;
        pkt->un.echo.id = htons(pid & 0xFFFF);
        pkt->un.echo.sequence = i;
        pkt->checksum = checksum((uint16_t *)pkt, sizeof(packet));

        sents++;
        if (0 > (sentsz = sendto(sockfd, packet, sizeof(packet), 0, (sockaddr *)&servaddr, sizeof(sockaddr_in)))) {
            continue;
        } else if (sentsz != sizeof(packet)) {
            MyLogWarn("incorrect send packet %d - %d", sentsz, sizeof(packet));
            continue;
        }

        memset(inbuf, 0, sizeof(inbuf));
        int addrlen = sizeof(sockaddr_in);
        if (0 > (rcvsz = recvfrom(sockfd, inbuf, sizeof(inbuf), 0, (sockaddr *)&servaddr, (socklen_t *)&addrlen))) {
            MyLogWarn("ComPinger::Ping() Receive timeout");
            continue;
        }

        iphdr *iph = (iphdr *)inbuf;
        int hlen = (iph->ihl << 2);
        rcvsz -= hlen;

        pkt = (icmphdr *)(inbuf + hlen);
        int id = ntohs(pkt->un.echo.id);
        if(pkt->type == ICMP_ECHOREPLY) {
            if(id != pid) {
                MyLogWarn("ICMP_ECHOREPLY Incorrect id %d - %d", id, pid);
            }
            rcvs++;
        } else if(pkt->type == ICMP_DEST_UNREACH) {
            int offset = sizeof(iphdr) + sizeof(icmphdr) + sizeof(iphdr);
            if(((rcvsz + hlen) - offset) == sizeof(icmphdr))
            {
                icmphdr *p = reinterpret_cast<icmphdr *>(inbuf + offset);
                id = ntohs(p->un.echo.id);
                if(id != pid) {
                    MyLogWarn("ICMP_DEST_UNREACH Incorrect id %d - %d", id, pid);
                }
            }
        }
    }

    close(sockfd);
    if (rcvs == 0 || sents == 0) {
        return 0;
    }

    return (rcvs/sents);
}

double ComPinger::SysPing(const COMStr& ip, int pingTimes, int timeout)
{
    if (ip.empty()) {
        return 0;
    }

    COMStr cmd = "timeout 1 ping -c " + std::to_string(pingTimes) + " " + ip;
    COMStr retStr = CommandRunner::ExecRead(cmd, false);
    uint rcv, snd;

    try {
        COMStr exp = "(\\d+) packets transmitted, (\\d+) received";
        std::regex rgx(exp);
        std::smatch match;

        if (std::regex_search(retStr, match, rgx)) {
            snd = ComUtil::getNumber(match[1].str());
            rcv = ComUtil::getNumber(match[2].str());
            return (rcv/snd);
        } else {
            MyLogDebug("not matched\n");
        }
    } catch (std::exception e) {
        MyLogErr("SysPing: %s", e.what());
    }

    return 0;
}

ComUrl::ComUrl(const COMStr &url):_host(""), _port(0)
{
    _simple_parse(url);
}

void ComUrl::_simple_parse(const COMStr &url)
{
    /**
     * try don't use POCO lib
     * we just support very basic url of http/https protocol.
     * consider using lib curl >= 7.62.0 for better parser
     */
    COMStr tmp_str;
    std::size_t found;

    // get url schema
    found = url.find("://");
    if (found != COMStr::npos) {

        tmp_str = url.substr(0, found);

        if (strcasecmp(tmp_str.c_str(), "http") == 0) {
            _port = 80;
        } else if (strcasecmp(tmp_str.c_str(), "https") == 0) {
            _port = 443;
        } else {
            /*
             * Hope find the port in url: ex xyz://abc.com:889
             */
            MyLogDebug("ComUrl::_simple_parse: cannot recognize %s schema", tmp_str.c_str());
        }

        // skip scheme
        tmp_str = url.substr(found + 3);
    }  else {
        /*
         * support no schema, ex: abc.com:889
         */
        tmp_str = url;
    }

    // skip url path
    found = tmp_str.find('/');
    if (found != COMStr::npos) {
        tmp_str = tmp_str.substr(0, found);
    }

    found = tmp_str.find(':');
    if (found == COMStr::npos) {
        _host = tmp_str;
    } else {
        _host = tmp_str.substr(0, found);
        _port = atoi(tmp_str.substr(found+1).c_str());
    }
}

#ifdef ENABLE_IFCONFIG_TEST
int main(int argc, char** argv)
{
    ComSysNetwork cfg;
    //cfg.printIf();
    printf("Connection: %f\n", ComSysNetwork::TestConnection("http://www.google.com"));
}

#endif // ENABLE_IFCONFIG_TEST
