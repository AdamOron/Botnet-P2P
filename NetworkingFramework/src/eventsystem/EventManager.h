#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include "IEvent.h"

using EventHandler = std::function<void (const IEvent *)>;

class EventManager
{
private:
    std::unordered_map<EventType, std::vector<EventHandler>> m_Handlers;

public:
    EventManager();
    void Register(EventType type, EventHandler handler);
    void Dispatch(const IEvent *event);
};
