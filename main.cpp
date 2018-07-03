#include <iostream>
#include "lwnetwork_connect.h"

using namespace std;
#define MODE 0x00000001

int main()
{
    KNetwork_Connect connect;
    connect.Init(MODE);
    sleep(2);
    char send_msg[1024] = {0};
    int count = 0;

    while(1)
    {
        sprintf(send_msg,"it is service the %d times calling you",count++);
        connect.Send(send_msg,1024);
        sleep(1);
    }

}
