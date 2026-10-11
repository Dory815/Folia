// Root-owned coercion traversal state. Keep nontrivial TLS outside exported
// inline module functions so importing MSVC translation units share its owner.
module folia.math.util.property_host;
import std;
namespace folia::math::property_host {
std::vector<Property>& activeArrays() {
    static thread_local std::vector<Property> active;
    return active;
}
}
