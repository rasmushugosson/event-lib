#include "Layer.h"
#include "Log.h"

// Author: Rasmus Hugosson

// Description: This is a simple example of how to use this library

// Custom event example - events with type IDs >= 1000 are automatically assigned
class PlayerDiedEvent final : public ae::Event
{
  public:
    explicit PlayerDiedEvent(uint32_t playerId) noexcept
        : ae::Event(ae::EventTypeId<PlayerDiedEvent>::Get(), ae::EventCategory::CUSTOM), m_PlayerId(playerId)
    {
    }

    [[nodiscard]] uint32_t GetPlayerId() const noexcept
    {
        return m_PlayerId;
    }

  private:
    uint32_t m_PlayerId;
};

// Example layer that handles keyboard and mouse events
class InputLayer : public ae::Layer
{
  public:
    InputLayer() : Layer("Input") {}

  private:
    void OnAttach() override
    {
        AE_LOG(AE_INFO, "InputLayer attached");
    }

    void OnDetach() override
    {
        AE_LOG(AE_INFO, "InputLayer detached");
    }

    void OnEvent(ae::Event &event) override
    {
        // Handle key pressed events
        if (event.GetTypeId() == ae::EventTypeId<ae::KeyPressedEvent>::Get())
        {
            auto &keyEvent = static_cast<ae::KeyPressedEvent &>(event);
            AE_LOG(AE_INFO, "InputLayer: Key pressed: {} (repeat: {})", keyEvent.GetKeyCode(), keyEvent.IsRepeat());

            // Consume the escape key to prevent other layers from handling it
            if (keyEvent.GetKeyCode() == 256) // GLFW_KEY_ESCAPE
            {
                AE_LOG(AE_INFO, "InputLayer: Escape key consumed!");
                event.Consume();
            }
        }

        // Handle mouse moved events
        if (event.GetTypeId() == ae::EventTypeId<ae::MouseMovedEvent>::Get())
        {
            auto &mouseEvent = static_cast<ae::MouseMovedEvent &>(event);
            AE_LOG(AE_TRACE, "InputLayer: Mouse moved to ({}, {})", mouseEvent.GetX(), mouseEvent.GetY());
        }
    }

    void OnUpdate(double deltaTime) override
    {
        AE_LOG(AE_TRACE, "InputLayer update: {:.2f} ms", deltaTime * 1000.0);
    }

    void OnRender() override
    {
        // Rendering would go here
    }
};

// Example overlay that handles custom game events
class GameOverlay : public ae::Layer
{
  public:
    GameOverlay() : Layer("GameOverlay") {}

  private:
    void OnAttach() override
    {
        AE_LOG(AE_INFO, "GameOverlay attached");
    }

    void OnDetach() override
    {
        AE_LOG(AE_INFO, "GameOverlay detached");
    }

    void OnEvent(ae::Event &event) override
    {
        // Overlays receive events first, before regular layers
        // This overlay only handles custom events
        if (event.GetTypeId() == ae::EventTypeId<PlayerDiedEvent>::Get())
        {
            auto &playerEvent = static_cast<PlayerDiedEvent &>(event);
            AE_LOG(AE_WARNING, "GameOverlay: Player {} died!", playerEvent.GetPlayerId());
        }

        // Demonstrate that overlays can see key events before layers
        if (event.GetTypeId() == ae::EventTypeId<ae::KeyPressedEvent>::Get())
        {
            auto &keyEvent = static_cast<ae::KeyPressedEvent &>(event);
            AE_LOG(AE_TRACE, "GameOverlay: Saw key press {} (passing through)", keyEvent.GetKeyCode());
        }
    }
};

void Demo()
{
    // Set up logging
    ae::Logger::Get().AddConsoleSink("console", ae::LogSinkConsoleKind::STDOUT, AE_TRACE);

    AE_LOG(AE_INFO, "Event-lib Sandbox");
    AE_LOG_NEWLINE();

    // Create a layer stack - it automatically registers as an event listener
    ae::LayerStack layerStack;

    // Create and push layers
    InputLayer inputLayer;
    GameOverlay gameOverlay;

    layerStack.PushLayer(&inputLayer);    // Regular layer
    layerStack.PushOverlay(&gameOverlay); // Overlay receives events first

    AE_LOG_NEWLINE();
    AE_LOG(AE_INFO, "--- Dispatching events via event.Dispatch() ---");
    AE_LOG_NEWLINE();

    // Events dispatched this way automatically reach all registered listeners (including the LayerStack)
    ae::KeyPressedEvent keyEventA(65, false); // 'A' key
    keyEventA.Dispatch();

    AE_LOG_NEWLINE();

    ae::MouseMovedEvent mouseEvent(100.0f, 200.0f);
    mouseEvent.Dispatch();

    AE_LOG_NEWLINE();

    // Dispatch custom event
    PlayerDiedEvent playerEvent(42);
    playerEvent.Dispatch();

    AE_LOG_NEWLINE();
    AE_LOG(AE_INFO, "--- Demonstrating event consumption ---");
    AE_LOG_NEWLINE();

    // The escape key will be consumed by InputLayer
    ae::KeyPressedEvent escapeEvent(256, false); // Escape key
    escapeEvent.Dispatch();

    AE_LOG_NEWLINE();
    AE_LOG(AE_INFO, "--- Demonstrating category filtering ---");
    AE_LOG_NEWLINE();

    // Events can be filtered by category
    ae::WindowResizeEvent resizeEvent(1920, 1080);
    AE_LOG(AE_INFO, "WindowResizeEvent is in WINDOW category: {}", resizeEvent.IsInCategory(ae::EventCategory::WINDOW));
    AE_LOG(AE_INFO, "WindowResizeEvent is in INPUT category: {}", resizeEvent.IsInCategory(ae::EventCategory::INPUT));

    ae::KeyPressedEvent keyEventB(66, false);
    AE_LOG(AE_INFO, "KeyPressedEvent is in KEYBOARD category: {}", keyEventB.IsInCategory(ae::EventCategory::KEYBOARD));
    AE_LOG(AE_INFO, "KeyPressedEvent is in INPUT category: {}", keyEventB.IsInCategory(ae::EventCategory::INPUT));

    AE_LOG_NEWLINE();
    AE_LOG(AE_INFO, "--- Manual update/render calls ---");
    AE_LOG_NEWLINE();

    // Update and render are called manually (not through event dispatch)
    layerStack.OnUpdate(0.016); // 16ms delta time
    layerStack.OnRender();

    AE_LOG_NEWLINE();

    // Layers are automatically detached when LayerStack is destroyed
    AE_LOG(AE_INFO, "Sandbox complete - LayerStack going out of scope");
}

int main()
{
    try
    {
        Demo();
        return EXIT_SUCCESS;
    }

    catch (...)
    {
        std::fputs("Fatal error: unknown exception\n", stderr);
        return EXIT_FAILURE;
    }
}
