// MathJax 4.1.3 input/tex.ts; Apache-2.0.
module folia.math.input.tex.parser;
import :runtime;
import :parse_options;
import :configuration;
import :tex_parser;
import :tags;
import folia.math.core.input_jax;
import folia.math.core.input_jax_host;
import folia.math.core.math_item;
import folia.math.core.math_item_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.input.tex.node_factory;
import folia.math.input.tex.node_util;
import folia.math.input.tex.tex_constants;
import folia.math.util.options;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
Key indexKey(std::size_t index){const auto ascii=std::to_string(index);return key(std::u16string(ascii.begin(),ascii.end()));}
Value argument(std::span<const Value> values,std::size_t i){return i<values.size()?values[i]:Value(Undefined{});}
std::u16string requireString(const Value& value,std::u16string_view name){
    if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
    throw Exception("TypeError",std::u16string(name)+u" must be a string");
}
template<class T>T& requireShared(const ParserShared<T>& value,std::u16string_view property){
    if(const auto* owner=std::get_if<std::shared_ptr<T>>(&value);owner&&*owner)return **owner;
    const bool isNull=std::holds_alternative<PropertyNull>(value);
    throw Exception("TypeError",u"Cannot read properties of "+std::u16string(isNull?u"null":u"undefined")+u" (reading '"+std::u16string(property)+u"')");
}
TeXRef requireJax(ParserRuntime& runtime,Value value){
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object)throw Exception("TypeError",u"TeX method receiver is not an object");
    return runtime.texIdentities.resolve(*object);
}
ParseOptionsRef requireParseOptions(const TeX& jax){
    const auto value=jax.parseOptions();
    if(const auto* owner=std::get_if<std::shared_ptr<ParseOptions>>(&value);owner&&*owner)return *owner;
    throw Exception("TypeError",u"Cannot read properties of null or undefined (reading 'clear')");
}
std::u16string propertyText(const Property& value){return property_host::string(value);}
Property valueProperty(const Value& value){
    if(std::holds_alternative<Undefined>(value.data))return Missing{};
    if(std::holds_alternative<Null>(value.data))return PropertyNull{};
    if(const auto* p=std::get_if<bool>(&value.data))return *p;
    if(const auto* p=std::get_if<double>(&value.data))return *p;
    if(const auto* p=std::get_if<std::u16string>(&value.data))return property_host::debugText(*p);
    throw Exception("Unimplemented",u"TeX adapter requires canonical native property identity");
}
Value propertyValue(const Property& value){
    if(std::holds_alternative<Missing>(value.value))return Value(Undefined{});
    if(std::holds_alternative<PropertyNull>(value.value))return Value(Null{});
    if(const auto* p=std::get_if<bool>(&value.value))return Value(*p);
    if(const auto* p=std::get_if<double>(&value.value))return Value(*p);
    if(const auto* p=std::get_if<std::string>(&value.value))return Value(property_host::text(*p));
    throw Exception("Unimplemented",u"TeX property requires its canonical Context identity");
}
ParseOptionsRef filterOptions(FunctionArgs args){
    if(args.empty())throw Exception("TypeError",u"Filter callback data is missing");
    const auto* data=std::any_cast<core::InputJaxFilterDataRef>(&args[0]);
    if(!data||!*data)throw Exception("TypeError",u"Filter callback argument is not InputJaxFilterData");
    const auto current=(*data)->currentData();
    const auto* owner=std::any_cast<ParseOptionsRef>(&current);
    if(!owner||!*owner)throw Exception("TypeError",u"Filter callback data is not ParseOptions");
    return *owner;
}
mml::Children makeChildren(std::span<const mml::NodeValue> values){
    auto result=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    for(const auto& value:values)result->push_back(value);
    return result;
}
mml::NodeValue createNode(ParseOptions& options,std::string_view kind,mml::Children children){
    auto& factory=options.nodeFactoryOwner();
    const std::array<std::any,2> args{std::any(Property(std::string(kind))),std::any(std::move(children))};
    return factory.create("node",args);
}
void replaceNode(mml::NodeValue oldNode,mml::NodeValue newNode){
    // FilterUtil._cleanSubSup uses the raw `mml.parent` field.  Parent skips
    // notParent wrappers and may return an ancestor whose child array does
    // not contain oldNode, leaving the old tree entry in place.
    const mml::NodeValue parent=oldNode.requireNode("parent")->getParent();
    parent.requireNode("replaceChild")->replaceChild(newNode,oldNode);
}
std::size_t indexProperty(mml::NodeValue node,std::string_view name){
    const Property value=node.requireNode("getProperty")->getProperty(name);
    const double index=property_host::number(value);
    if(!std::isfinite(index)||index<0)return std::numeric_limits<std::size_t>::max();
    return static_cast<std::size_t>(index);
}
void cleanPair(ParseOptions& options,std::string_view kind,std::size_t low,std::size_t high,
               std::string_view lowKind,std::string_view highKind){
    auto nodes=options.getList(std::u16string(kind.begin(),kind.end()));
    auto removed=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    for(std::size_t i=0;i<nodes->size();++i){
        const mml::NodeValue node=nodes->get(i);
        const auto children=node.requireNode("childNodes")->childNodes;
        if(!children)throw Exception("TypeError",u"Cannot read properties of null (reading '0')");
        const mml::NodeValue lower=children->get(low),upper=children->get(high);
        if(lower.isNode()&&upper.isNode())continue;
        const mml::NodeValue base=children->get(0);
        const mml::NodeValue child=lower.isNode()?lower:upper;
        const std::string_view resultKind=lower.isNode()?lowKind:highKind;
        const std::array<mml::NodeValue,2> pair{base,child};
        const mml::NodeValue replacement=createNode(options,resultKind,makeChildren(pair));
        NodeUtil::instance().copyAttributes(node,replacement);
        replaceNode(node,replacement);
        removed->push_back(node);
    }
    options.removeFromList(std::u16string(kind.begin(),kind.end()),removed);
}
void cleanSubSup(ParseOptions& options){
    if(options.error)return;
    cleanPair(options,"msubsup",1,2,"msub","msup");
    cleanPair(options,"munderover",1,2,"munder","mover");
}
void moveLimitPair(ParseOptions& options,std::string_view kind,std::string_view replacementKind){
    auto nodes=options.getList(std::u16string(kind.begin(),kind.end()));
    auto removed=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    for(std::size_t i=0;i<nodes->size();++i){
        const mml::NodeValue node=nodes->get(i);
        const auto attrs=node.requireNode("attributes")->attributes();
        if(property_host::truthy(attrs->get("displaystyle")))continue;
        const auto children=node.requireNode("childNodes")->childNodes;
        if(!children)throw Exception("TypeError",u"Cannot read properties of null (reading '0')");
        const auto base=children->get(0);
        if(!property_host::truthy(base.requireNode("getProperty")->getProperty("movablelimits")))continue;
        const auto core=base.requireNode("coreMO")->coreMO();
        const auto coreAttributes=core.requireNode("attributes")->attributes();
        if(coreAttributes->hasExplicit("movablelimits"))continue;
        const auto replacement=createNode(options,replacementKind,children);
        NodeUtil::instance().copyAttributes(node,replacement);
        replaceNode(node,replacement);
        removed->push_back(node);
    }
    options.removeFromList(std::u16string(kind.begin(),kind.end()),removed);
}
void moveLimits(ParseOptions& options){
    moveLimitPair(options,"munderover","msubsup");
    moveLimitPair(options,"munder","msub");
    moveLimitPair(options,"mover","msup");
}
void setInherited(ParseOptions& options,core::InputJaxFilterDataRef data){
    const auto root=options.root;
    const auto mathReference=data->mathReference();
    const auto* math=std::get_if<core::MathItem*>(&mathReference);
    if(!math||!*math)throw Exception("TypeError",u"Filter math item is missing");
    root.requireNode("setInheritedAttributes")->setInheritedAttributes(PropertyList{},(*math)->display,0,false);
}
void checkScriptlevel(ParseOptions& options){
    auto nodes=options.getList(u"mstyle");
    auto removed=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    for(std::size_t i=0;i<nodes->size();++i){
        const auto node=nodes->get(i);
        const auto children=node.requireNode("childNodes")->childNodes;
        if(!children)throw Exception("TypeError",u"Cannot read properties of null (reading '0')");
        const auto first=children->get(0);
        const auto inner=first.requireNode("childNodes")->childNodes;
        if(!inner||inner->size()!=1)continue;
        const auto attrs=node.requireNode("attributes")->attributes();
        for(const std::string_view name:{"displaystyle","scriptlevel"})
            if(attrs->getExplicit(name)==attrs->getInherited(name))attrs->unset(name);
        const auto names=attrs->getExplicitNames();
        const bool hasOther=std::any_of(names.begin(),names.end(),[](const std::string& name){return name.substr(0,10)!="data-latex";});
        if(hasOther)continue;
        const auto child=inner->get(0);
        for(const auto& name:names)child.requireNode("attributes")->attributes()->set(name,attrs->get(name));
        replaceNode(node,child);
        removed->push_back(node);
    }
    options.removeFromList(u"mstyle",removed);
}
void cleanStretchy(ParseOptions& options){
    const auto nodes=options.getList(u"fixStretchy");
    for(std::size_t i=0;i<nodes->size();++i){
        const auto node=nodes->get(i);
        if(!property_host::truthy(NodeUtil::instance().getProperty(node,"fixStretchy")))continue;
        const Property op=NodeUtil::instance().getForm(node);
        if(!property_host::nullish(op)){
            const Property attributes=property_host::get(op,"3");
            if(!property_host::nullish(attributes)&&property_host::truthy(property_host::get(attributes,"stretchy")))
                NodeUtil::instance().setAttribute(node,"stretchy",false);
        }
        const std::array<std::string,1> remove{"fixStretchy"};
        NodeUtil::instance().removeProperties(node,remove);
    }
}
void cleanAttributes(ParseOptions& options){
    const auto root=options.root;
    root.requireNode("walkTree")->walkTree([&](mml::MmlNode* node,const std::any&){
        mml::NodeValue value(node);
        std::unordered_set<std::string> keep;
        const Property keepValue=NodeUtil::instance().getProperty(value,"keep-attrs");
        if(property_host::truthy(keepValue)){
            const auto text=property_host::debugText(property_host::string(keepValue));
            std::size_t begin=0;while(begin<=text.size()){const auto end=text.find(' ',begin);const auto stop=end==std::string::npos?text.size():end;keep.insert(text.substr(begin,stop-begin));if(end==std::string::npos)break;begin=end+1;}
        }
        const auto attrs=node->attributes();
        attrs->unset("data-latex-item");
        const auto explicitNames=attrs->getExplicitNames();
        for(const auto& name:explicitNames)
            if(!keep.contains(name)&&attrs->get(name)==attrs->getInherited(name))attrs->unset(name);
    });
}
bool compareExplicit(mml::NodeValue first,mml::NodeValue second){
    const auto a=first.requireNode("attributes")->attributes(),b=second.requireNode("attributes")->attributes();
    auto names=[](const std::shared_ptr<folia::math::Attributes>& attrs,std::string_view space){
        auto list=attrs->getExplicitNames();
        std::erase_if(list,[&](const std::string& name){return name==space||(name=="stretchy"&&!property_host::truthy(attrs->getExplicit(name)))||name=="data-latex"||name=="data-latex-item";});
        return list;
    };
    const auto one=names(a,"lspace"),two=names(b,"rspace");
    if(one.size()!=two.size())return false;
    for(const auto& name:one)if(a->getExplicit(name)!=b->getExplicit(name))return false;
    return true;
}
void combineRelations(ParseOptions& options){
    auto nodes=options.getList(u"mo");auto removed=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    for(std::size_t i=0;i<nodes->size();++i){
        const auto mo=nodes->get(i);
        if(property_host::truthy(NodeUtil::instance().getProperty(mo,"relationsCombined")))continue;
        const auto parent=mo.requireNode("parent")->getParent();
        if(!parent.isNode()||!NodeUtil::instance().isType(parent,"mrow")||NodeUtil::instance().getTexClass(mo)!=Property(static_cast<double>(mml::TEXCLASS.REL)))continue;
        const auto siblings=parent.requireNode("childNodes")->childNodes;
        if(!siblings)throw Exception("TypeError",u"Cannot read properties of null (reading 'indexOf')");
        const auto position=siblings->indexOf(mo);if(!position)continue;
        const Property variant=NodeUtil::instance().getProperty(mo,"variantForm");
        const auto attrs=mo.requireNode("attributes")->attributes();
        for(std::size_t next=*position+1;next<siblings->size();){
            const auto other=siblings->get(next);
            if(!other.isNode()||!NodeUtil::instance().isType(other,"mo")||NodeUtil::instance().getTexClass(other)!=Property(static_cast<double>(mml::TEXCLASS.REL)))break;
            if(variant==NodeUtil::instance().getProperty(other,"variantForm")&&compareExplicit(mo,other)){
                NodeUtil::instance().appendChildren(mo,NodeUtil::instance().getChildren(other));
                const auto otherAttrs=other.requireNode("attributes")->attributes();
                for(const std::string_view name:{"stretchy","rspace"}){const Property attr=otherAttrs->getExplicit(name);if(!std::holds_alternative<Missing>(attr.value)&&!std::holds_alternative<PropertyNull>(attr.value))attrs->set(name,attr);}
                for(const auto& name:other.requireNode("getPropertyNames")->getPropertyNames())mo.requireNode("setProperty")->setProperty(name,other.requireNode("getProperty")->getProperty(name));
                const Property otherLatex=otherAttrs->get("data-latex");
                if(property_host::truthy(otherLatex)){
                    const Property currentLatex=attrs->get("data-latex");
                    attrs->set("data-latex",property_host::debugText(property_host::string(currentLatex)+property_host::string(otherLatex)));
                }
                siblings->splice(static_cast<std::int64_t>(next),std::size_t{1});
                removed->push_back(other);
                other.requireNodeForWrite("parent")->setParent(PropertyNull{});
                NodeUtil::instance().setProperty(other,"relationsCombined",true);
                mo.requireNode("setProperty")->setProperty("texClass",static_cast<double>(mml::TEXCLASS.REL));
            }else{
                if(!attrs->hasExplicit("rspace"))attrs->set("rspace","0pt");
                const auto otherAttributes=other.requireNode("attributes")->attributes();
                if(!otherAttributes->hasExplicit("lspace"))otherAttributes->set("lspace","0pt");
                break;
            }
        }
        auto* moNode=dynamic_cast<mml::MmlMo*>(mo.nodeOrNull());
        if(!moNode)throw Exception("TypeError",u"mo.getForms is not a function");
        const auto forms=moNode->getForms();
        attrs->setInherited("form",forms.get(0));
    }
    options.removeFromList(u"mo",removed);
}
FunctionRef filterFunction(std::function<void(FunctionArgs)> body){
    auto function=std::make_shared<Function>();
    function->call=[body=std::move(body)](const FunctionReceiver&,FunctionArgs args)->std::any{body(args);return std::monostate{};};
    return function;
}
ObjectRef requireObject(const Value& value,std::u16string_view message){
    if(const auto* object=std::get_if<ObjectRef>(&value.data))return *object;
    throw Exception("TypeError",std::u16string(message));
}
Value contextValue(Context& cx,const Property& property){
    if(property.missing())return Value(Undefined{});
    if(std::holds_alternative<PropertyNull>(property.value))return Value(Null{});
    if(const auto* value=std::get_if<bool>(&property.value))return Value(*value);
    if(const auto* value=std::get_if<double>(&property.value))return Value(*value);
    if(const auto* value=std::get_if<std::string>(&property.value))return Value(property_host::text(*value));
    if(const auto array=PropertyArray::array(property)){
        const ObjectRef output=cx.array(array->size());
        for(std::size_t i=0;i<array->size();++i)if(array->has(i))cx.set(output,indexKey(i),contextValue(cx,array->get(i)));
        return Value(output);
    }
    if(const auto object=PropertyList::object(property)){
        const ObjectRef output=cx.object();
        for(const auto& name:object->keys())cx.set(output,key(property_host::text(name)),contextValue(cx,object->getOwn(name)));
        return Value(output);
    }
    throw Exception("Unimplemented",u"ProtoMathItem contains a Property value without a Context conversion");
}
Value canonicalMmlValue(ParserRuntime& runtime,mml::NodeValue value){
    const auto identity=runtime.mmlNodeIdentities.find(value.requireNode());
    if(!identity)throw Exception("TypeError",u"TeX formatError result has no canonical Context identity");
    return Value(*identity);
}
Value invokeNative(Context& cx,Value receiver,std::span<const Value> arguments,
                   ParserRuntime& runtime,std::u16string_view operation){
    const ObjectRef object=requireObject(receiver,u"TeX method receiver is not an object");
    const auto jax=runtime.texIdentities.resolve(object);
    const Value arg0=argument(arguments,0);
    if(operation==u"setMmlFactory"){
        if(std::holds_alternative<Null>(arg0.data))jax->setMmlFactory(PropertyNull{});
        else if(std::holds_alternative<Undefined>(arg0.data))jax->setMmlFactory(Missing{});
        else if(const auto* factory=std::get_if<ObjectRef>(&arg0.data))jax->setMmlFactory(runtime.resolveMmlFactory(*factory));
        else throw Exception("TypeError",u"MmlFactory must be an object");
    }else if(operation==u"reset"){
        FunctionArgs args;
        if(!arguments.empty()){
            const std::array<std::any,1> resetArgs{std::any(arg0)};
            jax->reset(resetArgs);
        }else jax->reset();
    }else if(operation==u"findMath"){
        // The input strings may be a Context array; FindTeX owns the canonical
        // string-array bridge. This class method intentionally ignores arg 1.
        const auto items=jax->findMath(std::any(arg0),argument(arguments,1));
        if(!items)throw Exception("TypeError",u"FindTeX returned a null ProtoMathItems collection");
        const ObjectRef output=cx.array(items->size());
        for(std::size_t i=0;i<items->size();++i){
            if(const auto* record=std::get_if<Property>(&(*items)[i]))cx.set(output,indexKey(i),contextValue(cx,*record));
            else throw Exception("Unimplemented",u"ProtoMathItem native object requires an identity-preserving Context binding");
        }
        (void)cx;
        return Value(output);
    }else if(operation==u"compile"){
        throw Exception("Unimplemented",u"MathItem and MathDocument Context identity bindings are not installed");
    }else if(operation==u"formatError"){
        if(arguments.empty())throw Exception("TypeError",u"formatError requires a TexError");
        const ObjectRef errorObject=requireObject(arg0,u"formatError requires a TexError object");
        const Value message=cx.get(Value(errorObject),key(u"message"));
        const Value id=cx.get(Value(errorObject),key(u"id"));
        const auto* idText=std::get_if<std::u16string>(&id.data);
        const auto* messageText=std::get_if<std::u16string>(&message.data);
        if(!idText||!messageText)throw Exception("TypeError",u"TexError id and message must be strings");
        return canonicalMmlValue(runtime,jax->formatError(TexError(*idText,*messageText)));
    }
    (void)cx;
    return Value(Undefined{});
}
} // namespace

