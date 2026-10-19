// MathJax 4.1.3 input/tex/base/BaseItems.ts; Source class bodies.
// Apache-2.0; see src/math/LICENSE-MathJax.txt.
module folia.math.input.tex.parser;
import :base_items;
import :runtime;
import :parse_options;
import :tags;
import :parse_util;
import folia.math.input.tex.node_util;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.stack_item_host;
import folia.math.input.tex.base_item_host;
import folia.math.input.tex.stack_item_factory;
import :stack_item_factory_bindings;
import folia.math.input.tex.token;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.util.options;
import folia.math.util.context_property_bindings;
import folia.math.util.native_data_fields;
import folia.math.util.property_host;
import folia.math.util.option_functions;
import folia.math.util.regexp_host;
import folia.math.input.tex.unit_util;
import std;

namespace folia::math::tex {
namespace {
using namespace options;

void initializeBaseItemStaticErrors(){
    // These Source constructor statics inherit the actual BaseItem.errors
    // object. Defer materialization until module use to avoid cross-TU dynamic
    // initialization observing BaseItem::errors before its initializer ran.
    static std::once_flag initialized;
    std::call_once(initialized, [] {
        const auto derived=[](std::initializer_list<std::pair<std::string,Property>> entries){
            PropertyList result=PropertyList::withPrototype(
                PropertyList::record(BaseItem::baseErrors()).value());
            for(const auto& [name,value]:entries)result.setOwn(name,value);
            return Property(std::move(result));
        };
        OpenItem::errors=derived({{"stop",PropertyArray{"ExtraOpenMissingClose","Extra open brace or missing close brace"}}});
        SubsupItem::errors=derived({
            {"stop",PropertyArray{"MissingScript","Missing superscript or subscript argument"}},
            {"sup",PropertyArray{"MissingOpenForSup","Missing open brace for superscript"}},
            {"sub",PropertyArray{"MissingOpenForSub","Missing open brace for subscript"}}});
        LeftItem::errors=derived({{"stop",PropertyArray{"ExtraLeftMissingRight","Extra \\left or missing \\right"}}});
    });
}
}
Property OpenItem::errors;
Property SubsupItem::errors;
Property LeftItem::errors;
namespace {
using namespace options;

Key key(std::u16string_view value){return Key(std::u16string(value));}
Value argument(std::span<const Value> args,std::size_t i){return i<args.size()?args[i]:Value(Undefined{});}
mml::Children childArray(std::initializer_list<mml::NodeValue> nodes={});
Property itemProperty(ParserRuntime& runtime,tree::FactoryArgs args,std::size_t i){
    if(i>=args.size())return Missing{};
    if(const auto* p=std::any_cast<Property>(&args[i]))return *p;
    // Real Source call sites sometimes pass scalar values directly in their
    // FactoryArgs (EquationItem's `numbered` flag is a bool). Preserve those
    // canonical leaves instead of requiring every caller to wrap them.
    if(std::any_cast<Undefined>(&args[i]))return Missing{};
    if(std::any_cast<Null>(&args[i]))return PropertyNull{};
    if(const auto* value=std::any_cast<PropertyNull>(&args[i]))return *value;
    if(const auto* value=std::any_cast<bool>(&args[i]))return *value;
    if(const auto* value=std::any_cast<double>(&args[i]))return *value;
    if(const auto* value=std::any_cast<std::string>(&args[i]))return *value;
    if(const auto* value=std::any_cast<std::u16string>(&args[i]))return property_host::debugText(*value);
    if(const auto* value=std::any_cast<Value>(&args[i])){
        if(std::holds_alternative<Undefined>(value->data))return Missing{};
        if(std::holds_alternative<Null>(value->data))return PropertyNull{};
        if(const auto* b=std::get_if<bool>(&value->data))return *b;
        if(const auto* n=std::get_if<double>(&value->data))return *n;
        if(const auto* s=std::get_if<std::u16string>(&value->data))return property_host::debugText(*s);
        if(std::holds_alternative<ObjectRef>(value->data))return runtime.properties.from(*value);
    }
    property_host::typeError("BaseItem constructor argument is not a Property");
}
Value optionValue(ParserRuntime& runtime,tree::FactoryArgs args,std::size_t i){
    if(i>=args.size())return Undefined{};
    if(const auto* value=std::any_cast<Value>(&args[i]))return *value;
    const auto property=itemProperty(runtime,args,i);
    if(property.missing())return Undefined{};
    if(std::holds_alternative<PropertyNull>(property.value))return Null{};
    if(const auto* b=std::get_if<bool>(&property.value))return *b;
    if(const auto* n=std::get_if<double>(&property.value))return *n;
    if(const auto* s=std::get_if<std::string>(&property.value))return property_host::text(*s);
    if(std::holds_alternative<std::shared_ptr<PropertyStorage>>(property.value)||
       std::holds_alternative<FunctionRef>(property.value)||
       std::holds_alternative<ContextFunctionIdentity>(property.value)||
       std::holds_alternative<BuiltinProperty>(property.value))
        return runtime.properties.to(property);
    property_host::typeError("BaseItem Source constructor object argument has no Context identity adapter");
}
template<class T,class Initialize>
StackItemClassRef itemClass(ParserRuntime& runtime,std::u16string name,std::string kind,
                            Property errors,std::size_t length,Initialize initialize,
                            bool forwardsBaseNodes=false,
                            std::optional<ObjectRef> parentConstructor=std::nullopt,
                            bool ownsErrors=false){
    if(!parentConstructor){
        const auto* module=std::get_if<ObjectRef>(&runtime.stackItemModule.data);
        if(!module)throw options::Exception("Unimplemented",u"StackItem.BaseItem module is unavailable");
        const auto baseValue=runtime.context().get(options::Value(*module),options::Key(u"BaseItem"));
        const auto* base=std::get_if<ObjectRef>(&baseValue.data);
        if(!base)throw options::Exception("Unimplemented",u"StackItem.BaseItem constructor is unavailable");
        parentConstructor=*base;
    }
    return StackItemFactoryBindings::registerSubclass(runtime,std::move(name),std::move(kind),length,typeid(T),
        []{return std::make_unique<T>();},
        [initialize=std::move(initialize)](BaseItem& value,tree::FactoryArgs args){
            auto* typed=dynamic_cast<T*>(&value);
            if(!typed)property_host::typeError("Stack item Source initializer received a different class");
            initialize(*typed,args);
        },
        [](const BaseItem* value){return dynamic_cast<const T*>(value)!=nullptr;},
        [errors]{return errors;},std::move(parentConstructor),forwardsBaseNodes,
        std::nullopt,ownsErrors);
}

Property scalar(const BaseItem& item,std::string_view name){
    return stack_property_host::scalar(item.getProperty(name));
}
mml::Children childArray(std::initializer_list<mml::NodeValue> nodes){
    auto out=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    for(const auto& node:nodes)out->push_back(node);
    return out;
}
mml::NodeValue node(BaseItem& item,std::string_view kind,mml::Children children,Property definition=PropertyList{}){
    const std::array<std::any,3> args{Property(std::string(kind)),std::move(children),std::move(definition)};
    return item.create("node",args);
}
mml::NodeValue node(ArrayItem& item,std::string_view kind,mml::Children children,options::Value definition){
    const std::array<std::any,3> args{Property(std::string(kind)),std::move(children),item.runtime().properties.from(std::move(definition))};
    return item.create("node",args);
}
mml::NodeValue token(BaseItem& item,std::string_view kind,Property definition,Property text){
    const std::array<std::any,3> args{Property(std::string(kind)),std::move(definition),std::move(text)};
    return item.create("token",args);
}
mml::NodeValue textNode(BaseItem& item,Property text){
    const std::array<std::any,1> args{std::move(text)};
    return item.create("text",args);
}
StackItemFactoryService& service(const StackFactoryReference& factory){
    return stack_item_host::require<StackItemFactoryService>(factory,"configuration");
}
NodeFactory& nodeFactory(const StackFactoryReference& factory){
    auto& config=stack_item_host::require<StackItemFactoryConfiguration>(service(factory).configuration,"nodeFactory");
    return stack_item_host::require<NodeFactory>(config.nodeFactoryReference(),"create");
}
ParseOptions& parseOptions(const StackFactoryReference& factory){
    auto& native=service(factory);
    auto& configuration=stack_item_host::require<StackItemFactoryConfiguration>(native.configuration,"nodeFactory");
    auto* parse=dynamic_cast<ParseOptions*>(&configuration);
    if(!parse)property_host::typeError("Stack item factory configuration is not ParseOptions");
    return *parse;
}
Tags& tags(const StackFactoryReference& factory){
    auto& parse=parseOptions(factory);
    if(auto* value=std::get_if<std::shared_ptr<Tags>>(&parse.tags);value&&*value)return **value;
    property_host::typeError("Cannot read properties of undefined (reading 'tags')");
}
Property constant(std::string_view group,std::string_view name){
    return property_host::get(property_host::get(TexConstant,group),name);
}
options::ObjectRef requireObject(options::Context& context,const options::Value& value,std::string_view member){
    if(const auto* object=std::get_if<options::ObjectRef>(&value.data))return *object;
    property_host::typeError("Cannot read properties of "+std::string(std::holds_alternative<options::Null>(value.data)?"null":"undefined")+
                             " (reading '"+std::string(member)+"')");
}
options::Value arrayGet(options::Context& context,const options::Value& array,std::size_t index){
    const auto decimal=std::to_string(index);
    return context.get(array,options::Key(std::u16string(decimal.begin(),decimal.end())));
}
std::size_t arrayLength(options::Context& context,const options::Value& array){
    return context.arrayLength(requireObject(context,array,"length"));
}
void unshift(mml::Children array,mml::NodeValue value){
    const auto length=array->size();array->resize(length+1);
    for(std::size_t index=length;index>0;--index){
        const auto source=index-1;
        if(array->has(source))array->set(index,array->get(source));else array->erase(index);
    }
    array->set(0,value);
}
std::u16string u16(std::string_view value){return std::u16string(value.begin(),value.end());}
std::u16string valueText(options::Context& context,const options::Value& value){
    if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
    if(std::holds_alternative<options::Undefined>(value.data))return u"undefined";
    if(std::holds_alternative<options::Null>(value.data))return u"null";
    if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?u"true":u"false";
    if(const auto* number=std::get_if<double>(&value.data))return property_host::string(Property(*number));
    if(std::holds_alternative<options::SymbolRef>(value.data))property_host::typeError("Cannot convert a Symbol value to a string");
    property_host::typeError("Object ToPrimitive requires the canonical Context conversion binding");
}
std::vector<std::u16string> splitSpace(std::u16string_view source){
    std::vector<std::u16string> result;std::size_t start=0;
    for(std::size_t i=0;i<=source.size();++i)if(i==source.size()||source[i]==u' '){result.emplace_back(source.substr(start,i-start));start=i+1;}
    return result;
}
std::u16string joinUnits(const std::vector<std::u16string>& values,std::u16string_view separator){
    std::u16string result;for(std::size_t i=0;i<values.size();++i){if(i)result+=separator;result+=values[i];}return result;
}
Property toProperty(ParserRuntime& runtime,const options::Value& value){return runtime.properties.from(value);}
template<class T> std::shared_ptr<T> sharedArray(const ParserShared<T>& value,std::string_view member){
    if(const auto* array=std::get_if<std::shared_ptr<T>>(&value);array&&*array)return *array;
    const bool missing=std::holds_alternative<Missing>(value);
    property_host::typeError(std::string("Cannot read properties of ")+(missing?"undefined":"null")+" (reading '"+std::string(member)+"')");
}
mml::Children mmlArray(const ParserShared<tree::ChildArray<mml::MmlNode>>& value,std::string_view member){return sharedArray(value,member);}
BaseItem& stackItem(const StackItemRef& value,std::string_view member){
    if(!value)property_host::typeError(std::string("Cannot read properties of null (reading '")+std::string(member)+"')");
    auto* item=dynamic_cast<BaseItem*>(value.get());if(!item)property_host::typeError("Stack factory returned a non-BaseItem object");return *item;
}
TexParser& arrayParser(ArrayItem& item){
    if(const auto* parser=std::get_if<TexParser*>(&item.parser);parser&&*parser)return **parser;
    property_host::typeError("Cannot read properties of undefined (reading 'string')");
}
std::shared_ptr<Function> enTagFunction(Tags& service,ParserRuntime& runtime){
    auto& context=runtime.context();
    const auto callable=context.get(options::Value(service.sourceIdentity()),options::Key(u"enTag"));
    const auto value=runtime.functions().functionValue(callable);
    const auto* function=std::get_if<FunctionRef>(&value);
    if(!function||!*function||!(*function)->call)property_host::typeError("this.enTag is not a function");
    return *function;
}
mml::NodeValue callEnTag(Tags& service,ParserRuntime& runtime,mml::NodeValue nodeValue,mml::NodeValue tag){
    const auto function=enTagFunction(service,runtime);
    const auto receiver=std::static_pointer_cast<NativeObject>(service.shared_from_this());
    const std::array<std::any,2> args{std::any(std::move(nodeValue)),std::any(std::move(tag))};
    const auto result=function->call(FunctionReceiver{receiver},args);
    if(const auto* node=std::any_cast<mml::NodeValue>(&result))return *node;
    property_host::typeError("tags.enTag did not return an MML node value");
}
CheckType checked(std::initializer_list<StackEntry> entries,bool success=true){
    auto list=std::make_shared<CheckList::element_type>();
    for(const auto& entry:entries)list->push_back(entry);
    return std::make_shared<CheckTuple>(CheckTuple{std::move(list),success});
}
CheckType checked(const std::vector<StackEntry>& entries,bool success=true){
    return std::make_shared<CheckTuple>(CheckTuple{std::make_shared<std::vector<StackEntry>>(entries),success});
}
std::vector<Property> spliceErrorArguments(const Property& error,std::size_t start){
    auto array=PropertyArray::array(error);
    if(!array)property_host::typeError("error.splice is not a function");
    const auto length=array->size();
    std::vector<Property> removed;
    if(length<=start)return removed;
    removed.reserve(length-start);
    for(std::size_t index=start;index<length;++index)removed.push_back(array->get(index));
    array->resize(start);
    return removed;
}
StackItemRef asStackItem(const StackFactoryReference& factory,BaseItem* item,std::string_view member){
    if(!item)property_host::typeError("Cannot read properties of null (reading '"+std::string(member)+"')");
    auto* service=std::get_if<StackItemFactoryService*>(&factory);
    if(!service||!*service)property_host::typeError("Cannot read properties of undefined (reading 'create')");
    return (*service)->reference(item);
}
StackItemRef mmlItem(const StackFactoryReference& factory,mml::NodeValue value){
    auto* service=std::get_if<StackItemFactoryService*>(&factory);
    if(!service||!*service)property_host::typeError("Cannot read properties of undefined (reading 'create')");
    const std::any argument(value);
    return (*service)->reference((*service)->itemFactory().create("mml",tree::FactoryArgs(&argument,1)));
}
Property classErrors(std::initializer_list<std::pair<std::string,Property>> values){
    PropertyList result=PropertyList::withPrototype(PropertyList::record(BaseItem::baseErrors()).value());
    for(const auto& [name,value]:values)result.set(name,value);
    return result;
}
std::string text(const Property& value){return property_host::debugText(property_host::string(value));}
}

void BaseItems::initializeModule(ParserRuntime& runtime){
    initializeBaseItemStaticErrors();
    auto& context=runtime.context();
    auto module=context.object();
    std::unordered_map<std::string,ObjectRef> constructors;
    if(!std::holds_alternative<Undefined>(runtime.baseItemsModule.data))
        throw Exception("TypeError",u"BaseItems module is already initialized");
    auto add=[&](std::string_view className,StackItemClassRef type){
        const auto object=runtime.itemClassIdentities.find(type);
        if(!object)throw Exception("Unimplemented",u"Base stack item constructor identity was not registered");
        std::u16string property(className.begin(),className.end());
        context.set(Value(module),key(property),Value(*object));
        constructors.emplace(std::string(className),*object);
    };
    add("StartItem",itemClass<StartItem>(runtime,u"StartItem","start",BaseItem::baseErrors(),2,
        [&runtime](StartItem& item,tree::FactoryArgs args){item.initializeStartItem(itemProperty(runtime,args,0));}));
    add("StopItem",itemClass<StopItem>(runtime,u"StopItem","stop",BaseItem::baseErrors(),0,[](StopItem&,tree::FactoryArgs){},true));
    add("OpenItem",itemClass<OpenItem>(runtime,u"OpenItem","open",OpenItem::errors,0,[](OpenItem&,tree::FactoryArgs){},true,std::nullopt,true));
    add("CloseItem",itemClass<CloseItem>(runtime,u"CloseItem","close",BaseItem::baseErrors(),0,[](CloseItem&,tree::FactoryArgs){},true));
    add("NullItem",itemClass<NullItem>(runtime,u"NullItem","null",BaseItem::baseErrors(),0,[](NullItem&,tree::FactoryArgs){},true));
    add("PrimeItem",itemClass<PrimeItem>(runtime,u"PrimeItem","prime",BaseItem::baseErrors(),0,[](PrimeItem&,tree::FactoryArgs){},true));
    add("SubsupItem",itemClass<SubsupItem>(runtime,u"SubsupItem","subsup",SubsupItem::errors,0,[](SubsupItem&,tree::FactoryArgs){},true,std::nullopt,true));
    add("OverItem",itemClass<OverItem>(runtime,u"OverItem","over",BaseItem::baseErrors(),1,[](OverItem& item,tree::FactoryArgs){item.initializeOverItem();}));
    add("LeftItem",itemClass<LeftItem>(runtime,u"LeftItem","left",LeftItem::errors,2,[&runtime](LeftItem& item,tree::FactoryArgs args){item.initializeLeftItem(itemProperty(runtime,args,0));},false,std::nullopt,true));
    add("Middle",itemClass<Middle>(runtime,u"Middle","middle",BaseItem::baseErrors(),3,[&runtime](Middle& item,tree::FactoryArgs args){item.initializeMiddle(itemProperty(runtime,args,0),itemProperty(runtime,args,1));}));
    add("RightItem",itemClass<RightItem>(runtime,u"RightItem","right",BaseItem::baseErrors(),3,[&runtime](RightItem& item,tree::FactoryArgs args){item.initializeRightItem(itemProperty(runtime,args,0),itemProperty(runtime,args,1));}));
    add("BreakItem",itemClass<BreakItem>(runtime,u"BreakItem","break",BaseItem::baseErrors(),3,[&runtime](BreakItem& item,tree::FactoryArgs args){item.initializeBreakItem(itemProperty(runtime,args,0),itemProperty(runtime,args,1));}));
    add("BeginItem",itemClass<BeginItem>(runtime,u"BeginItem","begin",BaseItem::baseErrors(),0,[](BeginItem&,tree::FactoryArgs){},true));
    add("EndItem",itemClass<EndItem>(runtime,u"EndItem","end",BaseItem::baseErrors(),0,[](EndItem&,tree::FactoryArgs){},true));
    add("StyleItem",itemClass<StyleItem>(runtime,u"StyleItem","style",BaseItem::baseErrors(),0,[](StyleItem&,tree::FactoryArgs){},true));
    add("PositionItem",itemClass<PositionItem>(runtime,u"PositionItem","position",BaseItem::baseErrors(),0,[](PositionItem&,tree::FactoryArgs){},true));
    add("CellItem",itemClass<CellItem>(runtime,u"CellItem","cell",BaseItem::baseErrors(),0,[](CellItem&,tree::FactoryArgs){},true));
    add("MmlItem",itemClass<MmlItem>(runtime,u"MmlItem","mml",BaseItem::baseErrors(),0,[](MmlItem&,tree::FactoryArgs){},true));
    add("FnItem",itemClass<FnItem>(runtime,u"FnItem","fn",BaseItem::baseErrors(),0,[](FnItem&,tree::FactoryArgs){},true));
    add("NotItem",itemClass<NotItem>(runtime,u"NotItem","not",BaseItem::baseErrors(),0,[&runtime](NotItem& item,tree::FactoryArgs){item.initializeNotItem(runtime);}));
    add("NonscriptItem",itemClass<NonscriptItem>(runtime,u"NonscriptItem","nonscript",BaseItem::baseErrors(),0,[](NonscriptItem&,tree::FactoryArgs){},true));
    add("DotsItem",itemClass<DotsItem>(runtime,u"DotsItem","dots",BaseItem::baseErrors(),0,[](DotsItem&,tree::FactoryArgs){},true));
    add("ArrayItem",itemClass<ArrayItem>(runtime,u"ArrayItem","array",BaseItem::baseErrors(),0,[&runtime](ArrayItem& item,tree::FactoryArgs){item.initializeArrayItem(runtime);},true));
    add("EqnArrayItem",itemClass<EqnArrayItem>(runtime,u"EqnArrayItem","eqnarray",BaseItem::baseErrors(),1,[&runtime](EqnArrayItem& item,tree::FactoryArgs args){
        item.initializeArrayItem(runtime);item.initializeEqnArrayItem(args);
    },false,constructors.at("ArrayItem")));
    add("EquationItem",itemClass<EquationItem>(runtime,u"EquationItem","equation",BaseItem::baseErrors(),1,[](EquationItem& item,tree::FactoryArgs args){item.initializeEquationItem(args);}));
    add("MstyleItem",itemClass<MstyleItem>(runtime,u"MstyleItem","mstyle",BaseItem::baseErrors(),3,[&runtime](MstyleItem& item,tree::FactoryArgs args){item.initializeMstyleItem(itemProperty(runtime,args,0),itemProperty(runtime,args,1));},false,constructors.at("BeginItem")));
    runtime.baseItemsModule=Value(module);
}

std::string StartItem::kind()const{return "start";}
bool StartItem::isOpen()const{return true;}
void StartItem::initializeStartItem(Property value){global=std::move(value);}
CheckType StartItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"isKind");
    if(item.isKind("stop")){
        mml::NodeValue result=this->toMml();
        const Property inner=property_host::get(global,"isInner");
        if(!property_host::truthy(inner)){
            auto& tagService=tags(factory());
            result=tagService.finalize(result,this->env());
        }
        return checked({mmlItem(factory(),result)},true);
    }
    return BaseItem::checkItem(raw);
}
std::string StopItem::kind()const{return "stop";}
bool StopItem::isClose()const{return true;}
std::string OpenItem::kind()const{return "open";}
bool OpenItem::isOpen()const{return true;}
CheckType OpenItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"isKind");
    if(item.isKind("close")){
        const auto content=this->toMml();
        const auto atom=this->create("node",std::array<std::any,3>{Property("TeXAtom"),childArray({content}),Property(PropertyList{})});
        item.addLatexItem(atom);
        return checked({mmlItem(factory(),atom)},true);
    }
    return BaseItem::checkItem(raw);
}
std::string CloseItem::kind()const{return "close";}
bool CloseItem::isClose()const{return true;}
std::string NullItem::kind()const{return "null";}
std::string PrimeItem::kind()const{return "prime";}
CheckType PrimeItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"isKind");
    mml::NodeValue top0,top1;
    if(this->hasSourceNodesOwner()){
        const auto peek=this->sourceReadPeek(options::Value(2.0),2);
        top0=this->sourceNodeFromSlice(peek,0);top1=this->sourceNodeFromSlice(peek,1);
    }else{
        const auto peek=this->Peek(2);
        top0=peek->get(0);top1=peek->get(1);
    }
    const bool subSup=(NodeUtil::instance().isType(top0,"msubsup")||NodeUtil::instance().isType(top0,"msup"))&&
        !NodeUtil::instance().getChildAt(top0,dynamic_cast<mml::MmlMsubsup*>(top0.requireNode("kind"))->sup()).isNode();
    const bool underOver=(NodeUtil::instance().isType(top0,"munderover")||NodeUtil::instance().isType(top0,"mover"))&&
        !NodeUtil::instance().getChildAt(top0,dynamic_cast<mml::MmlMunderover*>(top0.requireNode("kind"))->over()).isNode()&&
        !property_host::truthy(NodeUtil::instance().getProperty(top0,"subsupOK"));
    if(!subSup&&!underOver){
        const std::string kind=property_host::truthy(NodeUtil::instance().getProperty(top0,"movesupsub"))?"mover":"msup";
        const auto result=node(*this,kind,childArray({top0,top1}));
        return checked({result,asStackItem(factory(),raw,"isKind")},true);
    }
    const std::size_t pos=subSup?dynamic_cast<mml::MmlMsubsup*>(top0.requireNode("kind"))->sup():
        dynamic_cast<mml::MmlMunderover*>(top0.requireNode("kind"))->over();
    NodeUtil::instance().setChild(top0,pos,top1);
    return checked({top0,asStackItem(factory(),raw,"isKind")},true);
}
std::string SubsupItem::kind()const{return "subsup";}
CheckType SubsupItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"isKind");
    if(item.isKind("open")||item.isKind("left"))return BaseItem::success;
    const auto top=this->First();
    const auto position=property_host::number(scalar(*this,"position"));
    if(item.isKind("mml")){
        const auto primes=this->getProperty("primes");
        if(stack_property_host::truthy(primes)){
            if(position!=2)NodeUtil::instance().setChild(top,2,stack_property_host::node(primes));
            else{
                const auto primeNode=stack_property_host::node(primes);
                NodeUtil::instance().setProperty(primeNode,"variantForm",true);
                const auto current=item.First();
                const auto row=node(*this,"mrow",childArray({primeNode,current}));
                item.setFirst(row);
            }
        }
        const auto current=item.First();
        NodeUtil::instance().setChild(top,static_cast<std::size_t>(position),current);
        const auto move=this->getProperty("movesupsub");
        if(!property_host::nullish(stack_property_host::scalar(move)))
            NodeUtil::instance().setProperty(top,"movesupsub",stack_property_host::scalar(move));
        return checked({mmlItem(factory(),top)},true);
    }
    (void)BaseItem::checkItem(raw);
    const auto error=this->getErrors(position==1?"sub":position==2?"sup":"");
    const auto id=base_item_host::errorField(error,0),message=base_item_host::errorField(error,1);
    const auto args=spliceErrorArguments(error,2);
    throw TexError(property_host::string(id),property_host::string(message),args);
}
void OverItem::initializeOverItem(){(void)this->setProperty("name",StackProperty("\\over"));}
std::string OverItem::kind()const{return "over";}
bool OverItem::isClose()const{return true;}
CheckType OverItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"isKind");
    if(item.isKind("over")){
        const auto name=item.getName();const std::array<Property,1> args{stack_property_host::scalar(name)};
        throw TexError(u"AmbiguousUseOf",u"Ambiguous use of %1",args);
    }
    if(item.isClose()){
        auto result=node(*this,"mfrac",childArray({stack_property_host::node(this->getProperty("num")),this->toMml(false)}));
        const auto thickness=this->getProperty("thickness");
        if(!property_host::nullish(stack_property_host::scalar(thickness)))NodeUtil::instance().setAttribute(result,"linethickness",stack_property_host::scalar(thickness));
        const auto left=this->getProperty("ldelim"),right=this->getProperty("rdelim");
        if(stack_property_host::truthy(left)||stack_property_host::truthy(right)){
            NodeUtil::instance().setProperty(result,"withDelims",true);
            result=ParseUtil::fixedFence(parseOptions(factory()),property_host::string(stack_property_host::scalar(left)),result,property_host::string(stack_property_host::scalar(right)));
        }
        const auto name=this->getProperty("name");
        result.requireNode("attributes")->attributes()->set(property_host::debugText(property_host::string(constant("Attr","LATEXITEM"))),stack_property_host::scalar(name));
        return checked({mmlItem(factory(),result),asStackItem(factory(),raw,"isKind")},true);
    }
    return BaseItem::checkItem(raw);
}
std::string OverItem::toString()const{
    if(this->hasSourceNodesOwner()){
        auto& access=this->sourceAccess();const auto getter=access.getReceiver(u"getProperty");const std::array<options::Value,1> propertyArgs{options::Value(u"num")};
        const auto numerator=access.callMember(getter,access.receiver(),propertyArgs);const std::string prefix="over["+access.sourceString(numerator)+" / ";
        const auto array=this->sourceNodesPropertyValue();const auto join=access.get(array,options::Key(u"join"));
        const std::array<options::Value,1> args{options::Value(u"; ")};const auto joined=access.callMember(join,array,args);
        return prefix+access.sourceString(joined)+"]";
    }
    const std::string numerator=text(scalar(*this,"num"));
    return "over["+numerator+" / "+base_item_host::join(this->nodes(),"; ")+ "]";
}

