#ifndef NETWORK_CONNECT_H
#define NETWORK_CONNECT_H

//#include "lwconnect.h"
#include <list>
#include <vector>
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
#define DEFAULT_CONNECT_PORT 1234

#define CLEAR(T) memset(T,0,sizeof(*T))


struct stream_info {
    char name[20];
    char ipv4_addr[16];
    unsigned short port;
    unsigned short index;
};

struct socket_info{
    stream_info stream;
    sockaddr_in sockaddr;
    int socket_fd;
};



typedef std::pair<std::string,socket_info*> SOCK_INFO_PAIR;
typedef std::pair<std::string,socket_info*>* PSOCK_INFO_PAIR;
typedef std::list<PSOCK_INFO_PAIR> SOCK_LIST;

typedef std::pair<stream_info*,KThread*> THREAD_PAIR;
typedef std::pair<stream_info*,KThread*>* P_THREAD_PAIR;
typedef std::vector<P_THREAD_PAIR> THREAD_LIST;


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
    int _create_server(stream_info* remote_info);
    int _create_client();

    int _init_local();
    int _init_server(socket_info* local_socket,stream_info* stream_msg,int remote_index);


public:
    virtual void* listen_thread(void* arg);
    virtual void* server_thread(void* arg);
    virtual void* client_thread(void* arg);

private:
    static void* _listen_thread(void* arg);
    static void* _server_thread(void* arg);
    static void* _client_thread(void* arg);


private:
    socket_info local;

    SOCK_LIST remote;
    SOCK_LIST server;
    THREAD_LIST thread_server;

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

    int m_client_count;




    pthread_t m_listen_thread;
    pthread_t m_client_thread;


};

#endif // NETWORK_CONNECT_H