TeX::TeX(ParserRuntime& runtime,OptionFunctionBindings& bindings,ObjectRef identity)
    :core::AbstractInputJax(bindings,identity),runtime_(runtime){
    if(&bindings.context()!=&runtime.context())throw Exception("TypeError",u"TeX and callable bindings belong to different Contexts");
}
ParserRuntime& TeX::runtime()const{return runtime_;}

TeXRef TeX::make(ParserRuntime& runtime,Value supplied){
    const auto* constructor=std::get_if<ObjectRef>(&runtime.texClass.data);
    if(!constructor)throw Exception("Unimplemented",u"TeX class metadata is not initialized");
    Context& context=runtime.context();
    const ObjectRef identity=context.instance(*constructor);
    auto result=std::make_shared<TeX>(runtime,runtime.functions(),identity);
    result->bindFilterRecordValueCodec([&runtime](const std::any& data)->Value{
        if(const auto* value=std::any_cast<Value>(&data))return *value;
        if(const auto* property=std::any_cast<Property>(&data))return runtime.properties.to(*property);
        if(const auto* options=std::any_cast<ParseOptionsRef>(&data);options&&*options){
            const auto source=runtime.parseOptionsIdentities.find(*options);
            if(!source)throw Exception("TypeError",u"Filter ParseOptions has no canonical Context identity");
            return Value(*source);
        }
        throw Exception("Unimplemented",u"TeX filter data has no canonical Context encoder");
    },[&runtime](Value value)->std::any{
        const auto* object=std::get_if<ObjectRef>(&value.data);
        if(!object)throw Exception("TypeError",u"TeX filter data is not ParseOptions");
        return std::any(runtime.parseOptionsIdentities.resolve(*object));
    },runtime.executionLifetime(),runtime.inputJaxModule().filterRecordOwner());
    runtime.texIdentities.bind(result,identity,result);
    const auto bind=[&](std::u16string_view name,
                        std::function<Value()> read,
                        std::function<void(Value)> write,
                        std::function<void()> remove={}){
        context.bindData(identity,key(name),std::make_shared<NativeDataField>(result,
            std::move(read),std::move(write),std::move(remove)));
    };
    bind(u"adaptor",[result](){
            if(const auto* value=std::get_if<PropertyNull>(&result->adaptor))return Value(Null{});
            if(const auto* value=std::get_if<Missing>(&result->adaptor))return Value(Undefined{});
            if(const auto* value=std::get_if<ObjectRef>(&result->adaptor))return Value(*value);
            throw Exception("Unimplemented",u"Input adaptor has no canonical Context object identity");
        },[result](Value value){
            if(std::holds_alternative<Undefined>(value.data)){result->adaptor=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){result->adaptor=PropertyNull{};return;}
            if(const auto* object=std::get_if<ObjectRef>(&value.data)){result->adaptor=*object;return;}
            throw Exception("TypeError",u"adaptor must be a DOM adaptor object");
        },[result](){result->adaptor=Missing{};});
    bind(u"mmlFactory",[result,&runtime](){
            if(const auto* value=std::get_if<PropertyNull>(&result->mmlFactory))return Value(Null{});
            if(const auto* value=std::get_if<Missing>(&result->mmlFactory))return Value(Undefined{});
            if(const auto* value=std::get_if<mml::MmlFactory*>(&result->mmlFactory);value&&*value){
                const auto identity=runtime.mmlFactoryIdentities.find(*value);
                if(identity)return Value(*identity);
            }
            if(const auto* value=std::get_if<std::shared_ptr<mml::MmlFactory>>(&result->mmlFactory);value&&*value)
                return Value(runtime.bindMmlFactory(*value));
            return Value(Undefined{});
        },[result,&runtime](Value value){
            if(std::holds_alternative<Undefined>(value.data)){result->mmlFactory=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){result->mmlFactory=PropertyNull{};return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"mmlFactory must be an MmlFactory");
            result->mmlFactory=runtime.resolveMmlFactory(*object);
        },[result](){result->mmlFactory=Missing{};});
    // AbstractInputJax publishes its own fields before its constructor body
    // publishes options and filter lists.  Keep these live Context fields in
    // that inherited Source order; the callbacks read the real native owner.
    // TeX's derived fields are published below before configuration can run.
    // InputJax.options is a writable own Source field, attached to the same
    // Value cell used by native parsing.
    auto optionsField=std::make_shared<NativeDataField>(result,
        [result]()->Value{return result->options;},
        [result](Value value){result->options=std::move(value);});
    context.bindData(identity,key(u"options"),std::move(optionsField));
    bind(u"preFilters",[result,&runtime](){return Value(runtime.functionLists.bind(result->preFilters));},
        [result,&runtime](Value value){
            if(std::holds_alternative<Undefined>(value.data)){result->preFilters.reset();return;}
            if(std::holds_alternative<Null>(value.data)){result->preFilters.reset();return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"preFilters must be a FunctionList");
            result->preFilters=runtime.functionLists.resolve(*object);
        },[result](){result->preFilters.reset();});
    bind(u"postFilters",[result,&runtime](){return Value(runtime.functionLists.bind(result->postFilters));},
        [result,&runtime](Value value){
            if(std::holds_alternative<Undefined>(value.data)){result->postFilters.reset();return;}
            if(std::holds_alternative<Null>(value.data)){result->postFilters.reset();return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"postFilters must be a FunctionList");
            result->postFilters=runtime.functionLists.resolve(*object);
        },[result](){result->postFilters.reset();});
    bind(u"findTeX",[result](){
            if(const auto* owner=std::get_if<std::shared_ptr<FindTeX>>(&result->findTeX);owner&&*owner)
                return Value((*owner)->instanceIdentity());
            return std::holds_alternative<PropertyNull>(result->findTeX)?Value(Null{}):Value(Undefined{});
        },[result,&context](Value value){
            if(std::holds_alternative<Undefined>(value.data)){result->findTeX=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){result->findTeX=PropertyNull{};return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"findTeX must be a FindTeX instance");
            const auto* native=std::any_cast<NativeObjectRef>(&context.payload(*object));
            auto owner=native?std::dynamic_pointer_cast<FindTeX>(*native):std::shared_ptr<FindTeX>{};
            if(!owner)throw Exception("TypeError",u"findTeX must be a FindTeX instance");
            result->findTeX=std::move(owner);
        },[result](){result->findTeX=Missing{};});
    bind(u"configuration",[result,&runtime](){
            if(const auto* owner=std::get_if<ParserConfigurationRef>(&result->configuration);owner&&*owner){
                const auto identity=runtime.parserConfigurationIdentities.find(*owner);if(identity)return Value(*identity);
            }
            return std::holds_alternative<PropertyNull>(result->configuration)?Value(Null{}):Value(Undefined{});
        },[result,&runtime](Value value){
            if(std::holds_alternative<Undefined>(value.data)){result->configuration=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){result->configuration=PropertyNull{};return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"configuration must be a ParserConfiguration");
            result->configuration=runtime.parserConfigurationIdentities.resolve(*object);
        },[result](){result->configuration=Missing{};});
    bind(u"_parseOptions",[result,&runtime](){
            const auto* owner=std::get_if<ParseOptionsRef>(&result->parseOptions_);
            if(owner&&*owner){const auto identity=runtime.parseOptionsIdentities.find(*owner);if(identity)return Value(*identity);}
            return std::holds_alternative<PropertyNull>(result->parseOptions_)?Value(Null{}):Value(Undefined{});
        },[result,&runtime](Value value){
            if(std::holds_alternative<Undefined>(value.data)){result->parseOptions_=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){result->parseOptions_=PropertyNull{};return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"_parseOptions must be a ParseOptions instance");
            result->parseOptions_=runtime.parseOptionsIdentities.resolve(*object);
        },[result](){result->parseOptions_=Missing{};});
    // Publish the inherited fields and TeX constructor's configuration
    // fields before package config callbacks can traverse this canonical jax.
    // All fields remain dynamic accessors over the same native owner.
    result->initializeTeX(std::move(supplied));
    // These fields are first assigned while parsing, after configuration has
    // run in Source. Append their live accessors at that same phase boundary.
    bind(u"latex",[result](){return propertyValue(result->latex);},
        [result](Value value){result->latex=valueProperty(value);},
        [result](){result->latex=Missing{};});
    bind(u"mathNode",[result,&runtime](){
            if(result->mathNode.isNode()){
                const auto identity=runtime.mmlNodeIdentities.find(result->mathNode.requireNode());
                if(identity)return Value(*identity);
            }
            return result->mathNode.kind()==tree::ChildKind::Null?Value(Null{}):Value(Undefined{});
        },[result,&runtime](Value value){
            if(std::holds_alternative<Undefined>(value.data)){result->mathNode=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){result->mathNode=PropertyNull{};return;}
            const auto* object=std::get_if<ObjectRef>(&value.data);
            if(!object)throw Exception("TypeError",u"mathNode must be an MML node");
            result->mathNode=&runtime.resolveMmlNode(*object);
        },[result](){result->mathNode=Missing{};});
    return result;
}

