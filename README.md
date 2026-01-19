# Event Library

## General

This library provides a simple event system and layer stack for event-driven architecture in `C++` applications. It features global event broadcasting via listeners and layer-based event propagation. The library is built using [Premake5](https://premake.github.io/) for `C++23`.

This library depends on [log-lib](https://github.com/rasmushugosson/log-lib), which is included as a git submodule.

## Getting Started

1. **Clone the repository** with submodules and open a terminal in the project root.
   ```bash
   git clone --recursive https://github.com/rasmushugosson/event-lib.git
   ```

The next steps depend on your preferred build system below.

### Visual Studio

2. Run `premake5 vs20XX` to generate a Visual Studio solution file (`.sln`).
3. Open the solution file in Visual Studio and build using MSVC.

### Gmake (force G++)

2. Run the pre-defined action `premake5 gmake-gcc` to generate makefiles specifically for GCC.
3. Navigate into `/build/[os]-gcc` where the `Makefile` is created.
4. Run `make config=[build type]` where the possible options are `debug`, `release` or `dist`.
5. Navigate into `/bin/Sandbox/[build type]` and run the `Sandbox` executable.

### Gmake (force Clang++)

2. Run the pre-defined action `premake5 gmake-clang` to generate makefiles specifically for Clang.
3. Navigate into `/build/[os]-clang` where the `Makefile` is created.
4. Run `make config=[build type]` where the possible options are `debug`, `release` or `dist`.
5. Navigate into `/bin/Sandbox/[build type]` and run the `Sandbox` executable.

### Formatting and Linting

There are additional actions for formatting with `clang-format` and linting through `clang-tidy`. These are run through:

```bash
# Run clang-format
premake5 format

# Run clang-tidy
premake5 lint

# Run clang-tidy and apply fixes
premake5 lint-fix
```

These commands assume `clang-format` and `clang-tidy` are installed on your system.

### Additional Dependencies

- **Premake5:** This library uses [Premake5](https://premake.github.io/) as its build configuration tool.
  Ensure that `premake5` is installed on your system or copied into the root folder.
  You can download it [here](https://premake.github.io/download/).

## Using as a Submodule

This library can be used as a git submodule in other Premake5 projects. Add it as a submodule (with recursive init to get log-lib) and include the Event project definition in your `premake5.lua`:

```lua
include("path/to/event-lib/event-project.lua")

project("YourProject")
    -- ...
    includedirs({
        "path/to/event-lib/dep/log-lib/log-lib/include",
        "path/to/event-lib/event-lib/include"
    })
    links({ "Log", "Event" })
```

The `event-project.lua` file defines the Event project and automatically includes the Log project dependency. The `premake5.lua` is used for standalone builds including the Sandbox example.

## Usage

The event system has two main components: **EventManager/EventListener** for global event broadcasting, and **LayerStack** for layer-based event propagation. Include `Event.h` for event types and listeners, and `Layer.h` for layers and the layer stack.

### Architecture

The system provides a two-tier event model:

1. **EventManager/EventListener (Global Broadcast):** When `event.Dispatch()` is called, the event is broadcast to all registered `EventListener` objects. Each listener receives events independently.

2. **LayerStack (Layer-based Propagation):** The `LayerStack` has its own `EventListener` that automatically receives dispatched events and routes them to layers. Events propagate top-to-bottom (overlays first) and can be consumed to stop further propagation.

### Events

Events are lightweight objects with no virtual methods for minimal overhead. Built-in event types include:

- **Keyboard:** `KeyPressedEvent`, `KeyReleasedEvent`, `KeyTypedEvent`
- **Mouse:** `MouseButtonPressedEvent`, `MouseButtonReleasedEvent`, `MouseMovedEvent`, `MouseScrolledEvent`, `MouseEnteredEvent`, `MouseExitedEvent`
- **Window:** `WindowResizeEvent`, `WindowMinimizedEvent`, `WindowMaximizedEvent`, `WindowRestoredEvent`, `WindowMovedEvent`, `WindowFocusedEvent`, `WindowCloseEvent`, `FramebufferResizeEvent`, `ContentScaleChangedEvent`, `FileDropEvent`
- **Controller:** `ControllerConnectedEvent`, `ControllerDisconnectedEvent`
- **Application:** `UpdateEvent`, `RenderEvent`

### Event Listener

The `EventListener` class automatically registers with the `EventManager` on construction and unregisters on destruction:

```cpp
ae::EventListener listener;
listener.SetCallback([](ae::Event& event) {
    if (event.GetTypeId() == ae::EventTypeId<ae::KeyPressedEvent>::Get())
    {
        auto& keyEvent = static_cast<ae::KeyPressedEvent&>(event);
        // Handle key press
    }
});

// Later, dispatch an event (broadcasts to all listeners)
ae::KeyPressedEvent keyEvent(65, false);
keyEvent.Dispatch();
```

### Custom Events

Custom events automatically receive unique type IDs (>= 1000):

```cpp
class PlayerDiedEvent final : public ae::Event
{
public:
    explicit PlayerDiedEvent(uint32_t playerId) noexcept
        : ae::Event(ae::EventTypeId<PlayerDiedEvent>::Get(), ae::EventCategory::CUSTOM)
        , m_PlayerId(playerId) {}

    [[nodiscard]] uint32_t GetPlayerId() const noexcept { return m_PlayerId; }

private:
    uint32_t m_PlayerId;
};
```

### Layers and LayerStack

Layers are application components that can receive events, update, and render. The `LayerStack` manages layers and overlays:

```cpp
class GameLayer : public ae::Layer
{
public:
    GameLayer() : ae::Layer("GameLayer") {}

private:
    void OnAttach() override { /* Initialize */ }
    void OnDetach() override { /* Cleanup */ }

    void OnEvent(ae::Event& event) override
    {
        if (event.GetTypeId() == ae::EventTypeId<ae::KeyPressedEvent>::Get())
        {
            event.Consume(); // Stop propagation to other layers
        }
    }

    void OnUpdate(double deltaTime) override { /* Update logic */ }
    void OnRender() override { /* Render */ }
    void OnImGuiRender() override { /* ImGui UI */ }
};

// Usage
ae::LayerStack layerStack;
GameLayer gameLayer;
layerStack.PushLayer(&gameLayer);

// Events dispatched globally are automatically routed to layers
ae::KeyPressedEvent keyEvent(65, false);
keyEvent.Dispatch();

// Manual update/render calls
layerStack.OnUpdate(deltaTime);
layerStack.OnRender();
```

### Event Categories

Events can be filtered by category using bitmasks:

```cpp
if (event.IsInCategory(ae::EventCategory::INPUT))
{
    // Handle input event
}
```

Available categories: `NONE`, `INPUT`, `KEYBOARD`, `MOUSE`, `CONTROLLER`, `WINDOW`, `APPLICATION`, `CUSTOM`.

### Build Configurations

The build configuration determines which logging macros from log-lib are active:

| Configuration | Define        | Logging                                      |
|---------------|---------------|----------------------------------------------|
| `debug`       | `AE_DEBUG`    | `AE_LOG()` and `AE_LOG_BOTH()` are active    |
| `release`     | `AE_RELEASE`  | `AE_LOG_RELEASE()` and `AE_LOG_BOTH()` are active |
| `dist`        | `AE_DIST`     | All logging disabled                         |

### Code Example

A complete example project demonstrating the event system can be found in `sandbox/src/Sandbox.cpp`.

## Supported Platforms

- Windows (MSVC)
- Linux (GCC / Clang)
- Likely MacOS (not yet tested)

## License

This library is licensed under the **Apache License 2.0**.
See the [LICENSE](LICENSE) file in this repository for details.
