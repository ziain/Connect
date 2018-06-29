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
    CLEAR(&local);
    gethostname(local.stream.name,20);
    local.socket_fd = socket(AF_INET,SOCK_STREAM,0);

    ifreq ifr;
    CLEAR(&ifr);
    strcpy(ifr.ifr_ifrn.ifrn_name,NET_DEVICE);

    if(ioctl(local.socket_fd, SIOCGIFADDR, &ifr) < 0)
    {
        close(local.socket_fd);
        return -1;
    }
    memcpy(&local.sockaddr,&ifr.ifr_ifru.ifru_addr,sizeof(local.sockaddr));
    strncpy(local.stream.ipv4_addr,inet_ntoa(local.sockaddr.sin_addr),16);
    local.sockaddr.sin_family = AF_INET;

}

int KNetwork_Connect::_init_server(socket_info* local_socket,stream_info* stream_msg,int remote_index)
{
    socket_info* _server_info = new socket_info;
    CLEAR(_server_info);
    memcpy(_server_info,local_socket,sizeof(socket_info));
    _server_info->sockaddr.sin_port = htons(0);
    _server_info->socket_fd = socket(AF_INET,SOCK_STREAM,m_bBlocking?0:SOCK_NONBLOCK);
    ::bind(_server_info->socket_fd,(sockaddr*)&_server_info->sockaddr,sizeof(sockaddr));
    _server_info->stream.port = ntohs(_server_info->sockaddr.sin_port);
    _server_info->stream.index = remote_index;

    SOCK_INFO_PAIR* server_info = new SOCK_INFO_PAIR(stream_msg->name,_server_info);
    server.push_back(server_info);

    CLEAR(stream_msg);
    memcpy(stream_msg, local_socket->stream,sizeof(stream_info));
    stream_msg->port = _server_info->stream.port;

}



void* KNetwork_Connect::listen_thread(void* arg)
{
    int ret = 0;
    unsigned int index = 0;
    m_client_count = 0;
    socket_info* local_socket = &local;
    socket_info* listen_socket = new socket_info;
    CLEAR(listen_socket);
    memcpy(listen_socket,local_socket,sizeof(socket_info));
    listen_socket->sockaddr.sin_port = htons(DEFAULT_CONNECT_PORT);
    listen_socket->stream.port = DEFAULT_CONNECT_PORT;
    listen_socket->socket_fd = socket(AF_INET,SOCK_STREAM,0);

    ::bind(listen_socket->socket_fd, (sockaddr*)&listen_socket->sockaddr,sizeof(sockaddr));

    while(1)
    {
        stream_info stream_msg;
        CLEAR(&stream_msg);

        socket_info* _remote_info = new socket_info;
        CLEAR(_remote_info);

        listen(listen_socket->socket_fd,1024);
        socklen_t a = sizeof(sockaddr);
        _remote_info->socket_fd = accept(listen_socket->socket_fd, (sockaddr*)&_remote_info->sockaddr,&a);
        recv(_remote_info->socket_fd,(void*)&stream_msg,sizeof(stream_msg),0);
        memcpy(&_remote_info->stream,&stream_msg,sizeof(stream_msg));
        _remote_info->stream.index = index++;
        SOCK_INFO_PAIR* remote_info = new SOCK_INFO_PAIR(_remote_info->stream.name,_remote_info);
        remote.push_back(remote_info);

        m_client_count++;

        ret = _init_server(local_socket,&stream_msg,_remote_info->stream.index);

        send(_remote_info->socket_fd,(void*)&stream_msg,sizeof(stream_msg),0);

        _create_server(_remote_info->stream.name);

    }

    delete listen_socket;
}

void* KNetwork_Connect::server_thread(void* arg)
{
    socket_info* local_socket = &local;





}

void* KNetwork_Connect::client_thread(void* arg)
{
    socket_info* local_socket = &local;
    socket_info* server = new socket_info;
    CLEAR(server);

    server->sockaddr.sin_family = AF_INET;
    server->sockaddr.sin_port   = htons(DEFAULT_CONNECT_PORT);
    server->sockaddr.sin_addr.s_addr = inet_addr("192.168.1.200");
    server->socket_fd = socket(AF_INET,SOCK_STREAM,m_bBlocking?0:SOCK_NONBLOCK);



    if(connect(server->socket_fd,(sockaddr*)&server->sockaddr,sizeof(sockaddr)))
        perror("connect error");

    stream_info request_info;
    CLEAR(&request_info);
    memcpy(&request_info,local_socket->stream,sizeof(request_info));
    send(server->socket_fd,(void*)&request_info,sizeof(request_info),0);
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

int KNetwork_Connect::_create_server(stream_info* remote_info)
{
    stream_info* _remote_info = new stream_info;
    memcpy(_remote_info,remote_info,sizof(stream_info));

    THREAD_PAIR *thread_info = new THREAD_PAIR(_remote_info,new KThread);
    thread_server.push_back(thread_info);
    pthread_create(&thread_info->second->m_thread_id,NULL,_server_thread,(void*)thread_info);
    delete _remote_info;
}

int KNetwork_Connect::_create_client()
{
    pthread_create(&m_client_thread,NULL,_client_thread,(void*)this);
}











