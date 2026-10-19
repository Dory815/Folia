module folia.math.output.scene;
import std;

namespace folia::math::output::scene {
namespace {
bool finite(Unit value) noexcept { return std::isfinite(value); }
void validate(Point value) {
    if (!finite(value.x) || !finite(value.y))
        throw std::invalid_argument("Scene point coordinates must be finite");
}
bool valid(FillRule rule) noexcept { return rule == FillRule::nonzero || rule == FillRule::evenodd; }
Affine validate_affine(Affine const& transform) {
    if (!(finite(transform.a) && finite(transform.b) && finite(transform.c) &&
          finite(transform.d) && finite(transform.e) && finite(transform.f)))
        throw std::invalid_argument("Scene transform must be finite");
    return transform;
}
void validate(Transform const& transform) {
    for (auto const& operation : transform.operations()) std::visit([](auto const& value) {
        using T = std::remove_cvref_t<decltype(value)>;
        if constexpr (std::same_as<T, Translation>) {
            if (!finite(value.x) || !finite(value.y))
                throw std::invalid_argument("Scene translation must be finite");
        } else if constexpr (std::same_as<T, Scaling>) {
            if (!finite(value.x) || !finite(value.y))
                throw std::invalid_argument("Scene scale must be finite");
        } else if constexpr (std::same_as<T, Rotation>) {
            if (!finite(value.degrees)) throw std::invalid_argument("Scene rotation angle must be finite");
            validate(value.center);
        } else validate_affine(value);
    }, operation);
    (void)validate_affine(transform.matrix());
}
Metrics validate(Metrics metrics) {
    if (!(finite(metrics.width) && finite(metrics.height) && finite(metrics.depth)))
        throw std::invalid_argument("Scene metrics must be finite");
    return metrics;
}
FontMetrics validate(FontMetrics metrics) {
    if (!(finite(metrics.em) && finite(metrics.ex) && finite(metrics.x_height) && finite(metrics.cap_height)) ||
        metrics.em <= 0 || metrics.ex < 0 || metrics.x_height < 0 || metrics.cap_height < 0)
        throw std::invalid_argument("Scene font metrics are invalid");
    return metrics;
}
void validate(Paint const& paint) {
    if (!finite(paint.opacity) || paint.opacity < 0 || paint.opacity > 1 ||
        !finite(paint.stroke_width) || paint.stroke_width < 0 || !valid(paint.fill_rule))
        throw std::invalid_argument("Scene paint values are invalid");
}
void validate(PathGeometry const& path) {
    for (auto const& segment : path.segments) std::visit([](auto const& part) {
        using S = std::remove_cvref_t<decltype(part)>;
        if constexpr (std::same_as<S, MoveTo> || std::same_as<S, LineTo>) validate(part.to);
        else if constexpr (std::same_as<S, QuadTo>) { validate(part.control); validate(part.to); }
        else if constexpr (std::same_as<S, CubicTo>) { validate(part.control1); validate(part.control2); validate(part.to); }
        else if constexpr (std::same_as<S, ArcTo>) {
            validate(part.to);
            if (!finite(part.radius_x) || !finite(part.radius_y) || !finite(part.rotation) ||
                part.radius_x < 0 || part.radius_y < 0)
                throw std::invalid_argument("Scene arc parameters are invalid");
        }
    }, segment);
}
void validate(Clip const& clip) {
    if (!valid(clip.fill_rule)) throw std::invalid_argument("Scene clip fill rule is invalid");
    validate(clip.geometry);
}
Glyph clone_glyph(Glyph const& source) {
    // GlyphResource can only be created from an owned outline snapshot and has
    // no mutation surface, so retaining its shared const handle is safe.
    return source;
}
GlyphRun validate_and_clone(GlyphRun const& source) {
    GlyphRun copy;
    copy.glyphs.reserve(source.glyphs.size());
    for (auto const& glyph : source.glyphs) {
        if (!glyph.resource) throw std::invalid_argument("Scene glyph needs an immutable outline resource");
        validate(glyph.origin);
        if (!finite(glyph.advance)) throw std::invalid_argument("Scene glyph advance must be finite");
        validate(glyph.paint);
        validate(glyph.resource->outline().outline);
        copy.glyphs.push_back(clone_glyph(glyph));
    }
    return copy;
}
Metrics validate_metrics(Metrics value) { return validate(value); }
Primitive clone_primitive(Primitive const& source) {
    return std::visit([](auto const& value) -> Primitive {
        using T = std::remove_cvref_t<decltype(value)>;
        T copy = value;
        if constexpr (std::same_as<T, GlyphRun>) copy = validate_and_clone(value);
        else if constexpr (std::same_as<T, Rule>) {
            if (!finite(value.bounds.x) || !finite(value.bounds.y) || !finite(value.bounds.width) ||
                !finite(value.bounds.height) || value.bounds.width < 0 || value.bounds.height < 0)
                throw std::invalid_argument("Scene rule bounds are invalid");
            validate(value.paint);
        } else if constexpr (std::same_as<T, Path>) { validate(value.geometry); validate(value.paint); }
        else if constexpr (std::same_as<T, TextRun>) {
            copy.metrics = validate_metrics(value.metrics);
            validate(value.origin);
            validate(value.paint);
            copy.measured_glyphs = validate_and_clone(value.measured_glyphs);
        }
        else if constexpr (std::same_as<T, ForeignContent>) {
            copy.metrics = validate_metrics(value.metrics);
            validate(value.origin);
            for (std::size_t i=0; i<copy.measured_runs.size(); ++i)
                copy.measured_runs[i].run = std::get<TextRun>(clone_primitive(Primitive{value.measured_runs[i].run}));
        }
        return copy;
    }, source);
}
Group clone_group(Group const& source, std::unordered_set<Group const*>& active,
    std::unordered_set<NodeId>& ids) {
    if (!ids.insert(source.id).second)
        throw std::invalid_argument("Scene group ids must be unique");
    validate(source.transform);
    if (source.clip) validate(*source.clip);
    Group copy;
    copy.id = source.id;
    copy.transform = source.transform;
    copy.clip = source.clip;
    copy.link = source.link;
    copy.tooltip = source.tooltip;
    copy.metadata = source.metadata;
    copy.role = source.role;
    copy.entries.reserve(source.entries.size());
    for (auto const& entry : source.entries) {
        if (auto const* primitive = std::get_if<Primitive>(&entry))
            copy.entries.emplace_back(clone_primitive(*primitive));
        else {
            auto const& child = std::get<std::shared_ptr<Group const>>(entry);
            if (!child) throw std::invalid_argument("Scene group entries cannot contain null groups");
            if (!active.insert(child.get()).second)
                throw std::invalid_argument("Scene group hierarchy cannot contain cycles");
            auto cloned = clone_group(*child, active, ids);
            active.erase(child.get());
            copy.entries.emplace_back(std::make_shared<Group const>(std::move(cloned)));
        }
    }
    return copy;
}
Group snapshot_group(Group const& source) {
    std::unordered_set<Group const*> active;
    std::unordered_set<NodeId> ids;
    return clone_group(source, active, ids);
}
}

Scene::Scene(Group root, Metrics metrics, FontMetrics font_metrics)
    : root_(snapshot_group(root)), metrics_(validate(metrics)), font_metrics_(validate(font_metrics)) {}
Group const& Scene::root() const noexcept { return root_; }
Metrics const& Scene::metrics() const noexcept { return metrics_; }
FontMetrics const& Scene::font_metrics() const noexcept { return font_metrics_; }
}
