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
            out[i]->call=[kind=static_cast<Creator>(i)](const FunctionReceiver&,FunctionArgs args)->std::any{
                if(args.empty())mml::host::typeError("Native NodeFactory callback has no factory");
                const auto* identity=std::any_cast<NativeObjectRef>(&args[0]);
                const auto factory=identity?std::dynamic_pointer_cast<NodeFactory>(*identity):nullptr;
                if(!factory)mml::host::typeError("Native NodeFactory callback factory is invalid");
                const auto rest=args.subspan(1);namespace h=node_factory_host;
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
NodeFactory::NodeFactory():factory_{{"node",binding(Creator::Node)},{"token",binding(Creator::Token)},
    {"text",binding(Creator::Text)},{"error",binding(Creator::Error)}}{}
std::shared_ptr<NodeFactory> NodeFactory::make(){return std::shared_ptr<NodeFactory>(new NodeFactory);}
NativeObjectRef NodeFactory::identity(){return std::static_pointer_cast<NativeObject>(shared_from_this());}
mml::Factory& NodeFactory::requireMmlFactory()const{return required(mmlFactory,"create");}
NodeFactoryConfiguration& NodeFactory::requireConfiguration()const{return required(configuration,"addNode");}
}
