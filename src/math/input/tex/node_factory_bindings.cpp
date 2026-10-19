// Root allocation/identity/default-callable bindings for NodeFactory.ts.
module folia.math.input.tex.node_factory;
import folia.math.input.tex.node_factory_host;
import std;
namespace folia::math::tex {
namespace {
enum class Creator {Node,Token,Text,Error};
FunctionRef binding(Creator kind){
    static const auto functions=[] {
        std::array<FunctionRef,4> out;
        for(std::size_t i=0;i<out.size();++i){
            out[i]=std::make_shared<Function>();
            out[i]->call=[kind=static_cast<Creator>(i)](const FunctionReceiver& receiver,FunctionArgs args)->std::any{
                const auto* identity=std::get_if<NativeObjectRef>(&receiver);
                const bool receiverIsFactory=identity!=nullptr;
                if(!identity&&!args.empty())identity=std::any_cast<NativeObjectRef>(&args[0]);
                const auto factory=identity?std::dynamic_pointer_cast<NodeFactory>(*identity):nullptr;
                if(!factory)mml::host::typeError("Native NodeFactory callback factory is invalid");
                const auto rest=receiverIsFactory?args:args.subspan(std::min<std::size_t>(1,args.size()));namespace h=node_factory_host;
                switch(kind){
                case Creator::Node:return NodeFactory::createNode(*factory,h::scalar(rest,0),h::children(rest,1),h::scalar(rest,2),h::optionalNode(rest,3));
                case Creator::Token:return NodeFactory::createToken(*factory,h::scalar(rest,0),h::scalar(rest,1),h::scalar(rest,2));
                case Creator::Text:return NodeFactory::createText(*factory,h::scalar(rest,0));
                case Creator::Error:return NodeFactory::createError(*factory,h::scalar(rest,0));
                }
                std::unreachable();
            };
        }
        return out;
    }();
    return functions[static_cast<std::size_t>(kind)];
}
template<class T> T& required(const std::variant<Missing,PropertyNull,T*>& ref,std::string_view member){
    if(auto* p=std::get_if<T*>(&ref);p&&*p)return **p;
    mml::host::typeError(std::string("Cannot read properties of ")+
        (std::holds_alternative<Missing>(ref)?"undefined":"null")+" (reading '"+std::string(member)+"')");
}
}
std::array<FunctionRef,4> NodeFactory::defaultFunctions(){
    // These bodies capture only their enum kind. Each ParserRuntime binds them
    // to its own Context function object and property descriptors.
    static const std::array<FunctionRef,4> defaults{
        binding(Creator::Node),binding(Creator::Token),binding(Creator::Text),binding(Creator::Error)};
    return defaults;
}
NodeFactory::NodeFactory():factory_([&]{
    const auto defaults=defaultFunctions();
    return PropertyList{{"node",defaults[0]},{"token",defaults[1]},
        {"text",defaults[2]},{"error",defaults[3]}};
}()){}
std::shared_ptr<NodeFactory> NodeFactory::make(){return std::shared_ptr<NodeFactory>(new NodeFactory);}
NativeObjectRef NodeFactory::identity(){return std::static_pointer_cast<NativeObject>(shared_from_this());}
mml::Factory& NodeFactory::requireMmlFactory()const{
    if(const auto* borrowed=std::get_if<mml::Factory*>(&mmlFactory);borrowed&&*borrowed)return **borrowed;
    if(const auto* owned=std::get_if<std::shared_ptr<mml::MmlFactory>>(&mmlFactory);owned&&*owned)return **owned;
    mml::host::typeError(std::string("Cannot read properties of ")+
        (std::holds_alternative<Missing>(mmlFactory)?"undefined":"null")+" (reading 'create')");
}
NodeFactoryConfiguration& NodeFactory::requireConfiguration()const{
    if(const auto* owner=std::get_if<NodeFactoryConfiguration*>(&configuration);owner&&*owner)return **owner;
    mml::host::typeError(std::string("Cannot read properties of ")+
        (std::holds_alternative<Missing>(configuration)?"undefined":
         std::holds_alternative<PropertyNull>(configuration)?"null":"object")+" (reading 'addNode')");
}
}
