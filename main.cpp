#include <iostream>
#include "lwnetwork_connect.h"

using namespace std;
#define MODE 0x00000000

int main()
{
    KNetwork_Connect connect;
    connect.Init(MODE);
}
