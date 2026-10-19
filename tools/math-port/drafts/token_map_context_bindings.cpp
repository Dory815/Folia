#include "support/folia_test.hpp"
import folia.math.input.tex.token_map_context_bindings;
import folia.math.input.tex.tex_parser_context;
import folia.math.util.function_list_bindings;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace t=folia::math::tex;
namespace o=folia::math::options;

struct Parser final:t::ParserContext {
    Property current="";
    Property currentCS()const override{return current;}
    void setCurrentCS(Property value)override{current=std::move(value);}
};
struct Regex final:t::RegExpContext {
    bool test(const Property&)override{return true;}
};

suite token_map_context_bindings=[] {
    "token_map_context_bindings_preserve_native_class_fields_and_dispatch"_test=[] {
        o::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        o::MapContextBindings maps(context);
        t::MapHandler registry;
        Parser parser;
        auto parseFunction=t::makeParseCallable([](const auto&,auto&,const auto&,auto){
            return t::ParseResult{Property("parsed")};
        });
        OptionCallBinding callBinding;
        callBinding.receiver=[](o::Context&,o::Value){return FunctionReceiver{Missing{}};};
        callBinding.arguments=[](o::Context&,std::span<const o::Value> args){
            std::vector<std::any> values;for(const auto& value:args)values.emplace_back(value);return values;
        };
        callBinding.result=[](o::Context&,const std::any& result){
            if(const auto* value=std::any_cast<o::Value>(&result))return *value;
            if(const auto* value=std::any_cast<Property>(&result))return o::Value(property_host::text(std::get<std::string>(value->value)));
            return o::Value(o::Undefined{});
        };
        const auto parseFunctionObject=std::get<o::ObjectRef>(functions.bind(parseFunction,std::move(callBinding)).data);
        auto regex=std::make_shared<Regex>();
        auto hookProperty=[&](o::Value value){return properties.from(std::move(value));};
        auto hookValue=[&](const Property& value){return properties.to(value);};
        t::TokenMapContextHooks hooks;
        hooks.constructMap=[&](t::TokenMapClass cls,o::Context& cx,std::span<const o::Value> args)->t::TokenMapRef {
            const auto property=[&](std::size_t i){return i<args.size()?hookProperty(args[i]):Property(Missing{});};
            const auto parserMethod=[&](std::size_t i){
                return i<args.size()?hooks.parseMethodFromValue(cx,args[i]):t::ParseMethod{Property(Missing{})};
            };
            switch(cls){
            case t::TokenMapClass::AbstractTokenMap:return t::makeAbstractTokenMap(registry,property(0),parserMethod(1));
            case t::TokenMapClass::RegExpMap:return t::TokenMapHost::create<t::RegExpMap>(registry,property(0),parserMethod(1),hooks.regexpFromValue(cx,property(2).missing()?o::Value(o::Undefined{}):args[2]));
            case t::TokenMapClass::AbstractParseMap:return t::TokenMapHost::create<t::AbstractParseMap>(registry,property(0),parserMethod(1));
            case t::TokenMapClass::CharacterMap:return t::TokenMapHost::create<t::CharacterMap>(registry,property(0),parserMethod(1),property(2));
            case t::TokenMapClass::DelimiterMap:return t::TokenMapHost::create<t::DelimiterMap>(registry,property(0),parserMethod(1),property(2));
            case t::TokenMapClass::MacroMap:return t::TokenMapHost::create<t::MacroMap>(registry,property(0),property(1),property(2));
            case t::TokenMapClass::CommandMap:return t::TokenMapHost::create<t::CommandMap>(registry,property(0),property(1),property(2));
            case t::TokenMapClass::EnvironmentMap:return t::TokenMapHost::create<t::EnvironmentMap>(registry,property(0),parserMethod(1),property(2),property(3));
            default:throw o::Exception("TypeError",u"Abstract TokenMap class is not directly constructible");
            }
        };
        hooks.constructToken=[&](o::Context&,std::span<const o::Value> args){
            return std::make_shared<t::Token>(args.empty()?Property(Missing{}):hookProperty(args[0]),
                args.size()<2?Property(Missing{}):hookProperty(args[1]),
                args.size()<3?Property(Missing{}):hookProperty(args[2]));
        };
        hooks.constructMacro=[&](o::Context&,std::span<const o::Value> args){
            const auto method=args.size()>1?hooks.parseMethodFromValue(context,args[1]):t::ParseMethod{Property(Missing{})};
            return std::make_shared<t::Macro>(args.empty()?Property(Missing{}):hookProperty(args[0]),method,
                args.size()>2?hookProperty(args[2]):Property(Missing{}));
        };
        hooks.parseMethodFromValue=[&](o::Context&,o::Value value)->t::ParseMethod {
            if(const auto* object=std::get_if<o::ObjectRef>(&value.data))if(auto function=functions.find(*object))return *function;
            return functions::fromProperty(hookProperty(std::move(value)));
        };
        hooks.parseMethodToValue=[&](o::Context&,const t::ParseMethod& method)->o::Value {
            if(const auto* function=std::get_if<FunctionRef>(&method)){
                if(auto object=functions.find(*function))return o::Value(*object);
                throw o::Exception("Unimplemented",u"Unbound callback in test fixture");
            }
            return hookValue(std::get<Property>(method));
        };
        hooks.regexpFromValue=[&](o::Context&,o::Value){return std::static_pointer_cast<t::RegExpContext>(regex);};
        hooks.regexpToValue=[&](o::Context&,const t::RegExpRef&){return o::Value(o::Null{});};
        hooks.parseInputFromValue=[&](o::Context& cx,o::Value value){
            const auto* array=std::get_if<o::ObjectRef>(&value.data);
            if(!array)throw o::Exception("TypeError",u"expected ParseInput array");
            return t::ParseInput{parser,hookProperty(cx.get(*array,o::Key(u"1")))};
        };
        hooks.parseResultToValue=[&](o::Context&,t::ParseResult result){
            if(const auto* value=std::get_if<Property>(&result))return hookValue(*value);
            throw o::Exception("Unimplemented",u"test fixture only converts Property parse results");
        };
        auto execution=std::make_shared<std::atomic_bool>(true);
        t::TokenMapContextBindings bindings(context,functions,properties,maps,hooks,execution);

        const std::array concrete{
            t::TokenMapClass::AbstractTokenMap,t::TokenMapClass::RegExpMap,
            t::TokenMapClass::AbstractParseMap,t::TokenMapClass::CharacterMap,
            t::TokenMapClass::DelimiterMap,t::TokenMapClass::MacroMap,
            t::TokenMapClass::CommandMap,t::TokenMapClass::EnvironmentMap};
        // Construct each emitted class through its real Context constructor,
        // which dispatches into TokenMapHost and registers the native owner.
        auto jsonWithX=[&](o::Value value){const auto object=context.object();context.set(object,o::Key(u"x"),std::move(value));return o::Value(object);};
        const auto emptyParser=o::Value(o::Null{});
        const auto characterJson=jsonWithX(o::Value(u"X"));
        const auto delimiterJson=jsonWithX(o::Value(u"X"));
        const auto macroJson=jsonWithX(o::Value(parseFunctionObject));
        const std::array<std::vector<o::Value>,8> constructorArgs{{
            {o::Value(u"identity-abstract"),emptyParser},
            {o::Value(u"identity-regexp"),emptyParser,o::Value(o::Null{})},
            {o::Value(u"identity-abstract-parse"),emptyParser},
            {o::Value(u"identity-character"),emptyParser,characterJson},
            {o::Value(u"identity-delimiter"),emptyParser,delimiterJson},
            {o::Value(u"identity-macro"),macroJson},
            {o::Value(u"identity-command"),macroJson},
            {o::Value(u"identity-environment"),emptyParser,macroJson}}};
        for(std::size_t i=0;i<concrete.size();++i){
            const auto object=context.construct(bindings.constructor(concrete[i]),constructorArgs[i]);
            const auto owner=bindings.resolve(object);
            expect(owner!=nullptr);
            expect(bindings.bind(owner)==object);
            expect(context.prototype(object)==std::optional(bindings.prototype(concrete[i])));
            expect(context.get(object,o::Key(u"name"))==constructorArgs[i][0]);
        }
        const auto regexpOwner=registry.getMap(Property("identity-regexp"));
        const auto regexpObject=bindings.bind(regexpOwner);
        expect(bindings.bind(regexpOwner)==regexpObject);
        expect(context.get(regexpObject,o::Key(u"_regExp"))==o::Value(o::Null{}));
        expect(context.get(regexpObject,o::Key(u"_parser"))==o::Value(o::Null{}));
        const auto command=registry.getMap(Property("identity-command"));
        const auto commandObject=bindings.bind(command);
        const auto commandBase=bindings.prototype(t::TokenMapClass::MacroMap);
        expect(context.prototype(bindings.prototype(t::TokenMapClass::CommandMap))==std::optional(commandBase));
        expect(context.get(commandObject,o::Key(u"_name"))==o::Value(u"identity-command"));
        const auto projected=std::get<o::ObjectRef>(context.get(commandObject,o::Key(u"map")).data);
        const auto macroObject=std::get<o::ObjectRef>(maps.get(projected,o::Value(u"x")).data);
        const auto macro=bindings.resolveMacro(macroObject);
        expect(context.get(macroObject,o::Key(u"func"))==o::Value(parseFunctionObject));
        expect(bindings.bind(macro)==macroObject);
        const auto input=context.array(2);context.set(input,o::Key(u"0"),o::Value(o::Null{}));context.set(input,o::Key(u"1"),o::Value(u"x"));
        const auto parseMethod=context.get(bindings.prototype(t::TokenMapClass::CommandMap),o::Key(u"parse"));
        const std::array parseArgs{o::Value(input)};
        expect(context.callMember(parseMethod,o::Value(commandObject),parseArgs)==o::Value(u"parsed"));

        const auto replacementArgs=context.array(1);context.set(replacementArgs,o::Key(u"0"),o::Value(9));
        context.set(macroObject,o::Key(u"_args"),o::Value(replacementArgs));
        const auto currentArgs=PropertyArray::array(macro->args());
        expect(currentArgs&&currentArgs->get(0)==Property(9));
        const auto character=registry.getMap(Property("identity-character"));
        const auto token=std::get<t::TokenRef>(character->lookup(Property("x")));
        const auto tokenObject=bindings.bind(token);
        expect(context.get(tokenObject,o::Key(u"_token"))==o::Value(u"x"));
        context.set(tokenObject,o::Key(u"_char"),o::Value(u"Y"));
        expect(token->character()==Property("Y"));
        maps.set(projected,o::Value(u"y"),o::Value(macroObject));
        expect(std::get<t::MacroRef>(command->lookup(Property("y")))==macro);

        const auto replacement=maps.create();
        maps.set(replacement,o::Value(u"from-map"),o::Value(macroObject));
        context.set(commandObject,o::Key(u"map"),o::Value(replacement));
        expect(context.get(commandObject,o::Key(u"map"))==o::Value(replacement));
        expect(std::get<t::MacroRef>(command->lookup(Property("from-map")))==macro);
        std::dynamic_pointer_cast<t::AbstractParseMap>(command)->add(Property("from-native"),t::TokenLookup{macro});
        expect(maps.get(replacement,o::Value(u"from-native"))==o::Value(macroObject));

        execution->store(false);
        bool expired=false;
        try{(void)context.get(commandObject,o::Key(u"_name"));}catch(const o::Exception& e){expired=e.name=="TypeError";}
        expect(expired);
        execution.reset();
        expired=false;
        try{(void)context.get(commandObject,o::Key(u"_name"));}catch(const o::Exception& e){expired=e.name=="TypeError";}
        expect(expired);
    };
};
}
