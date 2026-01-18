#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ae
{

class KeyPressedEvent;
class KeyReleasedEvent;
class KeyTypedEvent;
class MouseButtonPressedEvent;
class MouseButtonReleasedEvent;
class MouseMovedEvent;
class MouseScrolledEvent;
class MouseEnteredEvent;
class MouseExitedEvent;
class WindowResizeEvent;
class WindowMinimizedEvent;
class WindowMaximizedEvent;
class WindowRestoredEvent;
class WindowMovedEvent;
class WindowFocusedEvent;
class WindowCloseEvent;
class FramebufferResizeEvent;
class ContentScaleChangedEvent;
class FileDropEvent;
class ControllerConnectedEvent;
class ControllerDisconnectedEvent;
class UpdateEvent;
class RenderEvent;

// NOTE: Built-in: 0-999, Custom: 1000+
enum class EventType : uint16_t
{
    NONE = 0,

    // Keyboard events (100-200)
    KEY_PRESSED = 100,
    KEY_RELEASED = 101,
    KEY_TYPED = 102,

    // Mouse events (200-300)
    MOUSE_BUTTON_PRESSED = 200,
    MOUSE_BUTTON_RELEASED = 201,
    MOUSE_MOVED = 202,
    MOUSE_SCROLLED = 203,
    MOUSE_ENTERED = 204,
    MOUSE_EXITED = 205,

    // Window events (300-400)
    WINDOW_RESIZE = 300,
    WINDOW_MINIMIZED = 301,
    WINDOW_MAXIMIZED = 302,
    WINDOW_RESTORED = 303,
    WINDOW_MOVED = 304,
    WINDOW_FOCUSED = 305,
    WINDOW_CLOSE = 306,
    FRAMEBUFFER_RESIZE = 307,
    CONTENT_SCALE_CHANGED = 308,
    FILE_DROP = 309,

    // Controller events (400-500)
    CONTROLLER_CONNECTED = 400,
    CONTROLLER_DISCONNECTED = 401,

    // Application events (500-600)
    APP_UPDATE = 500,
    APP_RENDER = 501,

    // Custom events start at 1000
    CUSTOM_START = 1000
};

enum class EventCategory : uint8_t
{
    NONE = 0,
    INPUT = 1 << 0,
    KEYBOARD = 1 << 1,
    MOUSE = 1 << 2,
    CONTROLLER = 1 << 3,
    WINDOW = 1 << 4,
    APPLICATION = 1 << 5,
    CUSTOM = 1 << 6
};

class EventCategoryWrapper
{
  public:
    constexpr EventCategoryWrapper() noexcept : m_Value(0) {}

    constexpr EventCategoryWrapper(EventCategory category) noexcept : m_Value(static_cast<uint8_t>(category)) {}

    [[nodiscard]] constexpr bool operator==(EventCategoryWrapper other) const noexcept
    {
        return m_Value == other.m_Value;
    }

    [[nodiscard]] constexpr bool operator!=(EventCategoryWrapper other) const noexcept
    {
        return m_Value != other.m_Value;
    }

    [[nodiscard]] constexpr EventCategoryWrapper operator|(EventCategoryWrapper other) const noexcept
    {
        return EventCategoryWrapper(static_cast<uint8_t>(m_Value | other.m_Value));
    }

    [[nodiscard]] constexpr EventCategoryWrapper operator&(EventCategoryWrapper other) const noexcept
    {
        return EventCategoryWrapper(static_cast<uint8_t>(m_Value & other.m_Value));
    }

    [[nodiscard]] constexpr EventCategoryWrapper operator~() const noexcept
    {
        return EventCategoryWrapper(static_cast<uint8_t>(~m_Value));
    }

    constexpr EventCategoryWrapper &operator|=(EventCategoryWrapper other) noexcept
    {
        m_Value |= other.m_Value;
        return *this;
    }

    constexpr EventCategoryWrapper &operator&=(EventCategoryWrapper other) noexcept
    {
        m_Value &= other.m_Value;
        return *this;
    }

  private:
    explicit constexpr EventCategoryWrapper(uint8_t value) noexcept : m_Value(value) {}

    uint8_t m_Value;
};

[[nodiscard]] inline constexpr EventCategoryWrapper operator|(EventCategory a, EventCategory b) noexcept
{
    return EventCategoryWrapper(a) | EventCategoryWrapper(b);
}

[[nodiscard]] inline constexpr EventCategoryWrapper operator&(EventCategory a, EventCategory b) noexcept
{
    return EventCategoryWrapper(a) & EventCategoryWrapper(b);
}

class EventManager;

class Event
{
    friend class EventManager;

  public:
    constexpr Event(uint16_t typeId, EventCategory category) noexcept
        : m_TypeId(typeId), m_Categories(category), m_Consumed(false)
    {
    }

    constexpr Event(uint16_t typeId, EventCategoryWrapper categories) noexcept
        : m_TypeId(typeId), m_Categories(categories), m_Consumed(false)
    {
    }

    ~Event() = default;

    void Dispatch();

    constexpr void Consume() noexcept
    {
        m_Consumed = true;
    }

    [[nodiscard]] constexpr bool IsConsumed() const noexcept
    {
        return m_Consumed;
    }

    [[nodiscard]] constexpr uint16_t GetTypeId() const noexcept
    {
        return m_TypeId;
    }

    [[nodiscard]] constexpr EventCategoryWrapper GetCategory() const noexcept
    {
        return m_Categories;
    }

    [[nodiscard]] constexpr bool IsInCategory(EventCategory category) const noexcept
    {
        return (m_Categories & category) != EventCategory::NONE;
    }

  private:
    uint16_t m_TypeId;
    EventCategoryWrapper m_Categories;
    bool m_Consumed;
};

namespace detail
{

[[nodiscard]] inline uint32_t GetNextCustomEventId() noexcept
{
    static uint32_t s_NextId = static_cast<uint32_t>(EventType::CUSTOM_START);
    return s_NextId++;
}

} // namespace detail

template <typename T> struct EventTypeId
{
    static uint32_t Get() noexcept
    {
        // NOTE: First call for type T: initializes s_Id by calling GetNextCustomEventId()
        static uint32_t s_Id = detail::GetNextCustomEventId();
        return s_Id;
    }
};

class EventListener
{
    friend class EventManager;

  public:
    EventListener();
    explicit EventListener(std::function<void(Event &)> callback);
    EventListener(const EventListener &) = delete;
    EventListener &operator=(const EventListener &) = delete;
    EventListener(EventListener &&other) noexcept;
    EventListener &operator=(EventListener &&other) noexcept;
    ~EventListener();

    void SetCallback(std::function<void(Event &)> callback);

  private:
    [[nodiscard]] const std::function<void(Event &)> &GetCallback() const noexcept
    {
        return m_Callback;
    }

  private:
    std::function<void(Event &)> m_Callback;
};

// Key Events

class KeyEvent : public Event
{
  public:
    [[nodiscard]] constexpr int32_t GetKeyCode() const noexcept
    {
        return m_KeyCode;
    }

  protected:
    constexpr KeyEvent(uint32_t typeId, int32_t keyCode) noexcept
        : Event(typeId, EventCategory::INPUT | EventCategory::KEYBOARD), m_KeyCode(keyCode)
    {
    }

  private:
    int32_t m_KeyCode;
};

class KeyPressedEvent final : public KeyEvent
{
  public:
    explicit constexpr KeyPressedEvent(int32_t keyCode, bool repeat = false) noexcept
        : KeyEvent(static_cast<uint32_t>(EventType::KEY_PRESSED), keyCode), m_Repeat(repeat)
    {
    }

    [[nodiscard]] constexpr bool IsRepeat() const noexcept
    {
        return m_Repeat;
    }

  private:
    bool m_Repeat;
};

class KeyReleasedEvent final : public KeyEvent
{
  public:
    explicit constexpr KeyReleasedEvent(int32_t keyCode) noexcept
        : KeyEvent(static_cast<uint32_t>(EventType::KEY_RELEASED), keyCode)
    {
    }
};

class KeyTypedEvent final : public Event
{
  public:
    explicit constexpr KeyTypedEvent(uint32_t character) noexcept
        : Event(static_cast<uint32_t>(EventType::KEY_TYPED), EventCategory::INPUT | EventCategory::KEYBOARD),
          m_Character(character)
    {
    }

    [[nodiscard]] constexpr uint32_t GetCharacter() const noexcept
    {
        return m_Character;
    }

  private:
    uint32_t m_Character;
};

// Mouse Events

class MouseButtonEvent : public Event
{
  public:
    [[nodiscard]] constexpr int32_t GetButton() const noexcept
    {
        return m_Button;
    }

  protected:
    constexpr MouseButtonEvent(uint32_t typeId, int32_t button) noexcept
        : Event(typeId, EventCategory::INPUT | EventCategory::MOUSE), m_Button(button)
    {
    }

  private:
    int32_t m_Button;
};

class MouseButtonPressedEvent final : public MouseButtonEvent
{
  public:
    explicit constexpr MouseButtonPressedEvent(int32_t button) noexcept
        : MouseButtonEvent(static_cast<uint32_t>(EventType::MOUSE_BUTTON_PRESSED), button)
    {
    }
};

class MouseButtonReleasedEvent final : public MouseButtonEvent
{
  public:
    explicit constexpr MouseButtonReleasedEvent(int32_t button) noexcept
        : MouseButtonEvent(static_cast<uint32_t>(EventType::MOUSE_BUTTON_RELEASED), button)
    {
    }
};

class MouseMovedEvent final : public Event
{
  public:
    constexpr MouseMovedEvent(float x, float y) noexcept
        : Event(static_cast<uint32_t>(EventType::MOUSE_MOVED), EventCategory::INPUT | EventCategory::MOUSE), m_X(x),
          m_Y(y)
    {
    }

    [[nodiscard]] constexpr float GetX() const noexcept
    {
        return m_X;
    }

    [[nodiscard]] constexpr float GetY() const noexcept
    {
        return m_Y;
    }

  private:
    float m_X;
    float m_Y;
};

class MouseScrolledEvent final : public Event
{
  public:
    constexpr MouseScrolledEvent(float xOffset, float yOffset) noexcept
        : Event(static_cast<uint32_t>(EventType::MOUSE_SCROLLED), EventCategory::INPUT | EventCategory::MOUSE),
          m_XOffset(xOffset), m_YOffset(yOffset)
    {
    }

    [[nodiscard]] constexpr float GetXOffset() const noexcept
    {
        return m_XOffset;
    }

    [[nodiscard]] constexpr float GetYOffset() const noexcept
    {
        return m_YOffset;
    }

  private:
    float m_XOffset;
    float m_YOffset;
};

class MouseEnteredEvent final : public Event
{
  public:
    constexpr MouseEnteredEvent() noexcept
        : Event(static_cast<uint32_t>(EventType::MOUSE_ENTERED), EventCategory::INPUT | EventCategory::MOUSE)
    {
    }
};

class MouseExitedEvent final : public Event
{
  public:
    constexpr MouseExitedEvent() noexcept
        : Event(static_cast<uint32_t>(EventType::MOUSE_EXITED), EventCategory::INPUT | EventCategory::MOUSE)
    {
    }
};

// Window Events

class WindowResizeEvent final : public Event
{
  public:
    constexpr WindowResizeEvent(uint32_t width, uint32_t height) noexcept
        : Event(static_cast<uint32_t>(EventType::WINDOW_RESIZE), EventCategory::WINDOW), m_Width(width),
          m_Height(height)
    {
    }

    [[nodiscard]] constexpr uint32_t GetWidth() const noexcept
    {
        return m_Width;
    }

    [[nodiscard]] constexpr uint32_t GetHeight() const noexcept
    {
        return m_Height;
    }

  private:
    uint32_t m_Width;
    uint32_t m_Height;
};

class WindowMinimizedEvent final : public Event
{
  public:
    constexpr WindowMinimizedEvent() noexcept
        : Event(static_cast<uint32_t>(EventType::WINDOW_MINIMIZED), EventCategory::WINDOW)
    {
    }
};

class WindowMaximizedEvent final : public Event
{
  public:
    constexpr WindowMaximizedEvent() noexcept
        : Event(static_cast<uint32_t>(EventType::WINDOW_MAXIMIZED), EventCategory::WINDOW)
    {
    }
};

class WindowRestoredEvent final : public Event
{
  public:
    constexpr WindowRestoredEvent() noexcept
        : Event(static_cast<uint32_t>(EventType::WINDOW_RESTORED), EventCategory::WINDOW)
    {
    }
};

class WindowMovedEvent final : public Event
{
  public:
    constexpr WindowMovedEvent(int32_t x, int32_t y) noexcept
        : Event(static_cast<uint32_t>(EventType::WINDOW_MOVED), EventCategory::WINDOW), m_X(x), m_Y(y)
    {
    }

    [[nodiscard]] constexpr int32_t GetX() const noexcept
    {
        return m_X;
    }

    [[nodiscard]] constexpr int32_t GetY() const noexcept
    {
        return m_Y;
    }

  private:
    int32_t m_X;
    int32_t m_Y;
};

class WindowFocusedEvent final : public Event
{
  public:
    explicit constexpr WindowFocusedEvent(bool focused) noexcept
        : Event(static_cast<uint32_t>(EventType::WINDOW_FOCUSED), EventCategory::WINDOW), m_Focused(focused)
    {
    }

    [[nodiscard]] constexpr bool IsFocused() const noexcept
    {
        return m_Focused;
    }

  private:
    bool m_Focused;
};

class WindowCloseEvent final : public Event
{
  public:
    constexpr WindowCloseEvent() noexcept : Event(static_cast<uint32_t>(EventType::WINDOW_CLOSE), EventCategory::WINDOW)
    {
    }
};

class FramebufferResizeEvent final : public Event
{
  public:
    constexpr FramebufferResizeEvent(uint32_t width, uint32_t height) noexcept
        : Event(static_cast<uint32_t>(EventType::FRAMEBUFFER_RESIZE), EventCategory::WINDOW), m_Width(width),
          m_Height(height)
    {
    }

    [[nodiscard]] constexpr uint32_t GetWidth() const noexcept
    {
        return m_Width;
    }

    [[nodiscard]] constexpr uint32_t GetHeight() const noexcept
    {
        return m_Height;
    }

  private:
    uint32_t m_Width;
    uint32_t m_Height;
};

class ContentScaleChangedEvent final : public Event
{
  public:
    constexpr ContentScaleChangedEvent(float xScale, float yScale) noexcept
        : Event(static_cast<uint32_t>(EventType::CONTENT_SCALE_CHANGED), EventCategory::WINDOW), m_XScale(xScale),
          m_YScale(yScale)
    {
    }

    [[nodiscard]] constexpr float GetXScale() const noexcept
    {
        return m_XScale;
    }

    [[nodiscard]] constexpr float GetYScale() const noexcept
    {
        return m_YScale;
    }

  private:
    float m_XScale;
    float m_YScale;
};

class FileDropEvent final : public Event
{
  public:
    explicit FileDropEvent(std::vector<std::string> paths) noexcept
        : Event(static_cast<uint32_t>(EventType::FILE_DROP), EventCategory::WINDOW), m_Paths(std::move(paths))
    {
    }

    [[nodiscard]] const std::vector<std::string> &GetPaths() const noexcept
    {
        return m_Paths;
    }

    [[nodiscard]] size_t GetCount() const noexcept
    {
        return m_Paths.size();
    }

  private:
    std::vector<std::string> m_Paths;
};

// Controller Events

class ControllerConnectedEvent final : public Event
{
  public:
    explicit constexpr ControllerConnectedEvent(int32_t controllerId) noexcept
        : Event(static_cast<uint32_t>(EventType::CONTROLLER_CONNECTED),
                EventCategory::INPUT | EventCategory::CONTROLLER),
          m_ControllerId(controllerId)
    {
    }

    [[nodiscard]] constexpr int32_t GetControllerId() const noexcept
    {
        return m_ControllerId;
    }

  private:
    int32_t m_ControllerId;
};

class ControllerDisconnectedEvent final : public Event
{
  public:
    explicit constexpr ControllerDisconnectedEvent(int32_t controllerId) noexcept
        : Event(static_cast<uint32_t>(EventType::CONTROLLER_DISCONNECTED),
                EventCategory::INPUT | EventCategory::CONTROLLER),
          m_ControllerId(controllerId)
    {
    }

    [[nodiscard]] constexpr int32_t GetControllerId() const noexcept
    {
        return m_ControllerId;
    }

  private:
    int32_t m_ControllerId;
};

// Application Events

class UpdateEvent final : public Event
{
  public:
    explicit constexpr UpdateEvent(double deltaTime) noexcept
        : Event(static_cast<uint32_t>(EventType::APP_UPDATE), EventCategory::APPLICATION), m_DeltaTime(deltaTime)
    {
    }

    [[nodiscard]] constexpr double GetDeltaTime() const noexcept
    {
        return m_DeltaTime;
    }

  private:
    double m_DeltaTime;
};

class RenderEvent final : public Event
{
  public:
    constexpr RenderEvent() noexcept : Event(static_cast<uint32_t>(EventType::APP_RENDER), EventCategory::APPLICATION)
    {
    }
};

// EventTypeId specializations for built-in events

template <> struct EventTypeId<KeyPressedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::KEY_PRESSED);
    }
};

