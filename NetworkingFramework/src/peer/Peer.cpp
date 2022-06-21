#include "Peer.h"
#include "../networking/server/ServerEvents.h"
#include "../networking/client/ClientEvents.h"
#include "PeerProtocol.h"
#include "../threading/ThreadPool.h"

void Peer::Server_HandleServerStarted(const IEvent *e)
{
    if (e->GetType() != ServerStarted::s_Type)
        return;

    const ServerStarted *event = static_cast<const ServerStarted *>(e);

    printf("Running Server\n");
}

void Peer::Server_HandleServerStopped(const IEvent *e)
{
    if (e->GetType() != ServerStopped::s_Type)
        return;

    const ServerStopped *event = static_cast<const ServerStopped *>(e);

    printf("Stopping Server\n");
}

void Peer::Server_HandleClientConnected(const IEvent *e)
{
    if (e->GetType() != ClientConnected::s_Type)
        return;

    const ClientConnected *event = static_cast<const ClientConnected *>(e);

    printf("Client %d Opened\n", event->m_ClientId);
}

void Peer::Server_HandleClientDisconnected(const IEvent *e)
{
    if (e->GetType() != ClientDisconnected::s_Type)
        return;

    const ClientDisconnected *event = static_cast<const ClientDisconnected *>(e);

    printf("Client %d Closed\n", event->m_ClientId);
}

void Peer::Server_HandleClientRecv(const IEvent *e)
{
    if (e->GetType() != ClientRecv::s_Type)
        return;

    const ClientRecv *event = static_cast<const ClientRecv *>(e);

    PeerProtocol::HEADER *header = (PeerProtocol::HEADER *) event->m_RecvBuffer;

    switch (header->m_Type)
    {
    case PeerProtocol::Type::PEER_HELLO:
        PeerProtocol::HELLO_MSG *hello = (PeerProtocol::HELLO_MSG *) event->m_RecvBuffer;
        AcceptPeer(hello->m_Host, hello->m_Port, event->m_ClientId);
        break;
    }

    if (header->m_IsFwd)
    {
        PeerProtocol::FWD_MSG *fwd = (PeerProtocol::FWD_MSG *) event->m_RecvBuffer;
        fwd->m_RecvdGuids += m_Guid;
    }
}

void Peer::Server_HandleClientSend(const IEvent *e)
{
    if (e->GetType() != ClientSend::s_Type)
        return;

    const ClientSend *event = static_cast<const ClientSend *>(e);

    // Unhandled
}

void Peer::Client_HandleClientStarted(const IEvent *e)
{
    if (e->GetType() != ClientStarted::s_Type)
        return;

    const ClientStarted *event = static_cast<const ClientStarted *>(e);

    printf("Running Client\n");
}

void Peer::Client_HandleClientStopped(const IEvent *e)
{
    if (e->GetType() != ClientStopped::s_Type)
        return;

    const ClientStopped *event = static_cast<const ClientStopped *>(e);

    printf("Stopping Client\n");
}

void Peer::Client_HandleServerConnected(const IEvent *e)
{
    if (e->GetType() != ServerConnected::s_Type)
        return;

    const ServerConnected *event = static_cast<const ServerConnected *>(e);

    printf("Server %d Connected\n", event->m_ServerId);

    if (m_Client.ConnectionCount() == 1)
    {
        PeerProtocol::HELLO_MSG hello("192.168.1.14", m_Port);
        m_Client.SendServer(event->m_ServerId, BYTES(hello), sizeof(PeerProtocol::HELLO_MSG));
    }
}

void Peer::Client_HandleServerDisconnected(const IEvent *e)
{
    if (e->GetType() != ServerDisconnected::s_Type)
        return;

    const ServerDisconnected *event = static_cast<const ServerDisconnected *>(e);

    printf("Server %d Closed\n", event->m_ServerId);
}

