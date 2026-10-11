// Root-frozen native host for pinned TokenMap CJS argument expansion.
export module folia.math.util.argument_expansion;
export import folia.math.core.tree.property;
import std;
export namespace folia::math::arguments {
std::vector<Property> expand(const Property& input);
}