void TeX::initializeModule(ParserRuntime& runtime){
    Context& context=runtime.context();
    if(!std::holds_alternative<Undefined>(runtime.texClass.data))
        throw Exception("TypeError",u"TeX module already initialized");
    runtime.functionLists.addNativeArgumentEncoder([](const std::any& argument)->std::optional<Value>{
        if(const auto* record=std::any_cast<core::InputJaxFilterDataRef>(&argument);record&&*record)
            return Value((*record)->sourceIdentity());
        return std::nullopt;
    });
    const ObjectRef constructor=context.nativeConstructor(u"TeX",
        [&runtime](Context&,std::span<const Value> arguments)->ObjectRef{
            const auto jax=TeX::make(runtime,argument(arguments,0));
            return jax->instanceIdentity();
        });
    context.define(constructor,key(u"length"),Descriptor::data(Value(1),false,false,true));
    runtime.texClass=Value(constructor);

    context.setPrototype(constructor,runtime.inputJaxModule().abstractClass());
    const ObjectRef prototype=std::get<ObjectRef>(context.get(Value(constructor),key(u"prototype")).data);
    const ObjectRef parentPrototype=std::get<ObjectRef>(context.get(
        Value(runtime.inputJaxModule().abstractClass()),key(u"prototype")).data);
    context.setPrototype(prototype,parentPrototype);

    context.set(constructor,key(u"NAME"),Value(u"TeX"));
    const Value parentOptions=context.get(Value(runtime.inputJaxModule().abstractClass()),key(u"OPTIONS"));
    const ObjectRef defaults=context.object();
    for(const auto& ownKey:context.ownKeys(parentOptions)){
        const auto descriptor=context.descriptor(parentOptions,ownKey);
        if(descriptor&&descriptor->enumerable)context.set(Value(defaults),ownKey,context.get(parentOptions,ownKey));
    }
    context.set(Value(defaults),key(u"FindTeX"),Value(Null{}));
    const ObjectRef packages=context.array(1);context.set(Value(packages),key(u"0"),Value(u"base"));
    context.set(Value(defaults),key(u"packages"),Value(packages));
    context.set(Value(defaults),key(u"maxBuffer"),Value(5*1024));
    context.set(Value(defaults),key(u"maxTemplateSubtitutions"),Value(10000));
    context.set(Value(defaults),key(u"mathStyle"),Value(u"TeX"));

    auto errorBody=std::make_shared<Function>();
    errorBody->call=[](const FunctionReceiver&,FunctionArgs args)->std::any{
        if(args.size()<2)throw Exception("TypeError",u"formatError requires a TeX instance and TexError");
        const auto* jax=std::any_cast<TeXRef>(&args[0]);
        const auto* error=std::any_cast<TexError>(&args[1]);
        if(!jax||!*jax||!error)throw Exception("TypeError",u"formatError arguments have incompatible native types");
        return std::any((*jax)->formatError(*error));
    };
    OptionCallBinding errorBinding;
    errorBinding.receiver=[](Context&,Value receiver)->FunctionReceiver{return FunctionReceiver(std::move(receiver));};
    errorBinding.arguments=[&runtime](Context& cx,std::span<const Value> args){
        if(args.size()<2)throw Exception("TypeError",u"formatError requires a TeX instance and TexError");
        const auto* jax=std::get_if<ObjectRef>(&args[0].data);
        const auto* error=std::get_if<ObjectRef>(&args[1].data);
        if(!jax||!error)throw Exception("TypeError",u"formatError arguments have incompatible Context values");
        const Value message=cx.get(Value(*error),key(u"message"));
        const Value id=cx.get(Value(*error),key(u"id"));
        const auto* idText=std::get_if<std::u16string>(&id.data);
        const auto* messageText=std::get_if<std::u16string>(&message.data);
        if(!idText||!messageText)throw Exception("TypeError",u"TexError id and message must be strings");
        return std::vector<std::any>{std::any(runtime.texIdentities.resolve(*jax)),std::any(TexError(*idText,*messageText))};
    };
    errorBinding.result=[&runtime](Context&,const std::any& result)->Value{
        const auto* node=std::any_cast<mml::NodeValue>(&result);
        if(!node)throw Exception("TypeError",u"TeX formatError result is not an MML node");
        return canonicalMmlValue(runtime,*node);
    };
    Value formatError=runtime.functions().bind(errorBody,std::move(errorBinding),u"formatError");
    const auto* errorFunction=std::get_if<ObjectRef>(&formatError.data);
    if(!errorFunction)throw Exception("TypeError",u"TeX formatError default is not callable");
    context.define(*errorFunction,key(u"name"),Descriptor::data(Value(u"formatError"),false,false,true));
    context.define(*errorFunction,key(u"length"),Descriptor::data(Value(2),false,false,true));
    context.set(Value(defaults),key(u"formatError"),formatError);
    context.set(constructor,key(u"OPTIONS"),Value(defaults));

    const auto configure=context.function(u"configure",[&runtime](Context&,Value,std::span<const Value> args)->Value{
        const auto parserConfiguration=TeX::configure(runtime,argument(args,0));
        const auto identity=runtime.parserConfigurationIdentities.find(parserConfiguration);
        if(!identity)throw Exception("TypeError",u"ParserConfiguration has no Context identity");
        return Value(*identity);
    });
    context.define(configure,key(u"name"),Descriptor::data(Value(u""),false,false,true));
    context.define(configure,key(u"name"),Descriptor::data(Value(u"configure"),false,false,true));
    context.define(configure,key(u"length"),Descriptor::data(Value(1),false,false,true));
    context.set(constructor,key(u"configure"),Value(configure));
    const auto initializeTags=context.function(u"tags",[&runtime](Context& cx,Value,std::span<const Value> args)->Value{
        const auto* optionsObject=args.empty()?nullptr:std::get_if<ObjectRef>(&args[0].data);
        const auto* configurationObject=args.size()<2?nullptr:std::get_if<ObjectRef>(&args[1].data);
        if(!optionsObject||!configurationObject)throw Exception("TypeError",u"TeX.tags requires ParseOptions and ParserConfiguration");
        const auto options=runtime.parseOptionsIdentities.resolve(*optionsObject);
        const auto configuration=runtime.parserConfigurationIdentities.resolve(*configurationObject);
        TeX::tags(*options,*configuration);
        return Value(Undefined{});
    });
    context.define(initializeTags,key(u"name"),Descriptor::data(Value(u""),false,false,true));
    context.define(initializeTags,key(u"name"),Descriptor::data(Value(u"tags"),false,false,true));
    context.define(initializeTags,key(u"length"),Descriptor::data(Value(2),false,false,true));
    context.set(constructor,key(u"tags"),Value(initializeTags));

    // Source methods live on TeX.prototype and are ordinary enumerable
    // assignments. The typed algorithm remains on the TeX class; these are
    // identity/argument adapters, not alternate parser implementations.
    const ObjectRef setMmlFactory=context.function(u"setMmlFactory",
        [&runtime](Context& cx,Value receiver,std::span<const Value> args){return invokeNative(cx,std::move(receiver),args,runtime,u"setMmlFactory");});
    const ObjectRef resetFunction=context.function(u"reset",
        [&runtime](Context& cx,Value receiver,std::span<const Value> args){return invokeNative(cx,std::move(receiver),args,runtime,u"reset");});
    const ObjectRef compileFunction=context.function(u"compile",
        [&runtime](Context& cx,Value receiver,std::span<const Value> args){return invokeNative(cx,std::move(receiver),args,runtime,u"compile");});
    const ObjectRef findMathFunction=context.function(u"findMath",
        [&runtime](Context& cx,Value receiver,std::span<const Value> args){return invokeNative(cx,std::move(receiver),args,runtime,u"findMath");});
    const ObjectRef formatErrorFunction=context.function(u"formatError",
        [&runtime](Context& cx,Value receiver,std::span<const Value> args){return invokeNative(cx,std::move(receiver),args,runtime,u"formatError");});
    const auto installMethod=[&](std::u16string_view name,ObjectRef function,double length){
        context.define(function,key(u"name"),Descriptor::data(Value(u""),false,false,true));
        context.define(function,key(u"length"),Descriptor::data(Value(length),false,false,true));
        context.define(prototype,key(name),Descriptor::data(Value(function),true,true,true));
    };
    installMethod(u"setMmlFactory",setMmlFactory,1);
    installMethod(u"reset",resetFunction,1);
    installMethod(u"compile",compileFunction,2);
    installMethod(u"findMath",findMathFunction,1);
    installMethod(u"formatError",formatErrorFunction,1);
    const ObjectRef parseOptionsGetter=context.function(u"get",[&runtime](Context&,Value receiver,std::span<const Value>)->Value{
        const auto* identity=std::get_if<ObjectRef>(&receiver.data);
        if(!identity)throw Exception("TypeError",u"parseOptions getter receiver is not an object");
        const auto jax=runtime.texIdentities.resolve(*identity);
        const auto value=jax->parseOptions();
        if(const auto* owner=std::get_if<std::shared_ptr<ParseOptions>>(&value);owner&&*owner){
            const auto parseIdentity=runtime.parseOptionsIdentities.find(*owner);
            if(parseIdentity)return Value(*parseIdentity);
        }
        return std::holds_alternative<PropertyNull>(value)?Value(Null{}):Value(Undefined{});
    });
    context.define(parseOptionsGetter,key(u"length"),Descriptor::data(Value(0),false,false,true));
    context.define(prototype,key(u"parseOptions"),Descriptor::access(parseOptionsGetter,std::nullopt,false,true));
}

