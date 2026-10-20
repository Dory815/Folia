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
void append_attribute(std::string &, std::string_view, std::string_view);
void append_metadata(std::string &, scene::SemanticMetadata const &,
                     std::span<std::string_view const>);
} // namespace detail
namespace {

std::string number(double value, int precision = 1) {
    if (!std::isfinite(value))
        throw std::invalid_argument("non-finite SVG coordinate");
    if (std::abs(value) < 0.0006)
        return "0";
    std::string result =
        folia::math::lengths::format_decimal(value, static_cast<unsigned>(precision));
    while (result.find('.') != std::string::npos && result.back() == '0')
        result.pop_back();
    if (!result.empty() && result.back() == '.')
        result.pop_back();
    return result;
}

std::string ex_value(double em, double x_height) {
    if (!(x_height > 0) || !std::isfinite(x_height))
        throw std::invalid_argument("Scene font x-height must be finite and positive");
    const auto amount = em / x_height;
    if (std::abs(amount) < 0.001)
        return "0";
    auto result = folia::math::lengths::format_decimal(amount, 3);
    while (result.find('.') != std::string::npos && result.back() == '0')
        result.pop_back();
    if (!result.empty() && result.back() == '.')
        result.pop_back();
    return result + "ex";
}

std::string color(scene::Color const &color) {
    static constexpr char digits[] = "0123456789ABCDEF";
    std::string value = "#000000";
    const std::array<std::uint8_t, 3> channels{color.red, color.green, color.blue};
    for (std::size_t i = 0; i < channels.size(); ++i) {
        value[1 + i * 2] = digits[channels[i] >> 4];
        value[2 + i * 2] = digits[channels[i] & 0x0f];
    }
    return value;
}

void append_color(std::string &out, std::string_view name, std::string_view opacity_name,
                  scene::PaintColor const &paint) {
    std::visit(
        [&](auto const &value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, scene::InheritedPaint>) {
                return;
            } else if constexpr (std::is_same_v<T, scene::NoPaint>) {
                detail::append_attribute(out, name, "none");
            } else if constexpr (std::is_same_v<T, scene::CurrentColorPaint>) {
                detail::append_attribute(out, name, "currentColor");
            } else if constexpr (std::is_same_v<T, scene::CssColor>) {
                detail::append_attribute(out, name, value.value);
            } else {
                detail::append_attribute(out, name, color(value));
                if (value.alpha != 255)
                    detail::append_attribute(out, opacity_name, number(value.alpha / 255.0, 3));
            }
        },
        paint);
}

std::string_view stroke_cap_name(scene::StrokeCap cap) {
    switch (cap) {
    case scene::StrokeCap::butt:
        return "butt";
    case scene::StrokeCap::round:
        return "round";
    case scene::StrokeCap::square:
        return "square";
    }
    throw std::invalid_argument("unknown Scene stroke cap");
}

std::string_view stroke_join_name(scene::StrokeJoin join) {
    switch (join) {
    case scene::StrokeJoin::miter:
        return "miter";
    case scene::StrokeJoin::round:
        return "round";
    case scene::StrokeJoin::bevel:
        return "bevel";
    }
    throw std::invalid_argument("unknown Scene stroke join");
}

void append_paint(std::string &out, scene::Paint const &paint, bool stroke_first = false) {
    if (stroke_first)
        append_color(out, "stroke", "stroke-opacity", paint.stroke);
    append_color(out, "fill", "fill-opacity", paint.fill);
    if (paint.fill_rule == scene::FillRule::evenodd)
        detail::append_attribute(out, "fill-rule", "evenodd");
    if (!stroke_first)
        append_color(out, "stroke", "stroke-opacity", paint.stroke);
    if (paint.stroke_width != 0)
        detail::append_attribute(out, "stroke-width", number(paint.stroke_width * 1000));
    if (paint.opacity != 1)
        detail::append_attribute(out, "opacity", number(paint.opacity, 3));
    if (paint.line_cap)
        detail::append_attribute(out, "stroke-linecap", stroke_cap_name(*paint.line_cap));
    if (paint.line_join)
        detail::append_attribute(out, "stroke-linejoin", stroke_join_name(*paint.line_join));
    if (paint.dash_array) {
        if (paint.dash_array->empty()) {
            detail::append_attribute(out, "stroke-dasharray", "none");
        } else {
            std::string values;
            for (auto const length : *paint.dash_array) {
                if (!values.empty())
                    values.push_back(' ');
                values += number(length * 1000);
            }
            detail::append_attribute(out, "stroke-dasharray", values);
        }
    }
}

