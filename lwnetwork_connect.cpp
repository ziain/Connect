#include "lwnetwork_connect.h"


static KThread m_send_protect;
static KThread m_recv_protect;
static char send_buffer[1024];
static char recv_buffer[1024];



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
    m_send_protect.Wake(true);
    m_send_protect.Unlock();
}

int KNetwork_Connect::Recv(void* buf, unsigned long length)
{
    m_recv_protect.Lock();
    m_recv_protect.Wait();
    memcpy(buf,recv_buffer,1024);
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
    m_remote_index = 0;
    CLEAR(&m_local_device);
    ret = gethostname(m_local_device.name,20);
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
    strncpy(m_local_device.ipv4_addr,inet_ntoa(_sockaddr_in->sin_addr),16);
    m_local_device.index = 0;
    m_local_device.port = 0;
    close(sock_fd);

}

int KNetwork_Connect::_init_server()
{
    int ret  = 0;

    memcpy(&m_server_device,&m_local_device,sizeof(device_info));
    m_server_socket.sin_family  = AF_INET;
    m_server_socket.sin_addr.s_addr  = inet_addr(m_local_device.ipv4_addr);
    m_server_socket_fd = socket(AF_INET,SOCK_STREAM,m_bBlocking?0:SOCK_NONBLOCK);
    NET_ASSERT_RET_INT(m_server_socket_fd == -1,"Get server socket fd error:"<<strerror(errno));

    ret = ::bind(m_server_socket_fd,(sockaddr*)&m_server_socket,sizeof(sockaddr));
    NET_ASSERT_RET_INT(ret,"Server bind Error:"<<strerror(errno));

    socklen_t size = (socklen_t)sizeof(socklen_t);
    ret = getsockname(m_server_socket_fd,(sockaddr*)&m_server_socket,&size);
    NET_ASSERT_RET_INT(ret,"Server getsockname error"<<strerror(errno));
    m_server_device.port = ntohs(m_server_socket.sin_port);

}



void* KNetwork_Connect::listen_thread(void* arg)
{
    int ret = 0;
    unsigned int index = 0;
    int device_count = 0;
    int length = 0;
    int listen_socket_fd = 0;


    sockaddr_in listen_socket;
    CLEAR(&listen_socket);
    listen_socket.sin_family = AF_INET;
    listen_socket.sin_port = htons(DEFAULT_CONNECT_PORT);
    listen_socket_fd = socket(AF_INET,SOCK_STREAM,0);
    NET_THREAD_ASSERT_EXIT(listen_socket_fd == -1,"Listen thread get socket fd error:"<<strerror(errno));

    ret = ::bind(listen_socket_fd, (sockaddr*)&listen_socket,sizeof(sockaddr));
    NET_THREAD_ASSERT_EXIT(ret,"Listen thread bind error:"<<strerror(errno));

    while(1)
    {
        device_info deivce_msg;
        CLEAR(&deivce_msg);

        sockaddr_in remote_socket;
        CLEAR(&remote_socket);

        int remote_socket_fd;
        listen(listen_socket_fd,1024);
        socklen_t size = sizeof(sockaddr);
        remote_socket_fd= accept(listen_socket_fd, NULL,NULL);

        length = recv(remote_socket_fd,(void*)&deivce_msg,sizeof(deivce_msg),0);
        NET_ASSERT(length == sizeof(deivce_msg),"Listen thread recv error");

        device_info _remote_device;
        memcpy(&_remote_device,&deivce_msg,sizeof(device_info));

        CLEAR(&deivce_msg);
        memcpy(&deivce_msg,&m_server_device,sizeof(device_info));
        length = send(remote_socket_fd,(void*)&deivce_msg,sizeof(device_info),0);
        NET_ASSERT(length == sizeof(device_info),"Send local device information to remote device success");

        ret = _create_server(_remote_device);
        NET_ASSERT(ret,"Create remote communicating server error");



    }

}


