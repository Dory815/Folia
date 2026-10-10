#include <d2d1_3.h>
#include <d3d11.h>
#include <dxgi.h>
#include <objidl.h>
#include <wrl/client.h>
#include "support/folia_test.hpp"
import folia.platform.editor_svg_drawing;

using namespace boost::ut;
using namespace folia::platform::editor;
using Microsoft::WRL::ComPtr;

namespace {
struct SvgTestSurface {
    ComPtr<ID3D11Device> d3d;
    ComPtr<ID2D1DeviceContext5> context;
    ComPtr<ID2D1Bitmap1> target;

    SvgTestSurface() {
        expect(fatal(SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP,
            nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
            D3D11_SDK_VERSION, &d3d, nullptr, nullptr))));
        ComPtr<IDXGIDevice> dxgi;
        expect(fatal(SUCCEEDED(d3d.As(&dxgi))));
        ComPtr<ID2D1Factory1> factory;
        expect(fatal(SUCCEEDED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&factory)))));
        ComPtr<ID2D1Device> device;
        expect(fatal(SUCCEEDED(factory->CreateDevice(dxgi.Get(), &device))));
        ComPtr<ID2D1DeviceContext> base;
        expect(fatal(SUCCEEDED(device->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &base))));
        expect(fatal(SUCCEEDED(base.As(&context))));
        auto properties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
        expect(fatal(SUCCEEDED(context->CreateBitmap(
            D2D1::SizeU(160, 120), nullptr, 0, &properties, &target))));
        context->SetTarget(target.Get());
    }

    ComPtr<ID2D1SvgDocument> Reference(std::string_view source, float width, float height) {
        ComPtr<IStream> stream;
        expect(fatal(SUCCEEDED(CreateStreamOnHGlobal(nullptr, TRUE, &stream))));
        expect(fatal(SUCCEEDED(stream->Write(source.data(),
            static_cast<ULONG>(source.size()), nullptr))));
        expect(fatal(SUCCEEDED(stream->Seek({}, STREAM_SEEK_SET, nullptr))));
        ComPtr<ID2D1SvgDocument> document;
        expect(fatal(SUCCEEDED(context->CreateSvgDocument(
            stream.Get(), D2D1::SizeF(width, height), &document))));
        return document;
    }

    std::vector<unsigned char> Pixels() {
        ComPtr<ID2D1Bitmap1> readable;
        auto properties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            target->GetPixelFormat());
        expect(fatal(SUCCEEDED(context->CreateBitmap(
            target->GetPixelSize(), nullptr, 0, &properties, &readable))));
        expect(fatal(SUCCEEDED(readable->CopyFromBitmap(nullptr, target.Get(), nullptr))));
        D2D1_MAPPED_RECT mapped{};
        expect(fatal(SUCCEEDED(readable->Map(D2D1_MAP_OPTIONS_READ, &mapped))));
        std::vector<unsigned char> pixels(160 * 120 * 4);
        for (std::size_t row = 0; row < 120; ++row)
            std::copy_n(mapped.bits + row * mapped.pitch, 160 * 4, pixels.data() + row * 160 * 4);
        expect(fatal(SUCCEEDED(readable->Unmap())));
        return pixels;
    }
};

constexpr auto redSvg = "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'>"
    "<circle cx='30' cy='30' r='25' fill='red'/></svg>";
constexpr auto blueSvg = "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'>"
    "<circle cx='30' cy='30' r='25' fill='blue'/></svg>";
}

