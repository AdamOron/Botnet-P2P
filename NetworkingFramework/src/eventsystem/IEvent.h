#pragma once
#include <string>

using EventType = const char *;

class IEvent
{
public:
    virtual EventType GetType() const = 0;
};
