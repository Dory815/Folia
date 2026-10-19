// MathJax 4.1.3 input/tex/ParseUtil.ts source methods; Apache-2.0.
module folia.math.input.tex.parser;
import :runtime;
import :parse_options;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.node_util;
import folia.math.input.tex.node_factory;
import folia.math.input.tex.unit_util;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.stack_item;
import folia.math.input.tex.stack_item_factory;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.util.js_numeric_input;
import folia.math.util.utf16;
import folia.math.util.regexp_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
using Node=mml::NodeValue;
Key key(std::u16string_view text){return Key(std::u16string(text));}
std::u16string text(const Property& p){return property_host::string(p);}
std::u16string text(Context& cx,const Value& v){
    if(const auto* s=std::get_if<std::u16string>(&v.data))return *s;
    if(std::holds_alternative<Undefined>(v.data))return u"undefined";
    if(std::holds_alternative<Null>(v.data))return u"null";
    if(const auto* b=std::get_if<bool>(&v.data))return *b?u"true":u"false";
    if(const auto* n=std::get_if<double>(&v.data))return property_host::text(js_number::toString(*n));
    if(std::holds_alternative<SymbolRef>(v.data))throw Exception("TypeError",u"Cannot convert a Symbol value to a string");
    const Value method=cx.get(v,key(u"toString"));
    const auto* fn=std::get_if<ObjectRef>(&method.data);
    if(fn&&cx.kind(*fn)==ObjectKind::function){const Value result=cx.call(*fn,v,{});if(const auto* s=std::get_if<std::u16string>(&result.data))return *s;}
    throw Exception("TypeError",u"Cannot convert object to primitive value");
}
Property propertyFromOptions(const Value& value){
    if(std::holds_alternative<Undefined>(value.data))return Missing{};
    if(std::holds_alternative<Null>(value.data))return PropertyNull{};
    if(const auto* b=std::get_if<bool>(&value.data))return *b;
    if(const auto* n=std::get_if<double>(&value.data))return *n;
    if(const auto* s=std::get_if<std::u16string>(&value.data))return property_host::debugText(*s);
    throw tree::FactoryError("Unimplemented",u"KeyValueDef callback result needs the canonical Property identity binding");
}
Value optionArg(std::span<const Value> args,std::size_t i){return i<args.size()?args[i]:Value(Undefined{});}
template<class T>T& requireShared(const ParserShared<T>& value,std::string_view member){
    if(const auto* p=std::get_if<std::shared_ptr<T>>(&value);p&&*p)return **p;
    property_host::typeError(std::string("Cannot read properties of undefined (reading '")+std::string(member)+"')");
}
ParseOptions& configuration(TexParser& parser){
    if(const auto* p=std::get_if<ParseOptions*>(&parser.configuration);p&&*p)return **p;
    property_host::typeError("Cannot read properties of undefined (reading 'configuration')");
}
Stack& stack(TexParser& parser){return requireShared(parser.stack,"env");}
NodeFactory& nodeFactory(ParseOptions& config){return config.nodeFactoryOwner();}
Property currentEnv(ParseOptions& config){
    const auto current=config.parser();
    if(const auto* parser=std::get_if<std::shared_ptr<TexParser>>(&current);parser&&*parser)return stack(**parser).env();
    property_host::typeError("Cannot read properties of undefined (reading 'stack')");
}
Node makeNode(ParseOptions& config,std::string_view kind,mml::Children children=std::make_shared<tree::ChildArray<mml::MmlNode>>(),
              Property definition=PropertyList{},Node textNode=Missing{}){
    std::vector<std::any> args;
    args.emplace_back(Property(std::string(kind)));
    args.emplace_back(std::move(children));
    args.emplace_back(std::move(definition));
    args.emplace_back(std::move(textNode));
    return nodeFactory(config).create("node",args);
}
Node makeToken(TexParser& parser,std::string_view kind,Property definition=PropertyList{},Property value=Property("")){
    const std::array<std::any,1> textArgs{std::any(value)};
    const Node textNode=nodeFactory(configuration(parser)).create("text",textArgs);
    return makeNode(configuration(parser),kind,std::make_shared<tree::ChildArray<mml::MmlNode>>(),std::move(definition),textNode);
}
Node parseReal(std::u16string source,Property env,ParseOptions& config){return TexParser::make(std::move(source),std::move(env),config)->mml();}
Property booleanProperty(bool value){return Property(value);}
void throwError(std::u16string id,std::u16string_view message,std::span<const Property> rest={}){throw TexError(std::move(id),message,rest);}
std::u16string trimSpaces(std::u16string value){
    const auto ws=[](char16_t c){return c==u' '||c==u'\t'||c==u'\n'||c==u'\r'||c==u'\v'||c==u'\f'||c==0x00A0||c==0x1680||(c>=0x2000&&c<=0x200A)||c==0x2028||c==0x2029||c==0x202F||c==0x205F||c==0x3000||c==0xFEFF;};
    std::size_t a=0,b=value.size();while(a<b&&ws(value[a]))++a;while(b>a&&ws(value[b-1]))--b;return value.substr(a,b-a);
}
std::u16string removeBraces(std::u16string value,unsigned count){
    if(!count){
        auto trimmed=trimSpaces(value);std::size_t end=trimmed.size();
        while(end>0&&trimmed[end-1]==u' '){std::size_t slash=end-1;while(slash>0&&trimmed[slash-1]==u'\\')--slash;
            const std::size_t n=end-slash;if(n%2==1){end=slash+n-1;break;}end=slash;}
        trimmed.resize(end);return trimmed;
    }
    while(count--){auto t=trimSpaces(value);value=t.size()>=2?t.substr(1,t.size()-2):std::u16string{};}
    return value;
}
struct ValueRead{std::u16string value,end,rest;};
ValueRead readValue(std::u16string_view input,std::u16string_view ends,bool l3=false,bool drop=false){
    const std::size_t length=input.size();std::size_t index=0,start=0;int braces=0;std::u16string value;bool countBraces=true;
    while(index<length){const char16_t c=input[index++];
        if(c==u'\\'){value.push_back(c);if(index<length)value.push_back(input[index++]);countBraces=false;continue;}
        if(c==u' '){value.push_back(c);continue;}
        if(c==u'{'){if(countBraces)++start;++braces;value.push_back(c);continue;}
        if(c==u'}'){if(!braces)throwError(u"ExtraCloseMissingOpen",u"Extra close brace or missing open brace");--braces;countBraces=false;value.push_back(c);continue;}
        if(!braces&&ends.find(c)!=std::u16string_view::npos){const auto cleaned=removeBraces(std::move(value),l3?std::min(1U,static_cast<unsigned>(start)):static_cast<unsigned>(start));
            return {cleaned,std::u16string(1,c),std::u16string(input.substr(index))};}
        if(start>static_cast<std::size_t>(braces))start=static_cast<std::size_t>(braces);countBraces=false;value.push_back(c);
    }
    if(braces)throwError(u"ExtraOpenMissingClose",u"Extra open brace or missing close brace");
    if(drop&&start)return {{},{},removeBraces(std::move(value),1)};
    return {removeBraces(std::move(value),l3?std::min(1U,static_cast<unsigned>(start)):static_cast<unsigned>(start)),{},std::u16string(input.substr(index))};
}
PropertyList readKeyval(std::u16string_view input,bool l3){
    PropertyList out;std::u16string rest(input);bool drop=true;
    while(!rest.empty()){
        auto keyRead=readValue(rest,u"=,",l3,drop);drop=false;const std::u16string name=std::move(keyRead.value);rest=std::move(keyRead.rest);
        if(keyRead.end==u"="){
            auto valueRead=readValue(rest,u",",l3,false);rest=std::move(valueRead.rest);Property val=Property(property_host::debugText(valueRead.value));
            if(valueRead.value==u"true")val=true;else if(valueRead.value==u"false")val=false;out.set(property_host::debugText(name),std::move(val));
        }else if(!name.empty())out.set(property_host::debugText(name),true);
    }
    return out;
}
std::u16string readPropertyText(const Property& p){return p.missing()?u"undefined":text(p);}
Property childAt(mml::Children children,std::size_t i){
    if(!children)property_host::typeError("Cannot read properties of null (reading '"+std::to_string(i)+"')");
    const auto value=children->get(i);
    if(value.isNode())return Missing{};
    return value.kind()==tree::ChildKind::Null?Property(PropertyNull{}):Property(Missing{});
}
}

