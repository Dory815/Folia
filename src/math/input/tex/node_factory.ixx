// MathJax 4.1.3 input/tex/NodeFactory.ts; Apache-2.0.
export module folia.math.input.tex.node_factory;
export import folia.math.core.mml_tree.mml_factory;
export import folia.math.util.callable_types;
export import folia.math.util.option_functions;
export import folia.math.util.context_property_bindings;
import std;
export namespace folia::math::tex {
// ParseOptions implements this back-edge. The TeX/session owns both services;
// these borrowed references never own or duplicate ParseOptions state.
class NodeFactoryConfiguration {
public:
    virtual ~NodeFactoryConfiguration()=default;
    virtual void addNode(Property property,mml::NodeValue node)=0;
};
using ContextNodeCreatorInvoker=std::function<mml::NodeValue(options::ObjectRef,FunctionArgs)>;
using NodeFactoryConfigurationReference=std::variant<Missing,PropertyNull,NodeFactoryConfiguration*,options::Value>;
using MmlFactoryReference=std::variant<Missing,PropertyNull,mml::Factory*,std::shared_ptr<mml::MmlFactory>>;
class NodeFactory:public NativeObject,public std::enable_shared_from_this<NodeFactory> {
    Property factory_=PropertyList{};
    std::weak_ptr<std::atomic_bool> executionLifetime_;
    ContextNodeCreatorInvoker contextCreatorInvoker_;
    bool runtimeBound_=false;
    void installCreators(std::span<const std::string>,const std::function<std::optional<Property>(std::string_view)>&);
protected:
    NodeFactory();
    MmlFactoryReference mmlFactory=PropertyNull{};
    mml::Factory& requireMmlFactory() const;
    NodeFactoryConfiguration& requireConfiguration() const;
public:
    NodeFactoryConfigurationReference configuration=Missing{};
    virtual ~NodeFactory()=default;
    NodeFactory(const NodeFactory&)=delete;
    NodeFactory& operator=(const NodeFactory&)=delete;
    static std::shared_ptr<NodeFactory> make();
    static std::array<FunctionRef,4> defaultFunctions();
    NativeObjectRef identity();
    MmlFactoryReference mmlFactoryReference()const{return mmlFactory;}
    Property factoryProperty()const{return factory_;}
    void setFactoryProperty(Property value){factory_=std::move(value);}
    void bindContextExecution(std::weak_ptr<std::atomic_bool>,ContextNodeCreatorInvoker);
    static mml::NodeValue createNode(NodeFactory&,Property kind,
        mml::Children children=std::make_shared<tree::ChildArray<mml::MmlNode>>(),
        Property definition=PropertyList{},mml::NodeValue text=Missing{});
    static mml::NodeValue createToken(NodeFactory&,Property kind,
        Property definition=PropertyList{},Property text="");
    static mml::NodeValue createText(NodeFactory&,Property text=Missing{});
    static mml::NodeValue createError(NodeFactory&,Property message=Missing{});
    virtual void setMmlFactory(MmlFactoryReference);
    virtual void set(std::string_view kind,Property creator);
    virtual void setCreators(Property maps);
    virtual void setCreators(options::Context&,ContextPropertyBindings&,options::Value maps);
    virtual mml::NodeValue create(std::string_view kind,FunctionArgs rest={});
    virtual Property get(std::string_view kind) const;
};
}
