// Root-owned canonical argument and data-record bridge, no source evaluation.
export module folia.math.input.tex.node_factory_host;
export import folia.math.input.tex.node_factory;
export import folia.math.core.mml_tree.mml_node_host;
import std;
export namespace folia::math::tex::node_factory_host {
inline Property scalar(FunctionArgs args,std::size_t index){
    if(index>=args.size())return Missing{};
    if(auto* p=std::any_cast<Property>(&args[index]))return *p;
    mml::host::typeError("Native NodeFactory scalar slot is not a Property");
}
inline mml::Children children(FunctionArgs args,std::size_t index){
    if(index>=args.size())return std::make_shared<tree::ChildArray<mml::MmlNode>>();
    if(auto* p=std::any_cast<mml::Children>(&args[index]))return *p;
    const auto p=scalar(args,index);
    if(p.missing())return std::make_shared<tree::ChildArray<mml::MmlNode>>();
    if(std::holds_alternative<PropertyNull>(p.value))return {};
    mml::host::typeError("Native NodeFactory children slot is not a child array");
}
inline mml::NodeValue node(const std::any& value){
    if(!value.has_value())return Missing{};
    if(auto* p=std::any_cast<mml::NodeValue>(&value))return *p;
    if(auto* p=std::any_cast<mml::MmlNode*>(&value))return *p;
    if(auto* p=std::any_cast<Property>(&value)){
        if(p->missing())return Missing{};
        if(std::holds_alternative<PropertyNull>(p->value))return PropertyNull{};
    }
    mml::host::typeError("Native NodeFactory node slot is not an MML product");
}
inline mml::NodeValue optionalNode(FunctionArgs args,std::size_t index){
    return index<args.size()?node(args[index]):mml::NodeValue(Missing{});
}
inline std::u16string text(const Property& value){
    if(auto* p=std::get_if<std::string>(&value.value))return mml::host::text(*p);
    mml::host::typeError("Native NodeFactory text slot is not a string");
}
inline mml::NodeValue setText(mml::NodeValue node,std::u16string text){
    auto* receiver=node.requireNode("setText");
    if(auto* token=dynamic_cast<mml::TextNode*>(receiver))return token->setText(std::move(text));
    mml::host::typeError("factory.mmlFactory.create(...).setText is not a function");
}
inline std::string kind(const Property& value){return mml::host::debugText(mml::host::string(value));}
// Enabled configuration maps are data records. Enumerate the same shared
// Property graph, including inherited enumerable keys and own shadowing.
inline std::vector<std::string> creatorKeys(const Property& maps){
    if(mml::host::nullish(maps))return {};
    std::vector<std::string> result;std::unordered_set<std::string> visited;
    Property current=maps;
    while(true){
        for(auto& key:mml::host::ownKeys(current))if(visited.insert(key).second)result.push_back(key);
        auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&current.value);
        if(!storage||!*storage||!(*storage)->prototype)break;
        current.value=(*storage)->prototype;
    }
    return result;
}
inline mml::NodeValue invoke(Property creator,const NativeObjectRef& factory,FunctionArgs rest){
    auto* reference=std::get_if<FunctionRef>(&creator.value);
    if(!reference||!*reference||!(*reference)->call)mml::host::typeError("func is not a function");
    const auto body=(*reference)->call;
    std::vector<std::any> args;args.reserve(std::max(rest.size(),std::size_t(1))+1);
    args.emplace_back(factory);
    if(rest.empty())args.emplace_back(Property(Missing{}));
    else for(const auto& value:rest)args.push_back(value);
    return node(body(FunctionReceiver{Missing{}},args));
}
}
