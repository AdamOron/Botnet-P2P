#include "ClientEvents.h"

EventType ClientStarted::s_Type = "CLIENT_STARTED";
EventType ClientStopped::s_Type = "CLIENT_STOPPED";
EventType ServerConnected::s_Type = "SERVER_CONN";
EventType ServerDisconnected::s_Type = "SERVER_DISCONN";
EventType ServerRecv::s_Type = "SERVER_RECV";
EventType ServerSend::s_Type = "SERVER_SEND";