KeyValueDef::KeyValueDef(std::u16string n,FunctionRef v,FunctionRef c):name(std::move(n)),verify(std::move(v)),convert(std::move(c)){}
KeyValueType KeyValueDef::oneof(ParserRuntime&,std::span<const std::u16string> values){
    std::vector<std::u16string> allowed(values.begin(),values.end());
    auto verify=std::make_shared<Function>();verify->call=[allowed](const FunctionReceiver&,FunctionArgs args)->std::any{
        const auto* value=args.empty()?nullptr:std::any_cast<std::u16string>(&args[0]);return value&&std::find(allowed.begin(),allowed.end(),*value)!=allowed.end();
    };
    auto convert=std::make_shared<Function>();convert->call=[](const FunctionReceiver&,FunctionArgs args)->std::any{
        if(args.empty())return std::u16string{};if(const auto* value=std::any_cast<std::u16string>(&args[0]))return *value;return std::u16string{};
    };
    return std::make_shared<KeyValueDef>(u"string",std::move(verify),std::move(convert));
}

void ParseUtil::initializeModule(ParserRuntime& runtime){
    Context& cx=runtime.context();
    const ObjectRef ctor=cx.nativeConstructor(u"KeyValueDef",[&runtime](Context& context,std::span<const Value> args)->ObjectRef{
        const Value name=optionArg(args,0),verifyValue=optionArg(args,1),convertValue=optionArg(args,2);
        const auto function=[&](const Value& value){const FunctionValue fv=runtime.functions().functionValue(value);const auto* f=std::get_if<FunctionRef>(&fv);if(!f||!*f)throw Exception("TypeError",u"KeyValueDef callbacks must be functions");return *f;};
        const auto def=std::make_shared<KeyValueDef>(text(context,name),function(verifyValue),function(convertValue));
        const ObjectRef classObject=std::get<ObjectRef>(runtime.keyValueClass.data);
        const ObjectRef instance=runtime.keyValueIdentities.instance(def,classObject);
        context.define(instance,key(u"name"),Descriptor::data(name));context.define(instance,key(u"verify"),Descriptor::data(verifyValue));context.define(instance,key(u"convert"),Descriptor::data(convertValue));return instance;
    });
    cx.define(ctor,key(u"length"),Descriptor::data(Value(3),false,false,true));
    runtime.keyValueClass=Value(ctor);
    const ObjectRef oneof=cx.function(u"oneof",[&runtime](Context& context,Value,std::span<const Value> args)->Value{
        std::vector<std::u16string> values;values.reserve(args.size());for(const auto& v:args)values.push_back(text(context,v));
        auto def=KeyValueDef::oneof(runtime,values);const ObjectRef classObject=std::get<ObjectRef>(runtime.keyValueClass.data);
        const ObjectRef instance=runtime.keyValueIdentities.instance(def,classObject);
        // The class callbacks are canonical FunctionRef bodies. Install actual
        // callable Context handles whose wrappers invoke those same bodies.
        auto bind=[&](FunctionRef fn,std::u16string label){
            OptionCallBinding binding;
            binding.receiver=[](Context&,Value){return FunctionReceiver(Missing{});};
            binding.arguments=[](Context& cx,std::span<const Value> values){std::vector<std::any> result;for(const auto& v:values)result.emplace_back(text(cx,v));return result;};
            binding.result=[](Context&,const std::any& result)->Value{if(const auto* b=std::any_cast<bool>(&result))return Value(*b);if(const auto* n=std::any_cast<double>(&result))return Value(*n);if(const auto* s=std::any_cast<std::u16string>(&result))return Value(*s);return Value(Undefined{});};
            return runtime.functions().bind(std::move(fn),std::move(binding),std::move(label));
        };
        context.define(instance,key(u"name"),Descriptor::data(Value(u"string")));
        context.define(instance,key(u"verify"),Descriptor::data(bind(def->verify,u"verify")));
        context.define(instance,key(u"convert"),Descriptor::data(bind(def->convert,u"convert")));
        return Value(instance);
    });
    cx.define(oneof,key(u"length"),Descriptor::data(Value(0),false,false,true));
    cx.define(ctor,key(u"oneof"),Descriptor::data(Value(oneof)));
    const auto bind=[&](FunctionRef fn,std::u16string label){
        OptionCallBinding binding;
        binding.receiver=[](Context&,Value){return FunctionReceiver(Missing{});};
        binding.arguments=[](Context& context,std::span<const Value> values){std::vector<std::any> out;for(const auto& v:values)out.emplace_back(text(context,v));return out;};
        binding.result=[](Context&,const std::any& value)->Value{
            if(const auto* b=std::any_cast<bool>(&value))return Value(*b);
            if(const auto* n=std::any_cast<double>(&value))return Value(*n);
            if(const auto* s=std::any_cast<std::u16string>(&value))return Value(*s);
            return Value(Undefined{});
        };
        return runtime.functions().bind(std::move(fn),std::move(binding),std::move(label));
    };
    const auto makeDef=[&](std::u16string name,FunctionRef verify,FunctionRef convert){
        auto def=std::make_shared<KeyValueDef>(name,std::move(verify),std::move(convert));
        const ObjectRef instance=runtime.keyValueIdentities.instance(def,ctor);
        cx.define(instance,key(u"name"),Descriptor::data(Value(name)));
        cx.define(instance,key(u"verify"),Descriptor::data(bind(def->verify,u"verify")));
        cx.define(instance,key(u"convert"),Descriptor::data(bind(def->convert,u"convert")));
        return instance;
    };
    const auto fn=[](auto callback){auto result=std::make_shared<Function>();result->call=std::move(callback);return result;};
    const auto boolVerifier=fn([](const FunctionReceiver&,FunctionArgs args)->std::any{const auto* s=args.empty()?nullptr:std::any_cast<std::u16string>(&args[0]);return s&&(*s==u"true"||*s==u"false");});
    const auto boolConverter=fn([](const FunctionReceiver&,FunctionArgs args)->std::any{const auto* s=args.empty()?nullptr:std::any_cast<std::u16string>(&args[0]);return s&&*s==u"true";});
    const auto numberVerifier=fn([](const FunctionReceiver&,FunctionArgs args)->std::any{const auto* s=args.empty()?nullptr:std::any_cast<std::u16string>(&args[0]);if(!s)return false;static const std::u16string pattern=u"^[-+]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:e[-+]?\\d+)?$";return regexp_host::RegExp(pattern,u"i").exec(*s).has_value();});
    const auto numberConverter=fn([](const FunctionReceiver&,FunctionArgs args)->std::any{const auto* s=args.empty()?nullptr:std::any_cast<std::u16string>(&args[0]);return s?js_input::number(*s):std::numeric_limits<double>::quiet_NaN();});
    const auto integerVerifier=fn([](const FunctionReceiver&,FunctionArgs args)->std::any{const auto* s=args.empty()?nullptr:std::any_cast<std::u16string>(&args[0]);if(!s)return false;static const std::u16string pattern=u"^[-+]?\\d+$";return regexp_host::RegExp(pattern).exec(*s).has_value();});
    const auto integerConverter=fn([](const FunctionReceiver&,FunctionArgs args)->std::any{const auto* s=args.empty()?nullptr:std::any_cast<std::u16string>(&args[0]);return s?js_input::parseInt(*s,js_input::IntegerRadix::Decimal):std::numeric_limits<double>::quiet_NaN();});
    const auto anyVerifier=fn([](const FunctionReceiver&,FunctionArgs)->std::any{return true;});
    const auto stringConverter=fn([](const FunctionReceiver&,FunctionArgs args)->std::any{const auto* s=args.empty()?nullptr:std::any_cast<std::u16string>(&args[0]);return s?*s:std::u16string{};});
    const auto dimenVerifier=fn([](const FunctionReceiver&,FunctionArgs args)->std::any{const auto* s=args.empty()?nullptr:std::any_cast<std::u16string>(&args[0]);return s&&!std::holds_alternative<PropertyNull>(UnitUtil::instance().matchDimen(Property(property_host::debugText(*s)))[0].value);});
    auto types=cx.object();
    const std::array<std::pair<std::u16string,ObjectRef>,5> entries{{
        {u"boolean",makeDef(u"boolean",boolVerifier,boolConverter)},
        {u"number",makeDef(u"number",numberVerifier,numberConverter)},
        {u"integer",makeDef(u"integer",integerVerifier,integerConverter)},
        {u"string",makeDef(u"string",anyVerifier,stringConverter)},
        {u"dimen",makeDef(u"dimen",dimenVerifier,stringConverter)}}};
    for(const auto& [name,value]:entries)cx.define(types,key(name),Descriptor::data(Value(value)));
    runtime.KeyValueTypes=Value(types);
}

