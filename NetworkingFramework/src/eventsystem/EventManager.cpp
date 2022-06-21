#include "EventManager.h"

EventManager::EventManager() :
    m_Handlers()
{
}

void EventManager::Register(EventType type, EventHandler handler)
{
    m_Handlers[type].push_back(handler);
}

void EventManager::Dispatch(const IEvent *event)
{
    for (EventHandler handler : m_Handlers[event->GetType()])
        handler(event);
}