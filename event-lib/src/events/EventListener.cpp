#include "general/pch.h"

#include "../internal/events/EventManager.h"
#include "Event.h"

ae::EventListener::EventListener()
{
    EventManager::Get().AddListener(this);
}

ae::EventListener::EventListener(ae::EventListener &&other) noexcept : m_Handlers(std::move(other.m_Handlers))
{
    EventManager::Get().RemoveListener(&other);
    EventManager::Get().AddListener(this);
}

ae::EventListener &ae::EventListener::operator=(ae::EventListener &&other) noexcept
{
    if (this != &other)
    {
        EventManager::Get().RemoveListener(&other);
        m_Handlers = std::move(other.m_Handlers);
    }

    return *this;
}

ae::EventListener::~EventListener()
{
    EventManager::Get().RemoveListener(this);
}

void ae::EventListener::Handle(ae::Event &event)
{
    auto it = m_Handlers.find(event.GetTypeId());

    if (it != m_Handlers.end())
    {
        it->second(event);
    }
}