void* KNetwork_Connect::client_thread(void* arg)
{
    int server_socket_fd;
    sockaddr_in server_socket;
    CLEAR(&server_socket);

    server_socket.sin_family = AF_INET;
    server_socket.sin_port   = htons(DEFAULT_CONNECT_PORT);
    server_socket.sin_addr.s_addr = inet_addr("192.168.1.200");
    server_socket_fd = socket(AF_INET,SOCK_STREAM,m_bBlocking?0:SOCK_NONBLOCK);

    if(connect(server_socket_fd,(sockaddr*)&server_socket,sizeof(sockaddr)))
        perror("connect error");

    device_info request_info;
    CLEAR(&request_info);
    memcpy(&request_info,&m_local_device,sizeof(request_info));
    send(server_socket_fd,(void*)&request_info,sizeof(request_info),0);

    device_info *remote_info = new device_info;
    CLEAR(remote_info);

    recv(server_socket_fd,(void*)remote_info,sizeof(device_info),0);

    m_remote_device_list.push_back(new DEVICE_PAIR(0,remote_info));
    close(server_socket_fd);

    server_socket.sin_port = htons(remote_info->port);
    server_socket_fd = socket(AF_INET,SOCK_STREAM,m_bBlocking?0:SOCK_NONBLOCK);
    if(connect(server_socket_fd,(sockaddr*)&server_socket,sizeof(sockaddr)))
        perror("connect error");
    while(1)
    {
        char msg[1024];
        CLEAR(msg);
        recv(server_socket_fd,(void*)msg,1024,0);

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
//    connect->server_thread(arg);
}

void* KNetwork_Connect::_client_thread(void* arg)
{
    KNetwork_Connect* connect = (KNetwork_Connect*)arg;
    connect->client_thread(arg);
}


void* KNetwork_Connect::_server_send_thread(void *arg)
{
    thread_info* server_thread = static_cast<thread_info*>(arg);
    KNetwork_Connect *connect = server_thread->connect;
    int ret = 0;
    char buf[1024];
    while(1)
    {
        m_send_protect.Lock();
        m_send_protect.Wait(0);
        memcpy(buf,send_buffer,1024);
        m_send_protect.Unlock();
        ret = send(server_thread->socket_fd,(void*)buf,1024,0);
        if(ret == -1)
        {
            NET_LOGE("server send thread send error"<<strerror(errno));
            connect->thread_clean(server_thread);
        }
    }
}

void* KNetwork_Connect::_server_recv_thread(void *arg)
{
    thread_info* server_thread = static_cast<thread_info*>(arg);
    KNetwork_Connect *connect = server_thread->connect;
    int ret = 0;
    char msg[1024];
    while(1)
    {
        ret = recv(server_thread->socket_fd,(void*)msg,sizeof(msg),0);
        if(ret == -1)
        {
            NET_LOGE("server send thread send error"<<strerror(errno));
            connect->thread_clean(server_thread);
        }

        m_recv_protect.Lock();
        memcpy(recv_buffer,msg,1024);
        m_recv_protect.Wake(0);
        m_recv_protect.Unlock();
    }
}




void KNetwork_Connect::thread_clean(thread_info* thread)
{
    int ret = 0;
    ret = pthread_cancel(thread->thread_id[0]);
    NET_ASSERT(ret,"The "<<thread->device_index<<" server send thread cant abort");

    pthread_cancel(thread->thread_id[1]);
    NET_ASSERT(ret,"The "<<thread->device_index<<" server recv thread cant abort");


    m_remote_device_list_protect.Lock();
    for(DEVICE_LIST::iterator iter = m_remote_device_list.begin(); iter != m_remote_device_list.end();iter++)
    {
        if((*iter)->first == thread->device_index)
        {
            DEVICE_PAIR* device_pair = (*iter);
            device_info* remote_device = (*iter)->second;

            delete remote_device;
            m_remote_device_list.erase(iter);
            delete *iter;

        }

    }
    m_remote_device_list_protect.Unlock();
    delete thread;

}




int KNetwork_Connect::_create_listen()
{
    pthread_create(&m_listen_thread,NULL,_listen_thread,(void*)this);
}

int KNetwork_Connect::_create_server(device_info &remote_device_info)
{
    int ret = 0;
    int remote_fd;

    remote_device_info.index = m_remote_index;

    ret = listen(m_server_socket_fd,1024);
    NET_ASSERT_RET_INT(ret,"Listen client connect error"<<strerror(errno));

    sockaddr _remote_socket;
    socklen_t size = (socklen_t)sizeof(sockaddr);
    remote_fd = accept(m_server_socket_fd,(sockaddr*)&_remote_socket,&size);

    thread_info* server_thread = new thread_info;
    server_thread->connect = this;
    server_thread->device_index = m_remote_index;
    server_thread->socket_fd = remote_fd;

    ret = pthread_create(&server_thread->thread_id[0],NULL,_server_send_thread,(void*)server_thread);
    if(ret)
    {
        NET_LOGE("server send thread create failed");
        //goto clean;
        return -1;
    }

    ret = pthread_create(&server_thread->thread_id[1],NULL,_server_recv_thread,(void*)server_thread);
    if(ret)
    {
        NET_LOGE("server recv thread create failed");
        pthread_cancel(server_thread->thread_id[0]);
        //goto clean;
        return -1;
    }

    device_info*  _remote_device = new device_info;
    memcpy(_remote_device,&remote_device_info,sizeof(device_info));

    m_remote_device_list_protect.Lock();
    m_remote_device_list.push_back(new DEVICE_PAIR(_remote_device->index,_remote_device));
    m_remote_device_list_protect.Unlock();

    NET_LOGI("Remote device \""<<_remote_device->name<<"\" connecting service");
    NET_LOGI("Add the remote device in list back,alloc index is "<<_remote_device->index);


    m_remote_index++;

    m_client_count_proetct.Lock();
    m_client_count++;
    m_client_count_proetct.Wake(false);
    m_client_count_proetct.Unlock();

//clean:
//    remote_socket.pop_back();
//    delete _remote_socket;


}

int KNetwork_Connect::_create_client()
{
    pthread_create(&m_client_thread,NULL,_client_thread,(void*)this);
}