std::u16string ParseUtil::cols(std::span<const double> widths){
    std::u16string result;for(const double width:widths){if(!result.empty())result.push_back(u' ');result+=property_host::text(UnitUtil::instance().em(width));}return result;
}

mml::NodeValue ParseUtil::fenced(ParseOptions& config,std::u16string open,mml::NodeValue enclosed,std::u16string close,std::u16string big,Property color){
    return fenced(config,Property(property_host::debugText(open)),std::move(enclosed),
                  Property(property_host::debugText(close)),std::move(big),std::move(color));
}

mml::NodeValue ParseUtil::fenced(ParseOptions& config,Property open,mml::NodeValue enclosed,Property close,std::u16string big,Property color){
    auto row=makeNode(config,"mrow",std::make_shared<tree::ChildArray<mml::MmlNode>>(),PropertyList{{"open",open},{"close",close},{"texClass",Property(static_cast<double>(mml::TEXCLASS.INNER))}});
    Node left;
    if(!big.empty())left=parseReal(u"\\"+big+u"l"+property_host::string(open),currentEnv(config),config);
    else left=makeNode(config,"mo",std::make_shared<tree::ChildArray<mml::MmlNode>>(),PropertyList{{"fence",true},{"stretchy",true},{"symmetric",true},{"texClass",static_cast<double>(mml::TEXCLASS.OPEN)}},NodeFactory::createText(nodeFactory(config),open));
    auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(left);children->push_back(enclosed);NodeUtil::instance().appendChildren(row,children);
    Node right;
    if(!big.empty())right=parseReal(u"\\"+big+u"r"+property_host::string(close),currentEnv(config),config);
    else right=makeNode(config,"mo",std::make_shared<tree::ChildArray<mml::MmlNode>>(),PropertyList{{"fence",true},{"stretchy",true},{"symmetric",true},{"texClass",static_cast<double>(mml::TEXCLASS.CLOSE)}},NodeFactory::createText(nodeFactory(config),close));
    if(property_host::truthy(color))right.requireNode("attributes")->attributes()->set("mathcolor",std::move(color));
    NodeUtil::instance().appendChildren(row,std::make_shared<tree::ChildArray<mml::MmlNode>>(std::initializer_list<mml::NodeValue>{right}));
    return row;
}

