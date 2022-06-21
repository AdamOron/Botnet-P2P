#include "peer/Peer.h"
#include "threading/ThreadPool.h"
#include "networking/client/ClientEvents.h"
#include "networking/server/ServerEvents.h"

void InitializeWinsock()
{
    WSADATA wsaData;

    int retCode = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (retCode)
        printf("WSAStartup failed: %d\n", retCode);
}

#define LOCALHOST "192.168.1.181"
#define PORT_1 "8080"
#define PORT_2 "8081"

Peer host(PORT_1);
Peer peer(PORT_2);

int main(int argc, char **argv)
{
    InitializeWinsock();

    if (argc == 1)
        host.Host();
    else
        peer.Connect(LOCALHOST, PORT_1);
    
    while (true);
}
