#include "pch.h"

import std;
#include "media/NativeMathSession.h"


#include "folia/math/native_renderer_bridge.h"
#include "media/MathJaxSvgParser.h"

namespace winrt::Folia
{
    namespace
    {
        using RendererHandle = std::unique_ptr<FoliaNativeMathRenderer,
            decltype(&FoliaNativeMathRendererDestroy)>;
        using CancellationHandle = std::unique_ptr<FoliaNativeMathCancellation,
            decltype(&FoliaNativeMathCancellationDestroy)>;
        using ResultHandle = std::unique_ptr<FoliaNativeMathResult,
            decltype(&FoliaNativeMathResultDestroy)>;

        MathJaxSvg InfrastructureFailure(std::string message, bool display)
        {
            MathJaxSvg result;
            result.display = display;
            result.error = std::move(message);
            result.errorKind = MathJaxErrorKind::Infrastructure;
            return result;
        }
    }

    struct NativeMathSession::State
    {
        State() : owner(std::this_thread::get_id()), renderer(nullptr, &FoliaNativeMathRendererDestroy) {}

        ~State()
        {
            // The bridge requires renderer destruction on the creating thread.
            if (renderer && owner != std::this_thread::get_id())
                std::terminate();
        }

        void RequireOwnerThread() const
        {
            if (owner != std::this_thread::get_id())
                throw std::logic_error("NativeMathSession must be used on its creating thread");
        }

        std::thread::id owner;
        RendererHandle renderer;
    };

    NativeMathSession::NativeMathSession() : state(std::make_unique<State>()) {}

    NativeMathSession::~NativeMathSession() = default;

    MathJaxSvg NativeMathSession::Render(
        std::string_view tex,
        bool display,
        float em,
        float containerWidth,
        std::stop_token stop)
    {
        state->RequireOwnerThread();
        if (stop.stop_requested())
            throw winrt::hresult_canceled();

        if (!state->renderer)
        {
            FoliaNativeMathRenderer* created = nullptr;
            const auto createStatus = FoliaNativeMathRendererCreate(&created);
            state->renderer.reset(created);
            if (createStatus != FOLIA_NATIVE_MATH_OK || !state->renderer)
            {
                state->renderer.reset();
                if (createStatus == FOLIA_NATIVE_MATH_OUT_OF_MEMORY) throw std::bad_alloc();
                return InfrastructureFailure("Native math renderer could not be created", display);
            }
        }

        FoliaNativeMathCancellation* cancellationRaw = nullptr;
        const auto cancellationStatus = FoliaNativeMathCancellationCreate(&cancellationRaw);
        CancellationHandle cancellation{ cancellationRaw, &FoliaNativeMathCancellationDestroy };
        if (cancellationStatus != FOLIA_NATIVE_MATH_OK || !cancellation)
        {
            state->renderer.reset();
            if (cancellationStatus == FOLIA_NATIVE_MATH_OUT_OF_MEMORY) throw std::bad_alloc();
            return InfrastructureFailure("Native math cancellation handle could not be created", display);
        }

        FoliaNativeMathResult* resultRaw = nullptr;
        ResultHandle nativeResult{ nullptr, &FoliaNativeMathResultDestroy };
        FoliaNativeMathStatus status = FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
        try
        {
            std::stop_callback stopCallback(
                stop, [handle = cancellation.get()] { FoliaNativeMathCancellationRequestStop(handle); });

            const auto text = winrt::to_hstring(std::string_view(tex.data(), tex.size()));
            std::vector<std::uint16_t> utf16Units;
            utf16Units.reserve(text.size());
            for (wchar_t codeUnit : text)
                utf16Units.push_back(static_cast<std::uint16_t>(codeUnit));

            status = FoliaNativeMathRenderSvg(
                state->renderer.get(), utf16Units.data(), utf16Units.size(), display ? 1 : 0,
                em, containerWidth, cancellation.get(), 2000, &resultRaw);
            nativeResult.reset(resultRaw);
        }
        catch (std::bad_alloc const&)
        {
            nativeResult.reset(resultRaw);
            state->renderer.reset();
            throw;
        }
        catch (winrt::hresult_canceled const&)
        {
            nativeResult.reset(resultRaw);
            state->renderer.reset();
            throw;
        }
        catch (std::exception const& error)
        {
            nativeResult.reset(resultRaw);
            state->renderer.reset();
            return InfrastructureFailure(error.what(), display);
        }
        catch (...)
        {
            nativeResult.reset(resultRaw);
            state->renderer.reset();
            return InfrastructureFailure("Unknown native math renderer failure", display);
        }

        if (status == FOLIA_NATIVE_MATH_CANCELLED || stop.stop_requested())
        {
            state->renderer.reset();
            throw winrt::hresult_canceled();
        }

        if (status != FOLIA_NATIVE_MATH_OK)
        {
            if (status == FOLIA_NATIVE_MATH_OUT_OF_MEMORY)
            {
                state->renderer.reset();
                throw std::bad_alloc();
            }
            std::size_t errorSize = 0;
            state->renderer.reset();
            const auto errorBytes = FoliaNativeMathResultError(nativeResult.get(), &errorSize);
            std::string error = errorBytes ? std::string(errorBytes, errorSize) : "Native math renderer failed";
            return InfrastructureFailure(std::move(error), display);
        }

        std::size_t svgSize = 0;
        const auto svgBytes = FoliaNativeMathResultSvg(nativeResult.get(), &svgSize);
        if (!svgBytes && svgSize != 0)
        {
            state->renderer.reset();
            return InfrastructureFailure("Native renderer returned an invalid SVG result", display);
        }

        try
        {
            std::string ownedSvg(svgBytes ? svgBytes : "", svgSize);
            nativeResult.reset();
            return ParseMathJaxSvgOutput(ownedSvg, display, em);
        }
        catch (std::bad_alloc const&)
        {
            state->renderer.reset();
            throw;
        }
        catch (std::exception const& error)
        {
            state->renderer.reset();
            return InfrastructureFailure(error.what(), display);
        }
        catch (...)
        {
            state->renderer.reset();
            return InfrastructureFailure("Unknown native math SVG parsing failure", display);
        }
    }
}
