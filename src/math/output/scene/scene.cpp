module folia.math.output.scene;
import std;

namespace folia::math::output::scene {
namespace {
bool finite(Unit value) noexcept {
    return std::isfinite(value);
}
void validate(Point value) {
    if (!finite(value.x) || !finite(value.y))
        throw std::invalid_argument("Scene point coordinates must be finite");
}
bool valid(FillRule rule) noexcept {
    return rule == FillRule::nonzero || rule == FillRule::evenodd;
}
bool valid(CoordinateMode mode) noexcept {
    return mode == CoordinateMode::absolute || mode == CoordinateMode::relative;
}
bool valid(ArcFlagSeparator separator) noexcept {
    return separator == ArcFlagSeparator::space || separator == ArcFlagSeparator::comma;
}
Affine validate_affine(Affine const &transform) {
    if (!(finite(transform.a) && finite(transform.b) && finite(transform.c) &&
          finite(transform.d) && finite(transform.e) && finite(transform.f)))
        throw std::invalid_argument("Scene transform must be finite");
    return transform;
}
void validate(Transform const &transform) {
    for (auto const &operation : transform.operations())
        std::visit(
            [](auto const &value) {
                using T = std::remove_cvref_t<decltype(value)>;
                if constexpr (std::same_as<T, Translation>) {
                    if (!finite(value.x) || !finite(value.y))
                        throw std::invalid_argument("Scene translation must be finite");
                    if (value.separator != TranslationSeparator::role_default &&
                        value.separator != TranslationSeparator::comma &&
                        value.separator != TranslationSeparator::space &&
                        value.separator != TranslationSeparator::comma_space)
                        throw std::invalid_argument("Scene translation separator is invalid");
                } else if constexpr (std::same_as<T, Scaling>) {
                    if (!finite(value.x) || !finite(value.y) || value.precision > 12)
                        throw std::invalid_argument("Scene scale values are invalid");
                } else if constexpr (std::same_as<T, Rotation>) {
                    if (!finite(value.degrees))
                        throw std::invalid_argument("Scene rotation angle must be finite");
                    validate(value.center);
                } else
                    validate_affine(value);
            },
            operation);
    (void)validate_affine(transform.matrix());
}
Metrics validate(Metrics metrics) {
    if (!(finite(metrics.width) && finite(metrics.height) && finite(metrics.depth)))
        throw std::invalid_argument("Scene metrics must be finite");
    return metrics;
}
FontMetrics validate(FontMetrics metrics) {
    if (!(finite(metrics.em) && finite(metrics.ex) && finite(metrics.x_height) &&
          finite(metrics.cap_height)) ||
        metrics.em <= 0 || metrics.ex < 0 || metrics.x_height < 0 || metrics.cap_height < 0)
        throw std::invalid_argument("Scene font metrics are invalid");
    return metrics;
}
void validate(Paint const &paint) {
    if (!finite(paint.opacity) || paint.opacity < 0 || paint.opacity > 1 ||
        !finite(paint.stroke_width) || paint.stroke_width < 0 || !valid(paint.fill_rule))
        throw std::invalid_argument("Scene paint values are invalid");
}
void validate(SemanticMetadata const &metadata) {
    for (auto const &attribute : metadata.ordered_attributes)
        if (attribute.name.empty())
            throw std::invalid_argument("Scene metadata attribute name cannot be empty");
}
void validate(ForeignContent const &foreign) {
    auto const &geometry = foreign.geometry;
    if (!(finite(geometry.x) && finite(geometry.y) && finite(geometry.width) &&
          finite(geometry.height) && finite(geometry.em_per_pixel)) ||
        geometry.width < 0 || geometry.height < 0 || geometry.em_per_pixel <= 0)
        throw std::invalid_argument("Scene foreignObject geometry is invalid");
    validate(foreign.metadata);
}
FrameLayout validate_frames(Group const &root, FrameLayout frames) {
    if (!finite(frames.minimum_extent) || frames.minimum_extent <= 0)
        throw std::invalid_argument("Scene frame minimum extent must be finite and positive");
    std::vector<NodeId> direct_groups;
    for (auto const &entry : root.entries) {
        if (auto child = std::get_if<std::shared_ptr<Group const>>(&entry)) {
            if (!*child)
                throw std::invalid_argument("Scene root cannot contain a null group");
            direct_groups.push_back((*child)->id);
        } else if (!frames.inline_fragments.empty()) {
            throw std::invalid_argument(
                "Inline frames require root entries to be completed groups");
        }
    }
    if (!frames.inline_fragments.empty()) {
        if (frames.percentage)
            throw std::invalid_argument("Percentage SVG frames cannot contain inline fragments");
        if (frames.inline_fragments.size() != direct_groups.size())
            throw std::invalid_argument("Scene inline frame roots must match root child groups");
        for (std::size_t i = 0; i < frames.inline_fragments.size(); ++i) {
            auto const &fragment = frames.inline_fragments[i];
            if (fragment.root == 0 || fragment.root != direct_groups[i])
                throw std::invalid_argument(
                    "Scene inline frame roots must be direct children in order");
            validate(fragment.metrics);
            if (fragment.before) {
                if (fragment.before->kind != InlineBreakKind::opportunity &&
                    fragment.before->kind != InlineBreakKind::forced)
                    throw std::invalid_argument("Scene inline break kind is invalid");
                if (!finite(fragment.before->spacing))
                    throw std::invalid_argument("Scene inline break spacing must be finite");
            }
        }
    }
    if (frames.percentage) {
        auto const &percentage = *frames.percentage;
        auto const &bounds = percentage.bounds;
        if (!finite(percentage.percent) || percentage.percent < 0 ||
            !(finite(bounds.x) && finite(bounds.y) && finite(bounds.width) &&
              finite(bounds.height)) ||
            bounds.width < 0 || bounds.height < 0 || !finite(percentage.scale) ||
            percentage.scale <= 0)
            throw std::invalid_argument("Scene percentage frame is invalid");
    }
    return frames;
}
void validate(PathGeometry const &path) {
    for (auto const &segment : path.segments)
        std::visit(
            [](auto const &part) {
                using S = std::remove_cvref_t<decltype(part)>;
                if constexpr (std::same_as<S, MoveTo>)
                    validate(part.to);
                else if constexpr (std::same_as<S, LineTo>) {
                    validate(part.to);
                    if (!valid(part.mode))
                        throw std::invalid_argument("Scene line coordinate mode is invalid");
                } else if constexpr (std::same_as<S, QuadTo>) {
                    validate(part.control);
                    validate(part.to);
                } else if constexpr (std::same_as<S, CubicTo>) {
                    validate(part.control1);
                    validate(part.control2);
                    validate(part.to);
                } else if constexpr (std::same_as<S, ArcTo>) {
                    validate(part.to);
                    if (!finite(part.radius_x) || !finite(part.radius_y) ||
                        !finite(part.rotation) || part.radius_x < 0 || part.radius_y < 0)
                        throw std::invalid_argument("Scene arc parameters are invalid");
                    if (!valid(part.mode) || !valid(part.flags_separator))
                        throw std::invalid_argument("Scene arc spelling is invalid");
                }
            },
            segment);
}
void validate(Clip const &clip) {
    if (!valid(clip.fill_rule))
        throw std::invalid_argument("Scene clip fill rule is invalid");
    validate(clip.geometry);
}
void validate(Viewport const &viewport) {
    auto const &bounds = viewport.bounds;
    auto const &view_box = viewport.view_box;
    if (!(finite(bounds.x) && finite(bounds.y) && finite(bounds.width) && finite(bounds.height)) ||
        bounds.width < 0 || bounds.height < 0)
        throw std::invalid_argument("Scene viewport bounds are invalid");
    if (!(finite(view_box.x) && finite(view_box.y) && finite(view_box.width) &&
          finite(view_box.height)) ||
        view_box.width <= 0 || view_box.height <= 0)
        throw std::invalid_argument("Scene viewport viewBox is invalid");
    if (viewport.aspect_ratio) {
        auto const &aspect = *viewport.aspect_ratio;
        if ((aspect.x != ViewportAlignment::min && aspect.x != ViewportAlignment::mid &&
             aspect.x != ViewportAlignment::max) ||
            (aspect.y != ViewportAlignment::min && aspect.y != ViewportAlignment::mid &&
             aspect.y != ViewportAlignment::max) ||
            (aspect.fit != ViewportFit::meet && aspect.fit != ViewportFit::slice &&
             aspect.fit != ViewportFit::none))
            throw std::invalid_argument("Scene viewport aspect ratio is invalid");
    }
}
Glyph clone_glyph(Glyph const &source) {
    // GlyphResource can only be created from an owned outline snapshot and has
    // no mutation surface, so retaining its shared const handle is safe.
    return source;
}
GlyphRun validate_and_clone(GlyphRun const &source) {
    GlyphRun copy;
    copy.glyphs.reserve(source.glyphs.size());
    for (auto const &glyph : source.glyphs) {
        if (!glyph.resource)
            throw std::invalid_argument("Scene glyph needs an immutable outline resource");
        validate(glyph.origin);
        if (!finite(glyph.advance))
            throw std::invalid_argument("Scene glyph advance must be finite");
        validate(glyph.paint);
        validate(glyph.transform);
        validate(glyph.resource->outline().outline);
        copy.glyphs.push_back(clone_glyph(glyph));
    }
    return copy;
}
Metrics validate_metrics(Metrics value) {
    return validate(value);
}
Primitive clone_primitive(Primitive const &source) {
    return std::visit(
        [](auto const &value) -> Primitive {
            using T = std::remove_cvref_t<decltype(value)>;
            T copy = value;
            if constexpr (std::same_as<T, GlyphRun>)
                copy = validate_and_clone(value);
            else if constexpr (std::same_as<T, Rule>) {
                if (!finite(value.bounds.x) || !finite(value.bounds.y) ||
                    !finite(value.bounds.width) || !finite(value.bounds.height) ||
                    value.bounds.width < 0 || value.bounds.height < 0)
                    throw std::invalid_argument("Scene rule bounds are invalid");
                if (value.corner_radius &&
                    (!finite(*value.corner_radius) || *value.corner_radius < 0))
                    throw std::invalid_argument("Scene rule corner radius is invalid");
                if (value.role == ElementRole::error_background && value.bounds.x != 0)
                    throw std::invalid_argument("Scene error background must start at x=0");
                validate(value.paint);
                validate(value.metadata);
            } else if constexpr (std::same_as<T, Path>) {
                validate(value.geometry);
                validate(value.paint);
                validate(value.metadata);
                validate(value.transform);
                if (value.layout_width &&
                    (!finite(*value.layout_width) || *value.layout_width < 0))
                    throw std::invalid_argument("Scene path layout width is invalid");
                if ((value.syntax != PathSyntax::compact && value.syntax != PathSyntax::spaced) ||
                    (value.paint_encoding != PathPaintEncoding::attributes &&
                     value.paint_encoding != PathPaintEncoding::stroke_width_style) ||
                    (value.join_spelling != StrokeJoinSpelling::standard &&
                     value.join_spelling != StrokeJoinSpelling::mitre))
                    throw std::invalid_argument("Scene path spelling is invalid");
                if (value.join_spelling == StrokeJoinSpelling::mitre &&
                    (value.paint_encoding != PathPaintEncoding::stroke_width_style ||
                     value.paint.line_join != StrokeJoin::miter))
                    throw std::invalid_argument(
                        "Scene mitre spelling requires stroked miter paint");
            } else if constexpr (std::same_as<T, Ellipse>) {
                validate(value.center);
                if (!finite(value.radius_x) || !finite(value.radius_y) || value.radius_x < 0 ||
                    value.radius_y < 0)
                    throw std::invalid_argument("Scene ellipse radii are invalid");
                validate(value.paint);
                validate(value.metadata);
            } else if constexpr (std::same_as<T, TextRun>) {
                copy.metrics = validate_metrics(value.metrics);
                validate(value.origin);
                validate(value.paint);
                if (value.font && (!finite(value.font->size) || value.font->size <= 0))
                    throw std::invalid_argument("Scene text font size must be finite and positive");
                validate(value.metadata);
                copy.measured_glyphs = validate_and_clone(value.measured_glyphs);
            }
            return copy;
        },
        source);
}
Group clone_group(Group const &source, std::unordered_set<Group const *> &active,
                  std::unordered_set<NodeId> &ids) {
    if (!ids.insert(source.id).second)
        throw std::invalid_argument("Scene group ids must be unique");
    validate(source.transform);
    validate(source.paint);
    validate(source.metadata);
    if (source.clip)
        validate(*source.clip);
    if (source.viewport)
        validate(*source.viewport);
    if (source.foreign_content) {
        validate(*source.foreign_content);
        if (!source.entries.empty() || source.viewport || source.clip || source.tooltip ||
            source.link)
            throw std::invalid_argument(
                "Scene foreignObject group cannot have entries, viewport, clip, tooltip or link");
    }
    Group copy;
    copy.id = source.id;
    copy.transform = source.transform;
    copy.clip = source.clip;
    copy.link = source.link;
    copy.tooltip = source.tooltip;
    copy.metadata = source.metadata;
    copy.role = source.role;
    copy.paint = source.paint;
    copy.viewport = source.viewport;
    copy.foreign_content = source.foreign_content;
    copy.entries.reserve(source.entries.size());
    for (auto const &entry : source.entries) {
        if (auto const *primitive = std::get_if<Primitive>(&entry))
            copy.entries.emplace_back(clone_primitive(*primitive));
        else {
            auto const &child = std::get<std::shared_ptr<Group const>>(entry);
            if (!child)
                throw std::invalid_argument("Scene group entries cannot contain null groups");
            if (!active.insert(child.get()).second)
                throw std::invalid_argument("Scene group hierarchy cannot contain cycles");
            auto cloned = clone_group(*child, active, ids);
            active.erase(child.get());
            copy.entries.emplace_back(std::make_shared<Group const>(std::move(cloned)));
        }
    }
    return copy;
}
Group snapshot_group(Group const &source) {
    std::unordered_set<Group const *> active;
    std::unordered_set<NodeId> ids;
    return clone_group(source, active, ids);
}
} // namespace

Scene::Scene(Group root, Metrics metrics, FontMetrics font_metrics, DisplayLayout display_layout,
             FrameLayout frame_layout)
    : root_(snapshot_group(root)), metrics_(validate(metrics)),
      font_metrics_(validate(font_metrics)), display_layout_(std::move(display_layout)),
      frame_layout_(validate_frames(root_, std::move(frame_layout))) {
    if (!finite(display_layout_.shift) || !finite(display_layout_.minimum_width) ||
        display_layout_.minimum_width < 0 || !finite(display_layout_.font_scale) ||
        display_layout_.font_scale < 0)
        throw std::invalid_argument(std::format(
            "Scene display layout values are invalid (shift={}, minimum_width={}, font_scale={})",
            display_layout_.shift, display_layout_.minimum_width, display_layout_.font_scale));
    if (display_layout_.alignment != "left" && display_layout_.alignment != "center" &&
        display_layout_.alignment != "right")
        throw std::invalid_argument("Scene display alignment is invalid");
}
Group const &Scene::root() const noexcept {
    return root_;
}
Metrics const &Scene::metrics() const noexcept {
    return metrics_;
}
FontMetrics const &Scene::font_metrics() const noexcept {
    return font_metrics_;
}
DisplayLayout const &Scene::display_layout() const noexcept {
    return display_layout_;
}
FrameLayout const &Scene::frame_layout() const noexcept {
    return frame_layout_;
}
} // namespace folia::math::output::scene
