#ifndef LWCONNECT_H
#define LWCONNECT_H
#include <iostream>

extern "C"
{
#include <unistd.h>
}

struct connect_info{
    std::string host_name;
    int connect_mode;
    int error;

};



class KConnect
{
public:
    KConnect();

    virtual int Init(int mode);
    virtual int Send(void* buf,unsigned long length);
    virtual int Recv(void* buf,unsigned long length);

private:
    connect_info m_connect;
};

#endif // LWCONNECT_H
