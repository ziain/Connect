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
//        _create_server();
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
    socket_info* socket_local_listen = new socket_info;
    memset(socket_local_listen,0,sizeof(socket_info));

    char host_name[20] = {0};
    gethostname(host_name,20);
    socket_local_listen->name = host_name;

    socket_local_listen->socket_fd = socket(AF_INET,SOCK_STREAM,m_bBlocking?0:SOCK_NONBLOCK);

    ifreq ifr;
    memset(&ifr,0,sizeof(ifreq));
    strcpy(ifr.ifr_ifrn.ifrn_name,NET_DEVICE);

    if(ioctl(socket_local_listen->socket_fd, SIOCGIFADDR, &ifr) < 0)
    {
        close(socket_local_listen->socket_fd);
        return -1;
    }

    memcpy(&socket_local_listen->sockaddr,&ifr.ifr_ifru.ifru_addr,sizeof(socket_local_listen->sockaddr));
    strncpy(socket_local_listen->ipv4_addr,inet_ntoa(socket_local_listen->sockaddr.sin_addr),16);
    socket_local_listen->sockaddr.sin_family = AF_INET;
    socket_local_listen->port = DEFAULT_ACCEPT_PORT;
    socket_local_listen->sockaddr.sin_port = htons(DEFAULT_ACCEPT_PORT);
    socket_local_listen->index = 0;

    SOCK_INFO* local_listen = new SOCK_INFO("local_listen", socket_local_listen);
    local.push_back(local_listen);

    if(m_bServer)
    {
        socket_info* socket_local_server = new socket_info;
        memset(socket_local_server,0,sizeof(socket_info));
        memcpy(socket_local_server,socket_local_listen,sizeof(socket_info));
        socket_local_server->sockaddr.sin_port = htons(0);
        socket_local_server->port = 0;
        socket_local_server->index = 1;

        socket_local_server->socket_fd = socket(AF_INET,SOCK_STREAM,m_bBlocking?0:SOCK_NONBLOCK);
        SOCK_INFO* local_server = new SOCK_INFO("local_server",socket_local_server);
        local.push_back(local_server);
    }

}


void* KNetwork_Connect::listen_thread(void* arg)
{
    socket_info* listen_socket;
    unsigned int index = 0;
    for(SOCK_LIST::iterator iter = local.begin();iter != local.end();iter++)
    {
        if((*iter)->first.find("listen") != std::string::npos)
            listen_socket = (*iter)->second;
    }

    ::bind(listen_socket->socket_fd, (sockaddr*)&listen_socket->sockaddr,sizeof(sockaddr));
    while(1)
    {
        socket_info* remote_socket = new socket_info;
        char remote_name[16] = {0};
        memset(remote_socket,0,sizeof(socket_info));

        listen(listen_socket->socket_fd,1024);
        remote_socket->socket_fd = accept(listen_socket->socket_fd, (sockaddr*)&remote_socket->sockaddr,NULL);

        int length = recv(remote_socket->socket_fd,remote_name,16,0);
        remote_socket->name = remote_name;
        remote_socket->index = index++;

        SOCK_INFO* remote_info = new SOCK_INFO(remote_name,remote_socket);
        remote.push_back(remote_info);
    }


}

void* KNetwork_Connect::server_thread(void* arg)
{
    KNetwork_Connect* connect = (KNetwork_Connect*)arg;

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











