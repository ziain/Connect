#ifndef NETWORK_CONNECT_H
#define NETWORK_CONNECT_H

//#include "lwconnect.h"
#include <vector>
#include <iostream>

extern "C"
{
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>

}

#define NET_DEVICE "enp3s0"
#define DEFAULT_ACCEPT_PORT 1234


struct socket_info{
    std::string name;
    sockaddr_in sockaddr;
    int socket_fd;
    unsigned long port;
    char ipv4_addr[16];
};


class KNetwork_Connect
{
public:
    KNetwork_Connect();
    ~KNetwork_Connect();
    virtual int Init(int mode);
    virtual int Send(void* buf, unsigned long length);
    virtual int Recv(void* buf, unsigned long length);



private:
    int _create_server();

private:
    std::vector<socket_info*> socket_local;
    std::vector<socket_info*> socket_remote;

    /* mode
     * bie[0]
     * 0 server
     * 1 client
     * bit[1]
     * 0 Synchronization
     * 1 Asynchronism
     * bit[2]
     * 0 Blocking
     * 1 Non-Blocking
     */
    int m_connect_mode;

    bool m_bSynchronization;
    bool m_bBlocking;
    bool m_bServer;


};

#endif // NETWORK_CONNECT_H