ParserConfigurationRef TeX::configure(ParserRuntime& runtime,Value packages){
    Context& context=runtime.context();
    const ObjectRef constructor=std::get<ObjectRef>(runtime.parserConfigurationClass.data);
    const ObjectRef parserNames=context.array(1);context.set(Value(parserNames),key(u"0"),Value(u"tex"));
    const std::array<Value,2> args{std::move(packages),Value(parserNames)};
    const ObjectRef identity=context.construct(constructor,args);
    const ParserConfigurationRef result=runtime.parserConfigurationIdentities.resolve(identity);
    result->init();
    return result;
}

void TeX::tags(ParseOptions& options,ParserConfiguration& configuration){
    ParserRuntime& runtime=options.runtime();
    auto& factory=runtime.tagsFactory;
    factory.addTags(configuration.sourceField(u"tags"));
    const Value style=runtime.context().get(options.options,key(u"tags"));
    const auto* name=std::get_if<std::u16string>(&style.data);
    if(!name)throw Exception("Unimplemented",u"TagsFactory default requires the Source string-key Map binding");
    factory.setDefault(*name);
    const TagsRef tags=factory.getDefault();
    options.setTags(tags);
    const auto identity=runtime.tagIdentities.find(tags);
    const auto parseOptions=runtime.parseOptionsIdentities.find(options.shared_from_this());
    if(!identity||!parseOptions)throw Exception("TypeError",u"TeX tags require canonical Tags and ParseOptions identities");
    runtime.context().set(Value(*identity),key(u"configuration"),Value(*parseOptions));
}