template <> struct EventTypeId<KeyReleasedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::KEY_RELEASED);
    }
};

template <> struct EventTypeId<KeyTypedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::KEY_TYPED);
    }
};

template <> struct EventTypeId<MouseButtonPressedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::MOUSE_BUTTON_PRESSED);
    }
};

template <> struct EventTypeId<MouseButtonReleasedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::MOUSE_BUTTON_RELEASED);
    }
};

template <> struct EventTypeId<MouseMovedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::MOUSE_MOVED);
    }
};

template <> struct EventTypeId<MouseScrolledEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::MOUSE_SCROLLED);
    }
};

template <> struct EventTypeId<MouseEnteredEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::MOUSE_ENTERED);
    }
};

template <> struct EventTypeId<MouseExitedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::MOUSE_EXITED);
    }
};

template <> struct EventTypeId<WindowResizeEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::WINDOW_RESIZE);
    }
};

template <> struct EventTypeId<WindowMinimizedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::WINDOW_MINIMIZED);
    }
};

template <> struct EventTypeId<WindowMaximizedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::WINDOW_MAXIMIZED);
    }
};

template <> struct EventTypeId<WindowRestoredEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::WINDOW_RESTORED);
    }
};

template <> struct EventTypeId<WindowMovedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::WINDOW_MOVED);
    }
};

template <> struct EventTypeId<WindowFocusedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::WINDOW_FOCUSED);
    }
};

template <> struct EventTypeId<WindowCloseEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::WINDOW_CLOSE);
    }
};

template <> struct EventTypeId<FramebufferResizeEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::FRAMEBUFFER_RESIZE);
    }
};

template <> struct EventTypeId<ContentScaleChangedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::CONTENT_SCALE_CHANGED);
    }
};

template <> struct EventTypeId<FileDropEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::FILE_DROP);
    }
};

template <> struct EventTypeId<ControllerConnectedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::CONTROLLER_CONNECTED);
    }
};

template <> struct EventTypeId<ControllerDisconnectedEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::CONTROLLER_DISCONNECTED);
    }
};

template <> struct EventTypeId<UpdateEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::APP_UPDATE);
    }
};

template <> struct EventTypeId<RenderEvent>
{
    [[nodiscard]] static constexpr uint32_t Get() noexcept
    {
        return static_cast<uint32_t>(EventType::APP_RENDER);
    }
};

} // namespace ae
