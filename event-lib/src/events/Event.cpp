#include "general/pch.h"

#include "../internal/events/EventManager.h"
#include "Event.h"

void ae::Event::Dispatch()
{
    EventManager::Get().DispatchEvent(*this);
}
