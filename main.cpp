#include <iostream>
#include "lwnetwork_connect.h"

using namespace std;
#define MODE 0x00000001

int main()
{
    KNetwork_Connect connect;
    connect.Init(MODE);
    sleep(1000);
}
