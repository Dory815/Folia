#include "pch.h"
#include "editor/interaction/EditorCodeLanguageController.h"

import folia.core.command;
import folia.core.utf;

namespace winrt::Folia
{
    namespace
    {
        std::string NormalizeLanguage(winrt::hstring const& value)
        {
            auto text = winrt::to_string(value);
            auto first = text.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) return {};
            auto last = text.find_first_of(" \t\r\n", first);
            return text.substr(first, last == std::string::npos ? last : last - first);
        }
    }

    void EditorCodeLanguageController::Attach(
        EditorSession& session,
        EditorSurfaceRenderer& renderer,
        winrt::Microsoft::UI::Xaml::Controls::SwapChainPanel const& editorSurface,
        winrt::Microsoft::UI::Xaml::Controls::Canvas const& overlay,
        winrt::Microsoft::UI::Xaml::Controls::ComboBox const& languageBox,
        ExecuteCommand executeCommand)
    {
        Detach();
        session_ = &session;
        renderer_ = &renderer;
        editorSurface_ = editorSurface;
        overlay_ = overlay;
        languageBox_ = languageBox;
        executeCommand_ = std::move(executeCommand);
        PopulateLanguages();
        selectionChangedToken_ = languageBox_.SelectionChanged([this](auto const&, auto const&)
        {
            if (updating_) return;
            if (auto selected = languageBox_.SelectedItem())
                languageBox_.Text(unbox_value<winrt::hstring>(selected));
            Commit();
        });
        dropDownClosedToken_ = languageBox_.DropDownClosed([this](auto const&, auto const&)
        {
            Commit();
        });
        lostFocusToken_ = languageBox_.LostFocus([this](auto const&, auto const&)
        {
            Commit();
        });
        keyDownToken_ = languageBox_.KeyDown([this](auto const&, auto const& args)
        {
            if (args.Key() != winrt::Windows::System::VirtualKey::Enter) return;
            Commit();
            if (editorSurface_) editorSurface_.Focus(
                winrt::Microsoft::UI::Xaml::FocusState::Programmatic);
            args.Handled(true);
        });
    }

    void EditorCodeLanguageController::Detach()
    {
        if (languageBox_)
        {
            if (selectionChangedToken_.value) languageBox_.SelectionChanged(selectionChangedToken_);
            if (dropDownClosedToken_.value) languageBox_.DropDownClosed(dropDownClosedToken_);
            if (lostFocusToken_.value) languageBox_.LostFocus(lostFocusToken_);
            if (keyDownToken_.value) languageBox_.KeyDown(keyDownToken_);
            languageBox_.Visibility(winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
        }
        if (overlay_) overlay_.IsHitTestVisible(false);
        activeBlock_.reset();
        executeCommand_ = {};
        languageBox_ = nullptr;
        overlay_ = nullptr;
        editorSurface_ = nullptr;
        renderer_ = nullptr;
        session_ = nullptr;
        selectionChangedToken_ = {};
        dropDownClosedToken_ = {};
        lostFocusToken_ = {};
        keyDownToken_ = {};
    }

    void EditorCodeLanguageController::PopulateLanguages()
    {
        if (!languageBox_) return;
        updating_ = true;
        languageBox_.Items().Clear();
        for (auto const* language : {
            L"text", L"cpp", L"c", L"csharp", L"java", L"javascript",
            L"typescript", L"python", L"rust", L"go", L"json", L"yaml",
            L"html", L"css", L"shell", L"powershell", L"sql", L"markdown",
            L"mermaid"})
        {
            languageBox_.Items().Append(box_value(language));
        }
        updating_ = false;
    }

    void EditorCodeLanguageController::Refresh()
    {
        using namespace winrt::Microsoft::UI::Xaml;
        using namespace winrt::Microsoft::UI::Xaml::Controls;
        if (!session_ || !renderer_ || !editorSurface_ || !overlay_ || !languageBox_) return;
        auto control = renderer_->RawBlockFor(session_->Selection().active.container_id);
        if (!control
            || control->kind != folia::platform::editor::EditorRawBlockKind::Code
            || !control->languageEditable)
        {
            activeBlock_.reset();
            languageBox_.Visibility(Visibility::Collapsed);
            overlay_.IsHitTestVisible(false);
            return;
        }

        auto sameOwner = activeBlock_
            && activeBlock_->contentStart.container_id == control->contentStart.container_id;
        activeBlock_ = control;
        if (!sameOwner || languageBox_.FocusState() == FocusState::Unfocused)
        {
            updating_ = true;
            languageBox_.SelectedItem(nullptr);
            languageBox_.Text(winrt::to_hstring(control->language.value_or("text")));
            updating_ = false;
        }

        auto transform = editorSurface_.TransformToVisual(overlay_);
        auto topLeft = transform.TransformPoint({control->headerRect.left, control->headerRect.top});
        auto width = (std::clamp)(
            static_cast<double>(control->headerRect.right - control->headerRect.left - 8.0f),
            96.0,
            190.0);
        languageBox_.Width(width);
        Canvas::SetLeft(languageBox_, static_cast<double>(topLeft.X) + 4.0);
        Canvas::SetTop(languageBox_, static_cast<double>(topLeft.Y) + 1.0);
        languageBox_.Visibility(Visibility::Visible);
        overlay_.IsHitTestVisible(true);
    }

    void EditorCodeLanguageController::Commit()
    {
        if (updating_ || !session_ || !activeBlock_ || !languageBox_ || !executeCommand_) return;
        auto language = NormalizeLanguage(languageBox_.Text());
        auto current = activeBlock_->language.value_or("text");
        if (language == current || (language == "text" && !activeBlock_->language)) return;
        if (session_->Selection().active.container_id
            != activeBlock_->contentStart.container_id) return;

        folia::Command command;
        command.kind = folia::CommandKind::SetCodeBlockLanguage;
        if (!language.empty() && language != "text")
            command.lang = folia::utf8_to_cps(language);
        else
            command.lang = std::u32string{};
        if (executeCommand_(command))
            activeBlock_->language = language.empty() || language == "text"
                ? std::nullopt
                : std::optional<std::string>{language};
    }
}
