// MML-specific initialization and access to the one generic Property host.
export module folia.math.core.mml_tree.mml_node_host;
export import folia.math.util.property_host;
import std;
export namespace folia::math::mml::host {
using namespace folia::math::property_host;
inline void inheritanceDefaults(Property& attributes,Property& display,Property& level,Property& prime){
    if(attributes.missing())attributes=PropertyList{};
    if(display.missing())display=false;
    if(level.missing())level=0;
    if(prime.missing())prime=false;
}
}