void append_path(std::string &out, scene::PathGeometry const &path) {
    for (auto const &segment : path.segments) {
        std::visit(
            [&](auto const &item) {
                using T = std::decay_t<decltype(item)>;
                auto point = [&](scene::Point const &p) {
                    out += number(p.x * 1000) + "," + number(p.y * 1000);
                };
                if constexpr (std::is_same_v<T, scene::MoveTo>) {
                    out += 'M';
                    point(item.to);
                } else if constexpr (std::is_same_v<T, scene::LineTo>) {
                    out += 'L';
                    point(item.to);
                } else if constexpr (std::is_same_v<T, scene::QuadTo>) {
                    out += 'Q';
                    point(item.control);
                    out += ' ';
                    point(item.to);
                } else if constexpr (std::is_same_v<T, scene::CubicTo>) {
                    out += 'C';
                    point(item.control1);
                    out += ' ';
                    point(item.control2);
                    out += ' ';
                    point(item.to);
                } else if constexpr (std::is_same_v<T, scene::ArcTo>) {
                    out += 'A';
                    out += number(item.radius_x * 1000) + " " + number(item.radius_y * 1000) + " ";
                    // Arc values remain in Scene's y-up local coordinate system.
                    // The enclosing SVG transform maps the finished path; changing
                    // arc flags here would apply the reflection twice.
                    out += number(item.rotation, 3) + " " + (item.large_arc ? "1" : "0") + " ";
                    out += (item.sweep ? "1 " : "0 ");
                    point(item.to);
                } else
                    out += 'Z';
            },
            segment);
    }
}

std::string_view translation_separator(
    scene::ElementRole role,
    scene::TranslationSeparator spelling = scene::TranslationSeparator::role_default) {
    switch (spelling) {
    case scene::TranslationSeparator::role_default:
        break;
    case scene::TranslationSeparator::comma:
        return ",";
    case scene::TranslationSeparator::space:
        return " ";
    case scene::TranslationSeparator::comma_space:
        return ", ";
    default:
        throw std::invalid_argument("unknown Scene translation separator");
    }
    switch (role) {
    case scene::ElementRole::math_wrapper:
    case scene::ElementRole::id_alignment_box:
    case scene::ElementRole::unknown_text:
        return ",";
    case scene::ElementRole::enclosure_content:
        return ", ";
    case scene::ElementRole::operator_adjustment:
    case scene::ElementRole::inferred_row_adjustment:
    case scene::ElementRole::enclosure_mark:
    case scene::ElementRole::table_content_viewport:
    case scene::ElementRole::table_label_frame:
    case scene::ElementRole::image_glyph:
    case scene::ElementRole::foreign_content_viewport:
    case scene::ElementRole::vertical_extender_viewport:
    case scene::ElementRole::horizontal_extender_viewport:
    case scene::ElementRole::anonymous_group:
    case scene::ElementRole::glyph_composition:
    case scene::ElementRole::background_box:
    case scene::ElementRole::link_hitbox:
    case scene::ElementRole::id_alignment_marker:
    case scene::ElementRole::fraction_rule:
    case scene::ElementRole::radical_rule:
        return " ";
    }
    throw std::invalid_argument("unknown Scene element role");
}

std::string transform_operation(scene::TransformOperation const &operation,
                                scene::ElementRole role) {
    return std::visit(
        [&](auto const &value) -> std::string {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, scene::Translation>) {
                std::string result = "translate(";
                result += number(value.x * 1000);
                result += translation_separator(role, value.separator);
                result += number(value.y * 1000);
                result += ')';
                return result;
            } else if constexpr (std::is_same_v<T, scene::Scaling>) {
                if (value.x == value.y && !value.explicit_axes)
                    return "scale(" + number(value.x, 3) + ")";
                return "scale(" + number(value.x, 3) + "," + number(value.y, 3) + ")";
            } else if constexpr (std::is_same_v<T, scene::Rotation>) {
                return "rotate(" + number(value.degrees, 3) + " " + number(value.center.x * 1000) +
                       " " + number(value.center.y * 1000) + ")";
            } else {
                return "matrix(" + number(value.a, 6) + " " + number(value.b, 6) + " " +
                       number(value.c, 6) + " " + number(value.d, 6) + " " +
                       number(value.e * 1000) + " " + number(value.f * 1000) + ")";
            }
        },
        operation);
}

