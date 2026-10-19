module folia.math.core.math_item;
import folia.math.util.context_property_bindings;
import std;
namespace folia::math::core {
PropertyList STATE{{"UNPROCESSED",0},{"FINDMATH",10},{"COMPILED",20},{"CONVERT",100},
    {"METRICS",110},{"RERENDER",125},{"TYPESET",150},{"INSERTED",200},{"LAST",10000}};

options::Value sourceOutputValue(const MathOutput& output,ContextPropertyBindings& properties){
    if(std::holds_alternative<Missing>(output))return options::Undefined{};
    if(std::holds_alternative<PropertyNull>(output))return options::Null{};
    if(const auto* node=std::get_if<DOMNodeRef>(&output))
        return properties.to(nodeProperty(*node));
    throw options::Exception("Unimplemented",
        u"Native output owner has no canonical Context identity binding");
}
}
