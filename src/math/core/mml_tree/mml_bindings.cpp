module folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.mml_node_host;
import std;
namespace folia::math::mml {
namespace {
struct DefaultsState {
    PropertyList normal{{"mathbackground",std::string(INHERIT)},{"mathcolor",std::string(INHERIT)},{"mathsize",std::string(INHERIT)},{"dir",std::string(INHERIT)}};
    PropertyList token,layout=normal,base=normal;
    DefaultsState(){token.assign(normal);token.set("mathvariant","normal");token.set("mathsize",std::string(INHERIT));}
};
DefaultsState& defaultState(){static DefaultsState state;return state;}
}
PropertyList& AbstractMmlNode::defaults(){return defaultState().normal;}
PropertyList& AbstractMmlTokenNode::defaults(){return defaultState().token;}
PropertyList& AbstractMmlLayoutNode::defaults(){return defaultState().layout;}
PropertyList& AbstractMmlBaseNode::defaults(){return defaultState().base;}
Property attributeTuple(std::string origin,Property value){return PropertyArray{std::move(origin),std::move(value)};}
std::pair<std::string,Property> unpackAttribute(const Property& p){
    auto tuple=PropertyArray::array(p);
    if(!tuple)host::typeError("Inherited attribute has no native tuple binding");
    auto origin=tuple->get(0);auto str=std::get_if<std::string>(&origin.value);
    if(!str)host::typeError("Inherited attribute origin is not a string");
    return {*str,tuple->get(1)};
}
}
