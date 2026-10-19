// Native port of the CasesMethods source owner.
module folia.math.input.tex.cases.methods;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.parser;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.empheq.util;
import folia.math.util.property_host;
import folia.math.util.option_functions;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
TexParser& parser(ParserContext& source){auto* value=dynamic_cast<TexParser*>(&source);if(!value)property_host::typeError("Cases method requires a TexParser");return *value;}
BaseItem& begin(const ParseAtom& atom){const auto* ref=std::get_if<StackItemRef>(&atom);if(!ref||!*ref)property_host::typeError("Cannot read properties of undefined (reading 'getName')");return stack_host::item(StackEntry(*ref),"getName");}
Stack& stack(TexParser& p){const auto* ref=std::get_if<std::shared_ptr<Stack>>(&p.stack);if(!ref||!*ref)property_host::typeError("Cannot read properties of null or undefined (reading 'Top')");return **ref;}
StackItemFactoryService& factory(TexParser& p){
    const auto shared=p.itemFactory();
    if(const auto* value=std::get_if<std::shared_ptr<StackItemFactory>>(&shared);value&&*value)return **value;
    property_host::typeError("Cannot read properties of null or undefined (reading 'create')");
}
ParseMethod baseMethod(ParserRuntime& runtime,std::u16string_view name){return functions::fromProperty(BaseMethods::get(runtime,name));}
std::u16string sourceText(const Property& value){return property_host::string(stack_property_host::scalar(value));}
std::u16string sourceText(const StackProperty& value){return property_host::string(stack_property_host::scalar(value));}
ParseResult environment(const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest rest){
    auto& p=parser(source);const Property name=std::holds_alternative<Property>(atom)?std::get<Property>(atom):std::get<TokenRef>(atom)->character();
    BaseItem* raw=factory(p).itemFactory().create("cases-begin",{});if(!raw)property_host::typeError("CasesBeginItem constructor is not registered");
    const StackItemRef item=factory(p).reference(raw);auto& beginItem=stack_host::item(StackEntry(item),"setProperty");
    beginItem.setProperty("name",StackProperty(name));beginItem.setProperty("end",StackProperty(Property(true)));
    if(rest.size()<2)property_host::typeError("Cannot read properties of undefined (reading 'slice')");
    const auto* function=std::any_cast<FunctionRef>(&rest[0]);if(!function||!*function)property_host::typeError("Cases environment method is not callable");
    const auto* supplied=std::any_cast<Property>(&rest[1]);const auto args=supplied?PropertyArray::array(*supplied):std::optional<PropertyArray>{};
    if(!args)property_host::typeError("args.slice is not a function");
    const auto trailing=args->slice(1);std::vector<std::any> expanded;expanded.reserve(trailing.size());for(std::size_t i=0;i<trailing.size();++i)expanded.emplace_back(trailing.get(i));
    const auto parsed=invokeParseMethod(ParseMethod{*function},FunctionReceiver{Missing{}},static_cast<ParserContext&>(p),ParseAtom(item),expanded);
    if(const auto* ref=std::get_if<StackItemRef>(&parsed))p.Push(*ref);else if(const auto* value=std::get_if<Property>(&parsed))p.Push(*value);else property_host::typeError("Cases environment parser did not return a stack item");
    return Property(Missing{});
}
ParseMethod ownMethod(const Property& table,std::string_view name){return functions::fromProperty(property_host::get(table,name));}
ParseResult entry(const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest){
    auto& p=parser(source);const auto top=stack(p).Top();BaseItem& current=stack_host::item(top,"getProperty");
    if(!stack_property_host::truthy(current.getProperty("numCases"))){auto* configured=std::get_if<ParseOptions*>(&p.configuration);if(!configured||!*configured)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");return invokeParseMethod(baseMethod((**configured).runtime(),u"Entry"),FunctionReceiver{},p,atom);}
    Property name=Missing{};if(const auto* value=std::get_if<Property>(&atom))name=*value;else if(const auto* token=std::get_if<TokenRef>(&atom);token&&*token)name=(*token)->token();
    auto* cell=factory(p).itemFactory().create("cell",{});if(!cell)property_host::typeError("CellItem constructor is not registered");
    PropertyList properties{{"isEntry",true},{"name",name}};cell->setProperties(StackProperty(Property(std::move(properties))));p.Push(factory(p).reference(cell));
    const std::u16string input=p.string();std::size_t i=static_cast<std::size_t>(p.i),braces=0;const std::size_t length=input.size();
    while(i<length){const char16_t c=input[i];if(c==u'{'){++braces;++i;continue;}if(c==u'}'){if(braces==0)break;--braces;++i;continue;}
        if(c==u'&'&&braces==0)throw TexError(u"ExtraCasesAlignTab",u"Extra alignment tab in text for numcase environment");
        if(c==u'\\'&&braces==0){if(i+1>=length)break;const char16_t first=input[i+1];std::size_t end=i+2;if((first>=u'a'&&first<=u'z')||(first>=u'A'&&first<=u'Z'))while(end<length&&((input[end]>=u'a'&&input[end]<=u'z')||(input[end]>=u'A'&&input[end]<=u'Z')))++end;
            const std::u16string_view control(input.data()+i+1,end-i-1);if(control==u"\\"||control==u"cr"||control==u"end"||control==u"label")break;i+=control.size();continue;}
        ++i;
    }
    const auto whitespace=[](char16_t c){return c==u'\t'||c==u'\n'||c==u'\v'||c==u'\f'||c==u'\r'||c==u' '||c==0x00a0||c==0x1680||(c>=0x2000&&c<=0x200a)||c==0x2028||c==0x2029||c==0x202f||c==0x205f||c==0x3000||c==0xfeff;};
    std::size_t content=static_cast<std::size_t>(p.i);while(content<i&&whitespace(input[content]))++content;
    const auto parsed=ParseUtil::internalMath(p,input.substr(content,i-content),Property(0));p.PushAll(parsed);p.i=static_cast<double>(i);return Property(PropertyNull{});
}
}
Property CasesMethods::initializeModule(ParserRuntime& runtime){
    if(!runtime.casesMethods.missing())return runtime.casesMethods;
    const auto entryFunction=makeParseCallable(entry);const auto entryBound=BaseMethods::bind(runtime,entryFunction,u"Entry");
    const auto* identity=std::get_if<ObjectRef>(&entryBound.data);if(!identity)throw Exception("TypeError",u"Cases Entry method has no Context identity");
    runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    const auto environmentFunction=makeParseCallable(environment);const auto environmentBound=BaseMethods::bind(runtime,environmentFunction,u"environment");
    if(const auto* environmentIdentity=std::get_if<ObjectRef>(&environmentBound.data))runtime.context().define(*environmentIdentity,Key(u"length"),Descriptor::data(Value(4.0),false,false,true));
    const auto numCasesFunction=makeParseCallable(CasesMethods::numCases);const auto numCasesBound=BaseMethods::bind(runtime,numCasesFunction,u"NumCases");
    if(const auto* numCasesIdentity=std::get_if<ObjectRef>(&numCasesBound.data))runtime.context().define(*numCasesIdentity,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    // Preserve the exact Source CasesMethods object-literal export order.
    PropertyList methods;methods.setOwn("NumCases",Property(numCasesFunction));
    methods.setOwn("Entry",Property(entryFunction));methods.setOwn("environment",Property(environmentFunction));
    runtime.casesMethods=Property(std::move(methods));return runtime.casesMethods;
}
ParseResult CasesMethods::numCases(const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest){
    auto& p=parser(source);const auto* ref=std::get_if<StackItemRef>(&atom);if(!ref||!*ref)property_host::typeError("Cannot read properties of null or undefined (reading 'getName')");
    BaseItem& begin=stack_host::item(StackEntry(*ref),"getName");const auto name=sourceText(begin.getName());const Property environmentValue=stack(p).env();
    const Property closing=property_host::get(environmentValue,"closing");
    if(property_host::string(closing)==name){auto envRecord=PropertyList::record(environmentValue);if(!envRecord)property_host::typeError("Cannot convert undefined or null to object");envRecord->erase("closing");
        BaseItem* end=factory(p).itemFactory().create("end",{});if(!end)property_host::typeError("EndItem constructor is not registered");end->setProperty("name",StackProperty(begin.getName()));p.Push(factory(p).reference(end));
        BaseItem& cases=stack_host::item(stack(p).Top(),"Last");const auto table=cases.Last();
        const auto original=ParseUtil::copyNode(table,p);const std::u16string left=sourceText(cases.getProperty("left"));
        EmpheqUtil::left(table,original,left+u"\\mmlToken{mo}{\\U{7B}}\\,",p,u"numcases-left");
        BaseItem* closingItem=factory(p).itemFactory().create("end",{});if(!closingItem)property_host::typeError("EndItem constructor is not registered");closingItem->setProperty("name",StackProperty(begin.getName()));p.Push(factory(p).reference(closingItem));return Property(Missing{});
    }
    const auto left=p.GetArgument(u"\\begin{"+name+u"}");begin.setProperty("left",StackProperty(left));
    const std::array<std::any,4> args{std::any(true),std::any(true),std::any(std::u16string(u"ll")),std::any(std::u16string(u"tt"))};
    const auto result=invokeParseMethod(baseMethod((**std::get_if<ParseOptions*>(&p.configuration)).runtime(),u"EqnArray"),FunctionReceiver{Missing{}},static_cast<ParserContext&>(p),ParseAtom(*ref),args);
    const auto* arrayRef=std::get_if<StackItemRef>(&result);if(!arrayRef||!*arrayRef)property_host::typeError("EqnArray did not return an ArrayItem");
    auto& arrayItem=stack_host::item(StackEntry(*arrayRef),"arraydef");auto* equationArray=dynamic_cast<EqnArrayItem*>(&arrayItem);if(!equationArray)property_host::typeError("EqnArray constructor did not create an EqnArrayItem");
    Context& cx=(**std::get_if<ParseOptions*>(&p.configuration)).runtime().context();
    cx.set(equationArray->arraydef,Key(u"displaystyle"),Value(false));cx.set(equationArray->arraydef,Key(u"rowspacing"),Value(u".2em"));
    equationArray->setProperty("numCases",StackProperty(Property(true)));
    // Source CasesMethods.NumCases pushes the begin item after EqnArray has
    // pushed it.  That second stack entry is the marker that consumes the
    // closing end item and leaves the completed table on top for Last/copy.
    p.Push(*ref);
    return *arrayRef;
}
}
