// Source-owned implementation of AmsCdMethods.ts.
module folia.math.input.tex.amscd.methods;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.node_util;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.stack_item_factory;
import folia.math.input.tex.tex_constants;
import folia.math.util.property_host;
import folia.math.core.mml_tree.mml_nodes.munderover;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
TexParser& parser(ParserContext& value){auto* p=dynamic_cast<TexParser*>(&value);if(!p)property_host::typeError("AMS CD requires TexParser");return *p;}
Stack& stack(TexParser& p){const auto* s=std::get_if<std::shared_ptr<Stack>>(&p.stack);if(!s||!*s)property_host::typeError("Cannot read properties of null or undefined (reading 'Top')");return **s;}
StackItemFactory& factory(TexParser& p){const auto f=p.itemFactory();const auto* s=std::get_if<std::shared_ptr<StackItemFactory>>(&f);if(!s||!*s)property_host::typeError("Cannot read properties of null or undefined (reading 'create')");return **s;}
ParseOptions& parseOptions(TexParser& p){const auto* s=std::get_if<ParseOptions*>(&p.configuration);if(!s||!*s)property_host::typeError("Cannot read properties of null or undefined (reading 'options')");return **s;}
Property atom(const ParseAtom& value){if(const auto* p=std::get_if<Property>(&value))return *p;if(const auto* t=std::get_if<TokenRef>(&value);t&&*t)return (*t)->token();property_host::typeError("AMS CD method name is not a token");}
std::u16string text(const Property& value){return property_host::string(value);}
mml::Children children(std::initializer_list<mml::NodeValue> values={}){auto result=std::make_shared<tree::ChildArray<mml::MmlNode>>();for(const auto& value:values)result->push_back(value);return result;}
mml::NodeValue makeToken(TexParser& p,std::u16string kind,Property attrs,std::u16string value){const std::array<std::any,3> args{std::any(Property(property_host::debugText(kind))),std::any(std::move(attrs)),std::any(Property(property_host::debugText(value)))};return p.create(u"token",FunctionArgs(args));}
mml::NodeValue makeNode(TexParser& p,std::u16string kind,mml::Children nodes,Property attrs=PropertyList{}){const std::array<std::any,3> args{std::any(Property(property_host::debugText(kind))),std::any(std::move(nodes)),std::any(std::move(attrs))};return p.create(u"node",FunctionArgs(args));}
Value amsOptions(ParseOptions& options){const auto* obj=std::get_if<ObjectRef>(&options.options.data);if(!obj)property_host::typeError("Cannot read properties of undefined (reading 'amscd')");return options.runtime().context().get(*obj,Key(u"amscd"));}
Property option(ParseOptions& options,const Value& record,std::u16string_view name){const auto* object=std::get_if<ObjectRef>(&record.data);if(!object)property_host::typeError("Cannot read properties of undefined (reading 'harrowsize')");return options.runtime().properties.from(options.runtime().context().get(*object,Key(std::u16string(name))));}
ParseResult callOther(ParserRuntime& runtime,TexParser& p,const ParseAtom& name){
    const auto loaded=runtime.configurationHandler.get(u"base");const auto* config=std::get_if<ConfigurationRef>(&loaded);if(!config||!*config)property_host::typeError("Cannot read properties of undefined (reading 'fallback')");
    const auto identity=runtime.configurationIdentities.find(*config);if(!identity)property_host::typeError("Base configuration has no Context identity");
    Context& cx=runtime.context();const Value fallback=cx.get(Value(*identity),Key(u"fallback"));const Value other=cx.get(fallback,Key(u"character"));
    const Property callable=runtime.properties.from(other);const ParseMethod method=functions::fromProperty(callable);
    return invokeParseMethod(method,FunctionReceiver{Missing{}},static_cast<ParserContext&>(p),name);
}
}
Property AmsCdMethods::initializeModule(ParserRuntime& runtime){
    if(!runtime.amsCdMethods.missing())return runtime.amsCdMethods;
    FunctionRef cell=makeParseCallable([](const FunctionReceiver&,ParserContext& source,const ParseAtom& rawName,ParseRest)->ParseResult{
        auto& p=parser(source);const Property name=atom(rawName);auto& top=stack(p);const StackEntry entry=top.Top();auto* current=dynamic_cast<ArrayItem*>(&stack_host::item(entry,"table"));
        if(!current)property_host::typeError("Cannot read properties of undefined (reading 'table')");
        const auto table=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&current->table);const auto row=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&current->row);
        if((!table||!*table||(*table)->size()%2==0)&&(!row||!*row||(*row)->empty()))p.Push(makeNode(p,u"mpadded",children(),Property(PropertyList{{"height",Property("8.5pt")},{"depth",Property("2pt")}})));
        auto* item=factory(p).create("cell",{});if(!item)property_host::typeError("Cannot read properties of null (reading 'setProperties')");item->setProperties(StackProperty(PropertyList{{"isEntry",true},{"name",name}}));p.Push(factory(p).reference(item));return Property(Missing{});
    });
    FunctionRef cd=makeParseCallable([&runtime](const FunctionReceiver&,ParserContext& source,const ParseAtom& begin,ParseRest)->ParseResult{
        auto& p=parser(source);if(const auto* item=std::get_if<StackItemRef>(&begin);item&&*item)p.Push(*item);else property_host::typeError("Cannot read properties of undefined (reading 'getName')");
        auto* array=dynamic_cast<ArrayItem*>(factory(p).create("array",{}));if(!array)property_host::typeError("ArrayItem constructor is unavailable");
        // Source reads options.amscd before evaluating the object passed to
        // setProperties, then evaluates each fallback in property order.
        ParseOptions& configuration=parseOptions(p);const Value options=amsOptions(configuration);
        const Property minw=property_host::get(stack(p).env(),"CD_minw");const Property width=property_host::truthy(minw)?minw:option(configuration,options,u"harrowsize");
        const Property minh=property_host::get(stack(p).env(),"CD_minh");const Property height=property_host::truthy(minh)?minh:option(configuration,options,u"varrowsize");
        array->setProperties(StackProperty(PropertyList{{"minw",width},{"minh",height}}));
        Context& cx=runtime.context();const ObjectRef def=cx.object();cx.set(Value(def),Key(u"columnalign"),Value(u"center"));cx.set(Value(def),Key(u"columnspacing"),runtime.properties.to(option(configuration,options,u"colspace")));cx.set(Value(def),Key(u"rowspacing"),runtime.properties.to(option(configuration,options,u"rowspace")));cx.set(Value(def),Key(u"displaystyle"),Value(true));array->arraydef=Value(def);return factory(p).reference(array);
    });
    FunctionRef arrow=makeParseCallable([&runtime,cell](const FunctionReceiver&,ParserContext& source,const ParseAtom& rawName,ParseRest)->ParseResult{
        auto& p=parser(source);const Property name=atom(rawName);const double old=p.i;const std::u16string code=p.GetNext();
        static constexpr std::u16string_view accepted=u"><VA.|=";
        if(code.empty()||accepted.find(code)==std::u16string_view::npos){p.i=old;return callOther(runtime,p,rawName);}++p.i;
        StackEntry first=stack(p).Top();auto* current=dynamic_cast<ArrayItem*>(&stack_host::item(first,"isKind"));if(!current||!current->isKind("array")||current->Size()){
            (void)invokeParseMethod(ParseMethod{cell},FunctionReceiver{},p,rawName);first=stack(p).Top();current=dynamic_cast<ArrayItem*>(&stack_host::item(first,"table"));
        }
        auto* top=current;if(!top)property_host::typeError("Cannot read properties of undefined (reading 'table')");
        const auto table=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&top->table);const auto row=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&top->row);
        const bool arrowRow=table&&*table&&(*table)->size()%2==1;double n=((row&&*row?static_cast<double>((*row)->size()):0)+(arrowRow?0:1));n=static_cast<double>(static_cast<unsigned long long>(n)%2);
        while(n){(void)invokeParseMethod(ParseMethod{cell},FunctionReceiver{},p,rawName);n-=1;}
        const Property hdef(PropertyList{{"minsize",stack_property_host::scalar(top->getProperty("minw"))},{"stretchy",true}});
        const Property vdef(PropertyList{{"minsize",stack_property_host::scalar(top->getProperty("minh"))},{"stretchy",true},{"symmetric",true},{"lspace",0},{"rspace",0}});
        mml::NodeValue output=Missing{};
        if(code==u"|")output=makeToken(p,u"mo",vdef,u"∥");
        else if(code==u"=")output=makeToken(p,u"mo",hdef,u"=");
        else if(code!=u"."){
            const std::u16string arrowChar=code==u">"?u"→":code==u"<"?u"←":code==u"V"?u"↓":u"↑";
            std::u16string a=p.GetUpTo(text(name)+text(Property(property_host::debugText(code))),code);const std::u16string b=p.GetUpTo(text(name)+text(Property(property_host::debugText(code))),code);
            if(code==u">"||code==u"<"){
                mml::NodeValue mo=makeToken(p,u"mo",hdef,arrowChar);if(a.empty())a=u"\\kern "+text(stack_property_host::scalar(top->getProperty("minw")));
                const Property pad(PropertyList{{"width",Property("+.67em")},{"lspace",Property(".33em")}});
                output=makeNode(p,u"munderover",children({mo}));auto* underOver=dynamic_cast<mml::MmlMunderover*>(output.nodeOrNull());if(!underOver)property_host::typeError("munderover constructor returned another node type");
                const Property env=stack(p).env();auto& config=parseOptions(p);const mml::NodeValue nodeA=TexParser::make(a,env,config)->mml();mml::NodeValue upper=makeNode(p,u"mpadded",children({nodeA}),pad);NodeUtil::instance().setAttribute(upper,"voffset",Property(".1em"));NodeUtil::instance().setChild(output,underOver->over(),upper);
                if(!b.empty()){const mml::NodeValue nodeB=TexParser::make(b,env,config)->mml();NodeUtil::instance().setChild(output,underOver->under(),makeNode(p,u"mpadded",children({nodeB}),pad));}
                if(property_host::truthy(option(config,amsOptions(config),u"hideHorizontalLabels")))output=makeNode(p,u"mpadded",children({output}),Property(PropertyList{{"depth",Property(0)},{"height",Property(".67em")}}));
            }else{
                const mml::NodeValue arrowNode=makeToken(p,u"mo",vdef,arrowChar);output=arrowNode;
                if(!a.empty()||!b.empty()){
                    output=makeNode(p,u"mrow",children());if(!a.empty()){const std::u16string source=u"\\scriptstyle\\raise.125em{\\vcenter{\\llap{"+a+u"}}}";NodeUtil::instance().appendChildren(output,children({TexParser::make(source,stack(p).env(),parseOptions(p))->mml()}));}
                    arrowNode.requireNodeForWrite("texClass")->setTexClass(mml::TEXCLASS.ORD);NodeUtil::instance().appendChildren(output,children({arrowNode}));if(!b.empty()){const std::u16string source=u"\\scriptstyle\\raise.125em{\\vcenter{\\rlap{"+b+u"}}}";NodeUtil::instance().appendChildren(output,children({TexParser::make(source,stack(p).env(),parseOptions(p))->mml()}));}
                }
            }
        }
        if(output.isNode())p.Push(output);(void)invokeParseMethod(ParseMethod{cell},FunctionReceiver{},p,rawName);return Property(Missing{});
    });
    FunctionRef minWidth=makeParseCallable([](const FunctionReceiver&,ParserContext& source,const ParseAtom& rawName,ParseRest)->ParseResult{auto& p=parser(source);const Property name=atom(rawName);Property env=stack(p).env();const std::u16string dimension=p.GetDimen(text(name));property_host::set(env,"CD_minw",Property(property_host::debugText(dimension)));return Property(Missing{});});
    FunctionRef minHeight=makeParseCallable([](const FunctionReceiver&,ParserContext& source,const ParseAtom& rawName,ParseRest)->ParseResult{auto& p=parser(source);const Property name=atom(rawName);Property env=stack(p).env();const std::u16string dimension=p.GetDimen(text(name));property_host::set(env,"CD_minh",Property(property_host::debugText(dimension)));return Property(Missing{});});
    for(const auto& [fn,label,len]:std::array{std::tuple{cd,u"CD",2.0},std::tuple{arrow,u"arrow",2.0},std::tuple{cell,u"cell",2.0},std::tuple{minWidth,u"minCDarrowwidth",2.0},std::tuple{minHeight,u"minCDarrowheight",2.0}}){const auto bound=BaseMethods::bind(runtime,fn,std::u16string(label));if(const auto* id=std::get_if<ObjectRef>(&bound.data))runtime.context().define(*id,Key(u"length"),Descriptor::data(Value(len),false,false,true));else throw Exception("TypeError",u"AMSCd method lost its Context identity");}
    runtime.amsCdMethods=Property(PropertyList{{"CD",Property(cd)},
        {"arrow",Property(arrow)},{"cell",Property(cell)},
        {"minCDarrowwidth",Property(minWidth)},
        {"minCDarrowheight",Property(minHeight)}});
    return runtime.amsCdMethods;
}
namespace {
FunctionRef requiredMethod(const Property& methods,std::string_view name){
    const Property value=property_host::get(methods,name);
    const auto* function=std::get_if<FunctionRef>(&value.value);
    if(!function||!*function)property_host::typeError("AMSCd method export is not callable");
    return *function;
}
}
AmsCdMethodSet AmsCdMethods::create(ParserRuntime& runtime){
    const Property methods=initializeModule(runtime);
    return {requiredMethod(methods,"CD"),requiredMethod(methods,"arrow"),
        requiredMethod(methods,"cell"),requiredMethod(methods,"minCDarrowwidth"),
        requiredMethod(methods,"minCDarrowheight")};
}
}