void append_transform(std::string &out, scene::Transform const &transform,
                      scene::ElementRole role) {
    std::string serialized;
    for (auto const &operation : transform.operations()) {
        if (!serialized.empty())
            serialized.push_back(' ');
        serialized += transform_operation(operation, role);
    }
    if (!serialized.empty())
        detail::append_attribute(out, "transform", serialized);
}

using Cache = std::vector<std::pair<std::string, std::string>>;

std::string codepoint(std::uint32_t value) {
    std::array<char, 16> buffer{};
    auto [end, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value, 16);
    if (ec != std::errc{})
        throw std::runtime_error("glyph id formatting failed");
    std::string result(buffer.data(), end);
    std::ranges::transform(result, result.begin(),
                           [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return result;
}

std::string path_data(scene::Glyph const &glyph, SvgWriteOptions const &options) {
    if (!glyph.resource)
        throw UnsupportedError("Scene glyph has no immutable common outline resource");
    if (!options.font_resources)
        throw UnsupportedError(
            "SVG glyph output requires the immutable Source font resource catalog");
    const auto *source = options.font_resources->find({glyph.key.variant, glyph.key.codepoint});
    if (!source)
        throw UnsupportedError("SVG glyph has no exact Source font resource key");
    if (!source->path_defined)
        throw UnsupportedError(
            "SVG glyph has no Source path; composition must be completed in Scene");
    return source->path_data;
}

std::string cache_id(scene::Glyph const &glyph, SvgWriteOptions const &options) {
    return "MJX-" + (options.local_id.empty() ? std::string{} : options.local_id + "-") +
           glyph.key.variant_cache_id + "-" + codepoint(glyph.key.codepoint);
}

void add_glyph_cache(scene::Glyph const &glyph, SvgWriteOptions const &options, Cache &cache) {
    auto id = cache_id(glyph, options);
    if (std::ranges::find(cache, id, &Cache::value_type::first) != cache.end())
        return;
    cache.emplace_back(std::move(id), path_data(glyph, options));
}

void collect_cache(scene::Group const &group, SvgWriteOptions const &options, Cache &cache) {
    for (auto const &entry : group.entries) {
        std::visit(
            [&](auto const &value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, std::shared_ptr<scene::Group const>>) {
                    if (value)
                        collect_cache(*value, options, cache);
                } else if constexpr (std::is_same_v<T, scene::Primitive>) {
                    if (auto run = std::get_if<scene::GlyphRun>(&value))
                        for (auto const &glyph : run->glyphs)
                            add_glyph_cache(glyph, options, cache);
                    else if (auto foreign = std::get_if<scene::ForeignContent>(&value))
                        for (auto const &run : foreign->measured_runs)
                            for (auto const &glyph : run.run.measured_glyphs.glyphs)
                                add_glyph_cache(glyph, options, cache);
                }
            },
            entry);
    }
}

void validate_metadata(scene::SemanticMetadata const &metadata) {
    if (!metadata.annotation.empty() || !metadata.annotation_encoding.empty())
        throw UnsupportedError("SVG metadata annotations are not yet serializable");
}

void validate_scene(scene::Group const &group) {
    validate_metadata(group.metadata);
    for (auto const &entry : group.entries) {
        std::visit(
            [&](auto const &value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, std::shared_ptr<scene::Group const>>) {
                    if (value)
                        validate_scene(*value);
                } else {
                    std::visit(
                        [&](auto const &primitive) {
                            using P = std::decay_t<decltype(primitive)>;
                            if constexpr (std::is_same_v<P, scene::ForeignContent>) {
                                throw UnsupportedError("foreign HTML markup requires a structured "
                                                       "SVG/XHTML serializer");
                            }
                        },
                        value);
                }
            },
            entry);
    }
}

