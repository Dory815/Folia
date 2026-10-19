// Output behavior references MathJax 4.1.3 at fb987178c2d279a99b0db5ea02e751691703955e:
// ts/output/svg.ts, ts/output/svg/Wrapper.ts, and ts/output/svg/FontCache.ts.
// Copyright (c) 2018-2026 The MathJax Consortium; Apache-2.0. See README.md.
module folia.math.output.svg.writer;
import std;
import folia.math.util.lengths;

namespace folia::math::output::svg {
namespace scene = folia::math::output::scene;
namespace detail {
std::string escape_attribute(std::string_view);
std::string escape_text(std::string_view);
void append_attribute(std::string&, std::string_view, std::string_view);
void append_metadata(std::string&, scene::SemanticMetadata const&,
    std::span<std::string_view const>);
}
namespace {

std::string number(double value, int precision = 1) {
    if (!std::isfinite(value)) throw std::invalid_argument("non-finite SVG coordinate");
    if (std::abs(value) < 0.0006) return "0";
    std::string result = folia::math::lengths::format_decimal(value, static_cast<unsigned>(precision));
    while (result.find('.') != std::string::npos && result.back() == '0') result.pop_back();
    if (!result.empty() && result.back() == '.') result.pop_back();
    return result;
}

std::string ex_value(double em, double x_height) {
    if (!(x_height > 0) || !std::isfinite(x_height))
        throw std::invalid_argument("Scene font x-height must be finite and positive");
    const auto amount = em / x_height;
    if (std::abs(amount) < 0.001) return "0";
    auto result = folia::math::lengths::format_decimal(amount, 3);
    while (result.find('.') != std::string::npos && result.back() == '0') result.pop_back();
    if (!result.empty() && result.back() == '.') result.pop_back();
    return result + "ex";
}

std::string color(scene::Color const& color) {
    static constexpr char digits[] = "0123456789ABCDEF";
    std::string value = "#000000";
    const std::array<std::uint8_t, 3> channels{color.red, color.green, color.blue};
    for (std::size_t i = 0; i < channels.size(); ++i) {
        value[1 + i * 2] = digits[channels[i] >> 4];
        value[2 + i * 2] = digits[channels[i] & 0x0f];
    }
    return value;
}

void append_color(std::string& out, std::string_view name,
    std::string_view opacity_name, scene::PaintColor const& paint) {
    std::visit([&](auto const& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, scene::InheritedPaint>) {
            return;
        } else if constexpr (std::is_same_v<T, scene::NoPaint>) {
            detail::append_attribute(out, name, "none");
        } else if constexpr (std::is_same_v<T, scene::CurrentColorPaint>) {
            detail::append_attribute(out, name, "currentColor");
        } else {
            detail::append_attribute(out, name, color(value));
            if (value.alpha != 255)
                detail::append_attribute(out, opacity_name, number(value.alpha / 255.0, 3));
        }
    }, paint);
}

void append_paint(std::string& out, scene::Paint const& paint) {
    append_color(out, "fill", "fill-opacity", paint.fill);
    if (paint.fill_rule == scene::FillRule::evenodd)
        detail::append_attribute(out, "fill-rule", "evenodd");
    append_color(out, "stroke", "stroke-opacity", paint.stroke);
    if (paint.stroke_width != 0)
        detail::append_attribute(out, "stroke-width", number(paint.stroke_width * 1000));
    if (paint.opacity != 1) detail::append_attribute(out, "opacity", number(paint.opacity, 3));
}

void append_path(std::string& out, scene::PathGeometry const& path) {
    for (auto const& segment : path.segments) {
        std::visit([&](auto const& item) {
            using T = std::decay_t<decltype(item)>;
            auto point = [&](scene::Point const& p) {
                out += number(p.x * 1000) + "," + number(p.y * 1000);
            };
            if constexpr (std::is_same_v<T, scene::MoveTo>) { out += 'M'; point(item.to); }
            else if constexpr (std::is_same_v<T, scene::LineTo>) { out += 'L'; point(item.to); }
            else if constexpr (std::is_same_v<T, scene::QuadTo>) {
                out += 'Q'; point(item.control); out += ' '; point(item.to);
            } else if constexpr (std::is_same_v<T, scene::CubicTo>) {
                out += 'C'; point(item.control1); out += ' '; point(item.control2); out += ' '; point(item.to);
            } else if constexpr (std::is_same_v<T, scene::ArcTo>) {
                out += 'A';
                out += number(item.radius_x * 1000) + " " + number(item.radius_y * 1000) + " ";
                // Arc values remain in Scene's y-up local coordinate system.
                // The enclosing SVG transform maps the finished path; changing
                // arc flags here would apply the reflection twice.
                out += number(item.rotation, 3) + " " + (item.large_arc ? "1" : "0") + " ";
                out += (item.sweep ? "1 " : "0 ");
                point(item.to);
            } else out += 'Z';
        }, segment);
    }
}

std::string transform_operation(scene::TransformOperation const& operation) {
    return std::visit([](auto const& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, scene::Translation>) {
            return "translate(" + number(value.x * 1000) + "," + number(value.y * 1000) + ")";
        } else if constexpr (std::is_same_v<T, scene::Scaling>) {
            if (value.x == value.y) return "scale(" + number(value.x, 3) + ")";
            return "scale(" + number(value.x, 3) + "," + number(value.y, 3) + ")";
        } else if constexpr (std::is_same_v<T, scene::Rotation>) {
            return "rotate(" + number(value.degrees, 3) + " " +
                number(value.center.x * 1000) + " " + number(value.center.y * 1000) + ")";
        } else {
            return "matrix(" + number(value.a, 6) + " " + number(value.b, 6) + " " +
                number(value.c, 6) + " " + number(value.d, 6) + " " +
                number(value.e * 1000) + " " + number(value.f * 1000) + ")";
        }
    }, operation);
}

