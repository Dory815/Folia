/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Root-frozen declarations for core/MmlTree/MmlNode.ts (4.1.3).
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.core.mml_tree.mml_node;
export import folia.math.core.tree.node;
export import folia.math.core.mml_tree.attributes;
import std;

export namespace folia::math::mml {
using AttributeList = PropertyList;
class MmlNode;
using Children = tree::FactoryChildren<MmlNode>;
using Factory = tree::FactoryServices<MmlNode>;
using Error = tree::FactoryError;
class ErrorOptions {
    std::optional<PropertyList> value_;
public:
    ErrorOptions():value_(PropertyList{}){}
    ErrorOptions(PropertyList value):value_(std::move(value)){}
    ErrorOptions(std::nullptr_t):value_(std::nullopt){}
    // The same nullable object handle is available to virtual error callbacks.
    // A PropertyList copy retains identity; this is not an options snapshot.
    std::optional<PropertyList> value()const{return value_;}
    Property get(std::string_view name)const{
        if(!value_)throw Error("TypeError",u"Cannot read null MML error options");
        return value_->get(name);
    }
};
struct MmlClassInfo {
    std::optional<PropertyList> defaults;
    std::function<std::optional<PropertyList>()> defaultsBinding;
    std::optional<PropertyList> getDefaults()const{return defaultsBinding?defaultsBinding():defaults;}
    // Captured constructor static binding; Munder inherits the parent's array,
    // Mover owns another one. Replacement is live, captured handles stay live.
    std::function<std::shared_ptr<std::vector<Property>>()> accentsBinding;
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
std::pair<std::string,Property> unpackAttribute(const Property& value);

class MmlNode : public tree::Node<MmlNode> {
public:
    using tree::Node<MmlNode>::Node;
    virtual bool isToken() const=0;
    virtual bool isEmbellished() const=0;
    virtual bool isSpacelike() const=0;
    virtual bool linebreakContainer() const=0;
    virtual std::string linebreakAlign() const=0;
    virtual bool isEmpty() const=0;
    virtual double arity() const=0;
    virtual bool isInferred() const=0;
    virtual bool notParent() const=0;
    virtual MmlNode* Parent() const=0;
    virtual Property texClass()=0;
    virtual void setTexClass(Property)=0;
    virtual Property prevClass()=0;
    virtual void setPrevClass(Property)=0;
    virtual Property prevLevel() const=0;
    virtual void setPrevLevel(Property)=0;
    virtual std::shared_ptr<Attributes> attributes() const=0;
    virtual void setAttributes(std::shared_ptr<Attributes>)=0;
    virtual MmlNode* core()=0;
    virtual MmlNode* coreMO()=0;
    virtual std::size_t coreIndex()=0;
    virtual std::optional<std::size_t> childPosition()=0;
    virtual MmlNode* setTeXclass(MmlNode*)=0;
    virtual Property texSpacing()=0;
    virtual bool hasSpacingAttributes()=0;
    virtual void setInheritedAttributes(AttributeList={},Property display=false,Property level=0,Property prime=false)=0;
    virtual void inheritAttributesFrom(MmlNode*)=0;
    virtual void verifyTree(std::optional<PropertyList> options={})=0;
    virtual MmlNode* mError(std::string message,ErrorOptions options,bool shortError=false)=0;
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
    Property texclass_=PropertyNull{},prevClass_=PropertyNull{};
    Property prevLevel_=PropertyNull{};
    std::shared_ptr<Attributes> attributes_;
    std::shared_ptr<MmlClassInfo> constructorInfo_;
public:
    static PropertyList& defaults();
    static PropertyList& noInherit();
    static PropertyList& stopInherit();
    static PropertyList& alwaysInherit();
    static PropertyList& verifyDefaults();
    explicit AbstractMmlNode(Factory& factory):Generic(factory){}
    void initializeMml(tree::FactoryArgs args={});
    // Native class binding preserves this.constructor independently of registry.
    void bindConstructorClass(std::shared_ptr<MmlClassInfo> info){constructorInfo_=std::move(info);}
    virtual void initializeClassFields(){}
    MmlNode* copy(bool keepIds=false) override;
    Property texClass()override;
    void setTexClass(Property)override;
    Property prevClass()override;
    void setPrevClass(Property)override;
    Property prevLevel()const override;
    void setPrevLevel(Property)override;
    std::shared_ptr<Attributes> attributes()const override;
    void setAttributes(std::shared_ptr<Attributes>)override;
    bool isToken()const override;
    bool isEmbellished()const override;
    bool isSpacelike()const override;
    bool linebreakContainer()const override;
    std::string linebreakAlign()const override;
    bool isEmpty()const override;
    double arity()const override;
    bool isInferred()const override;
    bool notParent()const override;
    MmlNode* Parent()const override;
    void setChildren(Children)override;
    MmlNode* appendChild(MmlNode*)override;
    MmlNode* replaceChild(MmlNode*,MmlNode*)override;
    MmlNode* core()override;
    MmlNode* coreMO()override;
    std::size_t coreIndex()override;
    std::optional<std::size_t> childPosition()override;
    MmlNode* setTeXclass(MmlNode*)override;
    Property texSpacing()override;
    bool hasSpacingAttributes()override;
    void setInheritedAttributes(AttributeList={},Property display=false,Property level=0,Property prime=false)override;
    void inheritAttributesFrom(MmlNode*)override;
    void verifyTree(std::optional<PropertyList> options={})override;
    MmlNode* mError(std::string message,ErrorOptions options,bool shortError=false)override;
protected:
    const std::any& constructorStaticServices()const {
        if(!constructorInfo_)throw Error("TypeError",u"Missing constructor static services");
        return constructorInfo_->staticServices;
    }
    PropertyList constructorDefaults()const;
    std::shared_ptr<std::vector<Property>> constructorAccents()const {
        if(!constructorInfo_||!constructorInfo_->accentsBinding)
            throw Error("TypeError",u"Constructor has no ACCENTS binding");
        return constructorInfo_->accentsBinding();
    }
    void initializeTexClass(Property value){texclass_=std::move(value);}
    virtual void updateTeXclass(MmlNode*);
    virtual void getPrevClass(MmlNode*);
    virtual void setChildInheritedAttributes(AttributeList,Property,Property,Property);
    virtual AttributeList addInheritedAttributes(AttributeList,PropertyList);
    virtual void verifyAttributes(PropertyList);
    virtual void verifyChildren(PropertyList);
};
class AbstractMmlTokenNode : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    bool isToken()const override;
    bool isEmpty()const override;
    virtual std::u16string getText()const;
    std::any walkTree(const tree::TreeVisitor<MmlNode>&,const std::any& data={})override;
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
    MmlNode* core()override;
    MmlNode* coreMO()override;
    MmlNode* setTeXclass(MmlNode*)override;
};
class AbstractMmlBaseNode : public AbstractMmlNode {
public:
    using AbstractMmlNode::AbstractMmlNode;
    static PropertyList& defaults();
    bool isEmbellished()const override;
    MmlNode* core()override;
    MmlNode* coreMO()override;
    MmlNode* setTeXclass(MmlNode*)override;
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
    bool notParent()const override;
    MmlNode* Parent()const override;
    Property texClass()override;
    void setTexClass(Property)override;
    Property prevClass()override;
    void setPrevClass(Property)override;
    Property prevLevel()const override;
    void setPrevLevel(Property)override;
    std::shared_ptr<Attributes> attributes()const override;
    void setAttributes(std::shared_ptr<Attributes>)override;
    MmlNode* core()override;
    MmlNode* coreMO()override;
    std::size_t coreIndex()override;
    std::optional<std::size_t> childPosition()override;
    MmlNode* setTeXclass(MmlNode*)override;
    Property texSpacing()override;
    bool hasSpacingAttributes()override;
    void setInheritedAttributes(AttributeList={},Property display=false,Property level=0,Property prime=false)override;
    void inheritAttributesFrom(MmlNode*)override;
    void verifyTree(std::optional<PropertyList> options={})override;
    MmlNode* mError(std::string,ErrorOptions,bool shortError=false)override;
};
class TextNode : public AbstractMmlEmptyNode {
    std::u16string text_;
public:
    using AbstractMmlEmptyNode::AbstractMmlEmptyNode;
    std::string kind()const override;
    virtual std::u16string getText()const;
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
    std::string kind()const override;
    virtual XmlRef getXML()const;
    virtual XMLNode* setXML(XmlRef,XmlAdaptor* adaptor=nullptr);
    virtual std::string getSerializedXML()const;
    MmlNode* copy(bool keepIds=false)override;
    std::string toString()const override;
};
}
