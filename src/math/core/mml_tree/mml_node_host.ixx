// MML-specific initialization and access to the one generic Property host.
export module folia.math.core.mml_tree.mml_node_host;
export import folia.math.util.property_host;
export import folia.math.core.tree.child_array;
import std;
export namespace folia::math::mml::host {
using namespace folia::math::property_host;
// Source Math.min/max binary operations: propagate NaN and retain signed zero.
inline double mathMin(double left, double right) {
    if (std::isnan(left) || std::isnan(right)) return std::numeric_limits<double>::quiet_NaN();
    if (left == 0 && right == 0) return std::signbit(left) || std::signbit(right) ? -0.0 : 0.0;
    return left < right ? left : right;
}
inline double mathMax(double left, double right) {
    if (std::isnan(left) || std::isnan(right)) return std::numeric_limits<double>::quiet_NaN();
    if (left == 0 && right == 0) return std::signbit(left) && std::signbit(right) ? -0.0 : 0.0;
    return left > right ? left : right;
}
// Tagged node arrays expose indexed slots, including prototype numeric slots.
// Non-index keys do not become truncated native indices.
template<class Product>
tree::ChildValue<Product> numericChild(const std::shared_ptr<tree::ChildArray<Product>>& nodes,
                                       double index) {
    if (!nodes) typeError("Cannot read properties of null (reading '" +
                         debugText(string(Property(index))) + "')");
    if (!std::isfinite(index) || index < 0 || index >= 4294967295.0 ||
        std::trunc(index) != index) return Missing{};
    return nodes->get(static_cast<std::size_t>(index));
}
inline void inheritanceDefaults(Property& attributes,Property& display,Property& level,Property& prime){
    if(attributes.missing())attributes=PropertyList{};
    if(display.missing())display=false;
    if(level.missing())level=0;
    if(prime.missing())prime=false;
}
}
