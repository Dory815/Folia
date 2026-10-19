// Native implementation of MathJax 4.1.3 TextMacrosMethods.ts.
module folia.math.input.tex.textmacros.methods;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.macro;
import folia.math.input.tex.parser;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.node_util;
import folia.math.util.property_host;
import folia.math.util.options;
import folia.math.util.retries;
import std;

namespace folia::math::tex {
namespace {
using namespace options;

TextParser& textParser(ParserContext& context){
    auto* parser=dynamic_cast<TextParser*>(&context);
    if(!parser)property_host::typeError("TextMacros method requires a TextParser");
    return *parser;
}
Property atomProperty(const ParseAtom& atom){
    if(const auto* property=std::get_if<Property>(&atom))return *property;
    if(const auto* token=std::get_if<TokenRef>(&atom)){
        if(!*token)property_host::typeError("Cannot read properties of null token");
        return (*token)->token();
    }
    property_host::typeError("TextMacros method token is not a Property");
}
std::u16string text(const Property& value){return property_host::string(value);}
Property rest(const ParseRest& args,std::size_t index){
    if(index>=args.size())return Missing{};
    if(const auto* value=std::any_cast<Property>(&args[index]))return *value;
    if(const auto* value=std::any_cast<std::u16string>(&args[index]))return property_host::debugText(*value);
    if(const auto* value=std::any_cast<double>(&args[index]))return *value;
    if(const auto* value=std::any_cast<bool>(&args[index]))return *value;
    return Missing{};
}
FunctionRef method(ParseBody body){return makeParseCallable(std::move(body));}
ParseResult undefined(){return Property(Missing{});}
Stack& stack(TextParser& parser){
    const auto* value=std::get_if<std::shared_ptr<Stack>>(&parser.stack);
    if(!value||!*value)property_host::typeError("Cannot read properties of null or undefined (reading 'env')");
    return **value;
}
FunctionRef baseMethod(ParserRuntime& runtime,std::u16string_view name){
    const Property value=BaseMethods::get(runtime,name);
    const auto* function=std::get_if<FunctionRef>(&value.value);
    if(!function||!*function)throw options::Exception("Unimplemented",u"TextMacros requires an unavailable canonical BaseMethods function");
    return *function;
}
}

Property TextMacrosMethods::initializeModule(ParserRuntime& runtime){
    if(!runtime.textMacrosMethods.missing())return runtime.textMacrosMethods;
    PropertyList methods;
    static constexpr std::array<std::pair<std::string_view,double>,18> sourceMethods{{
        {"Comment",2},{"Math",2},{"MathModeOnly",2},{"Misplaced",2},
        {"OpenBrace",2},{"CloseBrace",2},{"OpenQuote",2},{"CloseQuote",2},
        {"Tilde",2},{"Space",2},{"SelfQuote",2},{"Insert",3},{"Accent",3},
        {"Emph",2},{"TextFont",3},{"SetFont",3},{"SetSize",3},{"CheckAutoload",2}
    }};
    std::size_t nextSourceMethod=0;
    const auto add=[&](std::string name,FunctionRef function){
        if(nextSourceMethod>=sourceMethods.size()||sourceMethods[nextSourceMethod].first!=name)
            throw Exception("TypeError",u"TextMacros method order differs from Source");
        const double length=sourceMethods[nextSourceMethod++].second;
        const std::u16string sourceName(name.begin(),name.end());
        const Value identityValue=BaseMethods::bind(runtime,function,sourceName);
        const auto* identity=std::get_if<ObjectRef>(&identityValue.data);
        if(!identity)throw Exception("TypeError",u"TextMacros method has no Context identity");
        runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(length),false,false,true));
        methods.set(std::move(name),Property(std::move(function)));
    };
    add("Comment",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
        auto& parser=textParser(context);
        while(parser.i<static_cast<double>(parser.string().size())&&parser.string()[static_cast<std::size_t>(parser.i)]!=u'\n')++parser.i;
        ++parser.i;return undefined();
    }));
    add("Math",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& parser=textParser(context);parser.saveText();
        const std::u16string open=text(atomProperty(atom));const double start=parser.i;double end=start;int braces=0;
        std::u16string c;
        while(!(c=parser.GetNext()).empty()){
            end=parser.i++;
            if(c==u"\\"){
                const std::u16string cs=parser.GetCS();
                if(cs==u")")c=u"\\(";
            }
            if((c==u"\\"||c==u"$")&&braces==0&&open==c){
                auto& original=parser.texParser();
                const auto* config=std::get_if<ParseOptions*>(&original.configuration);
                if(!config||!*config)property_host::typeError("Cannot read properties of undefined (reading 'nodeFactory')");
                const std::u16string source=parser.string().substr(static_cast<std::size_t>(start),static_cast<std::size_t>(end-start));
                const Property env=stack(parser).env();
                parser.PushMath(TexParser::make(source,env,**config)->mml());
                return undefined();
            }
            if(c==u"{")++braces;
            else if(c==u"}"){
                if(braces==0)parser.Error(u"ExtraCloseMissingOpen",u"Extra close brace or missing open brace");
                --braces;
            }
        }
        parser.Error(u"MathNotTerminated",u"Math mode is not properly terminated");
    }));
    add("MathModeOnly",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& parser=textParser(context);const Property c=atomProperty(atom);parser.Error(u"MathModeOnly",u"'%1' allowed only in math mode",std::array<Property,1>{c});
    }));
    add("Misplaced",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& parser=textParser(context);parser.Error(u"Misplaced",u"Misplaced '%1'",std::array<Property,1>{atomProperty(atom)});
    }));
    add("OpenBrace",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
        auto& parser=textParser(context);const Property env=stack(parser).env();parser.pushEnvStack(env);
        stack(parser).setEnv(Property(property_host::objectAssign(env)));return undefined();
    }));
    add("CloseBrace",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
        auto& parser=textParser(context);
        if(!parser.envStackEmpty()){
            parser.saveText();Stack& active=stack(parser);Property previous=parser.popEnvStack();active.setEnv(std::move(previous));return undefined();
        }
        parser.Error(u"ExtraCloseMissingOpen",u"Extra close brace or missing open brace");
    }));
    add("OpenQuote",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& parser=textParser(context);const std::u16string c=text(atomProperty(atom));
        if(!c.empty()&&parser.i<parser.string().size()&&parser.string()[static_cast<std::size_t>(parser.i)]==c.front()){
            parser.text().push_back(u'\u201C');++parser.i;
        }else parser.text().push_back(u'\u2018');return undefined();
    }));
    add("CloseQuote",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& parser=textParser(context);const std::u16string c=text(atomProperty(atom));
        if(!c.empty()&&parser.i<parser.string().size()&&parser.string()[static_cast<std::size_t>(parser.i)]==c.front()){
            parser.text().push_back(u'\u201D');++parser.i;
        }else parser.text().push_back(u'\u2019');return undefined();
    }));
    add("Tilde",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{textParser(context).text().push_back(u'\u00A0');return undefined();}));
    add("Space",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{auto& parser=textParser(context);parser.text().push_back(u' ');parser.GetNext();return undefined();}));
    add("SelfQuote",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{auto& parser=textParser(context);const auto name=text(atomProperty(atom));if(name.size()>1)parser.text().append(name.substr(1));return undefined();}));
    add("Insert",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest args)->ParseResult{auto& parser=textParser(context);parser.text().append(text(rest(args,0)));return undefined();}));
    add("Accent",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest args)->ParseResult{
        auto& parser=textParser(context);const std::u16string name=text(atomProperty(atom));const auto base=parser.ParseArg(name);
        const std::u16string accentText=text(rest(args,0));const std::array<std::any,3> tokenArgs{std::any(Property("mo")),std::any(Property(PropertyList{})),std::any(Property(property_host::debugText(accentText)))};
        const auto accent=parser.create(u"token",FunctionArgs(tokenArgs));parser.addAttributes(accent);
        const auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(base);children->push_back(accent);
        const std::array<std::any,2> nodeArgs{std::any(Property("mover")),std::any(children)};parser.Push(parser.create(u"node",FunctionArgs(nodeArgs)));return undefined();
    }));
    add("Emph",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{auto& parser=textParser(context);const Property current=property_host::get(stack(parser).env(),"mathvariant");const std::u16string variant=current==Property("-tex-mathit")?u"normal":u"-tex-mathit";parser.Push(parser.ParseTextArg(text(atomProperty(atom)),Property(PropertyList{{"mathvariant",property_host::debugText(variant)}})));return undefined();}));
    add("TextFont",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest args)->ParseResult{auto& parser=textParser(context);parser.saveText();parser.Push(parser.ParseTextArg(text(atomProperty(atom)),Property(PropertyList{{"mathvariant",rest(args,0)}})));return undefined();}));
    add("SetFont",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest args)->ParseResult{auto& parser=textParser(context);parser.saveText();property_host::set(stack(parser).env(),"mathvariant",rest(args,0));return undefined();}));
    add("SetSize",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest args)->ParseResult{auto& parser=textParser(context);parser.saveText();property_host::set(stack(parser).env(),"mathsize",rest(args,0));return undefined();}));
    add("CheckAutoload",method([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& parser=textParser(context);std::u16string name=text(atomProperty(atom));
        const auto* config=std::get_if<ParseOptions*>(&parser.configuration);
        if(!config||!*config)property_host::typeError("Cannot read properties of undefined (reading 'packageData')");
        const auto* packageMap=std::get_if<std::shared_ptr<SourceStringMap<options::Value>>>(&(*config)->packageData);
        if(!packageMap||!*packageMap)property_host::typeError("Cannot read properties of undefined (reading 'get')");
        const auto autoload=(*packageMap)->get(u"autoload");
        const auto* parserOptions=std::get_if<ParseOptions*>(&parser.configuration);
        if(!parserOptions||!*parserOptions)property_host::typeError("Cannot read properties of undefined (reading 'runtime')");
        ParserRuntime& runtime=(*parserOptions)->runtime();
        const Value autoloadValue=autoload?*autoload:Value(Undefined{});
        const Property autoloadProperty=runtime.properties.from(autoloadValue);
        TexParser& original=parser.texParser();
        if(!name.empty())name.erase(0,1);
        const TokenLookup macro=original.lookup(HandlerType.get("MACRO"),name);
        const bool macroPresent=std::visit([](const auto& value){
            using T=std::decay_t<decltype(value)>;
            if constexpr(std::same_as<T,Property>)return property_host::truthy(value);
            else return static_cast<bool>(value);
        },macro);
        bool autoloadMacro=false;
        if(macroPresent&&property_host::truthy(autoloadProperty)){
            const Property autoloadFunction=property_host::get(autoloadProperty,"Autoload");
            if(const auto* value=std::get_if<MacroRef>(&macro);value&&*value){
                const auto function=(*value)->func();
                const FunctionRef reference=std::holds_alternative<FunctionRef>(function)?std::get<FunctionRef>(function):functions::reference(std::get<Property>(function));
                const auto identity=reference?runtime.functions().find(reference):std::nullopt;
                const auto* autoloadRef=std::get_if<FunctionRef>(&autoloadFunction.value);
                autoloadMacro=identity&&autoloadRef&&reference==*autoloadRef;
            }
        }
        if(!macroPresent||autoloadMacro){
            const ParseInput originalInput{original,property_host::debugText(name)};
            (void)original.parse(HandlerType.get("MACRO"),originalInput);
            if(!macroPresent)return undefined();
            if(autoloadMacro){
                retries::retryAfter(runtime.queue());
            }
        }
        const ParseInput textInput{parser,property_host::debugText(name)};
        (void)original.parse(HandlerType.get("MACRO"),textInput);return undefined();
    }));
    const std::array<std::u16string_view,11> aliases{u"Macro",u"Spacer",u"Hskip",u"rule",u"Rule",u"HandleRef",u"UnderOver",u"Lap",u"Phantom",u"Smash",u"MmlToken"};
    if(nextSourceMethod!=sourceMethods.size())throw Exception("TypeError",u"TextMacros source method table is incomplete");
    for(const auto name:aliases)methods.set(property_host::debugText(name),Property(baseMethod(runtime,name)));
    runtime.textMacrosMethods=Property(std::move(methods));
    return runtime.textMacrosMethods;
}

Property TextMacrosMethods::get(ParserRuntime& runtime,std::u16string_view name){
    if(runtime.textMacrosMethods.missing())(void)initializeModule(runtime);
    return property_host::get(runtime.textMacrosMethods,property_host::debugText(name));
}
}
