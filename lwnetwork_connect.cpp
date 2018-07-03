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
    int ret = 0;
    m_client_count = 0;
    m_bServer = !!(m_connect_mode & 0x01);
    m_bSynchronization = !(m_connect_mode & 0x02);
    m_bBlocking = !(m_connect_mode & 0x04);

    ret = _init_local();

    if(m_bServer)
    {
         _init_server();
         _create_listen();

    }
    else
    {
        _create_client();
    }


}

int KNetwork_Connect::Init(int mode)
{
    m_connect_mode = mode;
    KNetwork_Connect::Init();
}



int KNetwork_Connect::Send(void* buf, unsigned long length)
{
    m_send_protect.Lock();
    memcpy(send_buffer,buf,1024);
    m_send_protect.Wake(1);
    m_send_protect.Unlock();
}

int KNetwork_Connect::Recv(void* buf, unsigned long length)
{
    m_recv_protect.Lock();
    m_recv_protect.Wait();
    buf =(void*)recv_buffer;
    m_recv_protect.Unlock();

}


int KNetwork_Connect::GetDeviceCount()
{
    m_client_count_proetct.Lock();
    while(!m_client_count)
        m_client_count_proetct.Wait();
    return m_client_count;
    m_client_count_proetct.Unlock();
}



int KNetwork_Connect::_init_local()
{
    int ret = 0;
    CLEAR(&local);
    ret = gethostname(local.name,20);
    NET_ASSERT_RET_INT(ret,"Get local host name error:"<<strerror(errno));

    int sock_fd = 0;
    sock_fd= socket(AF_INET,SOCK_DGRAM,0);
    NET_ASSERT_RET_INT(ret == -1,"Get socket fd error:"<<strerror(errno));

    ifreq ifr;
    CLEAR(&ifr);
    strcpy(ifr.ifr_ifrn.ifrn_name,NET_DEVICE);

    if(ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
    {
        close(sock_fd);
        NET_LOGE("Request ip address error:"<<strerror(errno));
        return -1;
    }

    sockaddr_in* _sockaddr_in =(sockaddr_in*) &ifr.ifr_ifru.ifru_addr;
    strncpy(local.ipv4_addr,inet_ntoa(_sockaddr_in->sin_addr),16);
    local.index = 0;
    local.port = 0;
    close(sock_fd);

}

int KNetwork_Connect::_init_server()
{
    int ret  = 0;

    CLEAR(&server_socket);
    memcpy(&server_socket.device,&local,sizeof(device_info));

    server_socket.sockaddr.sin_family  = AF_INET;
    server_socket.sockaddr.sin_addr.s_addr  = inet_addr(server_socket.device.ipv4_addr);
    server_socket.sockaddr.sin_port = 0;

    server_socket.socket_fd = socket(AF_INET,SOCK_STREAM,m_bBlocking?0:SOCK_NONBLOCK);
    NET_ASSERT_RET_INT(server_socket.socket_fd == -1,"Get server socket fd error:"<<strerror(errno));

    ret = ::bind(server_socket.socket_fd,(sockaddr*)&server_socket.sockaddr,sizeof(sockaddr));
    NET_ASSERT_RET_INT(ret,"Server bind Error:"<<strerror(errno));

    socklen_t size = (socklen_t)sizeof(socklen_t);
    ret = getsockname(server_socket.socket_fd,(sockaddr*)&server_socket.sockaddr,&size);
    NET_ASSERT_RET_INT(ret,"Server getsockname error"<<strerror(errno));

    server_socket.device.port = ntohs(server_socket.sockaddr.sin_port);

}



void* KNetwork_Connect::listen_thread(void* arg)
{
    int ret = 0;
    unsigned int index = 0;
    int device_count = 0;
    int length = 0;
    socket_info* listen_socket = new socket_info;
    CLEAR(listen_socket);
    listen_socket->sockaddr.sin_family = AF_INET;
    listen_socket->sockaddr.sin_port = htons(DEFAULT_CONNECT_PORT);
    listen_socket->device.port = DEFAULT_CONNECT_PORT;
    listen_socket->socket_fd = socket(AF_INET,SOCK_STREAM,0);
    NET_THREAD_ASSERT_EXIT(listen_socket->socket_fd == -1,"Listen thread get socket fd error:"<<strerror(errno));

    ret = ::bind(listen_socket->socket_fd, (sockaddr*)&listen_socket->sockaddr,sizeof(sockaddr));
    NET_THREAD_ASSERT_EXIT(ret,"Listen thread bind error:"<<strerror(errno));

    while(1)
    {
        device_info deivce_msg;
        CLEAR(&deivce_msg);

        socket_info remote_socket;
        CLEAR(&remote_socket);

        listen(listen_socket->socket_fd,1024);
        socklen_t size = sizeof(sockaddr);
        remote_socket.socket_fd = accept(listen_socket->socket_fd, NULL,NULL);

        length = recv(remote_socket.socket_fd,(void*)&deivce_msg,sizeof(deivce_msg),0);
        NET_ASSERT(length == sizeof(deivce_msg),"Listen thread recv error");

        device_info* _remote_device = new device_info;
        memcpy(_remote_device,&deivce_msg,sizeof(device_info));
        _remote_device->index = index++;

        remote_device.push_back(new DEVICE_PAIR(_remote_device->index,_remote_device));
        NET_LOGI("Remote device \""<<_remote_device->name<<"\" connecting service");
        NET_LOGI("Add the remote device in list back,alloc index is "<<_remote_device->index);

        CLEAR(&deivce_msg);
        memcpy(&deivce_msg,&server_socket.device,sizeof(device_info));
        length = send(remote_socket.socket_fd,(void*)&deivce_msg,sizeof(device_info),0);
        NET_ASSERT(length == sizeof(device_info),"Send local device information to remote device success");

        device_count++;



        ret = _create_server(_remote_device->index);
        if(ret)
        {
            NET_LOGE("Create "<<_remote_device->index<<" remote communicating server error");
            DEVICE_PAIR *_tmp_device = remote_device.back();
            remote_device.pop_back();
            device_info *_tmp_device_info = _tmp_device->second;
            NET_LOGE("Delete the "<<_tmp_device_info->index<<" device");

            delete _tmp_device_info;
            delete _tmp_device;
            index--;
            device_count--;
        }

        m_client_count_proetct.Lock();
        m_client_count = device_count;
        m_client_count_proetct.Wake(false);
        m_client_count_proetct.Unlock();

    }

    delete listen_socket;
}

void* KNetwork_Connect::server_thread(void* arg)
{





}

void* KNetwork_Connect::client_thread(void* arg)
{
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
    memcpy(&request_info,&local,sizeof(request_info));
    send(server->socket_fd,(void*)&request_info,sizeof(request_info),0);

    device_info *remote_info = new device_info;
    CLEAR(remote_info);

    recv(server->socket_fd,(void*)remote_info,sizeof(device_info),0);

    remote_device.push_back(new DEVICE_PAIR(0,remote_info));
    close(server->socket_fd);

    server->sockaddr.sin_port = htons(remote_info->port);
    if(connect(server->socket_fd,(sockaddr*)&server->sockaddr,sizeof(sockaddr)))
        perror("connect error");
    while(1)
    {
        char msg[1024];
        CLEAR(msg);
        recv(server->socket_fd,(void*)msg,1024,0);

        m_recv_protect.Lock();
        memcpy(recv_buffer,msg,1024);
        m_recv_protect.Wake(0);
        m_recv_protect.Unlock();
    }
}




void* KNetwork_Connect::server_send_thread(void* arg)
{
    socket_info* _remote_socket = remote_socket[m_remote_index];
    char* buf;
    while(1)
    {
        m_send_protect.Lock();
        m_send_protect.Wait(0);
        buf = send_buffer;
        m_send_protect.Unlock();
        send(_remote_socket->socket_fd,(void*)buf,1024,0);
    }

}



void* KNetwork_Connect::server_recv_thread(void* arg)
{
    socket_info* _remote_socket = remote_socket[m_remote_index];
    char msg[1024];
    while(1)
    {
        recv(_remote_socket->socket_fd,(void*)msg,sizeof(msg),0);
        m_recv_protect.Lock();
        memcpy(recv_buffer,msg,1024);
        m_recv_protect.Wake(0);
        m_recv_protect.Unlock();
    }
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
    int ret = 0;

    ret = listen(server_socket.socket_fd,1024);
    NET_ASSERT_RET_INT(ret,"Listen client connect error"<<strerror(errno));

    socket_info* _remote_socket = new socket_info;
    socklen_t size = (socklen_t)sizeof(sockaddr);
    _remote_socket->socket_fd = accept(server_socket.socket_fd,(sockaddr*)&_remote_socket->sockaddr,&size);
    _remote_socket->device.index = index;
    m_remote_index = index;
    remote_socket.push_back(_remote_socket);


    pthread_t send_thread_id,recv_thread_id;
    ret = pthread_create(&send_thread_id,NULL,_server_send_thread,(void*)this);
    if(ret)
    {
        NET_LOGE("server send thread create failed");
        goto clean;
        return -1;
    }

    ret = pthread_create(&recv_thread_id,NULL,_server_recv_thread,(void*)this);
    if(ret)
    {
        NET_LOGE("server recv thread create failed");
        pthread_cancel(send_thread_id);
        goto clean;
        return -1;
    }

clean:
    remote_socket.pop_back();
    delete _remote_socket;


}

int KNetwork_Connect::_create_client()
{
    pthread_create(&m_client_thread,NULL,_client_thread,(void*)this);
}











