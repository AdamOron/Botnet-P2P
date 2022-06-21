#pragma once
#include "../../eventsystem/IEvent.h"

class Server;

class ServerEvent : public IEvent
{
public:
    Server *m_Server;

    ServerEvent(Server *server) :
        m_Server(server)
    {
    }
};

class ServerStarted : public ServerEvent
{
public:
    static EventType s_Type;

    ServerStarted(Server *server) :
        ServerEvent(server)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }
};

class ServerStopped : public ServerEvent
{
public:
    static EventType s_Type;

    ServerStopped(Server *server) :
        ServerEvent(server)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }
};

class ClientConnected : public ServerEvent
{
public:
    static EventType s_Type;

    size_t m_ClientId;

    ClientConnected(Server *server, size_t clientId) :
        ServerEvent(server),
        m_ClientId(clientId)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }
};

class ClientDisconnected : public ServerEvent
{
public:
    static EventType s_Type;

    size_t m_ClientId;

    ClientDisconnected(Server *server, size_t clientId) :
        ServerEvent(server),
        m_ClientId(clientId)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }

};

class ClientRecv : public ServerEvent
{
public:
    static EventType s_Type;

    size_t m_ClientId;
    const char *m_RecvBuffer;
    size_t m_RecvBytes;

    ClientRecv(Server *server, size_t clientId, const char *recvBuffer, size_t recvBytes) :
        ServerEvent(server),
        m_ClientId(clientId),
        m_RecvBuffer(recvBuffer),
        m_RecvBytes(recvBytes)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }
};

class ClientSend : public ServerEvent
{
public:
    static EventType s_Type;

    size_t m_ClientId;
    const char *m_SendBuffer;
    size_t m_SendBytes;

    ClientSend(Server *server, size_t clientId, const char *sendBuffer, size_t sendBytes) :
        ServerEvent(server),
        m_ClientId(clientId),
        m_SendBuffer(sendBuffer),
        m_SendBytes(sendBytes)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }
};