void Peer::Client_HandleServerRecv(const IEvent *e)
{
    if (e->GetType() != ServerRecv::s_Type)
        return;

    const ServerRecv *event = static_cast<const ServerRecv *>(e);

    PeerProtocol::HEADER *header= (PeerProtocol::HEADER *) event->m_RecvBuffer;

    switch (header->m_Type)
    {
    case PeerProtocol::Type::PEER_ACCEPT:
        PeerProtocol::ACCEPT_MSG *accept = (PeerProtocol::ACCEPT_MSG *) event->m_RecvBuffer;
        m_Guid = accept->m_NewGuid;

        printf("I have a GUID! %d\n", m_Guid);

        PEER_LIST::ENTRY &entry =
            m_Peers += PEER_LIST::ENTRY(accept->m_Header.m_SenderGuid);

        printf("I met %d\n", accept->m_Header.m_SenderGuid);

        entry.ServerId = event->m_ServerId;
        break;
    }
}

void Peer::Client_HandleServerSend(const IEvent *e)
{
    if (e->GetType() != ServerSend::s_Type)
        return;

    const ServerSend *event = static_cast<const ServerSend *>(e);

    // Unhandled
}

#include <random>

guid_t Peer::GenerateRandomGuid()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, MAX_GUID);

    guid_t guid;

    do
        guid = dist(rng);
    while (m_Peers.FindByGuid(guid) || guid == m_Guid);

    return guid;
}

Peer::Peer(const char *port) :
    m_Port(port),
    m_Client(),
    m_Server()
{
    RegisterServerHandlers();
    RegisterClientHandlers();
}

void Peer::RegisterServerHandlers()
{
    m_Server.RegisterEventHandler(ServerStarted::s_Type,
        [this](const IEvent *e) { Server_HandleServerStarted(e); });

    m_Server.RegisterEventHandler(ServerStopped::s_Type,
        [this](const IEvent *e) { Server_HandleServerStopped(e); });

    m_Server.RegisterEventHandler(ClientConnected::s_Type,
        [this](const IEvent *e) { Server_HandleClientConnected(e); });

    m_Server.RegisterEventHandler(ClientDisconnected::s_Type,
        [this](const IEvent *e) { Server_HandleClientDisconnected(e); });

    m_Server.RegisterEventHandler(ClientRecv::s_Type,
        [this](const IEvent *e) { Server_HandleClientRecv(e); });

    m_Server.RegisterEventHandler(ClientSend::s_Type,
        [this](const IEvent *e) { Server_HandleClientSend(e); });
}

void Peer::RegisterClientHandlers()
{
    m_Client.RegisterEventHandler(ClientStarted::s_Type,
        [this](const IEvent *e) { Client_HandleClientStarted(e); });

    m_Client.RegisterEventHandler(ClientStopped::s_Type,
        [this](const IEvent *e) { Client_HandleClientStopped(e); });

    m_Client.RegisterEventHandler(ServerConnected::s_Type,
        [this](const IEvent *e) { Client_HandleServerConnected(e); });

    m_Client.RegisterEventHandler(ServerDisconnected::s_Type,
        [this](const IEvent *e) { Client_HandleServerDisconnected(e); });

    m_Client.RegisterEventHandler(ServerSend::s_Type,
        [this](const IEvent *e) { Client_HandleServerSend(e); });

    m_Client.RegisterEventHandler(ServerRecv::s_Type,
        [this](const IEvent *e) { Client_HandleServerRecv(e); });
}

void Peer::SendPeer(guid_t guid, char *buffer, size_t bufferSize)
{
    m_Server.SendClient(m_Peers.FindByGuid(guid)->ClientId, buffer, bufferSize);
}

void Peer::AcceptPeer(const char *host, const char *port, size_t clientId)
{
    guid_t newGuid = GenerateRandomGuid();

    PeerProtocol::ACCEPT_MSG accept(m_Guid, newGuid);
    m_Server.SendClient(clientId, BYTES(accept), sizeof(PeerProtocol::ACCEPT_MSG));

    PeerProtocol::NOTIFY_JOIN_MSG notify(m_Guid, NULL, newGuid, host, port);
    // Send random peer
}

void Peer::Connect(const char *host, const char *port)
{
    m_Client.Run();
    m_Server.Start(m_Port);
    m_Client.Connect(host, port);
}

void Peer::Host()
{
    m_Guid = GenerateRandomGuid();
    m_Client.Run();
    m_Server.Start(m_Port);
}
