#pragma once
#include <WinSock2.h>
#include "../../eventsystem/EventManager.h"
#include "../SocketArray.h"

class Client
{
public:
	SocketArray m_Conns;
	bool m_IsRunning;
	EventManager m_EventManager;

	void CloseConn(size_t serverIndex);
	int ReadServer(size_t serverIndex);
	void InitReadSet(fd_set *readSet);
	size_t ReadServers(fd_set *readSet);
	void Select();

public:
	Client();
	void Run();
	void Stop();
	void Connect(const char *host, const char *port);
	void SendServer(size_t serverIndex, char *buffer, size_t bufferSize);
	size_t ConnectionCount();
	void RegisterEventHandler(EventType type, EventHandler handler);
};
