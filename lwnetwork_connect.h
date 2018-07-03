#ifndef NETWORK_CONNECT_H
#define NETWORK_CONNECT_H

//#include "lwconnect.h"
#include <list>
#include <vector>
#include <iostream>
#include "lwthread.h"
#include "log4z/LwLogger.h"

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

#ifdef LOG_OPEN

#define NETWORK_MODULE "Network"

#define NET_LOGI(log) LWLOGI(NETWORK_MODULE,log)
#define NET_LOGE(log) LWLOGF(NETWORK_MODULE,log)


#define NET_ASSERT(exp,log) \
do{ \
if(!!exp)   \
NET_LOGE(log);   \
}while(0);


#define NET_ASSERT_RET_INT(exp,log) \
do{ \
NET_ASSERT(exp,log)\
return -1;   \
}while(0);

#define NET_ASSERT_RET_BOOL(exp,log) \
do{ \
NET_ASSERT(exp,log) \
return false;   \
}while(0);




#define NET_THREAD_ASSERT_EXIT(exp,log) \
do{ \
if(!!exp)   \
NET_LOGE(log);   \
pthread_exit((void*)-1); \
}while(0);

#endif

#define NET_DEVICE "enp3s0"
#define DEFAULT_CONNECT_PORT 1234

#define CLEAR(T) memset(T,0,sizeof(*T))


struct device_info {
    char name[20];
    char ipv4_addr[16];
    unsigned short port;
    unsigned short index;
};

struct socket_info{
    device_info device;
    sockaddr_in sockaddr;
    int socket_fd;
};



typedef std::pair<int,device_info*> DEVICE_PAIR;
typedef std::pair<int,device_info*>* PDEVICE_PAIR;
typedef std::list<PDEVICE_PAIR> DEVICE_LIST;

typedef std::pair<int,socket_info*> SOCK_PAIR;
typedef std::pair<int,socket_info*>* PSOCK_PAIR;
typedef std::list<PSOCK_PAIR>   SOCK_LIST;



typedef std::pair<device_info*,KThread*> THREAD_PAIR;
typedef std::pair<device_info*,KThread*>* P_THREAD_PAIR;
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

    int GetDeviceCount();



private:
    int _create_listen();
    int _create_server(int index);
    int _create_client();

    int _init_local();
    int _init_server();


public:
    virtual void* listen_thread(void* arg);
    virtual void* server_thread(void* arg);
    virtual void* client_thread(void* arg);

    void* server_send_thread(void* arg);
    void* server_recv_thread(void* arg);

private:
    static void* _listen_thread(void* arg);
    static void* _server_thread(void* arg);
    static void* _client_thread(void* arg);

    static void* _server_send_thread(void* arg);
    static void* _server_recv_thread(void* arg);





public:



private:
    device_info local;
    DEVICE_LIST remote_device;


    socket_info server_socket;
    std::vector<socket_info*> remote_socket;


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


    int m_remote_index;
    int m_client_count;

    char send_buffer[1024];
    char recv_buffer[1024];


    pthread_t m_listen_thread;
    pthread_t m_client_thread;

    KThread m_send_protect;
    KThread m_recv_protect;

    KThread m_client_count_proetct;


};

#endif // NETWORK_CONNECT_H
