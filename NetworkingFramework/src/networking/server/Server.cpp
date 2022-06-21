#include "Server.h"
#include <WS2tcpip.h>
#include "ServerEvents.h"
#include "../../threading/ThreadPool.h"

#pragma comment(lib, "Ws2_32.lib")

#define RECV_BUFF_LEN 256

/*
Throws given error message and exits the program.
@param const char *format, the format of the message. ..., the additional arguments matching the format.
*/
void ThrowServerError(const char *format, ...)
{
    printf("Server Failed: ");
    /* Print given format & arguments */
    va_list varargs;
    va_start(varargs, format);
    vprintf(format, varargs);
    va_end(varargs);
    /* Exit with non-zero code */
    WSACleanup();
    exit(1);
}

Server::Server() :
    m_Port(),
    m_ServerSocket(INVALID_SOCKET),
    m_Clients(),
    m_bIsRunning(false)
{
}

void InitServerHints(addrinfo *pHints)
{
    ZeroMemory(pHints, sizeof(addrinfo));
    pHints->ai_family = AF_INET;
    pHints->ai_socktype = SOCK_STREAM;
    pHints->ai_protocol = IPPROTO_TCP;
    pHints->ai_flags = AI_PASSIVE;
}

void InitAddrInfo(addrinfo **ppInfo, const char *port)
{
    addrinfo hints;
    InitServerHints(&hints);

    int retCode = getaddrinfo(NULL, port, &hints, ppInfo);

    if (retCode)
        ThrowServerError("getaddrinfo failed: %d", retCode);
}

void Server::InitSocket(addrinfo *pInfo)
{
    m_ServerSocket = socket(pInfo->ai_family, pInfo->ai_socktype, pInfo->ai_protocol);

    if (m_ServerSocket == INVALID_SOCKET)
        ThrowServerError("Error at socket(): %ld\n", WSAGetLastError());
}

void Server::BindSocket(addrinfo *pInfo)
{
    int retCode = bind(m_ServerSocket, pInfo->ai_addr, (int) pInfo->ai_addrlen);

    if (retCode == SOCKET_ERROR)
        ThrowServerError("bind failed with error: %d\n", WSAGetLastError());
}

void Server::StartListening()
{
    if (listen(m_ServerSocket, SOMAXCONN) == SOCKET_ERROR)
        ThrowServerError("Listen failed with error: %ld\n", WSAGetLastError());

    ServerStarted event(this);
    m_EventManager.Dispatch(&event);
}

size_t Server::AcceptClient()
{
    SOCKET clientSocket = accept(m_ServerSocket, NULL, NULL);

    if (clientSocket == INVALID_SOCKET)
        ThrowServerError("accept failed: %d\n", WSAGetLastError());

    size_t clientIndex = m_Clients.Add(clientSocket);

    ClientConnected event(this, clientIndex);
    m_EventManager.Dispatch(&event);

    return clientIndex;
}

void Server::CloseClient(size_t clientIndex)
{
    closesocket(m_Clients[clientIndex]);
    m_Clients.Remove(clientIndex);

    ClientDisconnected event(this, clientIndex);
    m_EventManager.Dispatch(&event);
}

int Server::SendClient(size_t clientIndex, char *buffer, size_t bufferSize)
{
    SOCKET clientSocket = m_Clients[clientIndex];

    int sentBytes = send(clientSocket, buffer, bufferSize, 0);

    if (sentBytes == SOCKET_ERROR)
        ThrowServerError("send failed: %d\n", WSAGetLastError());

    return sentBytes;
}

void Server::InitReadSet(fd_set *readSet)
{
    FD_ZERO(readSet);

    FD_SET(m_ServerSocket, readSet);

    for (size_t i = 0; i < m_Clients.Size(); i++)
        FD_SET(m_Clients[i], readSet);
}

bool Server::CheckForConn(fd_set *readSet)
{
    if (FD_ISSET(m_ServerSocket, readSet))
    {
        AcceptClient();
        return true;
    }

    return false;
}

int Server::ReadClient(size_t clientIndex, char *recvBuff, int recvBuffLen)
{
    SOCKET clientSocket = m_Clients[clientIndex];

    int recvBytes = recv(clientSocket, recvBuff, recvBuffLen, 0);

    if (recvBytes != SOCKET_ERROR)
        return recvBytes;

    int errCode = WSAGetLastError();

    if (errCode != WSAECONNRESET)
        ThrowServerError("recv failed: %d\n", errCode);

    CloseClient(clientIndex);

    return recvBytes;
}

size_t Server::CheckClients(fd_set *readSet)
{
    size_t handledClients = 0;

    for (size_t i = 0; i < m_Clients.Size(); i++)
    {
        if (!FD_ISSET(m_Clients[i], readSet))
            continue;

        handledClients++;

        char buff[RECV_BUFF_LEN];
        int bytes = ReadClient(i, buff, RECV_BUFF_LEN);

        if (bytes == SOCKET_ERROR)
            continue;

        ClientRecv event(this, i, buff, bytes);
        m_EventManager.Dispatch(&event);
    }

    return handledClients;
}

timeval timeout = { 0, 500000 };

void Server::Select()
{
    fd_set readSet;
    InitReadSet(&readSet);

    int readableSockets = select(NULL, &readSet, NULL, NULL, &timeout);

    if (readableSockets == SOCKET_ERROR)
        ThrowServerError("select error: %d\n", WSAGetLastError());

    while (readableSockets)
    {
        if(CheckForConn(&readSet))
            readableSockets--;

        readableSockets -= CheckClients(&readSet);
    }
}

void Server::Run()
{
    using namespace std::chrono_literals;

    m_bIsRunning = true;

    ThreadPool::GetInstance()->Enqueue(
        [this]
        {
            while (m_bIsRunning)
            {
                Select();
                std::this_thread::sleep_for(500ms);
            }
        }
    );
}

void Server::Start(const char *port)
{
    /* Init addrinfo struct */
    addrinfo *pInfo;
    InitAddrInfo(&pInfo, port);
    /* Prepare Server */
    InitSocket(pInfo);
    BindSocket(pInfo);
    /* Cleanup addrinfo */
    freeaddrinfo(pInfo);

    /* Execute Server */
    StartListening();
    Run();
}

void Server::Disconnect()
{
    closesocket(m_ServerSocket);
    WSACleanup();

    m_bIsRunning = false;

    ServerStopped event(this);
    m_EventManager.Dispatch(&event);
}

void Server::RegisterEventHandler(EventType type, EventHandler handler)
{
    m_EventManager.Register(type, handler);
}