suite editor_svg_drawing_tests = [] {

"svg vector recordings match native curves strokes gradients clips and transparency"_test = [] {
    SvgTestSurface surface;
    SvgDrawingCache cache;
    const std::vector<std::string> samples{
        "<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' viewBox='0 0 64 64'>"
        "<defs><path id='glyph' d='M0 0 C18 0 18 18 0 18 L0 14 C10 14 10 4 0 4 Z'/></defs>"
        "<g fill='#222' transform='translate(3,7)'><use xlink:href='#glyph'/>"
        "<use xlink:href='#glyph' transform='translate(20,16) rotate(12)'/></g></svg>",
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'>"
        "<g opacity='.55'><circle cx='23' cy='23' r='21' fill='red'/>"
        "<circle cx='40' cy='30' r='21' fill='blue'/></g>"
        "<path d='M4 49 L57 49 L57 60 L4 60 Z M15 51 L15 57 L45 57 L45 51 Z' fill-rule='evenodd'/>"
        "<path d='M5 4 Q30 50 58 5' fill='none' stroke='green' stroke-width='2.5' "
        "stroke-dasharray='3 2' stroke-linecap='round'/></svg>",
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'>"
        "<defs><linearGradient id='l'><stop stop-color='red'/>"
        "<stop offset='1' stop-color='blue' stop-opacity='.4'/></linearGradient>"
        "<radialGradient id='r'><stop stop-color='white'/><stop offset='1' stop-color='green'/></radialGradient>"
        "<clipPath id='c'><circle cx='31' cy='31' r='25'/></clipPath></defs>"
        "<g clip-path='url(#c)' opacity='.7'><rect width='64' height='64' fill='url(#l)'/>"
        "<ellipse cx='40' cy='33' rx='25' ry='19' fill='url(#r)'/></g></svg>",
        "<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' viewBox='0 0 64 64'>"
        "<rect width='64' height='64' fill='#ddd'/>"
        "<image x='8' y='7' width='43' height='41' opacity='.7' xlink:href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAYAAABytg0kAAAAFUlEQVR4nGP4z8DwHwgbGIA0CDAAAEPTCHnJFRwPAAAAAElFTkSuQmCC'/></svg>",
        "<svg xmlns='http://www.w3.org/2000/svg' width='80' height='70' viewBox='-5 -7 64 64'>"
        "<svg x='5' y='4' width='40' height='37' viewBox='0 0 20 20' overflow='hidden'>"
        "<rect x='-3' y='-3' width='26' height='26' rx='3.5' fill='purple'/>"
        "<polyline points='1,2 8,15 15,3' fill='none' stroke='orange' stroke-width='1.7'/></svg></svg>"
    };
    for (std::size_t sample = 0; sample < samples.size(); ++sample) {
        auto document = surface.Reference(samples[sample], 80, 70);
        for (auto dpi : {96.0f, 144.0f, 192.0f}) {
            surface.context->SetDpi(dpi, dpi);
            for (auto scale : {0.75f, 1.0f, 1.4f}) {
                const auto transform = D2D1::Matrix3x2F::Scale(scale, scale)
                    * D2D1::Matrix3x2F::Rotation(7)
                    * D2D1::Matrix3x2F::Translation(8.25f, -3.75f);
                std::vector<unsigned char> reference;
                for (auto recording : {false, true}) {
                    surface.context->SetTransform(transform);
                    surface.context->BeginDraw();
                    surface.context->Clear(D2D1::ColorF(0.9f, 0.85f, 0.8f, 1));
                    surface.context->PushAxisAlignedClip(D2D1::RectF(2, 3, 70, 63),
                        D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                    if (!recording) surface.context->DrawSvgDocument(document.Get());
                    else {
                        auto drawing = cache.FindOrCreate(surface.context.Get(),
                            sample + 1, samples[sample], 80, 70);
                        expect(fatal(bool(drawing)));
                        ComPtr<ID2D1Image> target;
                        surface.context->GetTarget(&target);
                        expect(target.Get() == surface.target.Get());
                        D2D1_MATRIX_3X2_F current{};
                        surface.context->GetTransform(&current);
                        expect(current._31 == transform._31 && current._32 == transform._32);
                        surface.context->DrawImage(drawing.Get());
                    }
                    surface.context->PopAxisAlignedClip();
                    expect(fatal(SUCCEEDED(surface.context->EndDraw())));
                    auto pixels = surface.Pixels();
                    if (!recording) reference = std::move(pixels);
                    else expect(pixels == reference) << "sample=" << sample
                        << " dpi=" << dpi << " scale=" << scale;
                }
            }
        }
    }
};

"svg drawing cache evicts least recently used entries and accounts for admission"_test = [] {
    SvgTestSurface surface;
    SvgDrawingCache cache(1024 * 1024, 2);
    auto first = cache.FindOrCreate(surface.context.Get(), 1, redSvg, 64, 64);
    auto second = cache.FindOrCreate(surface.context.Get(), 2, blueSvg, 64, 64);
    expect(fatal(bool(first) && bool(second)));
    expect(cache.FindOrCreate(surface.context.Get(), 1, redSvg, 64, 64).Get() == first.Get());
    expect(fatal(bool(cache.FindOrCreate(surface.context.Get(), 3, redSvg, 64, 64))));
    const auto stats = cache.Statistics();
    expect(stats.entries == 2u && stats.hits == 1u && stats.misses == 3u && stats.evictions == 1u);
    expect(stats.estimatedBytes <= 1024 * 1024u);
    expect(cache.FindOrCreate(surface.context.Get(), 2, blueSvg, 64, 64).Get() != second.Get());
    SvgDrawingCache tiny(1, 2);
    expect(fatal(bool(tiny.FindOrCreate(surface.context.Get(), 1, redSvg, 64, 64))));
    expect(tiny.Statistics().entries == 0u && tiny.Statistics().estimatedBytes == 0u);
    SvgDrawingCache byteLimited(20000, 10);
    expect(fatal(bool(byteLimited.FindOrCreate(surface.context.Get(), 1, redSvg, 64, 64))));
    expect(fatal(bool(byteLimited.FindOrCreate(surface.context.Get(), 2, blueSvg, 64, 64))));
    expect(byteLimited.Statistics().entries == 1u && byteLimited.Statistics().evictions == 1u);
};

"svg drawing cache invalidates source viewport dpi and device without retaining old records"_test = [] {
    SvgTestSurface surface;
    SvgDrawingCache cache;
    auto first = cache.FindOrCreate(surface.context.Get(), 1, redSvg, 64, 64);
    expect(fatal(bool(first)));
    surface.context->SetTransform(D2D1::Matrix3x2F::Translation(0, -500));
    expect(cache.FindOrCreate(surface.context.Get(), 1, redSvg, 64, 64).Get() == first.Get());
    auto changed = cache.FindOrCreate(surface.context.Get(), 1, blueSvg, 64, 64);
    expect(fatal(bool(changed)));
    expect(changed.Get() != first.Get());
    auto resized = cache.FindOrCreate(surface.context.Get(), 1, blueSvg, 80, 70);
    expect(fatal(bool(resized)));
    expect(resized.Get() != changed.Get());
    surface.context->SetDpi(192, 192);
    auto dpiChanged = cache.FindOrCreate(surface.context.Get(), 1, blueSvg, 80, 70);
    expect(fatal(bool(dpiChanged)));
    expect(dpiChanged.Get() != resized.Get());
    surface.context->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    auto antialiasChanged = cache.FindOrCreate(surface.context.Get(), 1, blueSvg, 80, 70);
    expect(fatal(bool(antialiasChanged)));
    expect(antialiasChanged.Get() != dpiChanged.Get());
    expect(cache.Statistics().entries == 1u && cache.Statistics().hits == 1u
        && cache.Statistics().misses == 5u);
    SvgTestSurface replacement;
    auto deviceChanged = cache.FindOrCreate(replacement.context.Get(), 1, blueSvg, 80, 70);
    expect(fatal(bool(deviceChanged)));
    expect(cache.Statistics().entries == 1u && cache.Statistics().misses == 1u);
    replacement.context->BeginDraw();
    replacement.context->DrawImage(deviceChanged.Get());
    expect(fatal(SUCCEEDED(replacement.context->EndDraw())));
    cache.Clear();
    expect(cache.Statistics().entries == 0u && cache.Statistics().estimatedBytes == 0u);
    expect(!cache.FindOrCreate(replacement.context.Get(), 1, redSvg, 0, 64));
    expect(!cache.FindOrCreate(replacement.context.Get(), 1, "<svg", 64, 64));
};

};
