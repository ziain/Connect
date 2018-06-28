#include "lwnetwork_connect.h"

KNetwork_Connect::KNetwork_Connect()
{
    m_connect_mode = 0;
}

KNetwork_Connect::KNetwork_Connect(int mode)
{
    m_connect_mode = mode;

}

KNetwork_Connect::~KNetwork_Connect()
{

}

int KNetwork_Connect::Init()
{
    m_bServer = !!(m_connect_mode & 0x01);
    m_bSynchronization = !(m_connect_mode & 0x02);
    m_bBlocking = !(m_connect_mode & 0x04);

    _init_local();

    if(m_bServer)
    {

        _create_listen();
        _create_server();
    }


}

int KNetwork_Connect::Init(int mode)
{
    m_connect_mode = mode;
    KNetwork_Connect::Init();
}



int KNetwork_Connect::Send(void* buf, unsigned long length)
{

}

int KNetwork_Connect::Recv(void* buf, unsigned long length)
{

}

int KNetwork_Connect::_init_local()
{
    memset(local,0,sizeof(socket_info));
    char host_name[20] = {0};
    gethostname(host_name,20);
    local.name = host_name;
    local.socket_fd = socket(AF_INET,SOCK_STREAM,0);

    ifreq ifr;
    memset(&ifr,0,sizeof(ifreq));
    strcpy(ifr.ifr_ifrn.ifrn_name,NET_DEVICE);

    if(ioctl(local.socket_fd, SIOCGIFADDR, &ifr) < 0)
    {
        close(local.socket_fd);
        return -1;
    }
    memcpy(&local.sockaddr,&ifr.ifr_ifru.ifru_addr,sizeof(local.sockaddr));
    strncpy(local.ipv4_addr,inet_ntoa(local.sockaddr.sin_addr),16);
    local.sockaddr.sin_family = AF_INET;

}


void* KNetwork_Connect::listen_thread(void* arg)
{
    unsigned int index = 0;
    socket_info* locak_socket = &local;
    socket_info* listen_socket = new socket_info;
    memset(listen_socket,0,sizeof(socket_info));
    memcpy(listen_socket,locak_socket,sizeof(socket_info));
    listen_socket->sockaddr.sin_port = htons(DEFAULT_CONNECT_PORT);
    listen_socket->port = DEFAULT_CONNECT_PORT;
    listen_socket->socket_fd = socket(AF_INET,SOCK_STREAM,0);

    ::bind(listen_socket->socket_fd, (sockaddr*)&listen_socket->sockaddr,sizeof(sockaddr));
    while(1)
    {
        socket_info* remote_socket = new socket_info;
        memset(remote_socket,0,sizeof(socket_info));
        listen(listen_socket->socket_fd,1024);
        socklen_t a = sizeof(sockaddr);
        remote_socket->socket_fd = accept(listen_socket->socket_fd, (sockaddr*)&remote_socket->sockaddr,&a);

        recv(remote_socket->socket_fd,(void*)remote_socket,sizeof(socket_info),0);
        remote_socket->index = index++;
        SOCK_INFO_PAIR* remote_info = new SOCK_INFO_PAIR(remote_socket->name,remote_socket);
        remote.push_back(remote_info);
    }

    delete listen_socket;
}

void* KNetwork_Connect::server_thread(void* arg)
{
    socket_info* locak_socket = &local;
    socket_info* socket_local_server = new socket_info;
    memset(socket_local_server,0,sizeof(socket_info));
    memcpy(socket_local_server,locak_socket,sizeof(socket_info));
    socket_local_server->sockaddr.sin_port = htons(0);
    socket_local_server->port = 0;
    socket_local_server->socket_fd = socket(AF_INET,SOCK_STREAM,m_bBlocking?0:SOCK_NONBLOCK);



}

void* KNetwork_Connect::client_thread(void* arg)
{
    KNetwork_Connect* connect = (KNetwork_Connect*)arg;

}






void* KNetwork_Connect::_listen_thread(void* arg)
{
    KNetwork_Connect* connect = (KNetwork_Connect*)arg;
    connect->listen_thread(arg);
}

void* KNetwork_Connect::_server_thread(void* arg)
{
    KNetwork_Connect* connect = (KNetwork_Connect*)arg;
    connect->server_thread(arg);
}

void* KNetwork_Connect::_client_thread(void* arg)
{
    KNetwork_Connect* connect = (KNetwork_Connect*)arg;
    connect->client_thread(arg);
}



int KNetwork_Connect::_create_listen()
{
    pthread_create(&m_listen_thread,NULL,_listen_thread,(void*)this);
}

int KNetwork_Connect::_create_server()
{
    pthread_create(&m_server_thread,NULL,_server_thread,(void*)this);
}

int KNetwork_Connect::_create_client()
{
    pthread_create(&m_client_thread,NULL,_client_thread,(void*)this);
}