void TeX::initializeTeX(Value supplied){
    Context& context=runtime_.context();
    const std::array<Value,2> selectors{
        context.get(runtime_.texClass,key(u"OPTIONS")),
        context.get(Value(runtime_.findTeXModule().constructorIdentity()),key(u"OPTIONS"))};
    const auto separated=options::separateOptions(context,std::move(supplied),selectors);
    if(separated.size()!=3)throw Exception("TypeError",u"TeX option partition did not produce three records");
    initializeInputJax(separated[1]);
    findTeX=Missing{};
    if(const Value selected=context.get(options,key(u"FindTeX"));options::truthy(selected)){
        const auto* identity=std::get_if<ObjectRef>(&selected.data);
        if(!identity)throw Exception("TypeError",u"FindTeX option is not an object");
        const auto& payload=context.payload(*identity);
        const auto* native=std::any_cast<NativeObjectRef>(&payload);
        const auto found=native?std::dynamic_pointer_cast<FindTeX>(*native):std::shared_ptr<FindTeX>{};
        if(!found)throw Exception("Unimplemented",u"Supplied FindTeX object has no canonical native FindTeX identity");
        findTeX=found;
    }else findTeX=runtime_.findTeXModule().make(separated[2]);

    const Value packageNames=context.get(options,key(u"packages"));
    auto parserConfiguration=TeX::configure(runtime_,packageNames);
    configuration=parserConfiguration;
    const ObjectRef definitionRecords=context.array(2);
    context.set(definitionRecords,key(u"0"),this->options);
    context.set(definitionRecords,key(u"1"),runtime_.tagsFactory.OPTIONS);
    auto parseOptions=ParseOptions::make(runtime_,*parserConfiguration,Value(definitionRecords));
    parseOptions_=parseOptions;
    const std::array<Value,1> rest{separated[0]};
    (void)options::userOptions(context,parseOptions->options,rest);
    parserConfiguration->config(*this);
    TeX::tags(*parseOptions,*parserConfiguration);

    const std::array<std::pair<std::u16string_view,double>,7> filters{{
        {u"cleanSubSup",-7},{u"setInherited",-6},{u"checkScriptlevel",-5},
        {u"moveLimits",-4},{u"cleanStretchy",-3},{u"cleanAttributes",-2},
        {u"combineRelations",-1}}};
    auto definitions=std::make_shared<FunctionDefinitions>();
    const auto append=[&](std::u16string_view name,double priority,std::function<void(ParseOptions&,core::InputJaxFilterDataRef)> callback){
        auto body=std::make_shared<Function>();
        body->sourceMetadata=FunctionSourceMetadata{std::u16string(name),1};
        body->call=[callback=std::move(callback)](const FunctionReceiver&,FunctionArgs args)->std::any{
            if(args.empty())throw Exception("TypeError",u"TeX post-filter data is missing");
            const auto* data=std::any_cast<core::InputJaxFilterDataRef>(&args[0]);
            if(!data||!*data)throw Exception("TypeError",u"TeX post-filter record is invalid");
            const auto current=(*data)->currentData();
            const auto* owner=std::any_cast<ParseOptionsRef>(&current);
            if(!owner||!*owner)throw Exception("TypeError",u"TeX post-filter ParseOptions is invalid");
            callback(**owner,*data);return std::monostate{};
        };
        auto tuple=std::make_shared<FunctionTuple>();tuple->item=FunctionValue(FunctionRef(body));tuple->priority=priority;
        definitions->emplace_back(tuple);
    };
    append(filters[0].first,filters[0].second,[](ParseOptions& o,auto){cleanSubSup(o);});
    append(filters[1].first,filters[1].second,[](ParseOptions& o,auto d){setInherited(o,std::move(d));});
    append(filters[2].first,filters[2].second,[](ParseOptions& o,auto){checkScriptlevel(o);});
    append(filters[3].first,filters[3].second,[](ParseOptions& o,auto){moveLimits(o);});
    append(filters[4].first,filters[4].second,[](ParseOptions& o,auto){cleanStretchy(o);});
    append(filters[5].first,filters[5].second,[](ParseOptions& o,auto){cleanAttributes(o);});
    append(filters[6].first,filters[6].second,[](ParseOptions& o,auto){combineRelations(o);});
    if(!postFilters)throw Exception("TypeError",u"this.postFilters is not initialized");
    postFilters->addList(definitions);
}

