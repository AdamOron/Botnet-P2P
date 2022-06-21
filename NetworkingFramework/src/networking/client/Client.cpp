#include "Client.h"
#include <WS2tcpip.h>
#include "ClientEvents.h"
#include "../../threading/ThreadPool.h"

#pragma comment(lib, "Ws2_32.lib")

#define RECV_BUFF_LEN 256

Client::Client() :
    m_IsRunning(false)
{
}

/*
Throws given error message and exits the program.
@param const char *format, the format of the message. ..., the additional arguments matching the format.
*/
void ThrowClientError(const char *format, ...)
{
    printf("Client Failed: ");
    /* Print given format & arguments */
    va_list varargs;
    va_start(varargs, format);
    vprintf(format, varargs);
    va_end(varargs);
    /* Exit with non-zero code */
    WSACleanup();
    exit(1);
}

void InitClientHints(addrinfo *pHints)
{
    ZeroMemory(pHints, sizeof(addrinfo));
    pHints->ai_family = AF_UNSPEC;
    pHints->ai_socktype = SOCK_STREAM;
    pHints->ai_protocol = IPPROTO_TCP;
}

void InitAddrInfo(addrinfo **ppInfo, const char *host, const char *port)
{
    addrinfo hints;
    InitClientHints(&hints);

    int retCode = getaddrinfo(host, port, &hints, ppInfo);

    if (retCode != 0)
        ThrowClientError("getaddrinfo failed: %d\n", retCode);
}

SOCKET InitSocket(addrinfo *pInfo)
{
    SOCKET connSocket = socket(pInfo->ai_family, pInfo->ai_socktype, pInfo->ai_protocol);

    if (connSocket == INVALID_SOCKET)
        ThrowClientError("socket failed: %d\n", WSAGetLastError());

    return connSocket;
}

SOCKET ConnectSocket(addrinfo *pInfo)
{
    SOCKET connSocket = InitSocket(pInfo);

    int retCode = connect(connSocket, pInfo->ai_addr, (int) pInfo->ai_addrlen);

    if (retCode == SOCKET_ERROR)
        ThrowClientError("connect failed: %d\n", WSAGetLastError());

    return connSocket;
}

void Client::Connect(const char *host, const char *port)
{
    addrinfo *pInfo;
    InitAddrInfo(&pInfo, host, port);

    size_t serverIndex = m_Conns.Add(ConnectSocket(pInfo));

    freeaddrinfo(pInfo);

    ServerConnected event(this, serverIndex);
    m_EventManager.Dispatch(&event);
}

void Client::CloseConn(size_t serverIndex)
{
    closesocket(m_Conns[serverIndex]);
    m_Conns.Remove(serverIndex);

    ServerDisconnected event(this, serverIndex);
    m_EventManager.Dispatch(&event);

    printf("Closed Conn %d\n", serverIndex);
}

void Client::SendServer(size_t serverIndex, char *buffer, size_t bufferSize)
{
    int sentBytes = send(m_Conns[serverIndex], buffer, bufferSize, 0);

    if (sentBytes == SOCKET_ERROR)
        ThrowClientError("send failed: %d\n", WSAGetLastError());

    ServerSend event(this, serverIndex, buffer, sentBytes);
    m_EventManager.Dispatch(&event);
}

int Client::ReadServer(size_t serverIndex)
{
    SOCKET clientSocket = m_Conns[serverIndex];

    char recvBuff[RECV_BUFF_LEN];
    int recvBytes = recv(clientSocket, recvBuff, RECV_BUFF_LEN, 0);

    if (recvBytes == SOCKET_ERROR)
    {
        int errCode = WSAGetLastError();

        if (errCode != WSAECONNRESET)
            ThrowClientError("recv failed: %d\n", errCode);

        CloseConn(serverIndex);
        return SOCKET_ERROR;
    }

    ServerRecv event(this, serverIndex, recvBuff, recvBytes);
    m_EventManager.Dispatch(&event);

    return recvBytes;
}

void Client::InitReadSet(fd_set *readSet)
{
    FD_ZERO(readSet);

    for (size_t i = 0; i < m_Conns.Size(); i++)
        FD_SET(m_Conns[i], readSet);
}

size_t Client::ReadServers(fd_set *readSet)
{
    size_t handledClients = 0;

    for (size_t i = 0; i < m_Conns.Size(); i++)
    {
        if (!FD_ISSET(m_Conns[i], readSet))
            continue;

        handledClients++;

        ReadServer(i);
    }

    return handledClients;
}

timeval timeout_c = { 0, 500000 };

void Client::Select()
{
    if (!m_Conns.Size())
        return;

    fd_set readSet;
    InitReadSet(&readSet);

    int readableSockets = select(NULL, &readSet, NULL, NULL, &timeout_c);

    if (readableSockets == SOCKET_ERROR)
        ThrowClientError("select error: %d\n", WSAGetLastError());

    while (readableSockets)
        readableSockets -= ReadServers(&readSet);
}

void Client::Run()
{
    using namespace std::chrono_literals;

    m_IsRunning = true;

    ThreadPool::GetInstance()->Enqueue(
        [this]
        {
            while (m_IsRunning)
            {
                Select();
                std::this_thread::sleep_for(500ms);
            }
        }
    );
}

void Client::Stop()
{
    m_IsRunning = false;
}

void Client::RegisterEventHandler(EventType type, EventHandler handler)
{
    m_EventManager.Register(type, handler);
}

size_t Client::ConnectionCount()
{
    return m_Conns.Size();
}