void append_glyphs(std::string &out, scene::GlyphRun const &run, SvgWriteOptions const &options) {
    for (auto const &glyph : run.glyphs) {
        out += options.font_cache == FontCacheMode::none ? "<path" : "<use";
        detail::append_attribute(out, "data-c", codepoint(glyph.key.codepoint));
        if (options.font_cache == FontCacheMode::none)
            detail::append_attribute(out, "d", path_data(glyph, options));
        else
            detail::append_attribute(out, options.use_xlink ? "xlink:href" : "href",
                                     "#" + cache_id(glyph, options));
        std::string transform;
        if (glyph.origin.x || glyph.origin.y)
            transform = "translate(" + number(glyph.origin.x * 1000) + "," +
                        number(glyph.origin.y * 1000) + ")";
        for (auto const &operation : glyph.transform.operations()) {
            if (!transform.empty())
                transform.push_back(' ');
            transform += transform_operation(operation, scene::ElementRole::math_wrapper);
        }
        if (!transform.empty())
            detail::append_attribute(out, "transform", transform);
        append_paint(out, glyph.paint);
        out += "/>";
    }
}

void append_rule(std::string &out, scene::Rule const &rule) {
    out += "<rect";
    const bool background = rule.role == scene::ElementRole::background_box;
    if (background)
        append_paint(out, rule.paint);
    if (rule.role == scene::ElementRole::link_hitbox) {
        detail::append_attribute(out, "data-hitbox", "true");
        detail::append_attribute(out, "pointer-events", "all");
    }
    if (rule.role == scene::ElementRole::fraction_rule ||
        rule.role == scene::ElementRole::radical_rule) {
        detail::append_attribute(out, "width", number(rule.bounds.width * 1000));
        detail::append_attribute(out, "height", number(rule.bounds.height * 1000));
        detail::append_attribute(out, "x", number(rule.bounds.x * 1000));
        detail::append_attribute(out, "y", number(rule.bounds.y * 1000));
    } else {
        detail::append_attribute(out, "x", number(rule.bounds.x * 1000));
        detail::append_attribute(out, "y", number(rule.bounds.y * 1000));
        detail::append_attribute(out, "width", number(rule.bounds.width * 1000));
        detail::append_attribute(out, "height", number(rule.bounds.height * 1000));
    }
    auto paint = rule.paint;
    if (rule.role == scene::ElementRole::link_hitbox) {
        paint.fill = scene::NoPaint{};
        paint.stroke = scene::NoPaint{};
    }
    if (!background)
        append_paint(out, paint);
    static constexpr std::array<std::string_view, 16> reserved{"x",
                                                               "y",
                                                               "width",
                                                               "height",
                                                               "fill",
                                                               "stroke",
                                                               "fill-rule",
                                                               "fill-opacity",
                                                               "stroke-opacity",
                                                               "stroke-width",
                                                               "opacity",
                                                               "stroke-linecap",
                                                               "stroke-linejoin",
                                                               "stroke-dasharray",
                                                               "data-hitbox",
                                                               "pointer-events"};
    detail::append_metadata(out, rule.metadata, reserved);
    out += "/>";
}

std::vector<scene::Point> path_vertices(scene::PathGeometry const &geometry) {
    std::vector<scene::Point> points;
    for (auto const &segment : geometry.segments)
        std::visit(
            [&](auto const &part) {
                using T = std::decay_t<decltype(part)>;
                if constexpr (std::is_same_v<T, scene::MoveTo> || std::is_same_v<T, scene::LineTo>)
                    points.push_back(part.to);
            },
            segment);
    return points;
}