mml::NodeValue ParseUtil::fixedFence(ParseOptions& config,std::u16string open,mml::NodeValue enclosed,std::u16string close){
    auto row=makeNode(config,"mrow",std::make_shared<tree::ChildArray<mml::MmlNode>>(),PropertyList{{"open",Property(property_host::debugText(open))},{"close",Property(property_host::debugText(close))},{"texClass",static_cast<double>(mml::TEXCLASS.ORD)}});
    if(!open.empty())NodeUtil::instance().appendChildren(row,std::make_shared<tree::ChildArray<mml::MmlNode>>(std::initializer_list<mml::NodeValue>{mathPalette(config,open,u"l")}));
    if(NodeUtil::instance().isType(enclosed,"mrow"))NodeUtil::instance().appendChildren(row,NodeUtil::instance().getChildren(enclosed));
    else NodeUtil::instance().appendChildren(row,std::make_shared<tree::ChildArray<mml::MmlNode>>(std::initializer_list<mml::NodeValue>{enclosed}));
    if(!close.empty())NodeUtil::instance().appendChildren(row,std::make_shared<tree::ChildArray<mml::MmlNode>>(std::initializer_list<mml::NodeValue>{mathPalette(config,close,u"r")}));
    return row;
}

mml::NodeValue ParseUtil::mathPalette(ParseOptions& config,std::u16string fence,std::u16string side){
    if(fence==u"{"||fence==u"}")fence=u"\\"+fence;
    const std::u16string d=u"{\\bigg"+side+u" "+fence+u"}",t=u"{\\big"+side+u" "+fence+u"}";
    return parseReal(u"\\mathchoice"+d+t+t+t,Property(PropertyList{}),config);
}

void ParseUtil::fixInitialMO(ParseOptions& config,mml::Children nodes){
    if(!nodes)return;
    const auto length=nodes->size();
    for(std::size_t i=0;i<length;++i){
        auto child=nodes->get(i);if(!child.isNode())continue;
        const auto node=Node(child.nodeOrNull());
        if(NodeUtil::instance().isType(node,"mspace"))continue;
        if(NodeUtil::instance().isType(node,"TeXAtom")){
            const auto atomChildren=NodeUtil::instance().getChildren(node);
            const auto first=atomChildren&&atomChildren->size()?atomChildren->get(0):mml::NodeValue(Missing{});
            if(!first.isNode()||!NodeUtil::instance().getChildren(first).get()||NodeUtil::instance().getChildren(first)->empty())continue;
        }
        if(NodeUtil::instance().isEmbellished(node)||(NodeUtil::instance().isType(node,"TeXAtom")&&NodeUtil::instance().getTexClass(node)==Property(static_cast<double>(mml::TEXCLASS.REL)))){
            auto mi=makeNode(config,"mi");nodes->splice(0,0,std::array<mml::NodeValue,1>{mi});
        }
        break;
    }
}

void ParseUtil::fixInitialMO(ParseOptions& config,BaseItem& owner){
    if(!owner.hasSourceNodesOwner())return fixInitialMO(config,owner.sourceNodes());
    auto& access=owner.sourceAccess();const options::Value array=owner.sourceNodesPropertyValue();
    const options::Value length=access.get(array,options::Key(u"length"));
    for(double index=0;index<access.toNumber(length);index+=1.0){
        const options::Value raw=access.getIndex(array,index);
        if(!access.truthy(raw))continue;
        const mml::NodeValue child=owner.sourceDecodeNode(raw);
        const auto node=Node(child);
        if(NodeUtil::instance().isType(node,"mspace"))continue;
        if(NodeUtil::instance().isType(node,"TeXAtom")){
            const auto atomChildren=NodeUtil::instance().getChildren(node);
            const auto first=atomChildren&&atomChildren->size()?atomChildren->get(0):mml::NodeValue(Missing{});
            if(!first.isNode()||!NodeUtil::instance().getChildren(first).get()||NodeUtil::instance().getChildren(first)->empty())continue;
        }
        if(NodeUtil::instance().isEmbellished(node)||(NodeUtil::instance().isType(node,"TeXAtom")&&NodeUtil::instance().getTexClass(node)==Property(static_cast<double>(mml::TEXCLASS.REL)))){
            const auto mi=makeNode(config,"mi");
            const auto unshift=access.get(array,options::Key(u"unshift"));
            const std::array<options::Value,1> args{access.encodeNode(mi)};(void)access.callMember(unshift,array,args);
        }
        break;
    }
}

