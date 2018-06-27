#include "lwnetwork_connect.h"

KNetwork_Connect::KNetwork_Connect()
{

}

KNetwork_Connect::~KNetwork_Connect()
{

}

int KNetwork_Connect::Init(int mode)
{
    m_connect_mode = mode;
    m_bServer = !(mode & 0x01);
    m_bSynchronization = !(mode & 0x02);
    m_bBlocking = !(mode & 0x04);

    if(m_bServer)
        _create_server();




}

int KNetwork_Connect::Send(void* buf, unsigned long length)
{

}

int KNetwork_Connect::Recv(void* buf, unsigned long length)
{

}


int KNetwork_Connect::_create_server()
{
    socket_info *server = new socket_info;
    memset(server,0,sizeof(socket_info));
    server->socket_fd = socket(AF_INET,SOCK_STREAM,m_bBlocking?0:SOCK_NONBLOCK);

    ifreq ifr;
    memset(&ifr,0,sizeof(ifreq));
    strcpy(ifr.ifr_ifrn.ifrn_name,NET_DEVICE);

    if(ioctl(server->socket_fd, SIOCGIFADDR, &ifr) < 0)
    {
        close(server->socket_fd);
        return -1;
    }

    memcpy(&server->sockaddr,&ifr.ifr_ifru.ifru_addr,sizeof(server->sockaddr));
    strncpy(server->ipv4_addr,inet_ntoa(server->sockaddr.sin_addr),16);
    server->port = DEFAULT_ACCEPT_PORT;
    server->sockaddr.sin_port = htons(DEFAULT_ACCEPT_PORT);


}
