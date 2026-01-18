#include "general/pch.h"

#include "../internal/events/EventManager.h"
#include "Event.h"

ae::EventListener::EventListener() : m_Callback(nullptr)
{
    EventManager::Get().AddListener(this);
}

ae::EventListener::EventListener(std::function<void(ae::Event &)> callback) : m_Callback(std::move(callback))
{
    EventManager::Get().AddListener(this);
}

ae::EventListener::EventListener(ae::EventListener &&other) noexcept : m_Callback(std::move(other.m_Callback))
{
    EventManager::Get().RemoveListener(&other);
    EventManager::Get().AddListener(this);
    other.m_Callback = nullptr;
}

ae::EventListener &ae::EventListener::operator=(ae::EventListener &&other) noexcept
{
    if (this != &other)
    {
        EventManager::Get().RemoveListener(&other);
        m_Callback = std::move(other.m_Callback);
        other.m_Callback = nullptr;
    }

    return *this;
}

ae::EventListener::~EventListener()
{
    EventManager::Get().RemoveListener(this);
}

void ae::EventListener::SetCallback(std::function<void(Event &)> callback)
{
    m_Callback = std::move(callback);
}
