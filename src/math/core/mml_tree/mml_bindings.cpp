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
std::pair<Property,Property> unpackAttribute(const Property& p){
    if(auto tuple=PropertyArray::array(p))return {tuple->get(0),tuple->get(1)};
    if(auto str=std::get_if<std::string>(&p.value)){
        auto units=host::text(*str);std::array<Property,2> values;std::size_t i=0;
        for(std::size_t offset=0;offset<units.size()&&i<2;++i){
            const auto start=offset++;
            if(units[start]>=0xD800&&units[start]<=0xDBFF&&offset<units.size()&&units[offset]>=0xDC00&&units[offset]<=0xDFFF)++offset;
            values[i]=host::debugText(std::u16string_view(units).substr(start,offset-start));
        }
        return {values[0],values[1]};
    }
    // The pinned CJS __read helper returns receivers without an iterator as-is;
    // the emitted destructuring then reads their indexed properties. Nullish
    // receivers still throw during the helper/property access.
    return {host::get(p,"0"),host::get(p,"1")};
}
}