void append_transform(std::string& out, scene::Transform const& transform) {
    std::string serialized;
    for (auto const& operation : transform.operations()) {
        if (!serialized.empty()) serialized.push_back(' ');
        serialized += transform_operation(operation);
    }
    if (!serialized.empty()) detail::append_attribute(out, "transform", serialized);
}

using Cache = std::vector<std::pair<std::string, std::string>>;

std::string codepoint(std::uint32_t value) {
    std::array<char, 16> buffer{};
    auto [end, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value, 16);
    if (ec != std::errc{}) throw std::runtime_error("glyph id formatting failed");
    std::string result(buffer.data(), end);
    std::ranges::transform(result, result.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return result;
}

std::string path_data(scene::Glyph const& glyph, SvgWriteOptions const& options) {
    if (!glyph.resource)
        throw UnsupportedError("Scene glyph has no immutable common outline resource");
    if (!options.font_resources)
        throw UnsupportedError("SVG glyph output requires the immutable Source font resource catalog");
    const auto* source = options.font_resources->find(
        {glyph.key.variant, glyph.key.codepoint});
    if (!source)
        throw UnsupportedError("SVG glyph has no exact Source font resource key");
    if (!source->path_defined)
        throw UnsupportedError("SVG glyph has no Source path; composition must be completed in Scene");
    return source->path_data;
}

std::string cache_id(scene::Glyph const& glyph, SvgWriteOptions const& options) {
    return "MJX-" + (options.local_id.empty() ? std::string{} : options.local_id + "-") +
        glyph.key.variant_cache_id + "-" + codepoint(glyph.key.codepoint);
}

void add_glyph_cache(scene::Glyph const& glyph, SvgWriteOptions const& options, Cache& cache) {
    auto id = cache_id(glyph, options);
    if (std::ranges::find(cache, id, &Cache::value_type::first) != cache.end()) return;
    cache.emplace_back(std::move(id), path_data(glyph, options));
}

void collect_cache(scene::Group const& group, SvgWriteOptions const& options, Cache& cache) {
    for (auto const& entry : group.entries) {
        std::visit([&](auto const& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::shared_ptr<scene::Group const>>) {
                if (value) collect_cache(*value, options, cache);
            } else if constexpr (std::is_same_v<T, scene::Primitive>) {
                if (auto run = std::get_if<scene::GlyphRun>(&value))
                    for (auto const& glyph : run->glyphs) add_glyph_cache(glyph, options, cache);
                else if (auto foreign = std::get_if<scene::ForeignContent>(&value))
                    for (auto const& run : foreign->measured_runs)
                        for (auto const& glyph : run.run.measured_glyphs.glyphs) add_glyph_cache(glyph, options, cache);
            }
        }, entry);
    }
}

