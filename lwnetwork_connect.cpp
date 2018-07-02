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
         _init_server();
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
    gethostname(local.name,20);

    int sock_fd;
    sock_fd= socket(AF_INET,SOCK_DGRAM,0);

    ifreq ifr;
    CLEAR(&ifr);
    strcpy(ifr.ifr_ifrn.ifrn_name,NET_DEVICE);

    if(ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
    {
        close(sock_fd);
        return -1;
    }
    sockaddr_in _sockaddr_in =(sockaddr_in*) &ifr.ifr_ifru.ifru_addr;
    strncpy(local.ipv4_addr,inet_ntoa(_sockaddr_in.sin_addr),16);
    local.index = 0;
    local.port = 0;
    close(sock_fd);

}

int KNetwork_Connect::_init_server()
{

    CLEAR(&server_socket);
    memcpy(&server_socket.device,&local,sizeof(device_info));

    server_socket.sockaddr.sin_port = htons(0);
    server_socket.socket_fd = socket(AF_INET,SOCK_STREAM,m_bBlocking?0:SOCK_NONBLOCK);
    ::bind(server_socket.socket_fd,(sockaddr*)&server_socket.sockaddr,sizeof(sockaddr));
    server_socket.device.port = ntohs(server_socket.sockaddr.sin_port);

}



void* KNetwork_Connect::listen_thread(void* arg)
{
    int ret = 0;
    unsigned int index = 0;
    m_client_count = 0;
    socket_info* listen_socket = new socket_info;
    CLEAR(listen_socket);
    listen_socket->sockaddr.sin_family = AF_INET;
    listen_socket->sockaddr.sin_port = htons(DEFAULT_CONNECT_PORT);
    listen_socket->device.port = DEFAULT_CONNECT_PORT;
    listen_socket->socket_fd = socket(AF_INET,SOCK_STREAM,0);

    ::bind(listen_socket->socket_fd, (sockaddr*)&listen_socket->sockaddr,sizeof(sockaddr));

    while(1)
    {
        device_info deivce_msg;
        CLEAR(&deivce_msg);

        socket_info remote_info;
        CLEAR(remote_info);

        listen(listen_socket->socket_fd,1024);
        socklen_t a = sizeof(sockaddr);
        remote_info.socket_fd = accept(listen_socket->socket_fd, NULL,NULL);
        recv(remote_info.socket_fd,(void*)&deivce_msg,sizeof(deivce_msg),0);

        device_info* _remote_device = new device_info;
        memcpy(_remote_device,&deivce_msg,sizeof(device_info));
        _remote_device->index = index++;

        remote_device.push_back(new DEVICE_PAIR(_remote_device->index,_remote_device));


        CLEAR(&deivce_msg);
        memcpy(&deivce_msg,&server_socket.device,sizeof(device_info));
        send(remote_info.socket_fd,(void*)&deivce_msg,sizeof(device_info),0);
        _create_server(_remote_device->index);



        m_client_count++;


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

    device_info request_info;
    CLEAR(&request_info);
    memcpy(&request_info,local_socket->device,sizeof(request_info));
    send(server->socket_fd,(void*)&request_info,sizeof(request_info),0);
}




void* KNetwork_Connect::server_send_thread(void* arg)
{
    socket_info* remote_socket = remote_socket[m_remote_index];
    char msg[1024];
    while(1)
    {

        m_send_protect.Lock();
        m_send_protect.Wait(0);
        memcpy(msg,)
        m_send_protect.Unlock();
        send(remote_socket->socket_fd,(void*)msg,sizeof(msg),0);
    }

}



void* KNetwork_Connect::server_recv_thread(void* arg)
{
    socket_info* remote_socket = server[m_client_count];

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


void* KNetwork_Connect::_server_send_thread(void *arg)
{
    KNetwork_Connect* connect = (KNetwork_Connect*)arg;
    connect->server_send_thread(arg);
}

void* KNetwork_Connect::_server_recv_thread(void *arg)
{
    KNetwork_Connect* connect = (KNetwork_Connect*)arg;
    connect->server_recv_thread(arg);
}



int KNetwork_Connect::_create_listen()
{
    pthread_create(&m_listen_thread,NULL,_listen_thread,(void*)this);
}

int KNetwork_Connect::_create_server(int index)
{
    socket_info* _remote_socket = new socket_info;
    listen(server_socket,1024);
    socklen_t size = (socklen_t)sizeof(sockaddr);
    _remote_socket->socket_fd = accept(server_socket,(sockaddr*)_remote_socket,&size);
    _remote_socket->device.index = index;
    m_remote_index = index;
    remote_socket.push_back(_remote_socket);




    int send_thread_id,recv_thread_id;
    pthread_create(&send_thread_id,NULL,_server_send_thread,(void*)this);
    pthread_create(&recv_thread_id,NULL,_server_recv_thread,(void*)this);

}

int KNetwork_Connect::_create_client()
{
    pthread_create(&m_client_thread,NULL,_client_thread,(void*)this);
}