mml::NodeValue ParseUtil::internalText(TexParser& parser,std::u16string value,Property def){
    std::u16string normalized;normalized.reserve(value.size());
    for(std::size_t i=0;i<value.size();){
        if(value[i]==u'\n'){normalized.push_back(u' ');while(i<value.size()&&value[i]==u'\n')++i;}
        else normalized.push_back(value[i++]);
    }
    std::size_t first=0;while(first<normalized.size()&&normalized[first]==u' ')++first;
    if(first)normalized.replace(0,first,1,u'\u00A0');
    std::size_t last=normalized.size();while(last&&normalized[last-1]==u' ')--last;
    if(last<normalized.size())normalized.replace(last,normalized.size()-last,1,u'\u00A0');
    const std::array<std::any,1> textArgs{std::any(Property(property_host::debugText(normalized)))};
    const mml::NodeValue textNode=parser.create(u"text",textArgs);
    const std::array<std::any,4> nodeArgs{
        std::any(Property(std::string("mtext"))),
        std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>()),
        std::any(std::move(def)),
        std::any(textNode)};
    return parser.create(u"node",nodeArgs);
}

void ParseUtil::checkMovableLimits(mml::NodeValue base){
    Property symbol=PropertyNull{};
    if(NodeUtil::instance().isType(base,"mo"))symbol=NodeUtil::instance().getForm(base);
    const Property movable=NodeUtil::instance().getProperty(base,"movablelimits");
    bool symbolMovable=false;
    if(property_host::truthy(symbol)){
        const Property tuple=property_host::get(symbol,"3");
        if(property_host::truthy(tuple))symbolMovable=property_host::truthy(property_host::get(tuple,"movablelimits"));
    }
    if(property_host::truthy(movable)||symbolMovable)
        NodeUtil::instance().setProperties(base,Property(PropertyList{{"movablelimits",false}}));
}

ArrayItem& ParseUtil::setArrayAlign(ArrayItem& array,std::u16string align,TexParser* parser){
    if(!parser)align=trimSpaces(align);
    if(align==u"t"||align==u"b"||align==u"c"){
        auto& cx=array.runtime().context();
        const Value def=array.arraydef;
        if(align==u"t")cx.set(def,key(u"align"),Value(u"baseline 1"));
        else if(align==u"b")cx.set(def,key(u"align"),Value(u"baseline -1"));
        else cx.set(def,key(u"align"),Value(u"axis"));
    }else if(!align.empty()){
        if(parser){
            parser->setString(u"["+align+u"]"+parser->string().substr(static_cast<std::size_t>(std::max(0.0,parser->i))));
            parser->i=0;
        }else{
            array.runtime().context().set(array.arraydef,key(u"align"),Value(align));
        }
    }
    return array;
}

std::u16string ParseUtil::addArgs(TexParser& parser,std::u16string left,std::u16string right){
    const auto alpha=[](char16_t c){return(c>=u'a'&&c<=u'z')||(c>=u'A'&&c<=u'Z');};
    if(!right.empty()&&alpha(right.front())){
        std::size_t start=left.size();while(start&&alpha(left[start-1]))--start;
        std::size_t slash=start;while(slash&&left[slash-1]==u'\\')--slash;
        const std::size_t count=start-slash;
        if(count%2==1&&(slash==0||left[slash-1]!=u'\\'))left.push_back(u' ');
    }
    auto& config=configuration(parser);Context& cx=config.runtime().context();
    const Value max=cx.get(config.options,key(u"maxBuffer"));
    if(static_cast<double>(left.size()+right.size())>cx.toNumber(max))
        throwError(u"MaxBufferSize",u"MathJax internal buffer size exceeded; is there a recursive macro call?");
    left+=right;return left;
}

std::u16string ParseUtil::addArgs(TexParser& parser,Property rawFirst,std::u16string second){
    // Macro calls pass the original first operand to ParseUtil.addArgs.  The
    // source reads second.match first, conditionally reads/calls first.match,
    // then reads both lengths before maxBuffer and finally performs `+`.
    // Preserve the ordinary string path (the real TeX caller) and retain raw
    // primitive behavior for malformed-but-observable macro definitions.
    if(const auto* source=std::get_if<std::string>(&rawFirst.value)){
        auto left=property_host::text(*source);
        static const std::u16string alphaPattern=u"^[a-z]";
        static const std::u16string commandPattern=u"(^|[^\\\\])(\\\\\\\\)*\\\\[a-z]+$";
        if(!second.empty()&&regexp_host::RegExp(alphaPattern,u"i").exec(second)){
            if(regexp_host::RegExp(commandPattern,u"i").exec(left))left.push_back(u' ');
        }
        auto& config=configuration(parser);auto& cx=config.runtime().context();
        const auto max=cx.get(config.options,key(u"maxBuffer"));
        if(static_cast<double>(left.size()+second.size())>cx.toNumber(max))
            throwError(u"MaxBufferSize",u"MathJax internal buffer size exceeded; is there a recursive macro call?");
        left+=second;return left;
    }

    auto& config=configuration(parser);auto& runtime=config.runtime();auto& cx=runtime.context();
    const auto first=runtime.properties.to(rawFirst);
    static const std::u16string alphaPattern=u"^[a-z]";
    if(!second.empty()&&regexp_host::RegExp(alphaPattern,u"i").exec(second)){
        const auto match=cx.get(first,key(u"match"));
        const auto* function=std::get_if<ObjectRef>(&match.data);
        if(!function||cx.kind(*function)!=ObjectKind::function)
            property_host::typeError("s1.match is not a function");
        static const std::u16string commandPattern=u"(^|[^\\\\])(\\\\\\\\)*\\\\[a-z]+$";
        const auto regexp=runtime.regexp().create(commandPattern,u"i");
        const std::array<Value,1> arguments{Value(regexp)};
        const auto matched=cx.call(*function,first,arguments);
        if(options::truthy(matched)){
            const auto firstLength=cx.get(first,key(u"length"));
            const Value secondLength=static_cast<double>(second.size());
            const auto max=cx.get(config.options,key(u"maxBuffer"));
            const double total=cx.toNumber(firstLength)+cx.toNumber(secondLength);
            if(total>cx.toNumber(max))
                throwError(u"MaxBufferSize",u"MathJax internal buffer size exceeded; is there a recursive macro call?");
            return text(cx,first)+u" "+second;
        }
    }
    const auto firstLength=cx.get(first,key(u"length"));
    const Value secondLength=static_cast<double>(second.size());
    const auto max=cx.get(config.options,key(u"maxBuffer"));
    const double total=cx.toNumber(firstLength)+cx.toNumber(secondLength);
    if(total>cx.toNumber(max))
        throwError(u"MaxBufferSize",u"MathJax internal buffer size exceeded; is there a recursive macro call?");
    return text(cx,first)+second;
}

