#include "general/pch.h"

#include "Event.h"
#include "Layer.h"

#include <algorithm>
#include <ranges>

ae::LayerStack::LayerStack()
{
    m_Listener.SetCallback([this](Event &event) { OnEvent(event); });
}

ae::LayerStack::~LayerStack()
{
    for (Layer *pLayer : m_Layers)
    {
        if (pLayer != nullptr)
        {
            pLayer->OnDetach();
        }
    }
}

void ae::LayerStack::PushLayer(Layer *pLayer)
{
    if (pLayer == nullptr)
    {
        AE_LOG(AE_WARNING, "Tried to push null layer to LayerStack");
        return;
    }

#ifdef AE_DEBUG
    if (std::ranges::find(m_Layers, pLayer) != m_Layers.end())
    {
        AE_LOG(AE_WARNING, "Tried to push layer '{}' that is already in the stack", pLayer->GetName());
        return;
    }
#endif

    // NOTE: Insert before overlays
    m_Layers.insert(m_Layers.begin() + m_LayerInsertIndex, pLayer);
    m_LayerInsertIndex++;
    pLayer->OnAttach();

    AE_LOG(AE_TRACE, "Pushed layer: {}", pLayer->GetName());
}

void ae::LayerStack::PopLayer(Layer *pLayer)
{
    if (pLayer == nullptr)
    {
        AE_LOG(AE_WARNING, "Tried to pop null layer from LayerStack");
        return;
    }

    auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, pLayer);
    if (it != m_Layers.begin() + m_LayerInsertIndex)
    {
        pLayer->OnDetach();
        m_Layers.erase(it);
        m_LayerInsertIndex--;

        AE_LOG(AE_TRACE, "Popped layer: {}", pLayer->GetName());
    }
    else
    {
        AE_LOG(AE_WARNING, "Layer not found in stack: {}", pLayer->GetName());
    }
}

void ae::LayerStack::PushOverlay(Layer *pOverlay)
{
    if (pOverlay == nullptr)
    {
        AE_LOG(AE_WARNING, "Tried to push null overlay to LayerStack");
        return;
    }

#ifdef AE_DEBUG
    if (std::ranges::find(m_Layers, pOverlay) != m_Layers.end())
    {
        AE_LOG(AE_WARNING, "Tried to push overlay '{}' that is already in the stack", pOverlay->GetName());
        return;
    }
#endif

    // NOTE: Overlays go at the end
    m_Layers.push_back(pOverlay);
    pOverlay->OnAttach();

    AE_LOG(AE_TRACE, "Pushed overlay: {}", pOverlay->GetName());
}

void ae::LayerStack::PopOverlay(Layer *pOverlay)
{
    if (pOverlay == nullptr)
    {
        AE_LOG(AE_WARNING, "Tried to pop null overlay from LayerStack");
        return;
    }

    auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), pOverlay);
    if (it != m_Layers.end())
    {
        pOverlay->OnDetach();
        m_Layers.erase(it);

        AE_LOG(AE_TRACE, "Popped overlay: {}", pOverlay->GetName());
    }
    else
    {
        AE_LOG(AE_WARNING, "Overlay not found in stack: {}", pOverlay->GetName());
    }
}

void ae::LayerStack::OnEvent(Event &event)
{
    // NOTE: Events propagate top-to-bottom
    for (auto *pLayer : std::ranges::reverse_view(m_Layers))
    {
        if (event.IsConsumed())
        {
            break;
        }

        if (pLayer != nullptr && pLayer->IsEnabled())
        {
            pLayer->OnEvent(event);
        }
    }
}

void ae::LayerStack::OnUpdate(double deltaTime)
{
    // NOTE: Update propagates bottom-to-top
    for (Layer *pLayer : m_Layers)
    {
        if (pLayer != nullptr && pLayer->IsEnabled())
        {
            pLayer->OnUpdate(deltaTime);
        }
    }
}

void ae::LayerStack::OnRender()
{
    // NOTE: Render propagates bottom-to-top
    for (Layer *pLayer : m_Layers)
    {
        if (pLayer != nullptr && pLayer->IsEnabled())
        {
            pLayer->OnRender();
        }
    }
}

void ae::LayerStack::OnImGuiRender()
{
    // NOTE: ImGui render propagates bottom-to-top
    for (Layer *pLayer : m_Layers)
    {
        if (pLayer != nullptr && pLayer->IsEnabled())
        {
            pLayer->OnImGuiRender();
        }
    }
}
