// Context reflection for the actual StackItemFactory service.
export module folia.math.input.tex.parser:stack_item_factory_bindings;
export import folia.math.input.tex.stack_item_factory;
export import folia.math.util.option_types;
import std;

export namespace folia::math::tex {
class ParserRuntime;
struct StackItemMethodSurface {
    std::u16string name;
    double length=0;
    bool getter=false;
    std::function<options::Value(BaseItem&)> get;
};
struct StackItemSourceSurface {
    bool ownKind=true;
    // Source's own prototype members after `constructor`, in property order.
    // Methods carry their declared length; getters carry the native field read.
    std::vector<StackItemMethodSurface> members;
    struct InstanceField {
        std::u16string name;
        options::Value initial=options::Undefined{};
        std::function<options::Value(BaseItem&)> get;
        std::function<void(BaseItem&,options::Value)> set;
        // Some Source fields are initialized by an expression that needs the
        // already-initialized BaseItem (for example Stack(this.factory)).
        // Evaluate it at this field's position in Source assignment order.
        std::function<options::Value(BaseItem&)> initialize;
    };
    // Source-owned initialized instance fields, in assignment order.
    std::vector<InstanceField> instanceFields;
};
struct StackItemFactoryBindings {
    static void initializeModule(ParserRuntime&);
    // Bind the actual factory owned by ParseOptions to its canonical Context
    // instance. configuration is the actual ParseOptions Context identity.
    static options::ObjectRef bindActualFactory(
        ParserRuntime&, const std::shared_ptr<StackItemFactory>&,
        options::ObjectRef configuration);
    // Shared Source subclass registration for BaseItems and package-owned
    // stack items. The returned class is the single FactoryClass descriptor
    // used by the real StackItemFactory and Context constructor identity.
    static tree::FactoryClassRef<BaseItem> registerSubclass(
        ParserRuntime&, std::u16string className, std::string kind,
        std::size_t constructorLength, std::type_index nativeType,
        std::function<std::unique_ptr<BaseItem>()> allocate,
        std::function<void(BaseItem&,tree::FactoryArgs)> initializeSource,
        std::function<bool(const BaseItem*)> isInstance,
        std::function<Property()> errorsBinding,
        std::optional<options::ObjectRef> parentConstructor=std::nullopt,
        bool forwardsBaseNodes=false,
        std::optional<StackItemSourceSurface> sourceSurface=std::nullopt,
        bool ownsErrors=false);
    // Materialize the one Source static errors object as a live view of its
    // canonical Context identity, then expose it on the actual constructor.
    static Property bindErrors(ParserRuntime&,options::ObjectRef constructor,Property source);
};
}