std::u16string ParseUtil::substituteArgs(TexParser& parser,options::Value args,std::u16string source){
    std::u16string textPart,result;Context& cx=configuration(parser).runtime().context();
    for(std::size_t i=0;i<source.size();){
        char16_t c=source[i++];
        if(c==u'\\'){textPart.push_back(c);if(i<source.size())textPart.push_back(source[i++]);}
        else if(c==u'#'){
            c=i<source.size()?source[i++]:0;
            if(c==u'#')textPart.push_back(c);
            else if(c<u'1'||c>u'9'||static_cast<double>(c-u'0')>cx.toNumber(cx.get(args,key(u"length"))))
                throwError(u"IllegalMacroParam",u"Illegal macro parameter reference");
            else{
                result=addArgs(parser,addArgs(parser,std::move(result),std::move(textPart)),text(cx,cx.get(args,key(property_host::text(js_number::toString(static_cast<double>(c-u'0'-1)))))));
                textPart.clear();
            }
        }else textPart.push_back(c);
    }
    return addArgs(parser,std::move(result),std::move(textPart));
}

std::u16string ParseUtil::substituteArgs(TexParser& parser,options::Value args,Property rawSource){
    // Source reads `str.length` and then calls `str.charAt(i++)` without
    // coercing str. Strings use the fast typed path; other values retain the
    // original receiver and property/call behavior.
    if(const auto* bytes=std::get_if<std::string>(&rawSource.value))
        return substituteArgs(parser,std::move(args),property_host::text(*bytes));

    auto& config=configuration(parser);
    auto& runtime=config.runtime();
    auto& cx=runtime.context();
    const auto source=runtime.properties.to(rawSource);
    std::u16string textPart,result;
    std::size_t i=0;
    const auto charAt=[&](std::size_t index){
        const auto method=cx.get(source,key(u"charAt"));
        const auto* function=std::get_if<options::ObjectRef>(&method.data);
        if(!function||cx.kind(*function)!=options::ObjectKind::function)
            property_host::typeError("str.charAt is not a function");
        const std::array<options::Value,1> indexArg{static_cast<double>(index)};
        return text(cx,cx.call(*function,source,indexArg));
    };
    while(static_cast<double>(i)<cx.toNumber(cx.get(source,key(u"length")))){
        const auto current=charAt(i++);
        const char16_t c=current.empty()?0:current.front();
        if(c==u'\\'){
            textPart.push_back(c);
            textPart+=charAt(i++);
        }else if(c==u'#'){
            const auto nextString=charAt(i++);
            const char16_t next=nextString.empty()?0:nextString.front();
            if(next==u'#')textPart.push_back(next);
            else if(next<u'1'||next>u'9'||static_cast<double>(next-u'0')>cx.toNumber(cx.get(args,key(u"length"))))
                throwError(u"IllegalMacroParam",u"Illegal macro parameter reference");
            else{
                result=addArgs(parser,addArgs(parser,std::move(result),std::move(textPart)),text(cx,cx.get(args,key(property_host::text(js_number::toString(static_cast<double>(next-u'0'-1)))))));
                textPart.clear();
            }
        }else textPart.push_back(c);
    }
    return addArgs(parser,std::move(result),std::move(textPart));
}

void ParseUtil::checkMaxMacros(TexParser& parser,bool macro){
    ++parser.macroCount;auto& config=configuration(parser);Context& cx=config.runtime().context();
    const Value limit=cx.get(config.options,key(u"maxMacros"));if(!(static_cast<double>(parser.macroCount)>cx.toNumber(limit)))return;
    if(macro)throwError(u"MaxMacroSub1",u"MathJax maximum macro substitution count exceeded; is here a recursive macro call?");
    throwError(u"MaxMacroSub2",u"MathJax maximum substitution count exceeded; is there a recursive latex environment?");
}

void ParseUtil::checkEqnEnv(TexParser& parser,bool nestable){
    const auto top=stack(parser).Top();
    BaseItem& item=stack_host::item(top,"First");
    const auto first=item.First();
    const auto nested=[&](const StackProperty& value){const auto* p=std::get_if<Property>(&value.value);return p&&property_host::truthy(*p);};
    if(nested(item.getProperty("nestable"))&&nestable&&!first.isNode())return;
    if(nested(item.getProperty("nestStart")))return;
    if(!item.isKind("start")||first.isNode())throwError(u"ErroneousNestingEq",u"Erroneous nesting of equation structures");
}

std::u16string ParseUtil::mmlFilterAttribute(TexParser&,std::u16string,std::u16string value){return value;}

Property ParseUtil::getFontDef(TexParser& parser){
    const Property font=property_host::get(stack(parser).env(),"font");
    if(!property_host::truthy(font))return Property(PropertyList{});
    return Property(PropertyList{{"mathvariant",font}});
}

mml::NodeValue ParseUtil::underOver(TexParser& parser,mml::NodeValue base,mml::NodeValue script,std::u16string pos,bool shouldStack){
    checkMovableLimits(base);
    if(NodeUtil::instance().isType(base,"munderover")&&NodeUtil::instance().isEmbellished(base)){
        NodeUtil::instance().setProperties(NodeUtil::instance().getCoreMO(base),Property(PropertyList{{"lspace",0},{"rspace",0}}));
        auto mo=makeNode(configuration(parser),"mo",std::make_shared<tree::ChildArray<mml::MmlNode>>(),Property(PropertyList{{"rspace",0}}));
        base=makeNode(configuration(parser),"mrow",std::make_shared<tree::ChildArray<mml::MmlNode>>(std::initializer_list<mml::NodeValue>{mo,base}));
    }
    auto node=makeNode(configuration(parser),"munderover",std::make_shared<tree::ChildArray<mml::MmlNode>>(std::initializer_list<mml::NodeValue>{base}));
    auto* scripts=dynamic_cast<mml::MmlMunderover*>(node.nodeOrNull());
    if(!scripts)throw tree::FactoryError("TypeError",u"munderover factory returned an incompatible node");
    NodeUtil::instance().setChild(node,pos==u"over"?scripts->over():scripts->under(),script);
    if(shouldStack){
        auto style=makeNode(configuration(parser),"mstyle",std::make_shared<tree::ChildArray<mml::MmlNode>>(std::initializer_list<mml::NodeValue>{node}),Property(PropertyList{{"displaystyle",true},{"scriptlevel",0}}));
        node=makeNode(configuration(parser),"TeXAtom",std::make_shared<tree::ChildArray<mml::MmlNode>>(std::initializer_list<mml::NodeValue>{style}),Property(PropertyList{{"texClass",static_cast<double>(mml::TEXCLASS.OP)},{"movesupsub",true}}));
    }
    NodeUtil::instance().setProperty(node,"subsupOK",true);
    return node;
}

