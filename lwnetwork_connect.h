#ifndef NETWORK_CONNECT_H
#define NETWORK_CONNECT_H

//#include "lwconnect.h"
#include <list>
#include <iostream>
#include "lwthread.h"


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
    unsigned int index;
    sockaddr_in sockaddr;
    int socket_fd;
    unsigned long port;
    char ipv4_addr[16];
};
typedef std::pair<std::string,socket_info*> SOCK_INFO;
typedef std::pair<std::string,socket_info*>* PSOCK_INFO;
typedef std::list<PSOCK_INFO> SOCK_LIST;


class KNetwork_Connect
{
public:
    KNetwork_Connect();
    KNetwork_Connect(int mode);
    ~KNetwork_Connect();
    virtual int Init();
    virtual int Init(int mode);
    virtual int Send(void* buf, unsigned long length);
    virtual int Recv(void* buf, unsigned long length);



private:
    int _create_listen();
    int _create_server();
    int _create_client();
    int _init_local();


public:
    virtual void* listen_thread(void* arg);
    virtual void* server_thread(void* arg);
    virtual void* client_thread(void* arg);

private:
    static void* _listen_thread(void* arg);
    static void* _server_thread(void* arg);
    static void* _client_thread(void* arg);


private:
    SOCK_LIST local;
    SOCK_LIST remote;

    /* mode
     * bie[0]
     * 0 client
     * 1 server
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

    pthread_t m_listen_thread;
    pthread_t m_server_thread;
    pthread_t m_client_thread;


};

#endif // NETWORK_CONNECT_H
