module folia.math.output.common;
import :notation;
import std;

namespace folia::math::output::common {
namespace {
constexpr double pi = std::numbers::pi_v<double>;
constexpr double near_zero = .001;

double sourceMax(double a, double b) noexcept {
    if (std::isnan(a) || std::isnan(b))
        return std::numeric_limits<double>::quiet_NaN();
    return std::max(a, b);
}

PaddingData repeated(double value) noexcept {
    return {value, value, value, value};
}

std::optional<std::size_t> sideIndex(NotationKind kind) noexcept {
    switch (kind) {
    case NotationKind::top:
        return 0;
    case NotationKind::right:
        return 1;
    case NotationKind::bottom:
        return 2;
    case NotationKind::left:
        return 3;
    default:
        return std::nullopt;
    }
}

std::array<std::size_t, 2> pairSides(NotationKind kind) {
    switch (kind) {
    case NotationKind::actuarial:
        return {0, 1};
    case NotationKind::madruwb:
        return {2, 1};
    default:
        throw std::invalid_argument("notation has no paired border sides");
    }
}

// Menclose coordinates are emitted inside MathJax's surrounding
// `scale(1,-1)` group. Keep the wrapper-local em coordinates unchanged here;
// the common SVG root performs the one renderer-coordinate reflection.
scene::Point scenePoint(double x, double y) noexcept {
    return {x, y};
}

void move(scene::PathGeometry &path, double x, double y) {
    path.segments.emplace_back(scene::MoveTo{scenePoint(x, y)});
}

void line(scene::PathGeometry &path, double x, double y) {
    path.segments.emplace_back(scene::LineTo{scenePoint(x, y)});
}

void relative(scene::PathGeometry &path, double &x, double &y, double dx, double dy) {
    x += dx;
    y += dy;
    line(path, x, y);
}
} // namespace

std::optional<NotationKind> findNotation(std::span<NotationSpec const> catalog,
                                         std::u16string_view name) {
    for (const auto &spec : catalog) {
        if (name.size() != spec.source_name.size())
            continue;
        bool equal = true;
        for (std::size_t i = 0; i < name.size(); ++i)
            if (name[i] != static_cast<unsigned char>(spec.source_name[i])) {
                equal = false;
                break;
            }
        if (equal)
            return spec.kind;
    }
    return std::nullopt;
}

double arrowHead(double padding, double thickness, Arrowhead arrowhead) noexcept {
    return sourceMax(padding, thickness * (arrowhead.x + arrowhead.dx + 1));
}

std::pair<double, double> getArgMod(double width, double height) noexcept {
    return {std::atan2(height, width), std::sqrt(width * width + height * height)};
}

double getOffset(PaddingData trbl, std::string_view direction) noexcept {
    const double delta = direction == "X" ? (trbl[1] - trbl[3]) / 2 : (trbl[2] - trbl[0]) / 2;
    return std::abs(delta) > near_zero ? delta : 0;
}

ArrowMetrics arrowData(double padding, double thickness, Arrowhead arrowhead,
                       BBox const &child) noexcept {
    const double height = child.h + child.d;
    const double diagonal = std::sqrt(height * height + child.w * child.w);
    const double radius = thickness * (arrowhead.x + sourceMax(1, arrowhead.dx));
    const double x = sourceMax(padding, radius * child.w / diagonal);
    const double y = sourceMax(padding, radius * height / diagonal);
    const auto [angle, width] = getArgMod(child.w + 2 * x, height + 2 * y);
    return {angle, width, x, y};
}

std::pair<double, double> arrowAW(PaddingData trbl, BBox const &child) noexcept {
    return getArgMod(trbl[3] + child.w + trbl[1], trbl[0] + child.h + child.d + trbl[2]);
}

PaddingData notationBBox(NotationSpec const &spec, double padding, double thickness,
                         Arrowhead arrowhead, BBox const *content, PaddingData radical) {
    switch (spec.bbox) {
    case BBoxRecipe::full:
        return repeated(thickness + padding);
    case BBoxRecipe::full_padding:
        return repeated(padding);
    case BBoxRecipe::one_side: {
        PaddingData result{};
        const auto side = sideIndex(spec.kind);
        if (!side)
            throw std::invalid_argument("one-side recipe requires a side notation");
        result[*side] = thickness + padding;
        return result;
    }
    case BBoxRecipe::two_sides: {
        PaddingData result{};
        for (const auto side : pairSides(spec.kind))
            result[side] = thickness + padding;
        return result;
    }
    case BBoxRecipe::diagonal_strike:
        return repeated(thickness + padding);
    case BBoxRecipe::horizontal_strike:
        return {0, padding, 0, padding};
    case BBoxRecipe::vertical_strike:
        return {padding, 0, padding, 0};
    case BBoxRecipe::arrow: {
        PaddingData result{};
        const double head = arrowHead(padding, thickness, arrowhead);
        const double H = content ? content->h + content->d : 0;
        const double W = content ? content->w : 0;
        switch (spec.kind) {
        case NotationKind::up_arrow:
            result = {head, 0, padding, 0};
            if (content)
                result[1] = result[3] = sourceMax(0, thickness * arrowhead.y - W / 2);
            break;
        case NotationKind::down_arrow:
            result = {padding, 0, head, 0};
            if (content)
                result[1] = result[3] = sourceMax(0, thickness * arrowhead.y - W / 2);
            break;
        case NotationKind::right_arrow:
            result = {0, head, 0, padding};
            if (content)
                result[0] = result[2] = sourceMax(0, thickness * arrowhead.y - H / 2);
            break;
        case NotationKind::left_arrow:
            result = {0, padding, 0, head};
            if (content)
                result[0] = result[2] = sourceMax(0, thickness * arrowhead.y - H / 2);
            break;
        case NotationKind::up_down_arrow:
            result = {head, 0, head, 0};
            if (content)
                result[1] = result[3] = sourceMax(0, thickness * arrowhead.y - W / 2);
            break;
        case NotationKind::left_right_arrow:
            result = {0, head, 0, head};
            if (content)
                result[0] = result[2] = sourceMax(0, thickness * arrowhead.y - H / 2);
            break;
        default:
            throw std::invalid_argument("notation is not a linear arrow");
        }
        return result;
    }
    case BBoxRecipe::diagonal_arrow: {
        if (!content)
            throw std::invalid_argument("diagonal arrow requires child geometry");
        const auto data = arrowData(padding, thickness, arrowhead, *content);
        const auto [b, ar] = getArgMod(arrowhead.x + arrowhead.dx, arrowhead.y);
        const double dy = data.y + (b > data.angle ? thickness * ar * std::sin(b - data.angle) : 0);
        const double dx =
            data.x +
            (b > pi / 2 - data.angle ? thickness * ar * std::sin(b + data.angle - pi / 2) : 0);
        return {dy, dx, dy, dx};
    }
    case BBoxRecipe::phasor_angle: {
        const double p = padding / 2;
        return {2 * p, p, p + thickness, 3 * p + thickness};
    }
    case BBoxRecipe::long_div:
        return {padding + thickness, padding, padding, 2 * padding + thickness / 2};
    case BBoxRecipe::radical:
        return radical;
    }
    throw std::logic_error("unhandled menclose BBox recipe");
}

PaddingData notationBorder(NotationSpec const &spec, double thickness) {
    PaddingData result{};
    switch (spec.border) {
    case BorderRecipe::none:
        return result;
    case BorderRecipe::full:
        return repeated(thickness);
    case BorderRecipe::one_side:
        if (const auto side = sideIndex(spec.kind))
            result[*side] = thickness;
        else
            throw std::invalid_argument("one-side border recipe requires a side notation");
        return result;
    case BorderRecipe::two_sides:
        if (spec.kind == NotationKind::actuarial)
            result[0] = result[1] = thickness;
        else if (spec.kind == NotationKind::madruwb)
            result[2] = result[1] = thickness;
        else
            throw std::invalid_argument("two-side border recipe has no side mapping");
        return result;
    case BorderRecipe::phasor_bottom:
        result[2] = thickness;
        return result;
    }
    throw std::invalid_argument("notation has no implemented border recipe");
}

scene::PathGeometry lineGeometry(LineName kind, BBox const &box, double thickness) {
    const double h = box.h, d = box.d, w = box.w, t = thickness / 2;
    double x1{}, y1{}, x2{}, y2{};
    switch (kind) {
    case LineName::top:
        x1 = 0;
        y1 = h - t;
        x2 = w;
        y2 = h - t;
        break;
    case LineName::right:
        x1 = w - t;
        y1 = -d;
        x2 = w - t;
        y2 = h;
        break;
    case LineName::bottom:
        x1 = 0;
        y1 = t - d;
        x2 = w;
        y2 = t - d;
        break;
    case LineName::left:
        x1 = t;
        y1 = -d;
        x2 = t;
        y2 = h;
        break;
    case LineName::vertical:
        x1 = x2 = w / 2;
        y1 = h;
        y2 = -d;
        break;
    case LineName::horizontal:
        x1 = 0;
        y1 = y2 = (h - d) / 2;
        x2 = w;
        break;
    case LineName::up:
        x1 = t;
        y1 = t - d;
        x2 = w - t;
        y2 = h - t;
        break;
    case LineName::down:
        x1 = t;
        y1 = h - t;
        x2 = w - t;
        y2 = t - d;
        break;
    default:
        throw std::invalid_argument("unknown menclose line geometry");
    }
    scene::PathGeometry result;
    move(result, x1, y1);
    line(result, x2, y2);
    return result;
}

namespace {
scene::PathGeometry polygon(std::span<scene::Point const> points) {
    scene::PathGeometry path;
    if (points.empty())
        return path;
    path.segments.emplace_back(scene::MoveTo{points.front()});
    for (std::size_t i = 1; i < points.size(); ++i)
        path.segments.emplace_back(scene::LineTo{points[i]});
    path.segments.emplace_back(scene::ClosePath{});
    return path;
}

NotationGeometry styled(scene::PathGeometry path, double thickness, bool filled = false) {
    scene::Paint paint;
    paint.fill =
        filled ? scene::PaintColor{scene::InheritedPaint{}} : scene::PaintColor{scene::NoPaint{}};
    paint.stroke = scene::InheritedPaint{};
    paint.stroke_width = filled ? 0 : thickness;
    return {std::move(path), scene::Transform{}, std::move(paint)};
}
} // namespace

std::vector<NotationGeometry> notationGeometry(NotationSpec const &spec, BBox const &box,
                                               double thickness, double padding, Arrowhead head,
                                               PaddingData trbl, BBox const *content) {
    std::vector<NotationGeometry> result;
    auto addLine = [&](LineName name, std::string_view offset = {}) {
        auto geometry = lineGeometry(name, box, thickness);
        const double shift = getOffset(trbl, offset);
        if (shift != 0) {
            for (auto &segment : geometry.segments) {
                std::visit(
                    [&](auto &value) {
                        using S = std::remove_cvref_t<decltype(value)>;
                        if constexpr (std::same_as<S, scene::MoveTo> ||
                                      std::same_as<S, scene::LineTo>) {
                            if (offset == "X")
                                value.to.x -= shift;
                            else if (offset == "Y")
                                value.to.y -= shift;
                        }
                    },
                    segment);
            }
        }
        auto value = styled(std::move(geometry), thickness);
        value.paint.fill = scene::InheritedPaint{}; // Source line inherits the common root fill.
        result.push_back(std::move(value));
    };
    auto addArrow = [&](double width, double angle, bool both, std::string_view offset = {},
                        double distance = 0) {
        auto value = arrowGeometry(width, angle, both, offset, distance, thickness, head, box);
        result.push_back(styled(std::move(value.path), thickness, true));
        result.back().transform = std::move(value.transform);
    };
    switch (spec.paint) {
    case PaintRecipe::side_line:
        if (spec.kind == NotationKind::top)
            addLine(LineName::top);
        else if (spec.kind == NotationKind::right)
            addLine(LineName::right);
        else if (spec.kind == NotationKind::bottom)
            addLine(LineName::bottom);
        else if (spec.kind == NotationKind::left)
            addLine(LineName::left);
        else if (spec.kind == NotationKind::actuarial) {
            addLine(LineName::top);
            addLine(LineName::right);
        } else if (spec.kind == NotationKind::madruwb) {
            addLine(LineName::bottom);
            addLine(LineName::right);
        } else
            throw std::invalid_argument("unknown side-line notation");
        break;
    case PaintRecipe::diagonal_line:
        addLine(spec.kind == NotationKind::up_diagonal_strike ? LineName::up : LineName::down);
        break;
    case PaintRecipe::horizontal_line:
        addLine(LineName::horizontal, "Y");
        break;
    case PaintRecipe::vertical_line:
        addLine(LineName::vertical, "X");
        break;
    case PaintRecipe::box:
    case PaintRecipe::rounded_box: {
        const double t = thickness / 2;
        const double x0 = t, x1 = box.w - t, y0 = t - box.d, y1 = box.h - t;
        const double radius = spec.paint == PaintRecipe::rounded_box ? thickness + padding : 0;
        scene::PathGeometry path;
        if (radius <= 0) {
            const std::array<scene::Point, 4> pts{{{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}}};
            path = polygon(pts);
        } else {
            const double r = std::min({radius, (x1 - x0) / 2, (y1 - y0) / 2});
            path.segments.emplace_back(scene::MoveTo{{x0 + r, y0}});
            path.segments.emplace_back(scene::LineTo{{x1 - r, y0}});
            path.segments.emplace_back(scene::ArcTo{r, r, 0, false, true, {x1, y0 + r}});
            path.segments.emplace_back(scene::LineTo{{x1, y1 - r}});
            path.segments.emplace_back(scene::ArcTo{r, r, 0, false, true, {x1 - r, y1}});
            path.segments.emplace_back(scene::LineTo{{x0 + r, y1}});
            path.segments.emplace_back(scene::ArcTo{r, r, 0, false, true, {x0, y1 - r}});
            path.segments.emplace_back(scene::LineTo{{x0, y0 + r}});
            path.segments.emplace_back(scene::ArcTo{r, r, 0, false, true, {x0 + r, y0}});
            path.segments.emplace_back(scene::ClosePath{});
        }
        result.push_back(styled(std::move(path), thickness));
        break;
    }
    case PaintRecipe::circle: {
        const double rx = (box.w - thickness) / 2, ry = (box.h + box.d - thickness) / 2;
        const double cx = box.w / 2, cy = (box.h - box.d) / 2;
        scene::PathGeometry path;
        path.segments.emplace_back(scene::MoveTo{{cx + rx, cy}});
        path.segments.emplace_back(scene::ArcTo{rx, ry, 0, false, true, {cx - rx, cy}});
        path.segments.emplace_back(scene::ArcTo{rx, ry, 0, false, true, {cx + rx, cy}});
        path.segments.emplace_back(scene::ClosePath{});
        result.push_back(styled(std::move(path), thickness));
        break;
    }
    case PaintRecipe::phasor_angle: {
        const double a = std::atan2(box.h + box.d, 1.75 * padding);
        const double t = thickness / 2, height = box.h + box.d, c = std::cos(a);
        scene::PathGeometry path;
        move(path, box.w, t - box.d);
        line(path, t + c * t, t - box.d);
        line(path, c * height + t, height - box.d - t);
        auto angle = styled(std::move(path), thickness);
        angle.paint.line_cap = scene::StrokeCap::round;
        angle.paint.line_join = scene::StrokeJoin::miter;
        result.push_back(std::move(angle));
        break;
    }
    case PaintRecipe::arrow: {
        double angle = 0, width = box.w;
        bool both = false;
        std::string_view offset;
        switch (spec.kind) {
        case NotationKind::up_arrow:
            angle = -pi / 2;
            width = box.h + box.d;
            offset = "X";
            break;
        case NotationKind::down_arrow:
            angle = pi / 2;
            width = box.h + box.d;
            offset = "X";
            break;
        case NotationKind::left_arrow:
            angle = pi;
            width = box.w;
            offset = "Y";
            break;
        case NotationKind::right_arrow:
            angle = 0;
            width = box.w;
            offset = "Y";
            break;
        case NotationKind::up_down_arrow:
            angle = pi / 2;
            width = box.h + box.d;
            offset = "X";
            both = true;
            break;
        case NotationKind::left_right_arrow:
            angle = 0;
            width = box.w;
            offset = "Y";
            both = true;
            break;
        default:
            throw std::invalid_argument("unknown linear-arrow notation");
        }
        addArrow(width, angle, both, offset, getOffset(trbl, offset));
        break;
    }
    case PaintRecipe::diagonal_arrow: {
        if (!content)
            throw std::invalid_argument("diagonal arrow geometry requires the actual child BBox");
        const auto [angle, width] = arrowAW(trbl, *content);
        double c = -1, base = 0;
        bool both = false;
        switch (spec.kind) {
        case NotationKind::up_diagonal_arrow:
            c = -1;
            base = 0;
            break;
        case NotationKind::northeast_arrow:
            c = -1;
            base = 0;
            break;
        case NotationKind::southeast_arrow:
            c = 1;
            base = 0;
            break;
        case NotationKind::northwest_arrow:
            c = 1;
            base = pi;
            break;
        case NotationKind::southwest_arrow:
            c = -1;
            base = pi;
            break;
        case NotationKind::northeast_southwest_arrow:
            c = -1;
            base = 0;
            both = true;
            break;
        case NotationKind::northwest_southeast_arrow:
            c = 1;
            base = 0;
            both = true;
            break;
        default:
            throw std::invalid_argument("unknown diagonal-arrow notation");
        }
        addArrow(width, c * (angle - base), both);
        break;
    }
    case PaintRecipe::long_div: {
        const double t = thickness / 2, h = box.h, d = box.d, w = box.w;
        const double rx = padding - t / 2, ry = (h + d) / 2 - 4 * t;
        scene::PathGeometry path;
        move(path, t, t - d);
        path.segments.emplace_back(
            scene::ArcTo{rx, ry, 0, false, true, {t, t - d + (h + d - 2 * t)}});
        line(path, w - t, h - t);
        auto longDivision = styled(std::move(path), thickness);
        longDivision.paint.line_cap = scene::StrokeCap::round;
        longDivision.paint.line_join = scene::StrokeJoin::round;
        result.push_back(std::move(longDivision));
        break;
    }
    case PaintRecipe::radical:
        break; // Child geometry is rendered by the radical projection wrapper.
    default:
        throw std::invalid_argument("notation has no implemented Scene geometry recipe");
    }
    return result;
}

ArrowGeometry arrowGeometry(double width, double angle, bool doubleHeaded, std::string_view offset,
                            double distance, double thickness, Arrowhead head,
                            BBox const &content) {
    const double dw = (width - content.w) / 2;
    const double mid = (content.h - content.d) / 2;
    const double x = thickness * head.x;
    const double y = thickness * head.y;
    const double dx = thickness * head.dx;
    const double t2 = thickness / 2;
    scene::PathGeometry path;
    double px = content.w + dw, py = mid;
    move(path, px, py);
    if (doubleHeaded) {
        relative(path, px, py, -(x + dx), y);
        relative(path, px, py, dx, t2 - y);
        px = x - dw;
        py = mid + t2;
        line(path, px, py);
        relative(path, px, py, dx, y - t2);
        relative(path, px, py, -(x + dx), -y);
        relative(path, px, py, x + dx, -y);
        relative(path, px, py, -dx, y - t2);
        px = content.w + dw - x;
        py = mid - t2;
        line(path, px, py);
        relative(path, px, py, -dx, t2 - y);
    } else {
        relative(path, px, py, -(x + dx), y);
        relative(path, px, py, dx, t2 - y);
        px = -dw;
        py = mid + t2;
        line(path, px, py);
        relative(path, px, py, 0, -thickness);
        px = content.w + dw - x;
        py = mid - t2;
        line(path, px, py);
        relative(path, px, py, -dx, t2 - y);
    }
    path.segments.emplace_back(scene::ClosePath{});

    std::vector<scene::TransformOperation> operations;
    if (distance != 0) {
        if (offset == "X")
            operations.emplace_back(scene::Translation{-distance, 0});
        else if (offset == "Y")
            operations.emplace_back(scene::Translation{0, distance});
    }
    if (angle != 0) {
        operations.emplace_back(scene::Rotation{-angle * 180 / pi, {content.w / 2, mid}});
    }
    return {std::move(path), scene::Transform(std::move(operations))};
}
} // namespace folia::math::output::common
