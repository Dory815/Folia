// MathJax 4.1.3 BegingroupMethods.ts; callbacks retain one canonical FunctionRef.
module folia.math.input.tex.begingroup.methods;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.token_map;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
TexParser& parserFrom(ParserContext& value){
    if(auto* parser=dynamic_cast<TexParser*>(&value))return *parser;
    property_host::typeError("Begingroup parse method requires a TexParser");
}
Property methodProperty(const Property& table,std::u16string_view name){
    const auto record=PropertyList::record(table);
    if(!record)property_host::typeError("Begingroup method table is not initialized");
    return record->get(property_host::debugText(name));
}
void registerCommandMap(ParserRuntime& runtime){
    const auto table=PropertyList::record(runtime.begingroupMethods);
    if(!table)throw Exception("TypeError",u"Begingroup method table is not initialized");
    PropertyList commands;
    for(const std::u16string_view name:{u"begingroup",u"endgroup",u"global",u"begingroupReset",u"begingroupSandbox"}){
        const auto methodName=name==u"begingroupReset"?u"reset":name==u"begingroupSandbox"?u"sandbox":name;
        commands.setOwn(property_host::debugText(name),table->get(property_host::debugText(methodName)));
    }
    commands.setOwn("gdef",Property(PropertyArray{BaseMethods::get(runtime,u"Macro"),Property("\\global\\def")}));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("begingroup"),Property(commands));
}
}

Property BegingroupMethods::initializeModule(ParserRuntime& runtime){
    if(!runtime.begingroupMethods.missing())return runtime.begingroupMethods;
    PropertyList table;
    auto add=[&](std::u16string_view name,ParseBody body){
        const FunctionRef function=makeParseCallable(std::move(body));
        table.setOwn(property_host::debugText(name),Property(function));
        const Value bound=BaseMethods::bind(runtime,function,std::u16string(name));
        const auto* identity=std::get_if<ObjectRef>(&bound.data);
        if(!identity)throw Exception("TypeError",u"Begingroup method has no Context identity");
        runtime.context().define(*identity,key(u"length"),Descriptor::data(Value(2),false,false,true));
    };
    add(u"begingroup",[](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
        auto& parser=parserFrom(context);begingroupStack(parser)->push();return Property(Missing{});
    });
    add(u"endgroup",[](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
        auto& parser=parserFrom(context);begingroupStack(parser)->pop();return Property(Missing{});
    });
    add(u"reset",[](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
        auto& parser=parserFrom(context);begingroupStack(parser)->reset();return Property(Missing{});
    });
    add(u"sandbox",[](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
        auto& parser=parserFrom(context);begingroupStack(parser)->sandbox();
        const auto* stack=std::get_if<std::shared_ptr<Stack>>(&parser.stack);
        if(!stack||!*stack)property_host::typeError("Cannot read properties of undefined (reading 'global')");
        property_host::set((*stack)->global,"isSandbox",Property(true));return Property(Missing{});
    });
    add(u"global",[](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
        auto& parser=parserFrom(context);const double saved=parser.i;
        const std::u16string cs=parser.GetNext()==u"\\"?(++parser.i,parser.GetCS()):u"";
        parser.i=saved;
        const auto* configuration=std::get_if<ParseOptions*>(&parser.configuration);
        if(!configuration||!*configuration)property_host::typeError("Cannot read properties of undefined (reading 'options')");
        Context& cx=(**configuration).runtime().context();
        const Value options=parser.options();
        const Value begingroup=cx.get(options,key(u"begingroup"));
        const Value allow=cx.get(begingroup,key(u"allowGlobal"));
        const Value includes=cx.get(allow,key(u"includes"));
        const auto* callable=std::get_if<ObjectRef>(&includes.data);
        if(!callable||cx.kind(*callable)!=ObjectKind::function)
            throw Exception("TypeError",u"parser.options.begingroup.allowGlobal.includes is not a function");
        const std::array<Value,1> args{Value(cs)};
        const Value permitted=cx.call(*callable,allow,args);
        if(!truthy(permitted)){
            const std::array<Property,1> errorArgs{parser.currentCS()};
            throw TexError(u"IllegalGlobal",u"Invalid use of %1",errorArgs);
        }
        const auto* stack=std::get_if<std::shared_ptr<Stack>>(&parser.stack);
        if(!stack||!*stack)property_host::typeError("Cannot read properties of undefined (reading 'env')");
        property_host::set((*stack)->env(),"isGlobal",Property(true));return Property(Missing{});
    });
    const Property base=runtime.baseMethods;
    const Property macro=BaseMethods::get(base,u"Macro");
    table.setOwn("macro",macro);
    runtime.begingroupMethods=Property(table);
    registerCommandMap(runtime);
    return runtime.begingroupMethods;
}

Property BegingroupMethods::get(ParserRuntime& runtime,std::u16string_view name){
    if(runtime.begingroupMethods.missing())initializeModule(runtime);
    return methodProperty(runtime.begingroupMethods,name);
}
}
