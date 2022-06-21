#pragma once
#include <WinSock2.h>
#include "../SocketArray.h"
#include "../../eventsystem/EventManager.h"

class Server
{
public:
	SocketArray m_Clients;
	const char *m_Port;
	SOCKET m_ServerSocket;
	bool m_bIsRunning;

	EventManager m_EventManager;

	size_t AcceptClient();
	void CloseClient(size_t clientIndex);
	void Select();

	void InitReadSet(fd_set *readSet);
	bool CheckForConn(fd_set *readSet);
	int ReadClient(size_t clientIndex, char *recvBuff, int recvBuffLen);
	size_t CheckClients(fd_set *readSet);

	void InitSocket(addrinfo *pInfo);
	void BindSocket(addrinfo *pInfo);
	void StartListening();
	void Run();

public:
	Server();
	void Start(const char *port);
	int SendClient(size_t clientIndex, char *buffer, size_t bufferSize);
	void Disconnect();
	void RegisterEventHandler(EventType type, EventHandler handler);
};