void append_path_element(std::string &out, scene::Path const &path) {
    auto const tag = path.element == scene::PathElement::polygon ? "polygon"
                     : path.element == scene::PathElement::line  ? "line"
                                                                 : "path";
    out += '<';
    out += tag;
    if (path.element == scene::PathElement::path) {
        std::string data;
        append_path(data, path.geometry);
        detail::append_attribute(out, "d", data);
    } else {
        auto const points = path_vertices(path.geometry);
        if (path.element == scene::PathElement::polygon) {
            std::string values;
            for (auto const &point : points) {
                if (!values.empty())
                    values += ' ';
                values += number(point.x * 1000) + "," + number(point.y * 1000);
            }
            detail::append_attribute(out, "points", values);
        } else if (points.size() >= 2) {
            detail::append_attribute(out, "x1", number(points[0].x * 1000));
            detail::append_attribute(out, "y1", number(points[0].y * 1000));
            detail::append_attribute(out, "x2", number(points[1].x * 1000));
            detail::append_attribute(out, "y2", number(points[1].y * 1000));
        }
    }
    append_paint(out, path.paint, path.role == scene::ElementRole::background_box);
    static constexpr std::array<std::string_view, 16> reserved{"d",
                                                               "points",
                                                               "x1",
                                                               "y1",
                                                               "x2",
                                                               "y2",
                                                               "fill",
                                                               "stroke",
                                                               "fill-rule",
                                                               "fill-opacity",
                                                               "stroke-opacity",
                                                               "stroke-width",
                                                               "opacity",
                                                               "stroke-dasharray",
                                                               "stroke-linecap",
                                                               "stroke-linejoin"};
    detail::append_metadata(out, path.metadata, reserved);
    out += "/>";
}

void append_text_run(std::string &out, scene::TextRun const &run) {
    out += "<text";
    if (run.role == scene::ElementRole::id_alignment_marker)
        detail::append_attribute(out, "data-id-align", "true");
    std::string transform = "scale(1,-1)";
    if (run.origin.x || run.origin.y)
        transform = "translate(" + number(run.origin.x * 1000) + " " + number(run.origin.y * 1000) +
                    ") " + transform;
    detail::append_attribute(out, "transform", transform);
    if (run.font) {
        detail::append_attribute(out, "font-family", run.font->family);
        detail::append_attribute(out, "font-size", number(run.font->size * 1000) + "px");
        if (run.font->italic)
            detail::append_attribute(out, "font-style", "italic");
        if (run.font->bold)
            detail::append_attribute(out, "font-weight", "bold");
        if (!run.font->variant.empty())
            detail::append_attribute(out, "data-variant", run.font->variant);
    }
    append_paint(out, run.paint);
    static constexpr std::array<std::string_view, 17> reserved{
        "transform",       "font-family",  "font-size",        "font-style",   "font-weight",
        "data-variant",    "fill",         "stroke",           "fill-opacity", "stroke-opacity",
        "opacity",         "stroke-width", "stroke-dasharray", "fill-rule",    "stroke-linecap",
        "stroke-linejoin", "data-id-align"};
    detail::append_metadata(out, run.metadata, reserved);
    if (run.utf8.empty())
        out += "/>";
    else
        out += '>' + detail::escape_text(run.utf8) + "</text>";
}

void append_primitive(std::string &out, scene::Primitive const &primitive,
                      SvgWriteOptions const &options) {
    std::visit(
        [&](auto const &value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, scene::GlyphRun>)
                append_glyphs(out, value, options);
            else if constexpr (std::is_same_v<T, scene::Rule>)
                append_rule(out, value);
            else if constexpr (std::is_same_v<T, scene::Path>)
                append_path_element(out, value);
            else if constexpr (std::is_same_v<T, scene::TextRun>)
                append_text_run(out, value);
            else
                throw UnsupportedError(
                    "foreign HTML markup requires a structured SVG/XHTML serializer");
        },
        primitive);
}

void open_group(std::string &out, scene::Group const &group, SvgWriteOptions const &options);

void append_group_entries(std::string &out, scene::Group const &group,
                          SvgWriteOptions const &options) {
    for (auto const &entry : group.entries) {
        if (auto child = std::get_if<std::shared_ptr<scene::Group const>>(&entry)) {
            if (*child)
                open_group(out, **child, options);
        } else {
            append_primitive(out, std::get<scene::Primitive>(entry), options);
        }
    }
}

