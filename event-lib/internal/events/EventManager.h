#pragma once

#include "Event.h"

#include <unordered_set>

namespace ae
{
class EventManager
{
    friend class Event;

  public:
    EventManager(const EventManager &) = delete;
    EventManager &operator=(const EventManager &) = delete;
    ~EventManager() = default;

    [[nodiscard]] static EventManager &Get() noexcept
    {
        static EventManager s_Instance;
        return s_Instance;
    }

    void AddListener(EventListener *pListener);
    void RemoveListener(EventListener *pListener);

  private:
    EventManager() = default;

  private:
    void DispatchEvent(Event &event) const;

  private:
    std::unordered_set<EventListener *> m_pListeners;
};
} // namespace ae
