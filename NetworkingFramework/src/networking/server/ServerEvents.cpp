#include "ServerEvents.h"

EventType ServerStarted::s_Type = "SERVER_OPENED";
EventType ServerStopped::s_Type = "SERVER_CLOSED";
EventType ClientConnected::s_Type = "CLIENT_OPENED";
EventType ClientDisconnected::s_Type = "CLIENT_CLOSED";
EventType ClientRecv::s_Type = "CLIENT_RECV";
EventType ClientSend::s_Type = "CLIENT_SEND";
