#include "folia/math/native_renderer_bridge.h"

import folia.math;
import std;

struct FoliaNativeMathRenderer {
    std::thread::id owner = std::this_thread::get_id();
    folia::math::Renderer renderer;
};

struct FoliaNativeMathCancellation {
    std::stop_source source;
};

struct FoliaNativeMathResult {
    std::string svg;
    std::string error;
};

extern "C" FoliaNativeMathStatus FoliaNativeMathRendererCreate(
    FoliaNativeMathRenderer** out_renderer) {
    if (out_renderer) *out_renderer = nullptr;
    if (!out_renderer) return FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
    try {
        auto renderer = std::make_unique<FoliaNativeMathRenderer>();
        *out_renderer = renderer.release();
        return FOLIA_NATIVE_MATH_OK;
    } catch (std::bad_alloc const&) {
        return FOLIA_NATIVE_MATH_OUT_OF_MEMORY;
    } catch (...) {
        return FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
    }
}

extern "C" void FoliaNativeMathRendererDestroy(
    FoliaNativeMathRenderer* renderer) {
    delete renderer;
}

extern "C" FoliaNativeMathStatus FoliaNativeMathCancellationCreate(
    FoliaNativeMathCancellation** out_cancellation) {
    if (out_cancellation) *out_cancellation = nullptr;
    if (!out_cancellation) return FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
    try {
        auto cancellation = std::make_unique<FoliaNativeMathCancellation>();
        *out_cancellation = cancellation.release();
        return FOLIA_NATIVE_MATH_OK;
    } catch (std::bad_alloc const&) {
        return FOLIA_NATIVE_MATH_OUT_OF_MEMORY;
    } catch (...) {
        return FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
    }
}

extern "C" void FoliaNativeMathCancellationRequestStop(
    FoliaNativeMathCancellation* cancellation) {
    if (cancellation) (void)cancellation->source.request_stop();
}

extern "C" void FoliaNativeMathCancellationDestroy(
    FoliaNativeMathCancellation* cancellation) {
    delete cancellation;
}

extern "C" FoliaNativeMathStatus FoliaNativeMathRenderSvg(
    FoliaNativeMathRenderer* renderer,
    uint16_t const* tex,
    size_t tex_units,
    int display,
    double em,
    double container_width,
    FoliaNativeMathCancellation* cancellation,
    unsigned int timeout_ms,
    FoliaNativeMathResult** out_result) {
    if (out_result) *out_result = nullptr;
    if (!out_result || !renderer || !cancellation || (tex_units != 0 && !tex)
        || display < 0 || display > 1 || !std::isfinite(em) || em <= 0
        || !std::isfinite(container_width) || container_width <= 0) {
        return FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
    }
    if (renderer->owner != std::this_thread::get_id())
        return FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;

    try {
        std::u16string input;
        input.reserve(tex_units);
        for (size_t i = 0; i < tex_units; ++i)
            input.push_back(static_cast<char16_t>(tex[i]));

        folia::math::RenderOptions options;
        options.display = display != 0;
        options.em = em;
        options.containerWidth = container_width;
        if (timeout_ms != 0) {
            options.deadline = std::chrono::steady_clock::now()
                + std::chrono::milliseconds(timeout_ms);
        }

        auto result = std::make_unique<FoliaNativeMathResult>();
        try {
            auto rendered = renderer->renderer.render(
                std::u16string_view(input), cancellation->source.get_token(), options);
            if (rendered.svg.empty() || rendered.svg.find("<svg") == std::string::npos) {
                result->error = "Native renderer completed without SVG output";
                *out_result = result.release();
                return FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
            }
            result->svg = std::move(rendered.svg);
            *out_result = result.release();
            return FOLIA_NATIVE_MATH_OK;
        } catch (folia::math::RenderError const& error) {
            result->error = error.what();
            const auto status = error.name == "Cancelled"
                ? FOLIA_NATIVE_MATH_CANCELLED
                : error.name == "SessionUnusable"
                    ? FOLIA_NATIVE_MATH_SESSION_UNUSABLE
                    : FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
            *out_result = result.release();
            return status;
        } catch (std::bad_alloc const&) {
            throw;
        } catch (std::exception const& error) {
            result->error = error.what();
            *out_result = result.release();
            return FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
        } catch (...) {
            result->error = "Unknown native math renderer failure";
            *out_result = result.release();
            return FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
        }
    } catch (std::bad_alloc const&) {
        // The status path itself does not allocate an error result.
        return FOLIA_NATIVE_MATH_OUT_OF_MEMORY;
    } catch (...) {
        return FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR;
    }
}

extern "C" char const* FoliaNativeMathResultSvg(
    FoliaNativeMathResult const* result,
    size_t* size) {
    if (size) *size = result ? result->svg.size() : 0;
    return result ? result->svg.data() : nullptr;
}

extern "C" char const* FoliaNativeMathResultError(
    FoliaNativeMathResult const* result,
    size_t* size) {
    if (size) *size = result ? result->error.size() : 0;
    return result ? result->error.data() : nullptr;
}

extern "C" void FoliaNativeMathResultDestroy(FoliaNativeMathResult* result) {
    delete result;
}
