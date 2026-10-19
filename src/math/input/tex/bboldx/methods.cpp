// Native port of MathJax 4.1.3 input/tex/bboldx/BboldxMethods.ts.
module folia.math.input.tex.bboldx.methods;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.textmacros.methods;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace { using namespace options;
Key key(std::u16string_view s){return Key(std::u16string(s));}
TexParser& tex(ParserContext& c){auto*p=dynamic_cast<TexParser*>(&c);if(!p)property_host::typeError("Bboldx method requires a TexParser");return *p;}
TextParser& text(ParserContext& c){auto*p=dynamic_cast<TextParser*>(&c);if(!p)property_host::typeError("Bboldx method requires a TextParser");return *p;}
Property rawArgument(const ParseRest& r,std::size_t i){
    if(i>=r.size())return Missing{};
    if(const auto*p=std::any_cast<Property>(&r[i]))return *p;
    if(const auto*p=std::any_cast<std::u16string>(&r[i]))return property_host::debugText(*p);
    if(const auto*p=std::any_cast<bool>(&r[i]))return *p;
    if(const auto*p=std::any_cast<double>(&r[i]))return *p;
    if(const auto*p=std::any_cast<FunctionRef>(&r[i]))return *p;
    return Missing{};
}
Property selected(ParserRuntime& runtime,Value options,Property normal,Property light,Property bold){auto&cx=runtime.context();const auto bb=cx.get(options,key(u"bboldx"));if(options::truthy(cx.get(bb,key(u"bfbb"))))return bold;if(options::truthy(cx.get(bb,key(u"light"))))return light;return normal;}
Property selected(ParserRuntime& runtime,Value options,std::u16string_view normal,std::u16string_view light,std::u16string_view bold){return selected(runtime,options,property_host::debugText(normal),property_host::debugText(light),property_host::debugText(bold));}
ParseResult charNode(TexParser& p,const ParseAtom& atom,bool bold,bool delimiter){const auto* token=std::get_if<TokenRef>(&atom);if(!token||!*token)property_host::typeError("Cannot read properties of null or undefined (reading 'char')");const auto* config=std::get_if<ParseOptions*>(&p.configuration);if(!config||!*config)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");auto&runtime=(**config).runtime();const Property variant=bold?selected(runtime,p.options(),std::u16string_view(u"-bboldx-bold"),std::u16string_view(u"-bboldx"),std::u16string_view(u"-bboldx-bold")):selected(runtime,p.options(),std::u16string_view(u"-bboldx"),std::u16string_view(u"-bboldx-light"),std::u16string_view(u"-bboldx-bold"));Property attrs=delimiter?Property(PropertyList{{"stretchy",Property(false)},{"mathvariant",variant}}):Property(PropertyList{{"mathvariant",variant}});const std::array<std::any,3> args{std::any(Property(delimiter?"mo":"mi")),std::any(attrs),std::any((*token)->character())};p.Push(p.create(u"token",FunctionArgs(args)));return Property(Missing{});}
}
BboldxMethodSet BboldxMethods::initializeModule(ParserRuntime& runtime){
    // Source exports one BboldxMethods object per loaded runtime. Every map
    // stores these same function identities; rebuilding the table here would
    // create distinct callable objects for otherwise identical Source methods.
    if(!runtime.bboldxMethods.missing())return {runtime.bboldxMethods};
    PropertyList methods;const auto add=[&](std::u16string_view name,FunctionRef fn,double length){const Value id=BaseMethods::bind(runtime,fn,std::u16string(name));if(const auto*o=std::get_if<ObjectRef>(&id.data))runtime.context().define(*o,key(u"length"),Descriptor::data(Value(length),false,false,true));methods.set(property_host::debugText(name),Property(fn));};
    const auto macro=BaseMethods::get(runtime,u"Macro"),mathFont=BaseMethods::get(runtime,u"MathFont");
    // BboldxMethods.Macro is the exact BaseMethods.Macro function object. Its
    // Source function has five declared parameters even though the specialized
    // token-map entries supply only the first two at invocation time.
    add(u"Macro",std::get<FunctionRef>(macro.value),5);
    const auto choose=makeParseCallable([mathFont](const FunctionReceiver&,ParserContext& c,const ParseAtom& a,ParseRest r)->ParseResult{auto&p=tex(c);const auto* cfg=std::get_if<ParseOptions*>(&p.configuration);if(!cfg||!*cfg)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");const auto f=selected((**cfg).runtime(),p.options(),rawArgument(r,0),rawArgument(r,1),rawArgument(r,2));const std::array<std::any,1> args{std::any(f)};return invokeParseMethod(mathFont,FunctionReceiver{},p,a,args);});
    const auto chooseText=makeParseCallable([mathFont](const FunctionReceiver&,ParserContext& c,const ParseAtom& a,ParseRest r)->ParseResult{auto&p=text(c);const auto* cfg=std::get_if<ParseOptions*>(&p.texParser().configuration);if(!cfg||!*cfg)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");const auto f=selected((**cfg).runtime(),Value(p.texParser().options()),rawArgument(r,0),rawArgument(r,1),rawArgument(r,2));const auto body=TextMacrosMethods::get((**cfg).runtime(),u"TextFont");const std::array<std::any,1> args{std::any(f)};return invokeParseMethod(body,FunctionReceiver{},p,a,args);});
    add(u"ChooseFont",choose,5);add(u"ChooseTextFont",chooseText,5);
    for(const auto&[name,bold,delim]:std::array{std::tuple{u"mathchar0miNormal",false,false},std::tuple{u"delimiterNormal",false,true},std::tuple{u"mathchar0miBold",true,false},std::tuple{u"delimiterBold",true,true}}){
        auto fn=makeParseCallable([bold,delim](const FunctionReceiver&,ParserContext& c,const ParseAtom& a,ParseRest)->ParseResult{return charNode(tex(c),a,bold,delim);});add(name,fn,2);
    }
    runtime.bboldxMethods=Property(std::move(methods));
    return {runtime.bboldxMethods};
}
Property BboldxMethods::get(ParserRuntime& runtime,std::u16string_view name){const auto set=initializeModule(runtime);return property_host::get(set.table,property_host::debugText(name));}
}