void LeftItem::initializeLeftItem(Property delim){(void)this->setProperty("delim",StackProperty(std::move(delim)));}
std::string LeftItem::kind()const{return "left";}
bool LeftItem::isOpen()const{return true;}
CheckType LeftItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"isKind");
    if(item.isKind("right")){
        ParseOptions& configuration=parseOptions(factory());
        const std::u16string open=property_host::string(stack_property_host::scalar(this->getProperty("delim")));
        const auto enclosed=this->toMml();
        const std::u16string close=property_host::string(stack_property_host::scalar(item.getProperty("delim")));
        const Property big=property_host::debugText(u"");
        const Property color=stack_property_host::scalar(item.getProperty("color"));
        const auto fenced=ParseUtil::fenced(configuration,open,enclosed,close,property_host::string(big),color);
        const auto children=fenced.requireNode("childNodes")->childNodes;
        const auto left=children->getProperty("0"),right=children->getProperty(std::to_string(children->size()-1));
        const auto row=mmlItem(factory(),fenced);
        this->addLatexItem(left,Property("\\left"));item.addLatexItem(right,Property("\\right"));
        const auto rowNode=stackItem(row,"First").First();
        rowNode.requireNode("attributes")->attributes()->set(
            property_host::debugText(property_host::string(constant("Attr","LATEXITEM"))),Property("\\left"+property_host::debugText(this->startStr.substr(static_cast<std::size_t>(this->startI),static_cast<std::size_t>(item.stopI-this->startI)))));
        return checked({row},true);
    }
    if(item.isKind("middle")){
        PropertyList def{{"stretchy",true},{"symmetric",true}};
        const auto color=item.getProperty("color");if(stack_property_host::truthy(color))def.set("mathcolor",stack_property_host::scalar(color));
        const auto middle=token(*this,"mo",def,stack_property_host::scalar(item.getProperty("delim")));
        item.addLatexItem(middle,Property("\\middle"));
        const auto close=node(*this,"TeXAtom",childArray(),PropertyList{{"texClass",mml::TEXCLASS.CLOSE}});
        const auto open=node(*this,"TeXAtom",childArray(),PropertyList{{"texClass",mml::TEXCLASS.OPEN}});
        const std::array<mml::NodeValue,3> values{close,middle,open};this->Push(values);
        this->setEnv(PropertyList{});
        return checked({asStackItem(factory(),this,"checkItem")},true);
    }
    return BaseItem::checkItem(raw);
}
void Middle::initializeMiddle(Property delim,Property color){(void)setProperty("delim",StackProperty(std::move(delim)));if(property_host::truthy(color))(void)setProperty("color",StackProperty(std::move(color)));}
std::string Middle::kind()const{return "middle";}bool Middle::isClose()const{return true;}
void RightItem::initializeRightItem(Property delim,Property color){(void)setProperty("delim",StackProperty(std::move(delim)));if(property_host::truthy(color))(void)setProperty("color",StackProperty(std::move(color)));}
std::string RightItem::kind()const{return "right";}bool RightItem::isClose()const{return true;}
void BreakItem::initializeBreakItem(Property linebreak,Property insert){(void)setProperty("linebreak",StackProperty(std::move(linebreak)));(void)setProperty("insert",StackProperty(std::move(insert)));}
std::string BreakItem::kind()const{return "break";}
CheckType BreakItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"isKind");const auto linebreak=scalar(*this,"linebreak");
    if(item.isKind("mml")){
        const auto mml=item.First();
        if(mml.requireNode("isKind")->isKind("mo")){
            const auto op=NodeUtil::instance().getOp(mml);
            const auto props=PropertyArray::array(op);
            Property style=Missing{};if(props&&props->size()>3)style=property_host::get(props->get(3),"linebreakstyle");
            if(!property_host::truthy(style))style=NodeUtil::instance().getAttribute(mml,"linebreakstyle");
            if(style!=Property("after")){NodeUtil::instance().setAttribute(mml,"linebreak",linebreak);return checked({asStackItem(factory(),raw,"isKind")},true);}
            if(!property_host::truthy(scalar(*this,"insert")))return checked({asStackItem(factory(),raw,"isKind")},true);
        }
    }
    const auto space=token(*this,"mspace",PropertyList{{"linebreak",linebreak}},Property(""));
    return checked({mmlItem(factory(),space),asStackItem(factory(),raw,"isKind")},true);
}
std::string BeginItem::kind()const{return "begin";}bool BeginItem::isOpen()const{return true;}
CheckType BeginItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"isKind");
    if(item.isKind("end")){
        const auto endName=item.getName(),name=this->getName();
        if(endName!=name){const std::array<Property,2> args{stack_property_host::scalar(name),stack_property_host::scalar(endName)};
            throw TexError(u"EnvBadEnd",u"\\begin{%1} ended with \\end{%2}",args);}
        const auto content=this->toMml();item.addLatexItem(content);return checked({mmlItem(factory(),content)},true);
    }
    if(item.isKind("stop")){const auto name=this->getName();const std::array<Property,1> args{stack_property_host::scalar(name)};throw TexError(u"EnvMissingEnd",u"Missing \\end{%1}",args);}
    return BaseItem::checkItem(raw);
}
std::string EndItem::kind()const{return "end";}bool EndItem::isClose()const{return true;}
std::string StyleItem::kind()const{return "style";}
CheckType StyleItem::checkItem(BaseItem* raw){BaseItem& item=base_item_host::item(raw,"isClose");if(!item.isClose())return BaseItem::checkItem(raw);mml::NodeValue mml;if(this->hasSourceNodesOwner()){auto& access=this->sourceAccess();const auto create=access.getReceiver(u"create");const auto nodes=this->sourceNodesPropertyValue();const auto getProperty=access.getReceiver(u"getProperty");const std::array<options::Value,1> propertyArgs{options::Value(u"styles")};const auto styles=access.callMember(getProperty,access.receiver(),propertyArgs);const std::array<options::Value,4> args{options::Value(u"node"),options::Value(u"mstyle"),nodes,styles};mml=access.decodeNode(access.callMember(create,access.receiver(),args));}else mml=node(*this,"mstyle",this->nodes(),scalar(*this,"styles"));return checked({mmlItem(factory(),mml),asStackItem(factory(),raw,"isClose")},true);}
std::string PositionItem::kind()const{return "position";}
CheckType PositionItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"isClose");if(item.isClose()){const auto name=this->getName();const std::array<Property,1> args{stack_property_host::scalar(name)};throw TexError(u"MissingBoxFor",u"Missing box for %1",args);}
    if(item.isFinal()){
        auto result=item.toMml();const auto move=scalar(*this,"move");
        if(move==Property("vertical")){PropertyList def{{"height",scalar(*this,"dh")},{"depth",scalar(*this,"dd")},{"voffset",scalar(*this,"dh")}};result=node(*this,"mpadded",childArray({result}),def);return checked({mmlItem(factory(),result)},true);}
        if(move==Property("horizontal"))return checked({mmlItem(factory(),stack_property_host::node(this->getProperty("left"))),asStackItem(factory(),raw,"isFinal"),mmlItem(factory(),stack_property_host::node(this->getProperty("right")))});
    }
    return BaseItem::checkItem(raw);
}
std::string CellItem::kind()const{return "cell";}bool CellItem::isClose()const{return true;}
std::string MmlItem::kind()const{return "mml";}bool MmlItem::isFinal()const{return true;}
std::string FnItem::kind()const{return "fn";}
CheckType FnItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"First");const auto top=this->First();
    if(top.isNode()){
        if(item.isOpen())return BaseItem::success;
        if(!item.isKind("fn")){
            auto first=item.First();if(!item.isKind("mml")||!first.isNode())return checked({top,asStackItem(factory(),raw,"First")});
            if((NodeUtil::instance().isType(first,"mstyle")&&first.requireNode("childNodes")->childNodes->size()&&
                NodeUtil::instance().isType(first.requireNode("childNodes")->childNodes->getProperty("0").requireNode("childNodes")->childNodes->getProperty("0"),"mspace"))||NodeUtil::instance().isType(first,"mspace"))
                return checked({top,asStackItem(factory(),raw,"First")});
            if(NodeUtil::instance().isEmbellished(first))first=NodeUtil::instance().getCoreMO(first);
            const auto form=NodeUtil::instance().getForm(first);
            if(!property_host::nullish(form)){const auto index=property_host::number(property_host::get(form,"2"));constexpr bool allowed[]={false,false,true,true,false,true,true,false,false,false};if(index>=0&&index<10&&allowed[static_cast<int>(index)])return checked({top,asStackItem(factory(),raw,"First")});}
        }
        if(top.requireNode("kind")->isKind("TeXAtom")&&top.requireNode("kind")->isEmpty())return checked({top,asStackItem(factory(),raw,"First")});
        const auto apply=token(*this,"mo",PropertyList{{"texClass",mml::TEXCLASS.NONE}},Property("\xE2\x81\xA1"));
        return checked({top,apply,asStackItem(factory(),raw,"First")});
    }
    return BaseItem::checkItem(raw);
}
std::string NotItem::kind()const{return "not";}
void NotItem::initializeNotItem(ParserRuntime& runtime){remap=runtime.mapHandler.getMap(Property("not_remap"));}
CheckType NotItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"isKind");
    if(item.isKind("open")||item.isKind("left"))return BaseItem::success;
    if(item.isKind("mml")){
        const auto mml=item.First();if(NodeUtil::instance().isType(mml,"mo")||NodeUtil::instance().isType(mml,"mi")||NodeUtil::instance().isType(mml,"mtext")){
            const auto content=NodeUtil::instance().getText(mml);
            if(content.size()==1&&!property_host::truthy(NodeUtil::instance().getProperty(mml,"movesupsub"))&&NodeUtil::instance().getChildren(mml)->size()==1){
                const auto* remap=std::get_if<TokenMapRef>(&this->remap);
                if(remap&&*remap&&(*remap)->contains(Property(property_host::debugText(content)))){
                    const auto found=(*remap)->lookup(Property(property_host::debugText(content)));
                    const auto* character=std::get_if<TokenRef>(&found);
                    if(character&&*character)NodeUtil::instance().setChild(mml,0,NodeFactory::createText(nodeFactory(factory()),(*character)->character()));
                }else NodeUtil::instance().appendChildren(mml,childArray({NodeFactory::createText(nodeFactory(factory()),Property("\xCC\xB8"))}));
                return checked({asStackItem(factory(),raw,"isKind")});
            }
        }
    }
    const auto textNode=NodeFactory::createText(nodeFactory(factory()),Property("\xE2\xA7\xB8"));
    const auto mtext=node(*this,"mtext",childArray({textNode}));
    const auto padded=node(*this,"mpadded",childArray({mtext}),PropertyList{{"width",0}});
    const auto atom=node(*this,"TeXAtom",childArray({padded}),PropertyList{{"texClass",mml::TEXCLASS.REL}});
    return checked({atom,asStackItem(factory(),raw,"isKind")});
}
std::string NonscriptItem::kind()const{return "nonscript";}
CheckType NonscriptItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"Size");if(item.isKind("mml")&&item.Size()==1){auto nodeValue=item.First();if(nodeValue.requireNode("isKind")->isKind("mstyle")&&property_host::truthy(nodeValue.requireNode("kind")->notParent()))nodeValue=NodeUtil::instance().getChildren(NodeUtil::instance().getChildren(nodeValue)->getProperty("0"))->getProperty("0");
        if(nodeValue.requireNode("isKind")->isKind("mspace")){if(nodeValue!=item.First()){const auto popped=item.Pop();const auto row=node(*this,"mrow",childArray({popped}));item.Push(std::span<const mml::NodeValue>(&row,1));}parseOptions(factory()).addNode(Property("nonscript"),item.First());}}
    return checked({asStackItem(factory(),raw,"Size")});
}
std::string DotsItem::kind()const{return "dots";}
CheckType DotsItem::checkItem(BaseItem* raw){BaseItem& item=base_item_host::item(raw,"isKind");if(item.isKind("open")||item.isKind("left"))return BaseItem::success;auto dots=stack_property_host::node(this->getProperty("ldots"));const auto top=item.First();if(item.isKind("mml")&&NodeUtil::instance().isEmbellished(top)){const auto cls=NodeUtil::instance().getTexClass(NodeUtil::instance().getCoreMO(top));if(cls==Property(mml::TEXCLASS.BIN)||cls==Property(mml::TEXCLASS.REL))dots=stack_property_host::node(this->getProperty("cdots"));}return checked({dots,asStackItem(factory(),raw,"isKind")});}

