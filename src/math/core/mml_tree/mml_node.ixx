/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Root-frozen declarations for core/MmlTree/MmlNode.ts (4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.core.mml_tree.mml_node;
export import folia.math.core.tree.node;
export import folia.math.core.mml_tree.mml_node_algorithms;
export import folia.math.core.mml_tree.attributes;
import std;

export namespace folia::math::mml {
using AttributeList = Property;
class MmlNode;
using Children = tree::FactoryChildren<MmlNode>;
using NodeValue = tree::ChildValue<MmlNode>;
using Factory = tree::FactoryServices<MmlNode>;
using Error = tree::FactoryError;
using NativeMmlValue = MmlNativeDynamic<MmlNode>;
struct MmlSourceArgumentValues {
    std::function<options::Value(Factory*)> factory;
    std::function<options::Value(Property)> attributes;
    std::function<options::Value(Children)> children;
};
struct MmlClassInfo {
    // Pinned MML.js constructor identity metadata. These strings are supplied
    // by the ordered Source export table, never inferred from RTTI.
    std::u16string sourceConstructorName;
    std::u16string sourceKind;
    // Exact pinned Source prototype base: an abstract MML base or an exported class.
    std::u16string sourcePrototypeParent;
    std::uint8_t sourceConstructorArity=0;
    // Constant Source prototype getter overrides that are needed before an
    // ordinary `.call` receiver has acquired its native payload.
    std::optional<double> sourceArity;
    std::optional<bool> sourceIsInferred;
    // Source constructor-object own static fields. These flags describe the
    // pinned class declaration, not the presence of inherited C++ helpers.
    bool sourceOwnDefaults=false;
    bool sourceOwnAccents=false;
    std::optional<PropertyList> defaults;
    std::function<std::optional<PropertyList>()> defaultsBinding;
    std::optional<PropertyList> getDefaults()const{return defaultsBinding?defaultsBinding():defaults;}
    // Captured constructor static binding; Munder inherits the parent's array,
    // Mover owns another one. Replacement is live, captured handles stay live.
    std::function<std::shared_ptr<std::vector<Property>>()> accentsBinding;
    // Context-bound class objects provide the same live JavaScript array to
    // typed consumers through PropertyArray's canonical PropertyStorage view.
    std::function<PropertyArray()> sourceAccentsPropertyBinding;
    // Family-owned typed static services of the captured actual constructor.
    std::any staticServices;
};
struct TeXClasses { static constexpr int ORD=0,OP=1,BIN=2,REL=3,OPEN=4,CLOSE=5,PUNCT=6,INNER=7,NONE=-1; };
inline constexpr TeXClasses TEXCLASS;
inline constexpr std::array<std::string_view,8> TEXCLASSNAMES={"ORD","OP","BIN","REL","OPEN","CLOSE","PUNCT","INNER"};
extern std::vector<Property> MATHVARIANTS;
extern std::vector<std::string> indentAttributes;
// Native two-slot array tuples preserve the same scalar/object/array Property graph.
Property attributeTuple(std::string origin,Property value);
std::pair<Property,Property> unpackAttribute(const Property& value);

class MmlNode : public tree::Node<MmlNode> {
    tree::FactoryClassRef<MmlNode> nativeConstructorClass_;
    std::function<void()> sourceInitializationComplete_;
    std::function<MmlNodeReceiver<MmlNode,options::Value>()> sourceOperationView_;
    std::function<MmlNode*(options::Value)> sourceNodeProjection_;
    std::optional<MmlSourceArgumentValues> sourceArgumentValues_;
    tree::TreeNodeAccessRef<MmlNode> sourceTreeAccess_;
    std::optional<options::ObjectRef> sourceIdentity_;
public:
    using tree::Node<MmlNode>::Node;
    // Internal identity of the constructor that actually allocated this node.
    // Its lifetime is independent of later factory registration changes.
    void bindNativeConstructorClass(tree::FactoryClassRef<MmlNode> actual) {
        if (!actual) throw Error("TypeError", u"MML node requires its actual constructor class");
        if (nativeConstructorClass_ && nativeConstructorClass_ != actual)
            throw Error("TypeError", u"MML node is already bound to another constructor class");
        nativeConstructorClass_ = std::move(actual);
    }
    const tree::FactoryClassRef<MmlNode>& nativeConstructorClass() const {
        return nativeConstructorClass_;
    }
    virtual bool isToken() const=0;
    void sourceInitializationComplete(std::function<void()> value){sourceInitializationComplete_=std::move(value);}
    void completeSourceInitialization(){if(sourceInitializationComplete_){sourceInitializationComplete_();sourceInitializationComplete_={};}}
    void sourceOperationView(std::function<MmlNodeReceiver<MmlNode,options::Value>()> value,
                             std::function<MmlNode*(options::Value)> project){
        sourceOperationView_=std::move(value);sourceNodeProjection_=std::move(project);
    }
    const auto& sourceOperationView()const noexcept{return sourceOperationView_;}
    void sourceArgumentValues(MmlSourceArgumentValues value){sourceArgumentValues_=std::move(value);}
    const auto& sourceArgumentValues()const noexcept{return sourceArgumentValues_;}
    void sourceTreeAccess(tree::TreeNodeAccessRef<MmlNode> value){sourceTreeAccess_=std::move(value);}
    const tree::TreeNodeAccessRef<MmlNode>& sourceTreeAccess()const noexcept{return sourceTreeAccess_;}
    MmlNode* projectSourceNode(options::Value value)const{
        if(!sourceNodeProjection_)throw Error("TypeError",u"MML Context node has no native result projection");
        return sourceNodeProjection_(std::move(value));
    }
    void sourceIdentity(options::ObjectRef value){sourceIdentity_=value;}
    options::Value sourceValue()const override{
        if(!sourceIdentity_)return tree::Node<MmlNode>::sourceValue();
        return options::Value(*sourceIdentity_);
    }
    virtual bool isEmbellished() const=0;
    virtual bool isSpacelike() const=0;
    virtual bool linebreakContainer() const=0;
    virtual std::string linebreakAlign() const=0;
    virtual bool isEmpty() const=0;
    virtual double arity() const=0;
    virtual bool isInferred() const=0;
    virtual Property notParent() const=0;
    virtual NodeValue Parent() const=0;
    virtual Property texClass()=0;
    virtual void setTexClass(Property)=0;
    virtual Property prevClass()=0;
    virtual void setPrevClass(Property)=0;
    virtual Property prevLevel() const=0;
    virtual void setPrevLevel(Property)=0;
    virtual std::shared_ptr<Attributes> attributes() const=0;
    virtual void setAttributes(std::shared_ptr<Attributes>)=0;
    virtual NodeValue core()=0;
    virtual NodeValue coreMO()=0;
    virtual std::size_t coreIndex()=0;
    virtual std::optional<std::size_t> childPosition()=0;
    virtual NodeValue setTeXclass(NodeValue)=0;
    virtual Property texSpacing()=0;
    virtual bool hasSpacingAttributes()=0;
    void setInheritedAttributes(AttributeList attributes=Missing{},Property display=Missing{},Property level=Missing{},Property prime=Missing{}) { inheritAttributes(std::move(attributes),std::move(display),std::move(level),std::move(prime)); }
    virtual void inheritAttributesFrom(NodeValue)=0;
    virtual void verifyTree(Property options=Missing{})=0;
    virtual MmlNode* mError(std::string message,Property options,bool shortError=false)=0;
protected:
    virtual void inheritAttributes(AttributeList,Property,Property,Property)=0;
};
// Optional native method binding models 'textContent' in node before invocation.
// An engaged empty callback represents a present non-callable property.
class TextContentBinding {
public:
    std::optional<std::function<std::u16string()>> textContent;
    virtual ~TextContentBinding()=default;
};
class AbstractMmlNode : public tree::AbstractNode<MmlNode,MmlNode> {
    using Generic=tree::AbstractNode<MmlNode,MmlNode>;
    Property texclass_=Missing{},prevClass_=Missing{};
    Property prevLevel_=Missing{};
    std::shared_ptr<Attributes> attributes_;
    std::optional<MmlSourceValue<MmlNode>> sourceAttributesRaw_;
    std::function<std::shared_ptr<Attributes>(std::shared_ptr<Attributes>)> sourceAttributeAssignment_;
    std::function<std::shared_ptr<Attributes>()> sourceAttributeReader_;
    std::shared_ptr<MmlClassInfo> constructorInfo_;
public:
    static PropertyList& defaults();
    static PropertyList& noInherit();
    static PropertyList& stopInherit();
    static PropertyList& alwaysInherit();
    static PropertyList& verifyDefaults();
    explicit AbstractMmlNode(Factory& factory):Generic(factory){}
    void initializeMml(tree::FactoryArgs args={},
        std::function<void(Children)> setChildren={},Factory* constructorFactory=nullptr);
    MmlNodeReceiver<MmlNode,NativeMmlValue> nativeOperationView();
    // Native class binding preserves this.constructor independently of registry.
    void bindConstructorClass(std::shared_ptr<MmlClassInfo> info){constructorInfo_=std::move(info);}
    virtual void initializeClassFields(){}
    MmlNode* copy(bool keepIds=false) override;
    Property texClass()override;
    void setTexClass(Property)override;
    // Raw own fields used by the pinned Source object projection.
    Property sourceTexclassField()const{return texclass_;}
    void setSourceTexclassField(Property value){texclass_=std::move(value);}
    Property sourcePrevClassField()const{return prevClass_;}
    void setSourcePrevClassField(Property value){prevClass_=std::move(value);}
    Property sourcePrevLevelField()const{return prevLevel_;}
    void setSourcePrevLevelField(Property value){prevLevel_=std::move(value);}
    Property prevClass()override;
    void setPrevClass(Property)override;
    Property prevLevel()const override;
    void setPrevLevel(Property)override;
    std::shared_ptr<Attributes> attributes()const override;
    void sourceAttributeAssignment(std::function<std::shared_ptr<Attributes>(std::shared_ptr<Attributes>)> value){sourceAttributeAssignment_=std::move(value);}
    void sourceAttributeReader(std::function<std::shared_ptr<Attributes>()> value){sourceAttributeReader_=std::move(value);}
    void setAttributes(std::shared_ptr<Attributes>)override;
    std::shared_ptr<Attributes> sourceAttributesField()const{return attributes_;}
    void setSourceAttributesField(std::shared_ptr<Attributes> value){sourceAttributesRaw_.reset();attributes_=std::move(value);}
    std::optional<MmlSourceValue<MmlNode>> sourceAttributesRawField()const{return sourceAttributesRaw_;}
    void setSourceAttributesRawField(options::Value value,tree::TreeNodeAccessRef<MmlNode> provider={}){
        sourceAttributesRaw_=MmlSourceValue<MmlNode>{std::move(value),std::move(provider)};
    }
    bool isToken()const override;
    bool isEmbellished()const override;
    bool isSpacelike()const override;
    bool linebreakContainer()const override;
    std::string linebreakAlign()const override;
    bool isEmpty()const override;
    double arity()const override;
    bool isInferred()const override;
    Property notParent()const override;
    NodeValue Parent()const override;
    void setChildren(Children)override;
    NodeValue appendChild(NodeValue)override;
    NodeValue replaceChild(NodeValue,NodeValue)override;
    NodeValue core()override;
    NodeValue coreMO()override;
    std::size_t coreIndex()override;
    std::optional<std::size_t> childPosition()override;
    NodeValue setTeXclass(NodeValue)override;
    Property texSpacing()override;
    bool hasSpacingAttributes()override;
    protected: void inheritAttributes(AttributeList,Property display,Property level,Property prime)override; public:
    void inheritAttributesFrom(NodeValue)override;
    void verifyTree(Property options=Missing{})override;
    MmlNode* mError(std::string message,Property options,bool shortError=false)override;
protected:
    const std::any& constructorStaticServices()const {
        if(!constructorInfo_)throw Error("TypeError",u"Missing constructor static services");
        return constructorInfo_->staticServices;
    }
    PropertyList constructorDefaults()const;
    std::function<Property(std::size_t)> constructorAccentReader()const {
        if(constructorInfo_&&constructorInfo_->sourceAccentsPropertyBinding){
            const PropertyArray values=constructorInfo_->sourceAccentsPropertyBinding();
            return [values](std::size_t index){return values.get(index);};
        }
        if(!constructorInfo_||!constructorInfo_->accentsBinding)
            throw Error("TypeError",u"Constructor has no ACCENTS binding");
        const auto values=constructorInfo_->accentsBinding();
        if(!values)throw Error("TypeError",u"Constructor ACCENTS binding is null");
        return [values](std::size_t index){return index<values->size()?(*values)[index]:Property(Missing{});};
    }
    void initializeTexClass(Property value){texclass_=std::move(value);}
    virtual void updateTeXclass(NodeValue);
    virtual void getPrevClass(NodeValue);
    virtual void setChildInheritedAttributes(AttributeList,Property,Property,Property);
    virtual AttributeList addInheritedAttributes(AttributeList,Property);
    virtual void verifyAttributes(Property);
    virtual void verifyChildren(Property);
};
class AbstractMmlTokenNode : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    bool isToken()const override;
    bool isEmpty()const override;
    virtual std::u16string getText()const;
    std::any walkTree(const tree::TreeVisitor<MmlNode>&,const std::any& data={})override;
    std::any walkTreeRaw(const tree::TreeRawVisitor<MmlNode>&,const std::any& data={},
                         const std::any& visitorIdentity={})override;
protected:
    void setChildInheritedAttributes(AttributeList,Property,Property,Property)override;
};
class AbstractMmlLayoutNode : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    bool isSpacelike()const override;
    bool isEmbellished()const override;
    double arity()const override;
    NodeValue core()override;
    NodeValue coreMO()override;
    NodeValue setTeXclass(NodeValue)override;
};
class AbstractMmlBaseNode : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    bool isEmbellished()const override;
    NodeValue core()override;
    NodeValue coreMO()override;
    NodeValue setTeXclass(NodeValue)override;
};
class AbstractMmlEmptyNode : public tree::AbstractEmptyNode<MmlNode,MmlNode> {
public:
    using tree::AbstractEmptyNode<MmlNode,MmlNode>::AbstractEmptyNode;
    bool isToken()const override;
    bool isEmbellished()const override;
    bool isSpacelike()const override;
    bool linebreakContainer()const override;
    std::string linebreakAlign()const override;
    bool isEmpty()const override;
    double arity()const override;
    bool isInferred()const override;
    Property notParent()const override;
    NodeValue Parent()const override;
    Property texClass()override;
    void setTexClass(Property)override;
    Property prevClass()override;
    void setPrevClass(Property)override;
    Property prevLevel()const override;
    void setPrevLevel(Property)override;
    std::shared_ptr<Attributes> attributes()const override;
    void setAttributes(std::shared_ptr<Attributes>)override;
    NodeValue core()override;
    NodeValue coreMO()override;
    std::size_t coreIndex()override;
    std::optional<std::size_t> childPosition()override;
    NodeValue setTeXclass(NodeValue)override;
    Property texSpacing()override;
    bool hasSpacingAttributes()override;
    protected: void inheritAttributes(AttributeList,Property display,Property level,Property prime)override; public:
    void inheritAttributesFrom(NodeValue)override;
    void verifyTree(Property options=Missing{})override;
    MmlNode* mError(std::string,Property,bool shortError=false)override;
};
class TextNode : public AbstractMmlEmptyNode {
    std::u16string text_;
    std::function<void(const std::u16string&)> sourceTextAssignment_;
    std::function<std::u16string()> sourceTextReader_;
public:
    using AbstractMmlEmptyNode::AbstractMmlEmptyNode;
    void initializeSourceFields(){(void)TextNode::setText({});}
    std::string kind()const override;
    virtual std::u16string getText()const;
    void sourceTextAccessors(std::function<void(const std::u16string&)> assign,
                             std::function<std::u16string()> read){
        sourceTextAssignment_=std::move(assign);sourceTextReader_=std::move(read);
    }
    void sourceTextReader(std::function<std::u16string()> read){sourceTextReader_=std::move(read);}
    const std::u16string& sourceTextField()const{return text_;}
    void setSourceTextField(std::u16string value){text_=std::move(value);}
    virtual TextNode* setText(std::u16string);
    MmlNode* copy(bool keepIds=false)override;
    std::string toString()const override;
};
struct XmlObject { virtual ~XmlObject()=default; };
using XmlRef=std::shared_ptr<XmlObject>;
class XmlAdaptor {
public:
    virtual ~XmlAdaptor()=default;
    virtual XmlRef clone(XmlRef)=0;
    virtual std::string serializeXML(XmlRef)=0;
};
class XMLNode : public AbstractMmlEmptyNode {
protected:
    XmlRef xml_;
    XmlAdaptor* adaptor_=nullptr;
public:
    using AbstractMmlEmptyNode::AbstractMmlEmptyNode;
    void initializeSourceFields(){xml_.reset();adaptor_=nullptr;}
    std::string kind()const override;
    virtual XmlRef getXML()const;
    virtual XMLNode* setXML(XmlRef,XmlAdaptor* adaptor=nullptr);
    virtual std::string getSerializedXML()const;
    MmlNode* copy(bool keepIds=false)override;
    std::string toString()const override;
};
}
