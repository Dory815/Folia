#include "folia/math/native_renderer_bridge.h"

import std;

namespace {
using Renderer = std::unique_ptr<FoliaNativeMathRenderer,
    decltype(&FoliaNativeMathRendererDestroy)>;
using Cancellation = std::unique_ptr<FoliaNativeMathCancellation,
    decltype(&FoliaNativeMathCancellationDestroy)>;
using Result = std::unique_ptr<FoliaNativeMathResult,
    decltype(&FoliaNativeMathResultDestroy)>;

struct RendererCall {
    FoliaNativeMathStatus status;
    Renderer value;
};
struct CancellationCall {
    FoliaNativeMathStatus status;
    Cancellation value;
};
struct RenderCall {
    FoliaNativeMathStatus status;
    Result result;
};

bool Check(bool condition, std::string_view message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}

RendererCall MakeRenderer() {
    FoliaNativeMathRenderer* raw = nullptr;
    const auto status = FoliaNativeMathRendererCreate(&raw);
    return {status, Renderer(raw, FoliaNativeMathRendererDestroy)};
}
CancellationCall MakeCancellation() {
    FoliaNativeMathCancellation* raw = nullptr;
    const auto status = FoliaNativeMathCancellationCreate(&raw);
    return {status, Cancellation(raw, FoliaNativeMathCancellationDestroy)};
}
std::vector<uint16_t> Units(std::u16string_view input) {
    std::vector<uint16_t> result;
    result.reserve(input.size());
    for (char16_t unit : input) result.push_back(static_cast<uint16_t>(unit));
    return result;
}
RenderCall Render(FoliaNativeMathRenderer* renderer,
    FoliaNativeMathCancellation* cancellation, std::u16string_view input,
    unsigned int timeoutMs = 0) {
    const auto units = Units(input);
    FoliaNativeMathResult* raw = nullptr;
    const auto status = FoliaNativeMathRenderSvg(renderer, units.data(),
        units.size(), 0, 16.0, 640.0, cancellation, timeoutMs, &raw);
    return {status, Result(raw, FoliaNativeMathResultDestroy)};
}
std::string LargeFormula() {
    std::string tex = "x";
    for (int i = 0; i < 4000; ++i) {
        tex += "+\\frac{x_{" + std::to_string(i)
            + "}^2+x_{" + std::to_string(i) + "}+1}{y_{"
            + std::to_string(i) + "}^2+y_{" + std::to_string(i) + "}+1}";
    }
    return tex;
}

bool WasCancelled(FoliaNativeMathStatus status) {
    return status == FOLIA_NATIVE_MATH_CANCELLED
        || status == FOLIA_NATIVE_MATH_SESSION_UNUSABLE;
}
std::string Svg(FoliaNativeMathResult const* result) {
    size_t size = 0;
    const auto bytes = FoliaNativeMathResultSvg(result, &size);
    return bytes ? std::string(bytes, size) : std::string{};
}
}