void validate_metadata(scene::SemanticMetadata const& metadata) {
    if (!metadata.annotation.empty() || !metadata.annotation_encoding.empty())
        throw UnsupportedError("SVG metadata annotations are not yet serializable");
}

void validate_scene(scene::Group const& group) {
    validate_metadata(group.metadata);
    for (auto const& entry : group.entries) {
        std::visit([&](auto const& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::shared_ptr<scene::Group const>>) {
                if (value) validate_scene(*value);
            } else {
                std::visit([&](auto const& primitive) {
                    using P = std::decay_t<decltype(primitive)>;
                    if constexpr (std::is_same_v<P, scene::ForeignContent>) {
                        throw UnsupportedError("foreign HTML markup requires a structured SVG/XHTML serializer");
                    }
                }, value);
            }
        }, entry);
    }
}

void open_group(std::string& out, scene::Group const& group, SvgWriteOptions const& options) {
    if (group.link) {
        out += "<a";
        detail::append_attribute(out, "href", group.link->href);
        if (!group.link->target.empty()) detail::append_attribute(out, "target", group.link->target);
        if (!group.link->rel.empty()) detail::append_attribute(out, "rel", group.link->rel);
        out += '>';
    }
    out += "<g";
    static constexpr std::array<std::string_view, 1> reserved{"data-mml-node"};
    if (!group.metadata.source_tag.empty()) detail::append_attribute(out, "data-mml-node", group.metadata.source_tag);
    else detail::append_attribute(out, "data-mml-node", "mrow");
    if (!group.transform.operations().empty()) append_transform(out, group.transform);
    if (group.clip) detail::append_attribute(out, "clip-path", "url(#clip-" + std::to_string(group.id) + ")");
    detail::append_metadata(out, group.metadata, reserved);
    out += '>';
    if (group.tooltip) out += "<title>" + detail::escape_text(group.tooltip->text) + "</title>";
    if (group.clip) {
        out += "<defs><clipPath id=\"clip-" + std::to_string(group.id) + "\"><path d=\"";
        append_path(out, group.clip->geometry);
        out += "\"";
        if (group.clip->fill_rule == scene::FillRule::evenodd)
            detail::append_attribute(out, "clip-rule", "evenodd");
        out += "/></clipPath></defs>";
    }
    for (auto const& entry : group.entries) {
        std::visit([&](auto const& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::shared_ptr<scene::Group const>>) {
                if (value) open_group(out, *value, options);
            } else {
                std::visit([&](auto const& primitive) {
                    using P = std::decay_t<decltype(primitive)>;
                    if constexpr (std::is_same_v<P, scene::GlyphRun>) {
                        for (auto const& glyph : primitive.glyphs) {
                            if (options.font_cache == FontCacheMode::none) {
                                out += "<path";
                                detail::append_attribute(out, "data-c", codepoint(glyph.key.codepoint));
                                detail::append_attribute(out, "d", path_data(glyph, options));
                                if (glyph.origin.x || glyph.origin.y) detail::append_attribute(out, "transform", "translate(" + number(glyph.origin.x * 1000) + " " + number(glyph.origin.y * 1000) + ")");
                                append_paint(out, glyph.paint); out += "/>";
                            } else {
                                out += "<use";
                                detail::append_attribute(out, "data-c", codepoint(glyph.key.codepoint));
                                detail::append_attribute(out, options.use_xlink ? "xlink:href" : "href", "#" + cache_id(glyph, options));
                                if (glyph.origin.x || glyph.origin.y) detail::append_attribute(out, "transform", "translate(" + number(glyph.origin.x * 1000) + " " + number(glyph.origin.y * 1000) + ")");
                                append_paint(out, glyph.paint); out += "/>";
                            }
                        }
                    } else if constexpr (std::is_same_v<P, scene::Rule>) {
                        out += "<rect";
                        detail::append_attribute(out, "x", number(primitive.bounds.x * 1000));
                        detail::append_attribute(out, "y", number(-(primitive.bounds.y + primitive.bounds.height) * 1000));
                        detail::append_attribute(out, "width", number(primitive.bounds.width * 1000));
                        detail::append_attribute(out, "height", number(primitive.bounds.height * 1000));
                        append_paint(out, primitive.paint); out += "/>";
                    } else if constexpr (std::is_same_v<P, scene::Path>) {
                        out += "<path d=\""; append_path(out, primitive.geometry); out += '"'; append_paint(out, primitive.paint); out += "/>";
                    } else if constexpr (std::is_same_v<P, scene::TextRun>) {
                        out += "<text";
                        if (primitive.origin.x || primitive.origin.y) detail::append_attribute(out, "transform", "translate(" + number(primitive.origin.x * 1000) + " " + number(primitive.origin.y * 1000) + ")");
                        append_paint(out, primitive.paint); out += '>' + detail::escape_text(primitive.utf8) + "</text>";
                    } else {
                        throw UnsupportedError("foreign HTML markup requires a structured SVG/XHTML serializer");
                    }
                }, value);
            }
        }, entry);
    }
    out += "</g>";
    if (group.link) out += "</a>";
}
}