ParserShared<ParseOptions> TeX::parseOptions()const{return parseOptions_;}
void TeX::beginRequestOwnerScope(){
    if(requestOwnerScopeActive_)throw Exception("TypeError",u"TeX request owner scope is already active");
    auto& parse=requireShared(parseOptions_,u"itemFactory");
    auto& itemFactory=requireShared(parse.itemFactory,u"itemFactory");
    const auto mark=itemFactory.productMark();
    requestParserCapture_=runtime_.parserIdentities.beginRequestCapture();
    try{requestStackCapture_=runtime_.stackIdentities.beginRequestCapture();}
    catch(...){runtime_.parserIdentities.retireCapturedRequestBindings(*requestParserCapture_);requestParserCapture_.reset();throw;}
    try{requestStackItemCapture_=runtime_.stackItemIdentities.beginRequestCapture();}
    catch(...){runtime_.stackIdentities.retireCapturedRequestBindings(*requestStackCapture_);requestStackCapture_.reset();runtime_.parserIdentities.retireCapturedRequestBindings(*requestParserCapture_);requestParserCapture_.reset();throw;}
    try{requestStackPropertyCapture_=runtime_.stackPropertyIdentities.beginRequestCapture();}
    catch(...){runtime_.stackItemIdentities.retireCapturedRequestBindings(*requestStackItemCapture_);requestStackItemCapture_.reset();runtime_.stackIdentities.retireCapturedRequestBindings(*requestStackCapture_);requestStackCapture_.reset();runtime_.parserIdentities.retireCapturedRequestBindings(*requestParserCapture_);requestParserCapture_.reset();throw;}
    try{runtime_.beginRequestContextRecordCapture();}
    catch(...){runtime_.stackPropertyIdentities.retireCapturedRequestBindings(*requestStackPropertyCapture_);requestStackPropertyCapture_.reset();runtime_.stackItemIdentities.retireCapturedRequestBindings(*requestStackItemCapture_);requestStackItemCapture_.reset();runtime_.stackIdentities.retireCapturedRequestBindings(*requestStackCapture_);requestStackCapture_.reset();runtime_.parserIdentities.retireCapturedRequestBindings(*requestParserCapture_);requestParserCapture_.reset();throw;}
    requestStackItemFactory_=&itemFactory;
    requestStackItemMark_=mark.size;
    requestStackItemGeneration_=mark.generation;
    requestReferencesDetached_=requestParserIdentitiesRetired_=requestStackIdentitiesRetired_=
        requestStackItemsRetired_=requestStackPropertiesRetired_=requestStackProductsRetired_=false;
    requestContextRecords_.clear();requestContextRecordsCaptured_=false;requestTransientRecordsRetired_=false;
    requestOwnerScopeActive_=true;
}
void TeX::releaseRequestReferences(){
    if(!requestOwnerScopeActive_)return;
    const auto* owner=std::get_if<std::shared_ptr<ParseOptions>>(&parseOptions_);
    if(!owner||!*owner)throw Exception("TypeError",u"TeX request owner scope lost ParseOptions");
    Context& context=runtime_.context();
    if(!requestContextRecordsCaptured_){
        const auto parsersValue=(*owner)->parsers;
        const auto nodeListsValue=(*owner)->nodeLists;
        const auto addRecord=[&](Value value,bool nodeArrayRegistryOwner=false){
            const auto* object=std::get_if<ObjectRef>(&value.data);
            // ParseOptions::nodeLists may contain bindChildren() results.
            // Those Context arrays are already owned by nodeArrayIdentities
            // and are retired through that registry after this method.
            if(object&&nodeArrayRegistryOwner&&runtime_.nodeArrayIdentities.ownsHandle(*object))return;
            if(object&&std::find(requestContextRecords_.begin(),requestContextRecords_.end(),*object)==requestContextRecords_.end())
                requestContextRecords_.push_back(*object);
        };
        addRecord(parsersValue);
        if(const auto* lists=std::get_if<ObjectRef>(&nodeListsValue.data)){
            for(const auto& name:context.enumerableStringKeys(Value(*lists)))
                addRecord(context.get(Value(*lists),key(name)),true);
            addRecord(Value(*lists));
        }
        requestContextRecordsCaptured_=true;
    }
    if(!requestReferencesDetached_){
        (*owner)->setMathItem(nullptr);(*owner)->root=PropertyNull{};
        (*owner)->parsers=Undefined{};(*owner)->nodeLists=Undefined{};
        mathNode=PropertyNull{};latex=Missing{};requestReferencesDetached_=true;
    }
    while(!requestContextRecords_.empty()){
        context.retireObjectRecord(requestContextRecords_.back());requestContextRecords_.pop_back();
    }
    if(!requestTransientRecordsRetired_){runtime_.retireRequestContextRecords();requestTransientRecordsRetired_=true;}
    if(!requestParserIdentitiesRetired_){runtime_.parserIdentities.retireCapturedRequestBindings(*requestParserCapture_);requestParserCapture_.reset();requestParserIdentitiesRetired_=true;}
    if(!requestStackIdentitiesRetired_){runtime_.stackIdentities.retireCapturedRequestBindings(*requestStackCapture_);requestStackCapture_.reset();requestStackIdentitiesRetired_=true;}
    if(!requestStackItemsRetired_){runtime_.stackItemIdentities.retireCapturedRequestBindings(*requestStackItemCapture_);requestStackItemCapture_.reset();requestStackItemsRetired_=true;}
    if(!requestStackPropertiesRetired_){runtime_.stackPropertyIdentities.retireCapturedRequestBindings(*requestStackPropertyCapture_);requestStackPropertyCapture_.reset();requestStackPropertiesRetired_=true;}
    if(!requestStackProductsRetired_){
        requestStackItemFactory_->retireProductsFrom({requestStackItemFactory_,requestStackItemMark_,requestStackItemGeneration_});
        requestStackProductsRetired_=true;
    }
    requestStackItemFactory_=nullptr;requestOwnerScopeActive_=false;
}
void TeX::setMmlFactory(core::InputMmlFactoryReference factory){
    core::AbstractInputJax::setMmlFactory(factory);
    if(const auto* owned=std::get_if<std::shared_ptr<mml::MmlFactory>>(&factory);owned&&*owned)
        (void)runtime_.bindMmlFactory(*owned);
    auto& options=requireShared(parseOptions_,u"nodeFactory");
    auto& nodeFactory=options.nodeFactoryOwner();
    if(const auto* value=std::get_if<mml::MmlFactory*>(&factory))nodeFactory.setMmlFactory(*value);
    else if(const auto* value=std::get_if<std::shared_ptr<mml::MmlFactory>>(&factory))nodeFactory.setMmlFactory(*value);
    else if(std::holds_alternative<PropertyNull>(factory))nodeFactory.setMmlFactory(PropertyNull{});
    else nodeFactory.setMmlFactory(Missing{});
}
void TeX::reset(FunctionArgs args){
    requireShared(parseOptions(),u"clear").clear();
    auto& options=requireShared(parseOptions(),u"tags");
    auto& tags=requireShared(options.tags,u"reset");
    Value tag=Value(0);
    if(!args.empty()){
        if(const auto* value=std::any_cast<Value>(&args[0]))tag=*value;
        else if(const auto* property=std::any_cast<Property>(&args[0])){
            if(property->missing())tag=Value(0);
            else if(std::holds_alternative<PropertyNull>(property->value))tag=Value(Null{});
            else if(const auto* b=std::get_if<bool>(&property->value))tag=Value(*b);
            else if(const auto* n=std::get_if<double>(&property->value))tag=Value(*n);
            else if(const auto* s=std::get_if<std::string>(&property->value))tag=Value(property_host::text(*s));
            else throw Exception("Unimplemented",u"reset tag requires a native options value");
        }else throw Exception("Unimplemented",u"reset tag requires a native options value");
    }
    if(std::holds_alternative<Undefined>(tag.data))tag=Value(0);
    tags.reset(std::move(tag));
}