mml::NodeValue ParseUtil::copyNode(mml::NodeValue node,TexParser& parser){
    mml::MmlNode* copied=node.requireNode("copy")->copy();
    mml::NodeValue result(copied);
    auto& config=configuration(parser);
    copied->walkTree([&](mml::MmlNode* item,const std::any&){
        config.addNode(Property(item->kind()),mml::NodeValue(item));
        const Property listValue=NodeUtil::instance().getProperty(mml::NodeValue(item),"in-lists");
        const std::u16string lists=property_host::truthy(listValue)?property_host::string(listValue):std::u16string{};
        std::size_t begin=0;
        while(begin<=lists.size()){
            const auto end=lists.find(u',',begin);const auto stop=end==std::u16string::npos?lists.size():end;
            if(stop>begin)config.addNode(Property(property_host::debugText(lists.substr(begin,stop-begin))),mml::NodeValue(item));
            if(end==std::u16string::npos)break;begin=end+1;
        }
    });
    return result;
}

mml::Children ParseUtil::internalMath(TexParser& parser,std::u16string source,Property level,Property font){
    std::u16string compact;compact.reserve(source.size());
    for(std::size_t i=0;i<source.size();){if(source[i]==u' '){compact.push_back(u' ');while(i<source.size()&&source[i]==u' ')++i;}else compact.push_back(source[i++]);}
    source=std::move(compact);
    auto& config=configuration(parser);Context& cx=config.runtime().context();
    const Value internal=cx.get(config.options,key(u"internalMath"));
    if(options::truthy(internal)){
        const FunctionValue callable=config.runtime().functions().functionValue(internal);
        const auto* function=std::get_if<FunctionRef>(&callable);
        if(!function||!*function)
            throw options::Exception("TypeError",u"this.configuration.options.internalMath is not a function");
        const TexParserRef parserRef=config.runtime().parserIdentities.resolve(parser.instanceIdentity());
        const std::array<std::any,4> args{
            std::any(parserRef),std::any(source),std::any(level),std::any(font)};
        const std::any result=(*function)->call(FunctionReceiver{config.options},args);
        if(const auto* children=std::any_cast<mml::Children>(&result))return *children;
        throw tree::FactoryError("Unimplemented",u"ParseOptions.internalMath callback result is not a native MML child array");
    }
    Property mathvariant=font;
    if(!property_host::truthy(mathvariant))mathvariant=property_host::get(stack(parser).env(),"font");
    Property def=property_host::truthy(mathvariant)?Property(PropertyList{{"mathvariant",mathvariant}}):Property(PropertyList{});
    auto nodes=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    auto append=[&](mml::NodeValue node){nodes->push_back(node);};
    auto parseSegment=[&](std::u16string_view text){return TexParser::make(std::u16string(text),Property(PropertyList{}),config)->mml();};
    auto atom=[&](mml::NodeValue child,Property definition=Property(PropertyList{})){
        auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(child);
        std::vector<std::any> args{Property("TeXAtom"),children,std::move(definition)};
        return parser.create(u"node",std::move(args));
    };
    auto internalTextNode=[&](std::u16string_view value){return internalText(parser,std::u16string(value),def);};
    const auto hasSpecial=[&](){
        for(std::size_t i=0;i<source.size();++i){
            if(source[i]==u'$'||source[i]==u'{'||source[i]==u'}')return true;
            if(source[i]==u'\\'&&i+1<source.size()){
                if(source[i+1]==u'$'||source[i+1]==u'{'||source[i+1]==u'}'||source[i+1]==u'\\'||source[i+1]==u'('||source[i+1]==u'U')return true;
                std::size_t end=i+1;while(end<source.size()&&((source[end]>=u'a'&&source[end]<=u'z')||(source[end]>=u'A'&&source[end]<=u'Z')))++end;
                const bool ref=(source.substr(i+1,end-i-1)==u"ref"||source.substr(i+1,end-i-1)==u"eqref");
                if(ref){std::size_t p=end;while(p<source.size()&&js_input::isWhitespace(source[p]))++p;if(p<source.size()&&source[p]==u'{')return true;}
            }
        }return false;
    };
    if(!hasSpecial()){if(!source.empty())append(internalTextNode(source));}
    else{
        std::size_t i=0,k=0;int braces=0;std::u16string match;
        while(i<source.size()){
            char16_t c=source[i++];
            if(c==u'$'){
                if(match==u"$"&&braces==0){append(atom(parseSegment(std::u16string_view(source).substr(k,i-k-1))));match.clear();k=i;}
                else if(match.empty()){if(k<i-1)append(internalTextNode(source.substr(k,i-k-1)));match=u"$";k=i;}
            }else if(c==u'{'&& !match.empty())++braces;
            else if(c==u'}'){
                if(match==u"}"&&braces==0){append(atom(parseSegment(source.substr(k,i-k)) ,def));match.clear();k=i;}
                else if(!match.empty()&&braces)--braces;
            }else if(c==u'\\'){
                if(match.empty()){
                    std::size_t j=i;while(j<source.size()&&((source[j]>=u'a'&&source[j]<=u'z')||(source[j]>=u'A'&&source[j]<=u'Z')))++j;
                    const auto command=source.substr(i,j-i);
                    if(command==u"ref"||command==u"eqref"){
                        std::size_t p=j;while(p<source.size()&&js_input::isWhitespace(source[p]))++p;
                        if(p<source.size()&&source[p]==u'{'){if(k<i-1)append(internalTextNode(source.substr(k,i-k-1)));match=u"}";k=i-1;i=p+1;}
                    }
                }
                if(i<source.size()){
                    c=source[i++];
                    if(c==u'('&&match.empty()){if(k<i-2)append(internalTextNode(source.substr(k,i-k-2)));match=u")";k=i;}
                    else if(c==u')'&&match==u")"&&braces==0){append(atom(parseSegment(source.substr(k,i-k-2))));match.clear();k=i;}
                    else if((c==u'$'||c==u'{'||c==u'}'||c==u'\\')&&match.empty()){source.erase(i-2,1);--i;}
                    else if(c==u'U'){
                        std::size_t p=i;while(p<source.size()&&js_input::isWhitespace(source[p]))++p;
                        const bool braced=p<source.size()&&source[p]==u'{';
                        std::size_t first=braced?p+1:p;
                        if(braced)while(first<source.size()&&js_input::isWhitespace(source[first]))++first;
                        std::size_t digits=first;
                        while(digits<source.size()&&((source[digits]>=u'0'&&source[digits]<=u'9')||(source[digits]>=u'A'&&source[digits]<=u'F')))++digits;
                        std::size_t end=digits;
                        if(braced)while(end<source.size()&&js_input::isWhitespace(source[end]))++end;
                        if(digits==first||(!braced&&digits-first!=1)||(braced&&(end>=source.size()||source[end]!=u'}')))
                            throwError(u"BadRawUnicode",u"Argument to %1 must a hexadecimal number with 1 to 6 digits",std::array<Property,1>{Property("\\U")});
                        const double cp=js_input::parseInt(source.substr(first,digits-first),js_input::IntegerRadix::Hex);
                        if(cp>0x10FFFF)throw Exception("RangeError",u"Invalid code point");
                        std::u16string scalar;if(cp<=0xFFFF)scalar.push_back(static_cast<char16_t>(cp));else{const auto n=static_cast<std::uint32_t>(cp)-0x10000;scalar.push_back(static_cast<char16_t>(0xD800+(n>>10)));scalar.push_back(static_cast<char16_t>(0xDC00+(n&0x3FF)));}
                        const std::size_t consumed=(braced?end+1:digits)-(i-2);source.replace(i-2,consumed,scalar);i=i-2+scalar.size();
                    }
                }
            }
        }
        if(!match.empty())throwError(u"MathNotTerminated",u"Math mode is not properly terminated");
        if(k<source.size())append(internalTextNode(source.substr(k)));
    }
    if(!level.missing()&&!std::holds_alternative<PropertyNull>(level.value)){
        auto style=makeNode(config,"mstyle",nodes,Property(PropertyList{{"displaystyle",false},{"scriptlevel",level}}));
        nodes=std::make_shared<tree::ChildArray<mml::MmlNode>>();nodes->push_back(style);
    }else if(nodes->size()>1){
        auto row=makeNode(config,"mrow",nodes);nodes=std::make_shared<tree::ChildArray<mml::MmlNode>>();nodes->push_back(row);
    }
    return nodes;
}

