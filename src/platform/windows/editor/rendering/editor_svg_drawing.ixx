module;
#include <d2d1_3.h>
#include <objidl.h>
#include <wrl/client.h>

export module folia.platform.editor_svg_drawing;
import std;

export namespace folia::platform::editor {

struct SvgDrawingCacheStatistics {
    std::size_t entries = 0;
    std::size_t estimatedBytes = 0;
    std::uint64_t hits = 0;
    std::uint64_t misses = 0;
    std::uint64_t evictions = 0;
};

// Retain Direct2D's complete vector recording, not an SVG DOM plus a bitmap.
// Direct2D resources are opaque: the byte budget is a conservative admission
// estimate, not a measurement or a hard cap on graphics-driver allocations.
class SvgDrawingCache {
    template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
    struct DrawingState {
        float dpiX = 96.0f;
        float dpiY = 96.0f;
        D2D1_ANTIALIAS_MODE antialias = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
        D2D1_PRIMITIVE_BLEND blend = D2D1_PRIMITIVE_BLEND_SOURCE_OVER;
        D2D1_UNIT_MODE units = D2D1_UNIT_MODE_DIPS;
        bool operator==(DrawingState const&) const = default;
    };
    struct Entry {
        ComPtr<ID2D1CommandList> drawing;
        std::string source;
        float width = 0.0f;
        float height = 0.0f;
        DrawingState state;
        std::size_t estimatedBytes = 0;
        std::list<std::uint64_t>::iterator order;
    };

public:
    explicit SvgDrawingCache(std::size_t byteBudget = 24 * 1024 * 1024,
                             std::size_t entryLimit = 128)
        : byteBudget(byteBudget), entryLimit(entryLimit) {}
    SvgDrawingCache(SvgDrawingCache const&) = delete;
    SvgDrawingCache& operator=(SvgDrawingCache const&) = delete;

    void Clear() {
        entries.clear();
        order.clear();
        statistics = {};
        recorder.Reset();
        deviceIdentity.Reset();
    }

    SvgDrawingCacheStatistics Statistics() const noexcept {
        return statistics;
    }

    ComPtr<ID2D1CommandList> FindOrCreate(
        ID2D1DeviceContext5* context, std::uint64_t renderId,
        std::string_view source, float width, float height) {
        if (!context || !renderId || source.empty()
            || !std::isfinite(width) || !std::isfinite(height)
            || width <= 0.0f || height <= 0.0f
            || source.size() > (std::numeric_limits<ULONG>::max)()) return {};
        if (!EnsureRecorder(context)) return {};
        DrawingState state;
        context->GetDpi(&state.dpiX, &state.dpiY);
        state.antialias = context->GetAntialiasMode();
        state.blend = context->GetPrimitiveBlend();
        state.units = context->GetUnitMode();
        if (auto found = entries.find(renderId); found != entries.end()) {
            auto const& entry = found->second;
            if (entry.width == width && entry.height == height
                && entry.state == state && entry.source == source) {
                order.splice(order.end(), order, entry.order);
                ++statistics.hits;
                return entry.drawing;
            }
            Remove(found);
        }
        ++statistics.misses;
        const auto cost = EstimateBytes(source.size());
        const auto retain = entryLimit && cost <= byteBudget;
        if (retain) {
            while (!order.empty() && (entries.size() >= entryLimit
                || statistics.estimatedBytes > byteBudget - cost)) {
                Remove(entries.find(order.front()));
                ++statistics.evictions;
            }
        }
        auto drawing = Record(source, width, height, state);
        if (!drawing || !retain) return drawing;
        order.push_back(renderId);
        entries.emplace(renderId, Entry{drawing, std::string(source), width,
            height, state, cost, std::prev(order.end())});
        statistics.estimatedBytes += cost;
        statistics.entries = entries.size();
        return drawing;
    }

private:
    static std::size_t EstimateBytes(std::size_t sourceBytes) noexcept {
        constexpr std::size_t overhead = 16 * 1024;
        constexpr std::size_t multiplier = 16;
        if (sourceBytes > ((std::numeric_limits<std::size_t>::max)() - overhead) / multiplier)
            return (std::numeric_limits<std::size_t>::max)();
        return overhead + sourceBytes * multiplier;
    }

    void Remove(std::unordered_map<std::uint64_t, Entry>::iterator entry) {
        statistics.estimatedBytes -= entry->second.estimatedBytes;
        order.erase(entry->second.order);
        entries.erase(entry);
        statistics.entries = entries.size();
    }

    bool EnsureRecorder(ID2D1DeviceContext5* context) {
        ComPtr<ID2D1Device> device;
        context->GetDevice(&device);
        ComPtr<IUnknown> identity;
        if (!device || FAILED(device.As(&identity))) return false;
        if (identity.Get() == deviceIdentity.Get() && recorder) return true;
        Clear();
        ComPtr<ID2D1DeviceContext> base;
        if (FAILED(device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &base))
            || FAILED(base.As(&recorder))) return false;
        deviceIdentity = std::move(identity);
        return true;
    }

    ComPtr<ID2D1CommandList> Record(std::string_view source,
        float width, float height, DrawingState const& state) {
        ComPtr<IStream> stream;
        if (FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream))) return {};
        const auto length = static_cast<ULONG>(source.size());
        ULONG written = 0;
        if (FAILED(stream->Write(source.data(), length, &written)) || written != length
            || FAILED(stream->Seek({}, STREAM_SEEK_SET, nullptr))) return {};
        recorder->SetDpi(state.dpiX, state.dpiY);
        recorder->SetAntialiasMode(state.antialias);
        recorder->SetPrimitiveBlend(state.blend);
        recorder->SetUnitMode(state.units);
        recorder->SetTransform(D2D1::Matrix3x2F::Identity());
        ComPtr<ID2D1SvgDocument> document;
        if (FAILED(recorder->CreateSvgDocument(stream.Get(), D2D1::SizeF(width, height), &document)))
            return {};
        ComPtr<ID2D1CommandList> drawing;
        if (FAILED(recorder->CreateCommandList(&drawing))) return {};
        // A separate context keeps recording out of the active editor BeginDraw/
        // EndDraw transaction and does not mutate its target, transform or clips.
        recorder->SetTarget(drawing.Get());
        recorder->BeginDraw();
        recorder->DrawSvgDocument(document.Get());
        const auto result = recorder->EndDraw();
        recorder->SetTarget(nullptr);
        if (FAILED(result) || FAILED(drawing->Close())) return {};
        return drawing;
    }

    const std::size_t byteBudget;
    const std::size_t entryLimit;
    ComPtr<IUnknown> deviceIdentity;
    ComPtr<ID2D1DeviceContext5> recorder;
    std::unordered_map<std::uint64_t, Entry> entries;
    std::list<std::uint64_t> order;
    SvgDrawingCacheStatistics statistics;
};

}
