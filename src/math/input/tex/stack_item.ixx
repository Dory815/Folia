// MathJax 4.1.3 input/tex/StackItem.ts, MmlStack. Apache-2.0.
export module folia.math.input.tex.stack_item;
export import folia.math.input.tex.stack_services;
export import folia.math.input.tex.stack_property;
export import folia.math.input.tex.types;
import std;
export namespace folia::math::tex {
class BaseItem;
// The view is a base of the actual factory, with one generic registry/owner.
// It does not inherit FactoryServices again or allocate a facade object.
class StackItemFactoryService {
public:
    virtual ~StackItemFactoryService()=default;
    StackConfigurationReference configuration=PropertyNull{};
    virtual tree::FactoryServices<BaseItem>& itemFactory()=0;
    virtual StackItemRef reference(BaseItem*)const=0;
};
using StackFactoryReference=std::variant<Missing,PropertyNull,StackItemFactoryService*,options::Value>;
class StackItemSourceAccess {
public:
    virtual ~StackItemSourceAccess()=default;
    virtual options::Value receiver()const=0;
    virtual options::Value getReceiver(std::u16string_view)=0;
    virtual Property environment()=0;
    virtual void setEnvironment(Property)=0;
    virtual options::Value get(const options::Value&,const options::Key&)=0;
    virtual options::Value getIndex(const options::Value&,double)=0;
    virtual void setReceiver(std::u16string_view,options::Value)=0;
    virtual void set(const options::Value&,const options::Key&,options::Value)=0;
    virtual void setIndex(const options::Value&,double,options::Value)=0;
    virtual options::Value callMember(const options::Value&,options::Value,std::span<const options::Value>)=0;
    virtual options::Value callApply(const options::Value&,options::Value,std::span<const options::Value>)=0;
    virtual options::Value readIterable(const options::Value&,std::uint32_t)=0;
    virtual options::ObjectRef makeArray()=0;
    virtual options::ObjectRef makeObject()=0;
    virtual double toNumber(const options::Value&)=0;
    virtual std::string sourceString(const options::Value&)=0;
    virtual bool truthy(const options::Value&)=0;
    virtual mml::Children resolveNodeArray(const options::Value&)=0;
    virtual mml::NodeValue decodeNode(const options::Value&)=0;
    virtual options::Value encodeNode(mml::NodeValue)=0;
    virtual options::Value encodeChildren(mml::Children)=0;
    virtual StackProperty decodeProperty(const options::Value&)=0;
    virtual options::Value encodeProperty(StackProperty)=0;
    virtual std::vector<options::Key> ownKeys(const options::Value&)=0;
    virtual std::optional<options::Descriptor> descriptor(const options::Value&,const options::Key&)=0;
};
class MmlStack {
    mml::Children nodes_;
    std::shared_ptr<StackItemSourceAccess> sourceAccess_;
protected:
    explicit MmlStack(mml::Children nodes):nodes_(std::move(nodes)){}
    void initializeMmlStack(mml::Children nodes){nodes_=std::move(nodes);sourceAccess_.reset();startStr.clear();startI=stopI=0;}
    void initializeMmlStackNodes(mml::Children nodes){nodes_=std::move(nodes);sourceAccess_.reset();}
    virtual mml::Children nodes()const{return sourceAccess_?sourceAccess_->resolveNodeArray(sourceAccess_->getReceiver(u"_nodes")):nodes_;}
    virtual StackFactoryReference factory()const=0;
public:
    std::u16string startStr;
    double startI=0,stopI=0;
    virtual ~MmlStack()=default;
    MmlStack(const MmlStack&)=delete;
    MmlStack& operator=(const MmlStack&)=delete;
    void setSourceAccess(std::shared_ptr<StackItemSourceAccess> access){sourceAccess_=std::move(access);}
    StackItemSourceAccess& sourceAccess()const{
        if(!sourceAccess_)throw options::Exception("TypeError",u"StackItem has no active Source access");
        return *sourceAccess_;
    }
    bool hasSourceNodesOwner()const noexcept{return static_cast<bool>(sourceAccess_);}
    mml::Children sourceNodes()const{return nodes();}
    options::Value sourceNodesCurrentValue()const{return sourceAccess_?sourceAccess_->getReceiver(u"_nodes"):options::Value(options::Undefined{});}
    options::Value sourceNodesPropertyValue()const{return sourceAccess().getReceiver(u"nodes");}
    options::Value sourceSizeValue()const{auto& access=sourceAccess();const auto array=sourceNodesCurrentValue();return access.get(array,options::Key(u"length"));}
    options::Value sourcePeekRaw()const{auto& access=sourceAccess();const auto receiver=access.receiver();const auto method=access.getReceiver(u"Peek");return access.callMember(method,receiver,{});}
    options::Value sourcePeekRaw(options::Value count)const{auto& access=sourceAccess();const auto receiver=access.receiver();const auto method=access.getReceiver(u"Peek");const std::array<options::Value,1> args{std::move(count)};return access.callMember(method,receiver,args);}
    options::Value sourcePeekValue(std::optional<options::Value> count={})const;
    options::Value sourcePopValue()const;
    void sourcePushValues(std::span<const options::Value>)const;
    options::Value sourceFirstValue()const;
    void sourceSetFirstValue(options::Value)const;
    options::Value sourceLastValue()const;
    void sourceSetLastValue(options::Value)const;
    options::Value sourceToMmlValue(options::Value inferred,options::Value forceRow)const;
    options::Value sourceCreateValues(std::span<const options::Value>)const;
    options::Value sourceReadPeek(options::Value count,std::uint32_t limit)const{auto& access=sourceAccess();const auto raw=sourcePeekRaw(std::move(count));return access.readIterable(raw,limit);}
    mml::NodeValue sourceNodeFromSlice(const options::Value& values,double index)const{return sourceAccess().decodeNode(sourceAccess().getIndex(values,index));}
    mml::NodeValue sourceDecodeNode(const options::Value& value)const{return sourceAccess().decodeNode(value);}
    virtual void Push(std::span<const mml::NodeValue> nodes);
    virtual mml::NodeValue Pop();
    virtual mml::NodeValue First()const;
    virtual void setFirst(mml::NodeValue);
    virtual mml::NodeValue Last()const;
    virtual void setLast(mml::NodeValue);
    virtual mml::Children Peek(Property count=Missing{})const;
    virtual double Size()const;
    virtual void Clear();
    virtual mml::NodeValue toMml(Property inferred=Missing{},Property forceRow=Missing{});
    virtual mml::NodeValue create(std::string_view kind,FunctionArgs rest={});
};
// Context-backed stacks keep arbitrary Source values raw until a later member
// operation. Native parser callers continue to use the existing alternatives.
using StackEntry=std::variant<Property,mml::NodeValue,StackItemRef,options::Value>;
using CheckList=std::shared_ptr<std::vector<StackEntry>>;
struct CheckTuple {
    std::variant<Missing,PropertyNull,CheckList> items=PropertyNull{};
    Property success=false;
};
using CheckType=std::variant<Missing,PropertyNull,std::shared_ptr<CheckTuple>>;
struct StackItemClassInfo {
    std::function<Property()> errorsBinding;
    std::function<Property()> baseErrorsBinding;
};
class BaseItem:public MmlStack,public StackItemContext,public tree::FactoryNode {
    StackFactoryReference factory_=Missing{};
    Property env_=Missing{};
    StackProperty properties_=Missing{};
    tree::FactoryClassRef<BaseItem> constructor_;
protected:
    StackFactoryReference factory()const override{return factory_;}
    virtual std::shared_ptr<StackItemClassInfo> classInfo()const;
    static std::shared_ptr<CheckTuple> fail,success;
    static Property errors;
public:
    // Source's BaseItem constructor is public in the emitted runtime.  Native
    // allocation adopts this object before initializeBaseItem runs its Source
    // super-phase.
    BaseItem():MmlStack({}){}
    Property global=Missing{};
    // Allocation/adoption precedes this Source super-phase. Subclass Source
    // fields must still be Missing when virtual isOpen is called here.
    void initializeBaseItem(StackFactoryReference,mml::Children,tree::FactoryClassRef<BaseItem>);
    void prepareSourceBaseItem(tree::FactoryClassRef<BaseItem>,mml::Children);
    void setSourceProperties(StackProperty value){properties_=std::move(value);}
    StackFactoryReference sourceFactory()const{return factory_;}
    void setSourceFactory(StackFactoryReference value){factory_=std::move(value);}
    std::string kind()const override;
    virtual Property env()const;
    Property envBacking()const{return env_;}
    void setEnvFromSource(Property value){env_=std::move(value);}
    virtual void setEnv(Property);
    virtual bool copyEnv()const;
    virtual StackProperty getProperty(std::string_view)const;
    virtual BaseItem* setProperty(std::string_view,StackProperty);
    virtual BaseItem* setProperties(StackProperty);
    virtual bool isOpen()const;
    virtual bool isClose()const;
    virtual bool isFinal()const;
    virtual bool isKind(std::string_view)const;
    virtual CheckType checkItem(BaseItem*);
    virtual void clearEnv();
    virtual StackProperty getName()const;
    virtual std::string toString()const;
    virtual Property getErrors(std::string_view)const;
    virtual void addLatexItem(mml::NodeValue,Property prefix=Missing{});
    static Property baseErrors();
};
}