Property ParseUtil::keyvalOptions(ParserRuntime& runtime,std::u16string attributes,options::Value allowed,bool error,bool l3keys){
    PropertyList definition=readKeyval(attributes,l3keys);
    if(std::holds_alternative<Undefined>(allowed.data)||std::holds_alternative<Null>(allowed.data))return Property(definition);
    Context& cx=runtime.context();
    for(const auto& name:definition.keys()){
        const Key propertyKey(property_host::text(name));
        if(cx.hasOwn(allowed,propertyKey)){
            const Value allowedValue=cx.get(allowed,propertyKey);
            const auto* object=std::get_if<ObjectRef>(&allowedValue.data);
            if(!object)continue;
            bool isType=false;
            const Value ctor=runtime.keyValueClass;
            if(const auto* constructor=std::get_if<ObjectRef>(&ctor.data)){
                const Value prototype=cx.get(ctor,key(u"prototype"));
                if(const auto* p=std::get_if<ObjectRef>(&prototype.data)){
                    auto current=cx.prototype(*object);
                    while(current){if(*current==*p){isType=true;break;}current=cx.prototype(*current);}
                }
            }
            if(!isType)continue;
            const auto value=property_host::string(definition.get(name));
            const Value verify=cx.get(allowedValue,key(u"verify"));
            const auto* verifyFn=std::get_if<ObjectRef>(&verify.data);
            if(!verifyFn||cx.kind(*verifyFn)!=ObjectKind::function)throw Exception("TypeError",u"type.verify is not a function");
            const std::array<Value,1> args{Value(value)};
            if(!options::truthy(cx.call(*verifyFn,allowedValue,args)))
                throwError(u"InvalidValue",u"Value for key '%1' is not of the expected type",std::array<Property,1>{Property(name)});
            const Value convert=cx.get(allowedValue,key(u"convert"));
            const auto* convertFn=std::get_if<ObjectRef>(&convert.data);
            if(!convertFn||cx.kind(*convertFn)!=ObjectKind::function)throw Exception("TypeError",u"type.convert is not a function");
            definition.set(name,propertyFromOptions(cx.call(*convertFn,allowedValue,args)));
        }else{
            if(error)throwError(u"InvalidOption",u"Invalid option: %1",std::array<Property,1>{Property(name)});
            definition.erase(name);
        }
    }
    return Property(definition);
}

bool ParseUtil::isLatinOrGreekChar(ParserRuntime& runtime,std::u16string character){
    const auto normalized=runtime.unicode().nfd(character);
    for(const char16_t c:normalized)
        if((c>=u'a'&&c<=u'z')||(c>=u'A'&&c<=u'Z')||(c>=0x0370&&c<=0x03F0))return true;
    return false;
}
} // namespace folia::math::tex