void ArrayItem::initializeArrayItem(ParserRuntime& runtime){
    runtime_=&runtime;auto& cx=runtime.context();
    table=std::make_shared<tree::ChildArray<mml::MmlNode>>();row=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    frame=cx.array();hfill=cx.array();arraydef=cx.object();cstart=cx.array();cend=cx.array();cextra=cx.array();
    atEnd=false;ralign=cx.array();breakAlign=cx.object();
    cx.set(breakAlign,key(u"cell"),Value(u""));cx.set(breakAlign,key(u"row"),Value(u""));cx.set(breakAlign,key(u"table"),Value(u""));
    templateSubs=0;
    const auto factoryValue=factory();
    const auto* service=std::get_if<StackItemFactoryService*>(&factoryValue);
    if(!service||!*service)throw Exception("TypeError",u"ArrayItem has no owning StackItemFactory");
    const StackItemRef reference=(*service)->reference(this);
    const auto identity=runtime.stackItemIdentities.find(reference);
    if(!identity)throw Exception("TypeError",u"ArrayItem has no canonical Context identity");
    const auto owner=std::dynamic_pointer_cast<ArrayItem>(reference);
    if(!owner)throw Exception("TypeError",u"ArrayItem Context owner has the wrong native class");
    bindNativeValue(cx,*identity,key(u"frame"),owner,&ArrayItem::frame);
    bindNativeValue(cx,*identity,key(u"hfill"),owner,&ArrayItem::hfill);
    bindNativeValue(cx,*identity,key(u"arraydef"),owner,&ArrayItem::arraydef);
    bindNativeValue(cx,*identity,key(u"cstart"),owner,&ArrayItem::cstart);
    bindNativeValue(cx,*identity,key(u"cend"),owner,&ArrayItem::cend);
    bindNativeValue(cx,*identity,key(u"cextra"),owner,&ArrayItem::cextra);
    bindNativeValue(cx,*identity,key(u"ralign"),owner,&ArrayItem::ralign);
    bindNativeValue(cx,*identity,key(u"breakAlign"),owner,&ArrayItem::breakAlign);
}
std::string ArrayItem::kind()const{return "array";}bool ArrayItem::isOpen()const{return true;}bool ArrayItem::copyEnv()const{return false;}
CheckType ArrayItem::checkItem(BaseItem* raw){BaseItem& item=base_item_host::item(raw,"isClose");if(item.isClose()&&!item.isKind("over")){if(stack_property_host::truthy(item.getProperty("isEntry"))){EndEntry();clearEnv();StartEntry();return BaseItem::fail;}if(stack_property_host::truthy(item.getProperty("isCR"))){EndEntry();EndRow();clearEnv();StartEntry();return BaseItem::fail;}EndTable();clearEnv();const auto newItem=mmlItem(factory(),createMml());if(stack_property_host::truthy(this->getProperty("requireClose"))){if(item.isKind("close"))return checked({newItem});throw TexError(u"MissingCloseBrace",u"Missing close brace");}return checked({newItem,asStackItem(factory(),raw,"isClose")});}return BaseItem::checkItem(raw);}
mml::NodeValue ArrayItem::createMml(){
    auto& context=runtime().context();
    const auto definition=arraydef;
    const auto object=requireObject(context,definition,"scriptlevel");
    const auto scriptlevel=context.get(definition,key(u"scriptlevel"));
    context.erase(object,key(u"scriptlevel"));
    auto result=node(*this,"mtable",mmlArray(table,"length"),runtime().properties.from(definition));
    if(options::truthy(scriptlevel))NodeUtil::instance().setProperty(result,"smallmatrix",Property(true));
    const auto breakTable=context.get(options::Value(breakAlign),key(u"table"));
    if(options::truthy(breakTable))NodeUtil::instance().setAttribute(result,"data-break-align",toProperty(runtime(),breakTable));
    if(stack_property_host::truthy(this->getProperty("arrayPadding"))){
        NodeUtil::instance().setAttribute(result,"data-frame-styles",Property(""));
        NodeUtil::instance().setAttribute(result,"framespacing",stack_property_host::scalar(this->getProperty("arrayPadding")));
    }
    result=handleFrame(result);
    if(!std::holds_alternative<options::Undefined>(scriptlevel.data)){
        const auto style=context.object();context.set(options::Value(style),key(u"scriptlevel"),scriptlevel);
        result=node(*this,"mstyle",childArray({result}),runtime().properties.from(options::Value(style)));
    }
    if(stack_property_host::truthy(this->getProperty("open"))||stack_property_host::truthy(this->getProperty("close"))){
        const auto open=stack_property_host::scalar(this->getProperty("open"));
        result=ParseUtil::fenced(parseOptions(factory()),open,result,stack_property_host::scalar(this->getProperty("close")));
    }
    return result;
}
mml::NodeValue ArrayItem::handleFrame(mml::NodeValue mml){
    auto& context=runtime().context();
    const auto frameObject=requireObject(context,frame,"length");
    const auto length=context.arrayLength(frameObject);
    if(!length)return mml;
    struct Side{std::u16string name,style;};std::vector<Side> sides;
    sides.reserve(length);
    for(std::uint32_t i=0;i<length;++i){
        const auto tuple=arrayGet(context,frame,i);
        const auto tupleObject=requireObject(context,tuple,"0");
        const auto side=arrayGet(context,tuple,0),style=arrayGet(context,tuple,1);
        const auto name=valueText(context,side),value=valueText(context,style);
        auto found=std::find_if(sides.begin(),sides.end(),[&](const Side& entry){return entry.name==name;});
        if(found==sides.end())sides.push_back({name,value});else found->style=value;
        (void)tupleObject;
    }
    const auto first=arrayGet(context,frame,0);
    const auto firstStyle=valueText(context,arrayGet(context,first,1));
    const auto fstyle=std::all_of(sides.begin(),sides.end(),[&](const Side& side){return side.style==firstStyle;})?firstStyle:std::u16string{};
    if(!fstyle.empty()){
        if(length==4){
            NodeUtil::instance().setAttribute(mml,"frame",Property(property_host::debugText(fstyle)));
            NodeUtil::instance().removeAttribute(mml,"data-frame-styles");return mml;
        }
        if(fstyle==u"solid"){
            NodeUtil::instance().setAttribute(mml,"data-frame-styles",Property(""));
            std::vector<std::u16string> names;names.reserve(sides.size());for(const auto& side:sides)names.push_back(side.name);
            const auto notation=joinUnits(names,u" ");
            return node(*this,"menclose",childArray({mml}),PropertyList{{"notation",property_host::debugText(notation)},{"data-padding",0}});
        }
    }
    std::u16string styles;
    for(const auto side:{u"top",u"right",u"bottom",u"left"}){
        if(!styles.empty())styles+=u" ";
        const auto found=std::find_if(sides.begin(),sides.end(),[&](const Side& value){return value.name==side;});
        styles+=found==sides.end()?u"none":found->style;
    }
    NodeUtil::instance().setAttribute(mml,"data-frame-styles",Property(property_host::debugText(styles)));
    return mml;
}
void ArrayItem::StartEntry(){
    auto& context=runtime().context();const auto n=mmlArray(row,"length")->size();
    auto at=[&](options::Value value,std::size_t index){return arrayGet(context,value,index);};
    auto start=at(cstart,n),end=at(cend,n);const auto align=at(ralign,n);const auto cextraValue=cextra;
    if(!options::truthy(start)&&!options::truthy(end)&&!options::truthy(align)&&
       !options::truthy(at(cextraValue,n))&&!options::truthy(at(cextraValue,n+1)))return;
    auto values=getEntry();const auto valueCount=arrayLength(context,values);
    auto prefix=valueCount>0?arrayGet(context,values,0):options::Value(options::Undefined{});
    auto entry=valueCount>1?arrayGet(context,values,1):options::Value(options::Undefined{});
    auto term=valueCount>2?arrayGet(context,values,2):options::Value(options::Undefined{});
    bool found=valueCount>3&&options::truthy(arrayGet(context,values,3));
    if(options::truthy(at(cextraValue,n))&&(!property_host::truthy(atEnd)||options::truthy(at(cextraValue,n+1)))){
        const auto string=valueText(context,start);
        start=options::Value(string+u"&");
    }
    if(term!=options::Value(u"&")){
        auto entryText=valueText(context,entry);
        while(!entryText.empty()&&(entryText.front()==u' '||entryText.front()==u'\t'||entryText.front()==u'\n'||entryText.front()==u'\r'))entryText.erase(entryText.begin());
        while(!entryText.empty()&&(entryText.back()==u' '||entryText.back()==u'\t'||entryText.back()==u'\n'||entryText.back()==u'\r'))entryText.pop_back();
        const auto termText=valueText(context,term);
        found=!entryText.empty()||(n!=0||(!termText.empty()&&termText.substr(0,4)!=u"\\end"));
        if(options::truthy(at(cextraValue,n+1))&&!options::truthy(at(cextraValue,n))){end=options::Value((options::truthy(end)?valueText(context,end):u"")+u"&");atEnd=true;}
    }
    if(!found&&!options::truthy(prefix))return;
    auto& parser=arrayParser(*this);
    if(found){
        if(options::truthy(start))entry=ParseUtil::addArgs(parser,valueText(context,start),valueText(context,entry));
        if(options::truthy(end))entry=ParseUtil::addArgs(parser,valueText(context,entry),valueText(context,end));
        if(options::truthy(align)){auto s=valueText(context,entry);while(!s.empty()&&(s.front()==u' '||s.front()==u'\t'||s.front()==u'\n'||s.front()==u'\r'))s.erase(s.begin());while(!s.empty()&&(s.back()==u' '||s.back()==u'\t'||s.back()==u'\n'||s.back()==u'\r'))s.pop_back();entry=options::Value(std::u16string(u"\\text{")+s+u"}");}
        if(options::truthy(start)||options::truthy(end)||options::truthy(align)){
            templateSubs=Property(property_host::number(templateSubs)+1);
            const auto options=parseOptions(factory()).options;
            const auto maximum=context.get(options,key(u"maxTemplateSubtitutions"));
            if(property_host::number(templateSubs)>context.toNumber(maximum))
                throw TexError(u"MaxTemplateSubs",u"Maximum template substitutions exceeded; is there an invalid use of \\\\ in the template?");
        }
    }
    if(options::truthy(prefix))entry=ParseUtil::addArgs(parser,valueText(context,prefix),valueText(context,entry));
    parser.setString(ParseUtil::addArgs(parser,valueText(context,entry),parser.string()));parser.i=0;
}
options::Value ArrayItem::getEntry(){
    auto& parser=arrayParser(*this);const auto source=parser.string();std::size_t i=static_cast<std::size_t>(parser.i),braces=0,envs=0;
    regexp_host::RegExp scanner(u"^([^]*?)([&{}]|\\\\\\\\|\\\\(?:begin|end)\\s*\\{array\\}|\\\\cr|\\\\)");
    regexp_host::RegExp prefixPattern(u"^(?:\\s*\\\\(?:h(?:dash)?line|hfil{1,3}|rowcolor\\s*\\{.*?\\}))+");
    const options::ObjectRef fail=runtime().context().array(4);
    for(std::size_t k=0;k<4;++k)runtime().context().set(options::Value(fail),options::Key(std::u16string(1,static_cast<char16_t>(u'0'+k))),k==3?options::Value(false):options::Value(u""));
    auto& context=runtime().context();
    while(i<=source.size()){
        const auto rest=source.substr(i);const auto match=scanner.exec(rest);if(!match)return options::Value(fail);
        const auto whole=match->at(0),tokenValue=match->at(2);if(!whole||!tokenValue)return options::Value(fail);
        i+=whole->size();const auto delimiter=*tokenValue;
        if(delimiter==u"\\") {++i;continue;}
        if(delimiter==u"{"){++braces;continue;}
        if(delimiter==u"}"){if(!braces)return options::Value(fail);--braces;continue;}
        if(delimiter==u"\\begin{array}"){if(!braces)++envs;continue;}
        if(delimiter==u"\\end{array}"&&!braces&&envs){--envs;continue;}
        if(braces||envs)continue;
        i-=delimiter.size();
        auto entry=source.substr(static_cast<std::size_t>(parser.i),i-static_cast<std::size_t>(parser.i));
        while(!entry.empty()&&(entry.front()==u' '||entry.front()==u'\t'||entry.front()==u'\n'||entry.front()==u'\r'))entry.erase(entry.begin());
        while(!entry.empty()&&(entry.back()==u' '||entry.back()==u'\t'||entry.back()==u'\n'||entry.back()==u'\r'))entry.pop_back();
        const auto prefix=prefixPattern.exec(entry);std::u16string prefixText;
        if(prefix&&prefix->at(0)){prefixText=*prefix->at(0);entry.erase(0,prefixText.size());}
        parser.setString(source.substr(i));parser.i=0;
        const auto result=context.array(4);const std::array<options::Value,4> values{prefixText,entry,delimiter,true};
        for(std::size_t n=0;n<values.size();++n)context.set(options::Value(result),options::Key(std::u16string(1,static_cast<char16_t>(u'0'+n))),values[n]);
        return options::Value(result);
    }
    return options::Value(fail);
}
void ArrayItem::EndEntry(){
    mml::NodeValue mtd;
    if(this->hasSourceNodesOwner()){auto& access=this->sourceAccess();const auto create=access.getReceiver(u"create");const auto children=this->sourceNodesPropertyValue();const std::array<options::Value,3> args{options::Value(u"node"),options::Value(u"mtd"),children};mtd=access.decodeNode(access.callMember(create,access.receiver(),args));}
    else mtd=this->create("node",std::array<std::any,3>{Property("mtd"),nodes(),Property(PropertyList{})});
    auto* mtdNode=mtd.requireNode("kind");auto& context=runtime().context();
    if(context.arrayLength(requireObject(context,hfill,"length"))){
        if(arrayGet(context,hfill,0)==options::Value(0))NodeUtil::instance().setAttribute(mtd,"columnalign",Property("right"));
        const auto lastHfillLength=context.arrayLength(requireObject(context,hfill,"length"));
        if(lastHfillLength&&arrayGet(context,hfill,lastHfillLength-1)==options::Value(this->Size()))
            NodeUtil::instance().setAttribute(mtd,"columnalign",property_host::truthy(NodeUtil::instance().getAttribute(mtd,"columnalign"))?Property("center"):Property("left"));
    }
    const auto alignment=arrayGet(context,ralign,mmlArray(row,"length")->size());
    if(options::truthy(alignment)){
        const auto values=context.readIterable(alignment,3);
        const auto valign=context.get(values,key(u"0"));
        const auto cwidth=context.get(values,key(u"1"));
        const auto calign=context.get(values,key(u"2"));
        auto first=mtd.requireNode("childNodes")->childNodes->getProperty("0");
        auto firstChildren=first.requireNode("childNodes")->childNodes;
        const auto definition=context.object();const std::array<std::u16string,3> names{u"width",u"data-overflow",u"data-align"};
        context.set(options::Value(definition),key(names[0]),cwidth);
        context.set(options::Value(definition),key(u"data-overflow"),options::Value(u"auto"));
        context.set(options::Value(definition),key(names[2]),calign);
        context.set(options::Value(definition),key(u"data-vertical-align"),valign);
        auto box=node(*this,"mpadded",firstChildren,runtime().properties.from(options::Value(definition)));
        box.requireNode("kind")->setProperty("vbox",runtime().properties.from(valign));
        auto currentFirst=mtd.requireNode("childNodes")->childNodes->getProperty("0");
        currentFirst.requireNode("kind")->childNodes=std::make_shared<tree::ChildArray<mml::MmlNode>>();
        (void)mtdNode->appendChild(box);
    }else{
        const auto breakCell=context.get(options::Value(breakAlign),key(u"cell"));
        if(options::truthy(breakCell))NodeUtil::instance().setAttribute(mtd,"data-vertical-align",toProperty(runtime(),breakCell));
    }
    context.set(options::Value(breakAlign),key(u"cell"),options::Value(u""));
    mmlArray(row,"length")->push_back(mtd);this->Clear();hfill=context.array();
}
void ArrayItem::EndRow(){
    std::string type="mtr";
    const auto numbered=this->getProperty("isNumbered");
    if(stack_property_host::truthy(numbered)&&mmlArray(row,"length")->size()==3){const auto label=mmlArray(row,"length")->get(2);mmlArray(row,"length")->pop_back();unshift(mmlArray(row,"length"),label);type="mlabeledtr";}
    else if(stack_property_host::truthy(this->getProperty("isLabeled"))){type="mlabeledtr";(void)this->setProperty("isLabeled",StackProperty(false));}
    const auto nodeValue=node(*this,type,mmlArray(row,"length"));
    auto& context=runtime().context();const auto breakRow=context.get(options::Value(breakAlign),key(u"row"));
    if(options::truthy(breakRow)){NodeUtil::instance().setAttribute(nodeValue,"data-break-align",toProperty(runtime(),breakRow));context.set(options::Value(breakAlign),key(u"row"),options::Value(u""));}
    this->addLatexItem(nodeValue);mmlArray(table,"length")->push_back(nodeValue);row=std::make_shared<tree::ChildArray<mml::MmlNode>>();atEnd=false;
}
void ArrayItem::EndTable(){if(this->Size()||mmlArray(row,"length")->size()){EndEntry();EndRow();}checkLines();}
void ArrayItem::checkLines(){
    auto& context=runtime().context();auto definition=requireObject(context,arraydef,"rowlines");
    if(options::truthy(context.get(arraydef,key(u"rowlines")))){
        const auto lineValue=context.get(arraydef,key(u"rowlines"));
        const auto* lineText=std::get_if<std::u16string>(&lineValue.data);
        if(!lineText){
            if(std::holds_alternative<options::Undefined>(lineValue.data)||std::holds_alternative<options::Null>(lineValue.data))
                property_host::typeError(std::string("Cannot read properties of ")+(std::holds_alternative<options::Null>(lineValue.data)?"null":"undefined")+" (reading 'split')");
            property_host::typeError("this.arraydef.rowlines.split is not a function");
        }
        auto lines=*lineText;std::vector<std::u16string> pieces;std::size_t start=0;
        for(std::size_t i=0;i<=lines.size();++i)if(i==lines.size()||lines[i]==u' '){pieces.emplace_back(lines.substr(start,i-start));start=i+1;}
        if(pieces.size()==mmlArray(table,"length")->size()&&!pieces.empty()){
            auto bottom=pieces.back();pieces.pop_back();const auto pair=context.array(2);context.set(options::Value(pair),key(u"0"),options::Value(u"bottom"));context.set(options::Value(pair),key(u"1"),options::Value(bottom));(void)context.arrayPush(requireObject(context,frame,"push"),std::array<options::Value,1>{options::Value(pair)});
            if(!pieces.empty())context.set(options::Value(definition),key(u"rowlines"),options::Value(joinUnits(pieces,u" ")));else context.erase(definition,key(u"rowlines"));
        }else if(mmlArray(table,"length")->size()>0&&pieces.size()<mmlArray(table,"length")->size()-1){context.set(options::Value(definition),key(u"rowlines"),options::Value(valueText(context,lineValue)+u" none"));}
    }
    if(stack_property_host::truthy(this->getProperty("rowspacing"))){
        auto raw=context.get(arraydef,key(u"rowspacing"));auto text=valueText(context,raw);std::vector<std::u16string> rows;std::size_t start=0;
        for(std::size_t i=0;i<=text.size();++i)if(i==text.size()||text[i]==u' '){rows.emplace_back(text.substr(start,i-start));start=i+1;}
        while(rows.size()<mmlArray(table,"length")->size())rows.push_back(property_host::string(stack_property_host::scalar(this->getProperty("rowspacing")))+u"em");
        context.set(options::Value(definition),key(u"rowspacing"),options::Value(joinUnits(rows,u" ")));
    }
}
void ArrayItem::addRowSpacing(std::u16string spacing){
    auto& context=runtime().context();auto definition=requireObject(context,arraydef,"rowspacing");const auto value=context.get(arraydef,key(u"rowspacing"));if(!options::truthy(value))return;
    auto text=valueText(context,value);std::vector<std::u16string> rows;std::size_t start=0;
    for(std::size_t i=0;i<=text.size();++i)if(i==text.size()||text[i]==u' '){rows.emplace_back(text.substr(start,i-start));start=i+1;}
    if(!stack_property_host::truthy(this->getProperty("rowspacing"))){const auto dim=UnitUtil::instance().dimen2em(Property(property_host::debugText(rows.empty()?std::u16string{}:rows[0])));(void)this->setProperty("rowspacing",StackProperty(dim));}
    const auto rowspacing=this->getProperty("rowspacing");
    const double spacingValue=property_host::number(stack_property_host::scalar(rowspacing));
    while(rows.size()<mmlArray(table,"length")->size())rows.push_back(u"0em");
    const auto converted=UnitUtil::instance().em(std::max(0.0,spacingValue+UnitUtil::instance().dimen2em(Property(property_host::debugText(spacing)))));
    if(mmlArray(table,"length")->size()){const auto index=mmlArray(table,"length")->size()-1;if(rows.size()<=index)rows.resize(index+1);rows[index]=std::u16string(converted.begin(),converted.end());}
    context.set(options::Value(definition),key(u"rowspacing"),options::Value(joinUnits(rows,u" ")));
}
std::string EqnArrayItem::kind()const{return "eqnarray";}
void EqnArrayItem::initializeEqnArrayItem(FunctionArgs args){
    maxrow=0;
    tags(factory()).start(optionValue(runtime(),args,0),optionValue(runtime(),args,2),optionValue(runtime(),args,1));
}
void EqnArrayItem::EndEntry(){
    auto& context=runtime().context();const auto alignment=context.get(arraydef,key(u"columnalign"));
    if(std::holds_alternative<options::Undefined>(alignment.data)||std::holds_alternative<options::Null>(alignment.data))
        property_host::typeError("Cannot read properties of undefined (reading 'split')");
    const auto calign=splitSpace(valueText(context,alignment));
    const auto align=mmlArray(row,"length")->size()&&!calign.empty()?calign[mmlArray(row,"length")->size()%calign.size()]:u"right";
    if(align!=u"right")ParseUtil::fixInitialMO(parseOptions(factory()),*this);
    ArrayItem::EndEntry();
}
void EqnArrayItem::EndRow(){
    if(mmlArray(row,"length")->size()>property_host::number(maxrow))maxrow=Property(static_cast<double>(mmlArray(row,"length")->size()));
    const auto tag=tags(factory()).getTag();
    if(tag.isNode()){
        unshift(mmlArray(row,"length"),tag);
        (void)this->setProperty("isLabeled",StackProperty(true));
    }
    tags(factory()).clearTag();
    ArrayItem::EndRow();
}
void EqnArrayItem::EndTable(){
    ArrayItem::EndTable();
    tags(factory()).end();
    const auto maximum=property_host::number(maxrow);
    extendArray(u"columnalign",maximum);extendArray(u"columnwidth",maximum);
    extendArray(u"columnspacing",maximum-1);extendArray(u"data-break-align",maximum);
    addIndentshift();
}
void EqnArrayItem::extendArray(std::u16string name,double maximum){
    auto& context=runtime().context();const auto value=context.get(arraydef,key(name));
    if(!options::truthy(value))return;
    auto repeat=splitSpace(valueText(context,value));auto columns=repeat;
    if(columns.size()>1){
        while(static_cast<double>(columns.size())<maximum)columns.insert(columns.end(),repeat.begin(),repeat.end());
        const auto end=maximum<=0?std::size_t(0):std::min(columns.size(),static_cast<std::size_t>(maximum));
        columns.resize(end);context.set(arraydef,key(name),options::Value(joinUnits(columns,u" ")));
    }
}
void EqnArrayItem::addIndentshift(){
    auto& context=runtime().context();const auto value=context.get(arraydef,key(u"columnalign"));
    if(std::holds_alternative<options::Undefined>(value.data)||std::holds_alternative<options::Null>(value.data))
        property_host::typeError("Cannot read properties of undefined (reading 'split')");
    const auto align=splitSpace(valueText(context,value));std::u16string prev;
    for(std::size_t i=0;i<align.size();++i){
        if(align[i]==u"left"&&i>0){
            const std::u16string indent=prev==u"center"?u".7em":u"2em";
            for(std::size_t rowIndex=0;rowIndex<mmlArray(table,"length")->size();++rowIndex){
                const auto rowNode=mmlArray(table,"length")->get(rowIndex);const auto* native=rowNode.nodeOrNull();
                if(!native)continue;
                const auto cellIndex=native->isKind("mlabeledtr")?i+1:i;
                const auto cells=native->childNodes;
                const auto cell=cellIndex<cells->size()?cells->get(cellIndex):mml::NodeValue(Missing{});
                if(!cell.isNode())continue;
                const auto cellNode=cell.nodeOrNull();const auto contents=cellNode->childNodes;
                const auto content=contents->get(0);
                const auto contentChildren=content.requireNode("childNodes")->childNodes;
                const auto style=node(*this,"mstyle",contentChildren,PropertyList{{"indentshift",property_host::debugText(indent)}});
                cellNode->childNodes->get(0).requireNodeForWrite("childNodes")->childNodes=std::make_shared<tree::ChildArray<mml::MmlNode>>();
                cellNode->appendChild(style);
            }
        }
        prev=align[i];
    }
}
void MstyleItem::initializeMstyleItem(Property attr,Property name){attrList=std::move(attr);(void)setProperty("name",StackProperty(std::move(name)));}
std::string MstyleItem::kind()const{return "mstyle";}
CheckType MstyleItem::checkItem(BaseItem* raw){BaseItem& item=base_item_host::item(raw,"isKind");if(item.isKind("end")&&item.getName()==this->getName()){const auto content=this->toMml();const auto mml=node(*this,"mstyle",childArray({content}),attrList);return checked({mml});}return BeginItem::checkItem(raw);}
void EquationItem::initializeEquationItem(FunctionArgs args){
    tags(factory()).start(Value(u"equation"),Value(true),optionValue(parseOptions(factory()).runtime(),args,0));
}
std::string EquationItem::kind()const{return "equation";}bool EquationItem::isOpen()const{return true;}
CheckType EquationItem::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"isKind");
    if(item.isKind("end")){
        const auto result=this->toMml();
        const auto tag=tags(factory()).getTag();
        tags(factory()).end();
        const auto tagged=tag.isNode()?callEnTag(tags(factory()),parseOptions(factory()).runtime(),result,tag):result;
        return checked({tagged,asStackItem(factory(),raw,"isKind")});
    }
    if(item.isKind("stop"))throw TexError(u"EnvMissingEnd",u"Missing \\end{%1}",
        std::array<Property,1>{stack_property_host::scalar(this->getName())});
    return BaseItem::checkItem(raw);
}

}