mml::NodeValue TeX::formatError(const TexError& error){
    std::u16string message=error.message;
    if(const auto line=message.find(u'\n');line!=std::u16string::npos){
        const auto next=message.find_first_of(u"\r\n\u2028\u2029",line+1);
        message.erase(line,next==std::u16string::npos?std::u16string::npos:next-line);
    }
    auto& options=requireShared(parseOptions(),u"nodeFactory");
    auto& factory=options.nodeFactoryOwner();
    const std::array<std::any,3> args{
        std::any(Property(property_host::debugText(message))),
        std::any(Property(property_host::debugText(error.id))),
        std::any(latex)};
    return factory.create("error",args);
}

mml::NodeValue TeX::compile(core::MathItem& math,core::MathDocumentReference document){
    requireShared(parseOptions(),u"clear").clear();
    requireShared(parseOptions(),u"mathItem").setMathItem(&math);
    const ParserShared<ParseOptions> preFilterOptions=parseOptions();
    const auto* preFilterOwner=std::get_if<ParseOptionsRef>(&preFilterOptions);
    if(!preFilterOwner||!*preFilterOwner)throw Exception("TypeError",u"TeX pre-filter ParseOptions is missing");
    (void)executeFilters(*preFilters,core::MathItemReference(&math),document,std::any(*preFilterOwner));
    latex=math.math;
    auto& tags=requireShared(requireShared(parseOptions(),u"tags").tags,u"startEquation");
    tags.startEquation(math);
    TexParserRef parser;
    mml::NodeValue node=Missing{};
    try{
        Context& context=runtime_.context();
        const ObjectRef environment=context.object();
        context.set(environment,key(u"display"),runtime_.properties.to(math.display));
        context.set(environment,key(u"isInner"),Value(false));
        const Property environmentView=runtime_.properties.from(Value(environment));
        parser=TexParser::make(propertyText(latex),environmentView,requireShared(parseOptions(),u"parser"));
        node=parser->mml();
    }catch(const TexError& error){
        requireShared(parseOptions(),u"error").error=true;
        const Value callback=runtime_.context().get(this->options,key(u"formatError"));
        const FunctionValue callable=runtime_.functions().functionValue(callback);
        const auto* function=std::get_if<FunctionRef>(&callable);
        if(!function||!*function)throw Exception("TypeError",u"this.options.formatError is not a function");
        const std::array<std::any,2> arguments{std::any(runtime_.texIdentities.resolve(instanceIdentity())),std::any(error)};
        const std::any formatted=(*function)->call(FunctionReceiver{this->options},arguments);
        const auto* result=std::any_cast<mml::NodeValue>(&formatted);
        if(!result)throw Exception("Unimplemented",u"TeX formatError callback result has no native MML node identity");
        node=*result;
    }
    auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(node);
    auto& factory=requireShared(parseOptions(),u"nodeFactory").nodeFactoryOwner();
    const std::array<std::any,2> nodeArguments{std::any(Property("math")),std::any(children)};
    node=factory.create("node",nodeArguments);
    node.requireNode("attributes")->attributes()->set("data-latex",latex);
    if(property_host::truthy(math.display))NodeUtil::instance().setAttribute(node,"display","block");
    requireShared(requireShared(parseOptions(),u"tags").tags,u"finishEquation").finishEquation(math);
    requireShared(parseOptions(),u"root").root=node;
    const ParserShared<ParseOptions> postFilterOptions=parseOptions();
    const auto* postFilterOwner=std::get_if<ParseOptionsRef>(&postFilterOptions);
    if(!postFilterOwner||!*postFilterOwner)throw Exception("TypeError",u"TeX post-filter ParseOptions is missing");
    (void)executeFilters(*postFilters,core::MathItemReference(&math),document,std::any(*postFilterOwner));
    if(parser){
        const Property hsize=std::get<std::shared_ptr<Stack>>(parser->stack)->env();
        if(property_host::truthy(property_host::get(hsize,"hsize"))){
            const Property width=property_host::get(hsize,"hsize");
            NodeUtil::instance().setAttribute(node,"maxwidth",width);
            NodeUtil::instance().setAttribute(node,"overflow","linebreak");
        }
    }
    mathNode=requireShared(parseOptions(),u"root").root;
    return mathNode;
}

core::ProtoMathItems TeX::findMath(std::any strings,Value){
    auto& source=requireShared(findTeX,u"findMath");
    return source.findMath(std::move(strings));
}

} // namespace folia::math::tex