void open_group(std::string &out, scene::Group const &group, SvgWriteOptions const &options) {
    auto const inherited_paint = [](scene::Paint const &paint) {
        return std::holds_alternative<scene::InheritedPaint>(paint.fill) &&
               std::holds_alternative<scene::InheritedPaint>(paint.stroke) &&
               paint.stroke_width == 0 && paint.opacity == 1 &&
               paint.fill_rule == scene::FillRule::nonzero && !paint.line_cap && !paint.line_join &&
               !paint.dash_array;
    };
    bool const sentinel =
        group.id == 0 && group.transform.operations().empty() && !group.clip && !group.link &&
        !group.tooltip && group.role == scene::ElementRole::math_wrapper &&
        group.metadata.source_tag.empty() && group.metadata.math_role.empty() &&
        group.metadata.annotation_encoding.empty() && group.metadata.annotation.empty() &&
        group.metadata.ordered_attributes.empty() && !group.metadata.initial_transform &&
        inherited_paint(group.paint) && !group.viewport;
    bool const self_closing = !sentinel && group.entries.empty() && !group.tooltip && !group.clip;
    bool const is_viewport = group.viewport.has_value();
    if (group.link) {
        out += "<a";
        detail::append_attribute(out, "href", group.link->href);
        if (!group.link->target.empty())
            detail::append_attribute(out, "target", group.link->target);
        if (!group.link->rel.empty())
            detail::append_attribute(out, "rel", group.link->rel);
        out += '>';
    }
    if (!sentinel)
        out += is_viewport ? "<svg" : "<g";
    static constexpr std::array<std::string_view, 24> reserved{"data-mml-node",
                                                               "transform",
                                                               "clip-path",
                                                               "fill",
                                                               "stroke",
                                                               "fill-rule",
                                                               "fill-opacity",
                                                               "stroke-opacity",
                                                               "stroke-width",
                                                               "opacity",
                                                               "stroke-linecap",
                                                               "stroke-linejoin",
                                                               "stroke-dasharray",
                                                               "href",
                                                               "target",
                                                               "rel",
                                                               "style",
                                                               "x",
                                                               "y",
                                                               "width",
                                                               "height",
                                                               "viewBox",
                                                               "preserveAspectRatio",
                                                               "xmlns"};
    if (!sentinel) {
        auto const is_extender = group.role == scene::ElementRole::vertical_extender_viewport ||
                                 group.role == scene::ElementRole::horizontal_extender_viewport;
        auto append_viewbox = [&] {
            auto const &v = *group.viewport;
            detail::append_attribute(
                out, "viewBox",
                number(v.view_box.x * 1000) + " " + number(v.view_box.y * 1000) + " " +
                    number(v.view_box.width * 1000) + " " + number(v.view_box.height * 1000));
        };
        auto append_aspect = [&] {
            if (!group.viewport->aspect_ratio)
                return;
            auto token = [](scene::ViewportAlignment a) -> std::string_view {
                switch (a) {
                case scene::ViewportAlignment::min:
                    return "Min";
                case scene::ViewportAlignment::mid:
                    return "Mid";
                case scene::ViewportAlignment::max:
                    return "Max";
                }
                throw std::invalid_argument("unknown viewport alignment");
            };
            auto const &a = *group.viewport->aspect_ratio;
            std::string value = "x" + std::string(token(a.x)) + "Y" + std::string(token(a.y));
            switch (a.fit) {
            case scene::ViewportFit::meet:
                break;
            case scene::ViewportFit::slice:
                value += " slice";
                break;
            case scene::ViewportFit::none:
                value = "none";
                break;
            default:
                throw std::invalid_argument("unknown viewport fit");
            }
            detail::append_attribute(out, "preserveAspectRatio", value);
        };
        auto append_viewport_bounds = [&] {
            auto const &b = group.viewport->bounds;
            detail::append_attribute(out, "width", number(b.width * 1000));
            detail::append_attribute(out, "height", number(b.height * 1000));
            if (group.role == scene::ElementRole::vertical_extender_viewport) {
                detail::append_attribute(out, "y", number(b.y * 1000));
                detail::append_attribute(out, "x", number(b.x * 1000));
            } else if (group.role == scene::ElementRole::horizontal_extender_viewport) {
                detail::append_attribute(out, "x", number(b.x * 1000));
                detail::append_attribute(out, "y", number(b.y * 1000));
            } else {
                detail::append_attribute(out, "x", number(b.x * 1000));
                detail::append_attribute(out, "y", number(b.y * 1000));
            }
        };
        if (!group.metadata.source_tag.empty())
            detail::append_attribute(out, "data-mml-node", group.metadata.source_tag);
        auto const style =
            std::ranges::find(group.metadata.ordered_attributes, std::string_view{"style"},
                              &scene::SemanticAttribute::name);
        if (style != group.metadata.ordered_attributes.end())
            detail::append_attribute(out, "style", style->value);
        if (group.metadata.initial_transform && !group.transform.operations().empty())
            append_transform(out, group.transform, group.role);
        if (is_viewport && is_extender)
            append_viewport_bounds();
        append_paint(out, group.paint);
        if (group.clip)
            detail::append_attribute(out, "clip-path",
                                     "url(#clip-" + std::to_string(group.id) + ")");
        detail::append_metadata(out, group.metadata, reserved);
        if (is_viewport) {
            append_aspect();
            if (!is_extender && group.role != scene::ElementRole::table_content_viewport &&
                group.role != scene::ElementRole::table_label_frame)
                append_viewport_bounds();
            append_viewbox();
            detail::append_attribute(out, "xmlns", "http://www.w3.org/2000/svg");
        }
        if (!group.metadata.initial_transform && !group.transform.operations().empty())
            append_transform(out, group.transform, group.role);
        out += self_closing ? "/>" : ">";
    }
    if (!self_closing) {
        if (group.tooltip) {
            if (group.tooltip->text.empty())
                out += "<title/>";
            else
                out += "<title>" + detail::escape_text(group.tooltip->text) + "</title>";
        }
        if (group.clip) {
            out += "<defs><clipPath id=\"clip-" + std::to_string(group.id) + "\"><path d=\"";
            append_path(out, group.clip->geometry);
            out += "\"";
            if (group.clip->fill_rule == scene::FillRule::evenodd)
                detail::append_attribute(out, "clip-rule", "evenodd");
            out += "/></clipPath></defs>";
        }
        append_group_entries(out, group, options);
    }
    if (!sentinel && !self_closing)
        out += is_viewport ? "</svg>" : "</g>";
    if (group.link)
        out += "</a>";
}

