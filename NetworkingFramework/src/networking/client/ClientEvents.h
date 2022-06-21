#pragma once
#include "../../eventsystem/IEvent.h"

class Client;

class ClientEvent : public IEvent
{
public:
    Client *m_Client;

    ClientEvent(Client *client) :
        m_Client(client)
    {
    }
};

class ClientStarted : public ClientEvent
{
public:
    static EventType s_Type;

    ClientStarted(Client *client) :
        ClientEvent(client)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }
};

class ClientStopped : public ClientEvent
{
public:
    static EventType s_Type;

    ClientStopped(Client *client) :
        ClientEvent(client)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }
};

class ServerConnected : public ClientEvent
{
public:
    static EventType s_Type;

    size_t m_ServerId;

    ServerConnected(Client *client, size_t serverId) :
        ClientEvent(client),
        m_ServerId(serverId)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }
};

class ServerDisconnected : public ClientEvent
{
public:
    static EventType s_Type;

    size_t m_ServerId;

    ServerDisconnected(Client *client, size_t serverId) :
        ClientEvent(client),
        m_ServerId(serverId)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }
};

class ServerRecv : public ClientEvent
{
public:
    static EventType s_Type;

    size_t m_ServerId;
    const char *m_RecvBuffer;
    size_t m_RecvBytes;

    ServerRecv(Client *client, size_t serverId, const char *recvBuffer, size_t recvBytes) :
        ClientEvent(client),
        m_ServerId(serverId),
        m_RecvBuffer(recvBuffer),
        m_RecvBytes(recvBytes)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }
};

class ServerSend : public ClientEvent
{
public:
    static EventType s_Type;

    size_t m_ServerId;
    const char *m_SendBuffer;
    size_t m_SendBytes;

    ServerSend(Client *client, size_t serverId, const char *sendBuffer, size_t sendBytes) :
        ClientEvent(client),
        m_ServerId(serverId),
        m_SendBuffer(sendBuffer),
        m_SendBytes(sendBytes)
    {
    }

    virtual EventType GetType() const override
    {
        return s_Type;
    }
};
