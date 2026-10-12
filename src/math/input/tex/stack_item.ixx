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
using StackFactoryReference=std::variant<Missing,PropertyNull,StackItemFactoryService*>;
class MmlStack {
    mml::Children nodes_;
protected:
    explicit MmlStack(mml::Children nodes):nodes_(std::move(nodes)){}
    void initializeMmlStack(mml::Children nodes){
        nodes_=std::move(nodes);startStr.clear();startI=stopI=0;
    }
    virtual mml::Children nodes()const{return nodes_;}
    virtual StackFactoryReference factory()const=0;
public:
    std::u16string startStr;
    double startI=0,stopI=0;
    virtual ~MmlStack()=default;
    MmlStack(const MmlStack&)=delete;
    MmlStack& operator=(const MmlStack&)=delete;
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
using StackEntry=std::variant<Property,mml::NodeValue,StackItemRef>;
using CheckList=std::shared_ptr<std::vector<StackEntry>>;
struct CheckTuple {
    std::variant<Missing,PropertyNull,CheckList> items=PropertyNull{};
    Property success=false;
};
using CheckType=std::variant<Missing,PropertyNull,std::shared_ptr<CheckTuple>>;
struct StackItemClassInfo {
    std::function<Property()> errorsBinding;
};
class BaseItem:public MmlStack,public StackItemContext,public tree::FactoryNode {
    StackFactoryReference factory_=Missing{};
    Property env_=Missing{};
    StackProperty properties_=Missing{};
    tree::FactoryClassRef<BaseItem> constructor_;
protected:
    BaseItem():MmlStack({}){}
    StackFactoryReference factory()const override{return factory_;}
    virtual std::shared_ptr<StackItemClassInfo> classInfo()const;
    static std::shared_ptr<CheckTuple> fail,success;
    static Property errors;
public:
    Property global=Missing{};
    // Allocation/adoption precedes this Source super-phase. Subclass Source
    // fields must still be Missing when virtual isOpen is called here.
    void initializeBaseItem(StackFactoryReference,mml::Children,tree::FactoryClassRef<BaseItem>);
    std::string kind()const override;
    virtual Property env()const;
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