void append_font_cache(std::string &out, Cache const &cache) {
    if (cache.empty())
        return;
    out += "<defs>";
    for (auto const &[id, path] : cache) {
        out += "<path";
        detail::append_attribute(out, "id", id);
        detail::append_attribute(out, "d", path);
        out += "/>";
    }
    out += "</defs>";
}

void append_svg_start(std::string &out, scene::Metrics const &metrics,
                      scene::FontMetrics const &font, SvgWriteOptions const &options,
                      scene::DisplayLayout const &layout, double minimum_extent,
                      bool apply_inline_layout, Cache const *cache = nullptr,
                      bool cache_before_group = false) {
    const double width = std::max(metrics.width, minimum_extent);
    const double height = std::max(metrics.height + metrics.depth, minimum_extent);
    out += "<svg";
    std::string style = "vertical-align: " + ex_value(-metrics.depth, font.x_height) + ";";
    if (apply_inline_layout) {
        if (layout.minimum_width > 0)
            style += " min-width: " + ex_value(layout.minimum_width, font.x_height) + ";";
        if (layout.shift != 0) {
            if (layout.alignment == "center" || layout.alignment == "left")
                style += " margin-left: " + ex_value(layout.shift, font.x_height) + ";";
            if (layout.alignment == "center" || layout.alignment == "right")
                style += " margin-right: " + ex_value(-layout.shift, font.x_height) + ";";
        }
    }
    detail::append_attribute(out, "style", style);
    detail::append_attribute(out, "xmlns", "http://www.w3.org/2000/svg");
    detail::append_attribute(out, "width", ex_value(width, font.x_height));
    detail::append_attribute(out, "height", ex_value(height, font.x_height));
    detail::append_attribute(out, "role", "img");
    detail::append_attribute(out, "focusable", "false");
    detail::append_attribute(out, "viewBox",
                             "0 " + number(-metrics.height * 1000) + " " + number(width * 1000) +
                                 " " + number(height * 1000));
    if (options.font_cache != FontCacheMode::none && options.use_xlink)
        detail::append_attribute(out, "xmlns:xlink", "http://www.w3.org/1999/xlink");
    out += ">";
    if (cache_before_group && cache)
        append_font_cache(out, *cache);
    out += "<g stroke=\"currentColor\" fill=\"currentColor\" stroke-width=\"0\" "
           "transform=\"scale(1,-1)\">";
}

void append_svg_end(std::string &out, Cache const *trailing_cache = nullptr) {
    out += "</g>";
    if (trailing_cache)
        append_font_cache(out, *trailing_cache);
    out += "</svg>";
}

