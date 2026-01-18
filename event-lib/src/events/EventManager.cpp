#include "general/pch.h"

#include "../internal/events/EventManager.h"
#include "Event.h"

void ae::EventManager::AddListener(EventListener *pListener)
{
#ifdef AE_DEBUG
    if (pListener == nullptr)
    {
        AE_LOG(AE_WARNING, "Tried to add null EventListener to EventManager");
        return;
    }

    if (m_pListeners.contains(pListener))
    {
        AE_LOG(AE_WARNING, "Tried to add EventListener that is already registered. "
                           "This should not be possible and may be due to a library bug");
        return;
    }
#endif

    m_pListeners.insert(pListener);
}

void ae::EventManager::RemoveListener(EventListener *pListener)
{
#ifdef AE_DEBUG
    if (pListener == nullptr)
    {
        AE_LOG(AE_WARNING, "Tried to remove null EventListener from EventManager");
        return;
    }
#endif

    auto it = m_pListeners.find(pListener);

    if (it == m_pListeners.end())
    {
        AE_LOG(AE_WARNING, "Tried to remove EventListener from EventManager that was not registered. "
                           "This should not be possible and may be due to a library bug");
        return;
    }

    m_pListeners.erase(it);
}

void ae::EventManager::DispatchEvent(Event &event) const
{
    for (EventListener *pListener : m_pListeners)
    {
        const auto &callback = pListener->GetCallback();

        if (!callback)
        {
            continue;
        }

        event.m_Consumed = false; // NOTE: All listeners get non-consumed event
        callback(event);
    }
}
