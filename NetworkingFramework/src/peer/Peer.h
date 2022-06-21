#pragma once
#include "../networking/client/Client.h"
#include "../networking/server/Server.h"
#include <unordered_map>

using guid_t = uint32_t;
#define MAX_GUID 2147483647

struct PEER_LIST
{
#define PEER_LIST_CAPACITY 32

    struct ENTRY
    {
        guid_t Guid;
        size_t ClientId, ServerId;

        ENTRY(guid_t guid, size_t clientId, size_t serverId) :
            Guid(guid),
            ClientId(clientId),
            ServerId(serverId)
        {
        }

        ENTRY(guid_t guid) :
            ENTRY(guid, 0, 0)
        {
        }

        ENTRY() :
            ENTRY(0, 0, 0)
        {
        }
    };

    size_t Size;
    ENTRY Peers[PEER_LIST_CAPACITY];

    PEER_LIST() :
        Size(0),
        Peers()
    {
    }

    ENTRY &operator+=(ENTRY peer)
    {
        return Peers[Size++] = {peer.Guid, peer.ClientId, peer.ServerId};
    }

    ENTRY operator[](size_t index) const
    {
        return Peers[index];
    }

    ENTRY *FindByGuid(guid_t guid)
    {
        for (size_t i = 0; i < Size; i++)
            if (Peers[i].Guid == guid)
                return &Peers[i];

        return NULL;
    }

    ENTRY *FindByClient(size_t clientId)
    {
        for (size_t i = 0; i < Size; i++)
            if (Peers[i].ClientId == clientId)
                return &Peers[i];

        return NULL;
    }

    ENTRY *FindByServer(guid_t serverId)
    {
        for (size_t i = 0; i < Size; i++)
            if (Peers[i].ServerId == serverId)
                return &Peers[i];

        return NULL;
    }
};

class Peer
{
private:
    guid_t m_Guid;

    Client m_Client;
    Server m_Server;
    const char *m_Port;

    PEER_LIST m_Peers;

    void Server_HandleServerStarted(const IEvent *e);
    void Server_HandleServerStopped(const IEvent *e);
    void Server_HandleClientConnected(const IEvent *e);
    void Server_HandleClientDisconnected(const IEvent *e);
    void Server_HandleClientRecv(const IEvent *e);
    void Server_HandleClientSend(const IEvent *e);

    void Client_HandleClientStarted(const IEvent *e);
    void Client_HandleClientStopped(const IEvent *e);
    void Client_HandleServerConnected(const IEvent *e);
    void Client_HandleServerDisconnected(const IEvent *e);
    void Client_HandleServerRecv(const IEvent *e);
    void Client_HandleServerSend(const IEvent *e);

    void RegisterServerHandlers();
    void RegisterClientHandlers();

    guid_t GenerateRandomGuid();

    void SendPeer(guid_t guid, char *buffer, size_t bufferSize);

    void AcceptPeer(const char *host, const char *port, size_t clientId);

public:
    Peer(const char *port);
    ~Peer()
    {
        printf("back to the lobby");
    }
    void Connect(const char *host, const char *port);
    void Host();
};
