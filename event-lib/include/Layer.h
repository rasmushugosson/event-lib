#pragma once

#include "Event.h"

#include <cstdint>
#include <string>
#include <vector>

namespace ae
{

class Layer
{
    friend class LayerStack;

  public:
    explicit Layer(const std::string &name = "Layer");
    Layer(const Layer &) = delete;
    Layer &operator=(const Layer &) = delete;
    Layer(Layer &&) = default;
    Layer &operator=(Layer &&) = default;
    virtual ~Layer() = default;

    [[nodiscard]] const std::string &GetName() const noexcept
    {
        return m_Name;
    }

    [[nodiscard]] bool IsEnabled() const noexcept
    {
        return m_Enabled;
    }

    void SetEnabled(bool enabled) noexcept
    {
        m_Enabled = enabled;
    }

  protected:
    std::string m_Name;
    bool m_Enabled = true;

  private:
    virtual void OnAttach() {}

    virtual void OnDetach() {}

    virtual void OnEvent([[maybe_unused]] Event &event) {}

    virtual void OnUpdate([[maybe_unused]] double deltaTime) {}

    virtual void OnRender() {}

    virtual void OnImGuiRender() {}
};

class LayerStack
{
  public:
    LayerStack();
    LayerStack(const LayerStack &) = delete;
    LayerStack &operator=(const LayerStack &) = delete;
    LayerStack(LayerStack &&) = default;
    LayerStack &operator=(LayerStack &&) = default;
    ~LayerStack();

    void PushLayer(Layer *pLayer);

    void PopLayer(Layer *pLayer);

    void PushOverlay(Layer *pOverlay);

    void PopOverlay(Layer *pOverlay);

    void OnEvent(Event &event);

    void OnUpdate(double deltaTime);

    void OnRender();

    void OnImGuiRender();

    [[nodiscard]] auto begin() noexcept
    {
        return m_Layers.begin();
    }

    [[nodiscard]] auto end() noexcept
    {
        return m_Layers.end();
    }

    [[nodiscard]] auto begin() const noexcept
    {
        return m_Layers.begin();
    }

    [[nodiscard]] auto end() const noexcept
    {
        return m_Layers.end();
    }

    [[nodiscard]] auto rbegin() noexcept
    {
        return m_Layers.rbegin();
    }

    [[nodiscard]] auto rend() noexcept
    {
        return m_Layers.rend();
    }

    [[nodiscard]] auto rbegin() const noexcept
    {
        return m_Layers.rbegin();
    }

    [[nodiscard]] auto rend() const noexcept
    {
        return m_Layers.rend();
    }

    [[nodiscard]] size_t Size() const noexcept
    {
        return m_Layers.size();
    }

    [[nodiscard]] bool Empty() const noexcept
    {
        return m_Layers.empty();
    }

  private:
    std::vector<Layer *> m_Layers;
    uint32_t m_LayerInsertIndex = 0; // NOTE: Boundary between layers and overlays
    EventListener m_Listener;
};

} // namespace ae