scene::Group const &frame_group(scene::Group const &root, scene::NodeId id) {
    for (auto const &entry : root.entries) {
        if (auto child = std::get_if<std::shared_ptr<scene::Group const>>(&entry);
            child && *child && (*child)->id == id)
            return **child;
    }
    throw std::logic_error("validated Scene frame root is missing");
}

void append_inline_break(std::string &out, scene::InlineBreak const &value) {
    auto node = [&](std::string_view attribute, std::string_view content) {
        out += "<mjx-break";
        detail::append_attribute(out, attribute, content);
        out += "> </mjx-break>";
    };
    if (value.kind == scene::InlineBreakKind::opportunity) {
        node("prebreak", "true");
        node("newline", "true");
        return;
    }
    auto const em = folia::math::lengths::em(value.spacing);
    static constexpr std::array<std::pair<std::string_view, std::string_view>, 6> spaces{
        {{"0", "0"},
         {"0.111em", "1"},
         {"0.167em", "2"},
         {"0.222em", "3"},
         {"0.278em", "4"},
         {"0.333em", "5"}}};
    using SpaceValue = std::pair<std::string_view, std::string_view>;
    if (auto found = std::ranges::find(spaces, std::string_view{em}, &SpaceValue::first);
        found != spaces.end()) {
        node("size", found->second);
    } else {
        node("style", "letter-spacing: " + folia::math::lengths::em(value.spacing - 1));
    }
}
} // namespace

std::string write(scene::Scene const &scene_value, SvgWriteOptions const &options) {
    validate_scene(scene_value.root());
    auto const &metrics = scene_value.metrics();
    auto const &font = scene_value.font_metrics();
    if (!std::isfinite(metrics.width) || !std::isfinite(metrics.height) ||
        !std::isfinite(metrics.depth))
        throw std::invalid_argument("Scene metrics must be finite");
    auto const &layout = scene_value.display_layout();
    Cache cache;
    if (options.font_cache != FontCacheMode::none)
        collect_cache(scene_value.root(), options, cache);
    if (!cache.empty() && options.font_cache == FontCacheMode::local && options.local_id.empty())
        throw std::invalid_argument(
            "local SVG font caching requires the resolved MathJax local ID");
    if (!cache.empty() && options.font_cache == FontCacheMode::global)
        throw UnsupportedError(
            "global SVG font caching requires the owning document cache serializer");

    std::string out;
    if (options.include_math_container) {
        out += "<mjx-container";
        bool container_style = false;
        for (auto const &attribute : options.container_attributes)
            if (attribute.name == "style" && layout.minimum_width > 0) {
                detail::append_attribute(out, attribute.name,
                                         attribute.value + "; min-width: " +
                                             ex_value(layout.minimum_width, font.x_height) + ";");
                container_style = true;
            } else
                detail::append_attribute(out, attribute.name, attribute.value);
        if (layout.display)
            detail::append_attribute(out, "display", "true");
        if (layout.full_width)
            detail::append_attribute(out, "width", "full");
        if (layout.display && layout.alignment != "center")
            detail::append_attribute(out, "justify", layout.alignment);
        if (layout.minimum_width > 0 && !container_style)
            detail::append_attribute(
                out, "style", "min-width: " + ex_value(layout.minimum_width, font.x_height) + ";");
        out += '>';
    }
    auto const &frames = scene_value.frame_layout();
    if (frames.inline_fragments.empty()) {
        auto const *local_cache = options.font_cache == FontCacheMode::local ? &cache : nullptr;
        append_svg_start(out, metrics, font, options, layout, frames.minimum_extent, true,
                         local_cache, true);
        open_group(out, scene_value.root(), options);
        append_svg_end(out);
    } else {
        for (std::size_t i = 0; i < frames.inline_fragments.size(); ++i) {
            auto const &fragment = frames.inline_fragments[i];
            if (fragment.before)
                append_inline_break(out, *fragment.before);
            append_svg_start(out, fragment.metrics, font, options, layout, frames.minimum_extent,
                             false);
            open_group(out, frame_group(scene_value.root(), fragment.root), options);
            append_svg_end(out,
                           i == 0 && options.font_cache == FontCacheMode::local ? &cache : nullptr);
        }
    }
    if (options.include_math_container)
        out += "</mjx-container>";
    return out;
}
} // namespace folia::math::output::svg
