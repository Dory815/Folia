module folia.math.core.mml_tree.mml_factory_context_values;
import std;

namespace folia::math::mml {
std::vector<std::any> mmlFactoryContextArguments(
    ContextPropertyBindings& properties,
    NativeIdentityBindings<Children>& arrays,
    tree::TreeNodeContextBindings<MmlNode>& treeNodes,
    std::span<const options::Value> arguments){
    Property inputProperties(PropertyList{});
    if(!arguments.empty()&&!std::holds_alternative<options::Undefined>(arguments[0].data))
        inputProperties=properties.from(arguments[0]);

    const options::Value childrenInput=arguments.size()>1?arguments[1]:options::Value(options::Undefined{});
    tree::FactoryChildren<MmlNode> children=std::make_shared<tree::ChildArray<MmlNode>>();
    if(!std::holds_alternative<options::Undefined>(childrenInput.data)){
        // Keep the actual input Value until AbstractNode's source-ordered
        // length check and dynamic setChildren call consume it.
        if(const auto* object=std::get_if<options::ObjectRef>(&childrenInput.data)){
            try{children=arrays.resolve(*object);}
            catch(const NativeIdentityResolutionFailure&){children=treeNodes.arrayView(childrenInput);}
        }else children=treeNodes.arrayView(childrenInput);
    }
    // ContextPropertyBindings::from(Context object) returns a Property handle
    // whose shared PropertyStorage is already the canonical live record. The
    // native Tree constructor consumes that same handle as PropertyList; unwrap
    // only its typed record view here, without copying entries or converting
    // non-record values to an empty object.
    std::any nativeProperties;
    if(auto record=PropertyList::record(inputProperties))nativeProperties=std::move(*record);
    else nativeProperties=std::move(inputProperties);
    return {std::move(nativeProperties),std::any(tree::FactoryChildren<MmlNode>(std::move(children)))};
}
}