std::string write(scene::Scene const& scene_value, SvgWriteOptions const& options) {
    validate_scene(scene_value.root());
    auto const& metrics = scene_value.metrics();
    auto const& font = scene_value.font_metrics();
    if (!std::isfinite(metrics.width) || !std::isfinite(metrics.height) || !std::isfinite(metrics.depth))
        throw std::invalid_argument("Scene metrics must be finite");
    const double width = std::max(metrics.width, 0.001);
    const double height = std::max(metrics.height + metrics.depth, 0.001);
    Cache cache;
    if (options.font_cache != FontCacheMode::none) collect_cache(scene_value.root(), options, cache);
    if (!cache.empty() && options.font_cache == FontCacheMode::local && options.local_id.empty())
        throw std::invalid_argument("local SVG font caching requires the resolved MathJax local ID");
    if (!cache.empty() && options.font_cache == FontCacheMode::global)
        throw UnsupportedError("global SVG font caching requires the owning document cache serializer");

    std::string out;
    if (options.include_math_container) {
        out += "<mjx-container";
        for (auto const& attribute : options.container_attributes)
            detail::append_attribute(out, attribute.name, attribute.value);
        out += '>';
    }
    out += "<svg";
    detail::append_attribute(out, "style", "vertical-align: " + ex_value(-metrics.depth, font.x_height) + ";");
    detail::append_attribute(out, "xmlns", "http://www.w3.org/2000/svg");
    detail::append_attribute(out, "width", ex_value(width, font.x_height));
    detail::append_attribute(out, "height", ex_value(height, font.x_height));
    detail::append_attribute(out, "role", "img");
    detail::append_attribute(out, "focusable", "false");
    detail::append_attribute(out, "viewBox", "0 " + number(-metrics.height * 1000) + " " +
        number(width * 1000) + " " + number(height * 1000));
    if (options.font_cache != FontCacheMode::none && options.use_xlink)
        detail::append_attribute(out, "xmlns:xlink", "http://www.w3.org/1999/xlink");
    out += "><g stroke=\"currentColor\" fill=\"currentColor\" stroke-width=\"0\" transform=\"scale(1,-1)\">";
    if (options.font_cache == FontCacheMode::local && !cache.empty()) {
        out += "<defs>";
        for (auto const& [id, path] : cache) {
            out += "<path";
            detail::append_attribute(out, "id", id);
            detail::append_attribute(out, "d", path);
            out += "/>";
        }
        out += "</defs>";
    }
    open_group(out, scene_value.root(), options);
    out += "</g></svg>";
    if (options.include_math_container) out += "</mjx-container>";
    return out;
}
}
