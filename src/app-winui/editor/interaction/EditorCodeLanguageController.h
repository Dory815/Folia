#pragma once

#include "editor/session/EditorSession.h"
#include "editor/rendering/EditorSurfaceRenderer.h"

namespace winrt::Folia
{
    class EditorCodeLanguageController
    {
    public:
        using ExecuteCommand = std::function<bool(folia::Command const&)>;

        void Attach(
            EditorSession& session,
            EditorSurfaceRenderer& renderer,
            winrt::Microsoft::UI::Xaml::Controls::SwapChainPanel const& editorSurface,
            winrt::Microsoft::UI::Xaml::Controls::Canvas const& overlay,
            winrt::Microsoft::UI::Xaml::Controls::ComboBox const& languageBox,
            ExecuteCommand executeCommand);
        void Detach();
        void Refresh();

    private:
        void Commit();
        void PopulateLanguages();

        EditorSession* session_ = nullptr;
        EditorSurfaceRenderer* renderer_ = nullptr;
        winrt::Microsoft::UI::Xaml::Controls::SwapChainPanel editorSurface_{ nullptr };
        winrt::Microsoft::UI::Xaml::Controls::Canvas overlay_{ nullptr };
        winrt::Microsoft::UI::Xaml::Controls::ComboBox languageBox_{ nullptr };
        ExecuteCommand executeCommand_;
        std::optional<folia::platform::editor::EditorVisualRawBlock> activeBlock_;
        winrt::event_token selectionChangedToken_{};
        winrt::event_token dropDownClosedToken_{};
        winrt::event_token lostFocusToken_{};
        winrt::event_token keyDownToken_{};
        bool updating_ = false;
    };
}