int main() {
    bool passed = true;

    auto invalidCreate = FoliaNativeMathRendererCreate(nullptr);
    passed &= Check(invalidCreate == FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR,
        "null create output is a non-allocating infrastructure error");

    auto rendererCall = MakeRenderer();
    auto cancellationCall = MakeCancellation();
    auto renderer = std::move(rendererCall.value);
    auto cancellation = std::move(cancellationCall.value);
    passed &= Check(rendererCall.status == FOLIA_NATIVE_MATH_OK && renderer
            && cancellationCall.status == FOLIA_NATIVE_MATH_OK && cancellation,
        "C ABI creates opaque renderer and cancellation handles");
    if (!renderer || !cancellation) return 1;

    FoliaNativeMathResult* invalidResult = nullptr;
    const auto invalid = FoliaNativeMathRenderSvg(nullptr, nullptr, 1, 0,
        16.0, 640.0, cancellation.get(), 0, &invalidResult);
    passed &= Check(invalid == FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR
            && invalidResult == nullptr,
        "invalid arguments leave result null and return infrastructure status");

    // UTF-16 input exercises the same code-unit boundary used by Renderer.
    auto utf16 = Render(renderer.get(), cancellation.get(), u"\\text{漢字😀 é}+x");
    passed &= Check(utf16.status == FOLIA_NATIVE_MATH_OK && utf16.result
            && Svg(utf16.result.get()).find("<svg") != std::string::npos,
        "UTF-16 TeX reaches the real Folia Renderer and returns SVG");

    // The fixed profile includes noundefined: an unknown control sequence is
    // recovered as red mtext, while malformed structure produces an merror.
    auto unknownCommand = Render(renderer.get(), cancellation.get(), u"\\notacommand");
    const auto unknownSvg = Svg(unknownCommand.result.get());
    passed &= Check(unknownCommand.status == FOLIA_NATIVE_MATH_OK
            && unknownCommand.result && unknownSvg.find("<svg") != std::string::npos
            && unknownSvg.find("data-mml-node=\"mtext\" fill=\"red\"")
                != std::string::npos
            && unknownSvg.find("data-mml-node=\"merror\"") == std::string::npos,
        "noundefined recovers unknown commands as explicit red SVG text");

    auto structuralError = Render(renderer.get(), cancellation.get(), u"\\frac{");
    const auto structuralSvg = Svg(structuralError.result.get());
    passed &= Check(structuralError.status == FOLIA_NATIVE_MATH_OK
            && structuralError.result && structuralSvg.find("<svg") != std::string::npos
            && structuralSvg.find("data-mml-node=\"merror\"") != std::string::npos
            && structuralSvg.find("data-mjx-error=\"Missing close brace\"")
                != std::string::npos,
        "structural TeX errors remain explicit SVG output");

    // Returned bytes are owned by the C result, not by the Renderer.
    const auto kept = Svg(utf16.result.get());
    renderer.reset();
    passed &= Check(Svg(utf16.result.get()) == kept && kept.find("<svg") != std::string::npos,
        "result remains readable after renderer destruction");

    // A Renderer cannot be called from a non-owner thread. The guard runs
    // before entering the non-thread-safe native session.
    rendererCall = MakeRenderer();
    renderer = std::move(rendererCall.value);
    std::atomic_int wrongThreadStatus{FOLIA_NATIVE_MATH_OK};
    std::jthread wrongThread([&] {
        auto call = Render(renderer.get(), cancellation.get(), u"x+1");
        wrongThreadStatus.store(call.status, std::memory_order_release);
        if (call.result) wrongThreadStatus.store(FOLIA_NATIVE_MATH_OK,
            std::memory_order_release);
    });
    wrongThread.join();
    passed &= Check(wrongThreadStatus.load(std::memory_order_acquire)
            == FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR,
        "cross-thread renderer call is rejected at the C boundary");
    renderer.reset();

    // Deterministic pre-cancellation exercises the opaque stop_source handle.
    rendererCall = MakeRenderer();
    renderer = std::move(rendererCall.value);
    cancellationCall = MakeCancellation();
    cancellation = std::move(cancellationCall.value);
    FoliaNativeMathCancellationRequestStop(cancellation.get());
    auto preCancelled = Render(renderer.get(), cancellation.get(), u"x+y");
    passed &= Check(WasCancelled(preCancelled.status) && preCancelled.result,
        "pre-cancel returns an explicit cancellation result");
    renderer.reset();

    // Active cancellation is an overlap stress probe: the request starts on
    // the renderer's owner thread and Stop is issued from the caller thread.
    cancellationCall = MakeCancellation();
    cancellation = std::move(cancellationCall.value);
    auto tex = LargeFormula();
    auto units = Units(std::u16string(tex.begin(), tex.end()));
    std::promise<void> callStarted;
    auto started = callStarted.get_future();
    std::atomic_int activeStatus{FOLIA_NATIVE_MATH_INFRASTRUCTURE_ERROR};
    std::jthread ownerWorker([&] {
        auto owned = MakeRenderer();
        if (!owned.value) { activeStatus.store(owned.status); callStarted.set_value(); return; }
        callStarted.set_value();
        FoliaNativeMathResult* raw = nullptr;
        const auto status = FoliaNativeMathRenderSvg(owned.value.get(), units.data(),
            units.size(), 0, 16.0, 640.0, cancellation.get(), 0, &raw);
        Result result(raw, FoliaNativeMathResultDestroy);
        activeStatus.store(status, std::memory_order_release);
    });
    started.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    FoliaNativeMathCancellationRequestStop(cancellation.get());
    ownerWorker.join();
    passed &= Check(WasCancelled(static_cast<FoliaNativeMathStatus>(
            activeStatus.load(std::memory_order_acquire))),
        "concurrent stop interrupts or pre-cancels a long active render");

    // A deadline on a long request must cancel or poison only that session;
    // the following renderer is created, used, and destroyed on this thread.
    rendererCall = MakeRenderer();
    renderer = std::move(rendererCall.value);
    cancellationCall = MakeCancellation();
    cancellation = std::move(cancellationCall.value);
    const auto timeoutUnits = Units(std::u16string(tex.begin(), tex.end()));
    FoliaNativeMathResult* timedRaw = nullptr;
    const auto timedStatus = FoliaNativeMathRenderSvg(renderer.get(),
        timeoutUnits.data(), timeoutUnits.size(), 0, 16.0, 640.0,
        cancellation.get(), 1, &timedRaw);
    Result timed(timedRaw, FoliaNativeMathResultDestroy);
    passed &= Check(WasCancelled(timedStatus), "short deadline cancels long render");
    renderer.reset();

    rendererCall = MakeRenderer();
    renderer = std::move(rendererCall.value);
    cancellationCall = MakeCancellation();
    cancellation = std::move(cancellationCall.value);
    auto recovered = Render(renderer.get(), cancellation.get(), u"x+y");
    passed &= Check(rendererCall.status == FOLIA_NATIVE_MATH_OK
            && recovered.status == FOLIA_NATIVE_MATH_OK && recovered.result
            && Svg(recovered.result.get()).find("<svg") != std::string::npos,
        "a fresh session renders after timeout cancellation");
    return passed ? 0 : 1;
}
