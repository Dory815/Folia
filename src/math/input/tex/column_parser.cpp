/*************************************************************
 * MathJax 4.1.3 input/tex/ColumnParser.ts; Apache-2.0.
 * Source method port. See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.input.tex.parser;
import :column_parser;
import :parse_util;
import :base_items;
import folia.math.input.tex.stack_item_factory;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.unit_util;
import folia.math.util.js_case;
import folia.math.util.js_number;
import folia.math.util.js_numeric_input;
import folia.math.util.property_host;
import folia.math.util.utf16;
import std;

namespace folia::math::tex {
namespace {
using namespace options;

Key key(std::u16string_view name){return Key(std::u16string(name));}
std::u16string index_text(std::uint32_t index){
    const auto text=std::to_string(index);
    return std::u16string(text.begin(),text.end());
}
Value primitive(Context& context,const Value& value,bool stringHint){
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object)return value;
    const std::array<std::u16string_view,2> stringOrder{u"toString",u"valueOf"};
    const std::array<std::u16string_view,2> numberOrder{u"valueOf",u"toString"};
    const auto& order=stringHint?stringOrder:numberOrder;
    for(const auto name:order){
        const Value method=context.get(value,key(name));
        const auto* callable=std::get_if<ObjectRef>(&method.data);
        if(!callable||context.kind(*callable)!=ObjectKind::function)continue;
        const Value result=context.call(*callable,value,{});
        if(!std::holds_alternative<ObjectRef>(result.data))return result;
    }
    throw Exception("TypeError",u"Cannot convert object to primitive value");
}
std::u16string text_value(Context& context,const Value& original){
    const Value value=primitive(context,original,true);
    if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
    if(std::holds_alternative<Undefined>(value.data))return u"undefined";
    if(std::holds_alternative<Null>(value.data))return u"null";
    if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?u"true":u"false";
    if(const auto* number=std::get_if<double>(&value.data))return property_host::text(js_number::toString(*number));
    if(std::holds_alternative<SymbolRef>(value.data))throw Exception("TypeError",u"Cannot convert a Symbol value to a string");
    throw Exception("TypeError",u"Cannot convert value to a string");
}
double number_value(Context& context,const Value& original){
    const Value value=primitive(context,original,false);
    if(const auto* number=std::get_if<double>(&value.data))return *number;
    if(std::holds_alternative<Undefined>(value.data))return std::numeric_limits<double>::quiet_NaN();
    if(std::holds_alternative<Null>(value.data))return 0;
    if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?1:0;
    if(const auto* text=std::get_if<std::u16string>(&value.data))return js_input::number(*text);
    throw Exception("TypeError",u"Cannot convert value to a number");
}
Key numeric_key(double value){return Key(property_host::text(js_number::toString(value)));}
std::size_t substring_index(double value,std::size_t length){
    if(std::isnan(value)||value<=0)return 0;
    if(std::isinf(value))return length;
    return static_cast<std::size_t>(std::min(std::trunc(value),static_cast<double>(length)));
}
Value read_index(Context& context,const Value& array,double index){
    return context.get(array,numeric_key(index));
}
void write_index(Context& context,const Value& array,double index,Value value){
    context.set(array,numeric_key(index),std::move(value));
}
ObjectRef require_array(Context& context,const Value& value,std::u16string_view name){
    const auto* object=std::get_if<ObjectRef>(&value.data);
    if(!object||context.kind(*object)!=ObjectKind::array)
        throw Exception("TypeError",std::u16string(name)+u" is not an array");
    return *object;
}
Value array_field(Context& context,const ColumnState& state,std::u16string_view name){
    const Value value=state.get(name);
    (void)require_array(context,value,name);
    return value;
}
std::u16string join_array(Context& context,const Value& input,std::u16string_view separator){
    const Value method=context.get(input,Key(u"join"));
    const auto* callable=std::get_if<ObjectRef>(&method.data);
    if(!callable||context.kind(*callable)!=ObjectKind::function)
        throw Exception("TypeError",u"input.join is not a function");
    const std::array<Value,1> arguments{Value(std::u16string(separator))};
    return text_value(context,context.call(*callable,input,arguments));
}
Value spread_array(Context& context,const Value& input){
    const auto source=require_array(context,input,u"Array");
    const auto values=context.arrayValues(source); // Source's ordinary iterator reads live length/Get.
    const auto result=context.array();
    for(const auto& value:values)
        context.set(Value(result),Key(index_text(context.arrayLength(result))),value);
    return Value(result);
}
void push(Context& context,const Value& input,Value value){
    const Value method=context.get(input,Key(u"push"));
    const auto* callable=std::get_if<ObjectRef>(&method.data);
    if(!callable||context.kind(*callable)!=ObjectKind::function)
        throw Exception("TypeError",u"input.push is not a function");
    const std::array<Value,1> arguments{std::move(value)};
    (void)context.call(*callable,input,arguments);
}
Value tuple(Context& context,std::initializer_list<Value> values){
    const auto result=context.array();
    for(const auto& value:values)push(context,Value(result),value);
    return Value(result);
}
Value map_array(Context& context,const Value& input,const std::function<Value(Value)>& map){
    const auto callback=context.function(u"ColumnParser map",[&map](Context&,Value,std::span<const Value> args){
        return map(args.empty()?Value(Undefined{}):args.front());
    });
    const Value method=context.get(input,Key(u"map"));
    const auto* callable=std::get_if<ObjectRef>(&method.data);
    if(!callable||context.kind(*callable)!=ObjectKind::function)
        throw Exception("TypeError",u"input.map is not a function");
    const std::array<Value,1> arguments{Value(callback)};
    return context.call(*callable,input,arguments);
}
Value slice_array(Context& context,const Value& input,double start){
    const Value method=context.get(input,Key(u"slice"));
    const auto* callable=std::get_if<ObjectRef>(&method.data);
    if(!callable||context.kind(*callable)!=ObjectKind::function)
        throw Exception("TypeError",u"input.slice is not a function");
    const std::array<Value,1> arguments{Value(start)};
    return context.call(*callable,input,arguments);
}
Value pop_array(Context& context,const Value& input){
    const Value method=context.get(input,Key(u"pop"));
    const auto* callable=std::get_if<ObjectRef>(&method.data);
    if(!callable||context.kind(*callable)!=ObjectKind::function)
        throw Exception("TypeError",u"input.pop is not a function");
    return context.call(*callable,input,{});
}
Value state_arg(std::span<const Value> args){return args.empty()?Value(Undefined{}):args.front();}
ColumnState state_from(ColumnParser& owner,Value value){
    return ColumnState(owner.runtime(),std::move(value));
}
[[noreturn]] void throw_tex(std::u16string id,std::u16string message){throw TexError(std::move(id),message);}
[[noreturn]] void throw_tex_arg(std::u16string id,std::u16string message,std::u16string arg){
    const std::array<Property,1> rest{Property(property_host::debugText(arg))};
    throw TexError(std::move(id),message,rest);
}
Value field(Context& context,const Value& receiver,std::u16string_view name){
    return context.get(receiver,key(name));
}
void set_field(Context& context,const Value& receiver,std::u16string_view name,Value value){
    context.set(receiver,key(name),std::move(value));
}
ColumnParser& receiver_column_parser(ParserRuntime& runtime,Value receiver){
    const auto* object=std::get_if<ObjectRef>(&receiver.data);
    if(!object)throw Exception("TypeError",u"ColumnParser method called on incompatible receiver");
    return *runtime.columnParserIdentities.resolve(*object);
}
void define_method(Context& context,ObjectRef prototype,std::u16string name,
                   double length,NativeCall call){
    const auto function=context.function(u"",std::move(call));
    context.define(function,key(u"length"),Descriptor::data(Value(length),false,false,true));
    context.define(prototype,key(std::move(name)),Descriptor::data(Value(function),true,true,true));
}
Value argument(std::span<const Value> args,std::size_t index){
    return index<args.size()?args[index]:Value(Undefined{});
}
struct SourceMethodCall {
    Context* context=nullptr;
    Value receiver=Undefined{};
    ObjectRef function{};
    Value call(std::span<const Value> arguments)const{return context->call(function,receiver,arguments);}
};
SourceMethodCall source_method(Context& context,ObjectRef receiver,std::u16string_view name){
    const Value method=context.get(Value(receiver),key(name));
    const auto* callable=std::get_if<ObjectRef>(&method.data);
    if(!callable||context.kind(*callable)!=ObjectKind::function)
        throw Exception("TypeError",std::u16string(u"this.")+std::u16string(name)+u" is not a function");
    return SourceMethodCall{&context,Value(receiver),*callable};
}
Value call_source_method(Context& context,ObjectRef receiver,std::u16string_view name,
                         std::span<const Value> arguments={}){
    return source_method(context,receiver,name).call(arguments);
}
ObjectRef array_item_identity(ParserRuntime& runtime,TexParser& parser,ArrayItem& array){
    const auto factoryValue=parser.itemFactory();
    const auto* factory=std::get_if<std::shared_ptr<StackItemFactory>>(&factoryValue);
    if(!factory||!*factory)throw Exception("Unimplemented",u"ArrayItem has no owning StackItemFactory");
    const auto product=(*factory)->reference(&array);
    const auto identity=runtime.stackItemIdentities.find(product);
    if(!identity)throw Exception("Unimplemented",u"ArrayItem has no canonical Context identity");
    return *identity;
}
std::u16string from_code_point(std::uint32_t point){
    if(point>0x10ffff)
        throw Exception("RangeError",u"Invalid code point");
    std::u16string result;
    if(point<=0xffff)result.push_back(static_cast<char16_t>(point));
    else{point-=0x10000;result.push_back(static_cast<char16_t>(0xd800+(point>>10)));result.push_back(static_cast<char16_t>(0xdc00+(point&0x3ff)));}
    return result;
}
} // namespace

ColumnState::ColumnState(ParserRuntime& sourceRuntime,options::Value sourceRecord)
    :runtime(&sourceRuntime),record(std::move(sourceRecord)){}
options::Value ColumnState::get(std::u16string_view name)const{
    if(!runtime)throw options::Exception("TypeError",u"ColumnState has no ParserRuntime");
    return runtime->context().get(record,key(name));
}
void ColumnState::set(std::u16string_view name,options::Value value){
    if(!runtime)throw options::Exception("TypeError",u"ColumnState has no ParserRuntime");
    runtime->context().set(record,key(name),std::move(value));
}

ColumnParser::ColumnParser(ParserRuntime& runtime):runtime_(runtime){}
ParserRuntime& ColumnParser::runtime()const{return runtime_;}
options::ObjectRef ColumnParser::sourceIdentity()const{return identity_;}

void ColumnParser::initializeModule(ParserRuntime& runtime){
    Context& ctx=runtime.context();
    const auto* constructor=std::get_if<ObjectRef>(&runtime.columnParserClass.data);
    if(!constructor||ctx.kind(*constructor)!=ObjectKind::function)
        throw Exception("TypeError",u"ColumnParser constructor metadata is not initialized");
    const Value prototypeValue=ctx.get(Value(*constructor),key(u"prototype"));
    const auto* prototype=std::get_if<ObjectRef>(&prototypeValue.data);
    if(!prototype)throw Exception("TypeError",u"ColumnParser.prototype is not an object");

    define_method(ctx,*prototype,u"process",3,[&runtime](Context&,Value receiver,std::span<const Value> args)->Value{
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        const auto parserValue=argument(args,0);
        const auto* parserObject=std::get_if<ObjectRef>(&parserValue.data);
        if(!parserObject)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'configuration')");
        const TexParserRef parser=runtime.parserIdentities.resolve(*parserObject);
        const Value arrayValue=argument(args,2);
        const auto* arrayObject=std::get_if<ObjectRef>(&arrayValue.data);
        if(!arrayObject)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'cstart')");
        const auto stackItem=runtime.stackItemIdentities.resolve(*arrayObject);
        const auto array=std::dynamic_pointer_cast<ArrayItem>(stackItem);
        if(!array)throw Exception("TypeError",u"ColumnParser.process array argument is not an ArrayItem");
        owner.process(*parser,text_value(runtime.context(),argument(args,1)),*array);
        return Value(Undefined{});
    });
    define_method(ctx,*prototype,u"processColumn",2,[&runtime](Context&,Value receiver,std::span<const Value> args)->Value{
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        auto state=state_from(owner,argument(args,0));
        owner.processColumn(state,text_value(runtime.context(),argument(args,1)));
        return Value(Undefined{});
    });
    const auto defineArrayMethod=[&](std::u16string name,auto method){
        define_method(ctx,*prototype,std::move(name),2,[&runtime,method](Context&,Value receiver,std::span<const Value> args)->Value{
            auto& owner=receiver_column_parser(runtime,std::move(receiver));
            auto state=state_from(owner,argument(args,0));
            (owner.*method)(state,argument(args,1));
            return Value(Undefined{});
        });
    };
    defineArrayMethod(u"setColumnAlign",&ColumnParser::setColumnAlign);
    defineArrayMethod(u"setColumnWidths",&ColumnParser::setColumnWidths);
    defineArrayMethod(u"setColumnSpacing",&ColumnParser::setColumnSpacing);
    defineArrayMethod(u"setColumnLines",&ColumnParser::setColumnLines);
    defineArrayMethod(u"setPadding",&ColumnParser::setPadding);
    define_method(ctx,*prototype,u"getColumn",2,[&runtime](Context&,Value receiver,std::span<const Value> args){
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        auto state=state_from(owner,argument(args,0));
        const Value rawRalign=argument(args,1),rawCalign=argument(args,2);
        owner.getColumn(state,rawRalign,rawCalign);
        return Value(Undefined{});
    });
    define_method(ctx,*prototype,u"getDimen",1,[&runtime](Context&,Value receiver,std::span<const Value> args){
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        auto state=state_from(owner,argument(args,0));
        return Value(owner.getDimen(state));
    });
    define_method(ctx,*prototype,u"getAlign",1,[&runtime](Context&,Value receiver,std::span<const Value> args){
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        auto state=state_from(owner,argument(args,0));
        return Value(owner.getAlign(state));
    });
    define_method(ctx,*prototype,u"getBraces",1,[&runtime](Context&,Value receiver,std::span<const Value> args){
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        auto state=state_from(owner,argument(args,0));
        return Value(owner.getBraces(state));
    });
    define_method(ctx,*prototype,u"macroColumn",3,[&runtime](Context&,Value receiver,std::span<const Value> args){
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        auto state=state_from(owner,argument(args,0));
        const auto macro=text_value(runtime.context(),argument(args,1));
        const double count=number_value(runtime.context(),argument(args,2));
        owner.macroColumn(state,macro,count);
        return Value(Undefined{});
    });
    define_method(ctx,*prototype,u"addRule",2,[&runtime](Context&,Value receiver,std::span<const Value> args){
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        auto state=state_from(owner,argument(args,0));
        owner.addRule(state,text_value(runtime.context(),argument(args,1)));
        return Value(Undefined{});
    });
    define_method(ctx,*prototype,u"addAt",2,[&runtime](Context&,Value receiver,std::span<const Value> args){
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        auto state=state_from(owner,argument(args,0));
        owner.addAt(state,text_value(runtime.context(),argument(args,1)));
        return Value(Undefined{});
    });
    define_method(ctx,*prototype,u"addBang",2,[&runtime](Context&,Value receiver,std::span<const Value> args){
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        auto state=state_from(owner,argument(args,0));
        owner.addBang(state,text_value(runtime.context(),argument(args,1)));
        return Value(Undefined{});
    });
    define_method(ctx,*prototype,u"repeat",1,[&runtime](Context&,Value receiver,std::span<const Value> args){
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        auto state=state_from(owner,argument(args,0));
        owner.repeat(state);
        return Value(Undefined{});
    });
    define_method(ctx,*prototype,u"brace",1,[&runtime](Context&,Value receiver,std::span<const Value> args){
        auto& owner=receiver_column_parser(runtime,std::move(receiver));
        auto state=state_from(owner,argument(args,0));
        owner.brace(state);
        return Value(Undefined{});
    });
}

void ColumnParser::initializeColumnParser(std::shared_ptr<void> actualOwner){
    Context& context=runtime_.context();
    const auto* constructor=std::get_if<ObjectRef>(&runtime_.columnParserClass.data);
    if(!constructor||context.kind(*constructor)!=ObjectKind::function)
        throw Exception("TypeError",u"ColumnParser constructor metadata is not initialized");
    identity_=runtime_.columnParserIdentities.instance(this,*constructor,std::move(actualOwner));
    const ObjectRef ownerIdentity=identity_;
    const ObjectRef handlers=context.object();
    const auto state_value=[this](std::span<const Value> args)->ColumnState{
        return state_from(*this,state_arg(args));
    };
    auto add=[&](std::u16string_view name,NativeCall call){
        const auto function=context.function(std::u16string(name),std::move(call));
        context.define(function,key(u"length"),Descriptor::data(Value(1),false,false,true));
        context.set(Value(handlers),key(name),Value(function));
    };
    add(u"l",[this,state_value](Context& active,Value,std::span<const Value> args){
        auto state=state_value(args);
        const Value align=array_field(active,state,u"calign");
        const double index=number_value(active,state.get(u"j"));
        state.set(u"j",index+1);
        write_index(active,align,index,u"left"); return Value(Undefined{});
    });
    add(u"c",[this,state_value](Context& active,Value,std::span<const Value> args){
        auto state=state_value(args);
        const Value align=array_field(active,state,u"calign");
        const double index=number_value(active,state.get(u"j")); state.set(u"j",index+1);
        write_index(active,align,index,u"center"); return Value(Undefined{});
    });
    add(u"r",[this,state_value](Context& active,Value,std::span<const Value> args){
        auto state=state_value(args);
        const Value align=array_field(active,state,u"calign");
        const double index=number_value(active,state.get(u"j")); state.set(u"j",index+1);
        write_index(active,align,index,u"right"); return Value(Undefined{});
    });
    const auto col=[this,state_value,ownerIdentity](std::u16string ralign,std::u16string calign){
        return [this,state_value,ownerIdentity,ralign=std::move(ralign),calign=std::move(calign)](Context& active,Value,std::span<const Value> args){
            auto state=state_value(args);
            const std::array<Value,3> values{state.record,Value(ralign),Value(calign)};
            (void)call_source_method(active,ownerIdentity,u"getColumn",values); return Value(Undefined{});
        };
    };
    add(u"p",col(u"top",u"left")); add(u"m",col(u"middle",u"left")); add(u"b",col(u"bottom",u"left"));
    add(u"w",col(u"top",u"")); add(u"W",col(u"top",u""));
    add(u"|",[state_value,ownerIdentity](Context& active,Value,std::span<const Value> args){auto s=state_value(args);const std::array<Value,2> values{s.record,Value(u"solid")};(void)call_source_method(active,ownerIdentity,u"addRule",values);return Value(Undefined{});});
    add(u":",[state_value,ownerIdentity](Context& active,Value,std::span<const Value> args){auto s=state_value(args);const std::array<Value,2> values{s.record,Value(u"dashed")};(void)call_source_method(active,ownerIdentity,u"addRule",values);return Value(Undefined{});});
    add(u">",[state_value,ownerIdentity](Context& active,Value,std::span<const Value> args){auto s=state_value(args);const auto start=array_field(active,s,u"cstart");const auto j=number_value(active,s.get(u"j"));const auto getBraces=source_method(active,ownerIdentity,u"getBraces");const std::array<Value,1> values{s.record};const auto braces=text_value(active,getBraces.call(values));const auto old=read_index(active,start,j);const auto text=truthy(old)?text_value(active,old):std::u16string{};write_index(active,start,j,braces+text);return Value(Undefined{});});
    add(u"<",[state_value,ownerIdentity](Context& active,Value,std::span<const Value> args){auto s=state_value(args);const auto end=array_field(active,s,u"cend");const auto j=number_value(active,s.get(u"j"))-1;const auto getBraces=source_method(active,ownerIdentity,u"getBraces");const std::array<Value,1> values{s.record};const auto braces=text_value(active,getBraces.call(values));const auto old=read_index(active,end,j);const auto text=truthy(old)?text_value(active,old):std::u16string{};write_index(active,end,j,braces+text);return Value(Undefined{});});
    add(u"@",[state_value,ownerIdentity](Context& active,Value,std::span<const Value> args){auto s=state_value(args);const auto addAt=source_method(active,ownerIdentity,u"addAt");const auto getBraces=source_method(active,ownerIdentity,u"getBraces");const std::array<Value,1> braceArgs{s.record};const Value macro=getBraces.call(braceArgs);const std::array<Value,2> values{s.record,macro};(void)addAt.call(values);return Value(Undefined{});});
    add(u"!",[state_value,ownerIdentity](Context& active,Value,std::span<const Value> args){auto s=state_value(args);const auto addBang=source_method(active,ownerIdentity,u"addBang");const auto getBraces=source_method(active,ownerIdentity,u"getBraces");const std::array<Value,1> braceArgs{s.record};const Value macro=getBraces.call(braceArgs);const std::array<Value,2> values{s.record,macro};(void)addBang.call(values);return Value(Undefined{});});
    add(u"*",[state_value,ownerIdentity](Context& active,Value,std::span<const Value> args){auto s=state_value(args);const std::array<Value,1> values{s.record};(void)call_source_method(active,ownerIdentity,u"repeat",values);return Value(Undefined{});});
    add(u"{",[state_value,ownerIdentity](Context& active,Value,std::span<const Value> args){auto s=state_value(args);const std::array<Value,1> values{s.record};(void)call_source_method(active,ownerIdentity,u"brace",values);return Value(Undefined{});});
    add(u"P",[state_value,ownerIdentity](Context& active,Value,std::span<const Value> args){auto s=state_value(args);const std::array<Value,3> values{s.record,Value(u">{$}p{#1}<{$}"),Value(1)};(void)call_source_method(active,ownerIdentity,u"macroColumn",values);return Value(Undefined{});});
    add(u"M",[state_value,ownerIdentity](Context& active,Value,std::span<const Value> args){auto s=state_value(args);const std::array<Value,3> values{s.record,Value(u">{$}m{#1}<{$}"),Value(1)};(void)call_source_method(active,ownerIdentity,u"macroColumn",values);return Value(Undefined{});});
    add(u"B",[state_value,ownerIdentity](Context& active,Value,std::span<const Value> args){auto s=state_value(args);const std::array<Value,3> values{s.record,Value(u">{$}b{#1}<{$}"),Value(1)};(void)call_source_method(active,ownerIdentity,u"macroColumn",values);return Value(Undefined{});});
    add(u" ",[](Context&,Value,std::span<const Value>){return Value(Undefined{});});
    add(u"\n",[](Context&,Value,std::span<const Value>){return Value(Undefined{});});
    context.set(Value(identity_),key(u"columnHandler"),Value(handlers));
    context.set(Value(identity_),key(u"MAXCOLUMNS"),Value(10000));
}

void ColumnParser::process(TexParser& parser,std::u16string templateText,ArrayItem& array){
    Context& ctx=runtime_.context();
    const Value arrayIdentity(array_item_identity(runtime_,parser,array));
    const ObjectRef stateRecord=ctx.object();
    runtime_.trackRequestContextRecord(stateRecord);
    ColumnState state(runtime_,Value(stateRecord));
    const auto parserObject=parser.instanceIdentity();
    const auto put=[&](std::u16string_view name,Value value){state.set(name,std::move(value));};
    put(u"parser",Value(parserObject)); put(u"template",templateText);
    put(u"i",0); put(u"j",0); put(u"c",u"");
    const auto requestArray=[&](){const ObjectRef record=ctx.array();runtime_.trackRequestContextRecord(record);return Value(record);};
    put(u"cwidth",requestArray()); put(u"calign",requestArray());
    put(u"cspace",requestArray()); put(u"clines",requestArray());
    put(u"cstart",ctx.get(arrayIdentity,key(u"cstart")));
    put(u"cend",ctx.get(arrayIdentity,key(u"cend")));
    put(u"ralign",ctx.get(arrayIdentity,key(u"ralign")));
    put(u"cextra",ctx.get(arrayIdentity,key(u"cextra")));
    if(!templateText.empty()&&templateText.front()==u'{'&&templateText.back()==u'}'){
        const std::array<Value,1> args{state.record};
        const auto braced=text_value(ctx,call_source_method(ctx,identity_,u"getBraces",args));
        if(braced.size()==templateText.size()-2)state.set(u"template",braced);
        state.set(u"i",0);
    }
    double n=0;
    for(;;){
        const double i=number_value(ctx,state.get(u"i"));
        const Value templateForLength=state.get(u"template");
        const Value templateLength=ctx.get(templateForLength,key(u"length"));
        if(!(i<number_value(ctx,templateLength)))break;
        const double examined=n++;
        const double limit=number_value(ctx,ctx.get(Value(identity_),key(u"MAXCOLUMNS")));
        if(examined>limit)throw TexError(u"MaxColumns",u"Too many column specifiers (perhaps looping column definitions?)");
        const Value templateForCode=state.get(u"template");
        const auto* templateText=std::get_if<std::u16string>(&templateForCode.data);
        if(!templateText)throw Exception("TypeError",u"state.template.codePointAt is not a function");
        double codeIndex=number_value(ctx,state.get(u"i"));
        if(std::isnan(codeIndex))codeIndex=0;
        std::optional<std::uint32_t> code;
        const double integerIndex=std::trunc(codeIndex);
        if(integerIndex>=0&&std::isfinite(integerIndex)){
            const auto position=static_cast<std::size_t>(integerIndex);
            code=utf16::codePointAt(*templateText,position);
        }
        if(!code)throw Exception("RangeError",u"Invalid code point NaN");
        const std::u16string c=from_code_point(*code);
        state.set(u"c",c);
        const double currentIndex=number_value(ctx,state.get(u"i"));
        state.set(u"i",currentIndex+static_cast<double>(c.size()));
        const std::array<Value,2> args{state.record,Value(c)};
        (void)call_source_method(ctx,identity_,u"processColumn",args);
    }
    const std::array<Value,2> arguments{state.record,arrayIdentity};
    (void)call_source_method(ctx,identity_,u"setColumnAlign",arguments);
    (void)call_source_method(ctx,identity_,u"setColumnWidths",arguments);
    (void)call_source_method(ctx,identity_,u"setColumnSpacing",arguments);
    (void)call_source_method(ctx,identity_,u"setColumnLines",arguments);
    (void)call_source_method(ctx,identity_,u"setPadding",arguments);
}

void ColumnParser::processColumn(ColumnState& state,std::u16string c){
    Context& ctx=runtime_.context();
    const Key handlerKey(c);
    const Value firstHandlers=ctx.get(Value(identity_),key(u"columnHandler"));
    if(!ctx.hasOwn(firstHandlers,handlerKey)){
        const std::array<Property,1> rest{Property(property_host::debugText(c))};
        throw TexError(u"BadPreamToken",u"Illegal pream-token (%1)",rest);
    }
    const Value handlers=ctx.get(Value(identity_),key(u"columnHandler"));
    const Value fn=ctx.get(handlers,handlerKey);
    const auto* callable=std::get_if<ObjectRef>(&fn.data);
    if(!callable||ctx.kind(*callable)!=ObjectKind::function)
        throw Exception("TypeError",u"this.columnHandler[c] is not a function");
    const Value stateObject=state.record;
    (void)ctx.call(*callable,handlers,std::span<const Value>(&stateObject,1));
}

void ColumnParser::setColumnAlign(ColumnState& state,Value array){
    Context& ctx=runtime_.context();
    const Value definition=ctx.get(array,key(u"arraydef"));
    const Value align=array_field(ctx,state,u"calign");
    const auto joined=join_array(ctx,align,u" ");
    ctx.set(definition,key(u"columnalign"),joined);
}
void ColumnParser::setColumnWidths(ColumnState& state,Value array){
    Context& ctx=runtime_.context();
    const Value widthForGuard=state.get(u"cwidth");
    const Value widthLength=ctx.get(widthForGuard,key(u"length"));
    if(!truthy(widthLength))return;
    const Value widthForSpread=state.get(u"cwidth");
    const Value cwidth=spread_array(ctx,widthForSpread);
    const ObjectRef copy=require_array(ctx,cwidth,u"cwidth");
    if(ctx.arrayLength(copy)<ctx.arrayLength(require_array(ctx,state.get(u"calign"),u"calign")))push(ctx,cwidth,u"auto");
    const Value definition=ctx.get(array,key(u"arraydef"));
    const Value mapped=map_array(ctx,cwidth,[](Value w){return truthy(w)?w:Value(u"auto");});
    const auto joined=join_array(ctx,mapped,u" ");
    ctx.set(definition,key(u"columnwidth"),joined);
}
void ColumnParser::setColumnSpacing(ColumnState& state,Value array){
    Context& ctx=runtime_.context();
    const Value spacing=state.get(u"cspace");
    const auto source=require_array(ctx,spacing,u"cspace");
    if(ctx.arrayLength(source)==0)return;
    const Value spacingForSpread=state.get(u"cspace");
    const Value copy=spread_array(ctx,spacingForSpread);
    const auto copyObject=require_array(ctx,copy,u"cspace");
    const Value align=state.get(u"calign");
    if(ctx.arrayLength(copyObject)<ctx.arrayLength(require_array(ctx,align,u"calign")))push(ctx,copy,u"1em");
    const Value definition=ctx.get(array,key(u"arraydef"));
    const Value suffix=slice_array(ctx,copy,1);
    const Value mapped=map_array(ctx,suffix,[](Value item){return truthy(item)?item:Value(u"1em");});
    const auto joined=join_array(ctx,mapped,u" ");
    ctx.set(definition,key(u"columnspacing"),joined);
}
void ColumnParser::setColumnLines(ColumnState& state,Value array){
    Context& ctx=runtime_.context();
    const Value linesForGuard=state.get(u"clines");
    const auto count=ctx.arrayLength(require_array(ctx,linesForGuard,u"clines"));
    if(!count)return;
    const Value linesForSpread=state.get(u"clines");
    Value work=spread_array(ctx,linesForSpread);
    auto workArray=require_array(ctx,work,u"clines");
    const auto first=ctx.get(work,key(u"0"));
    if(truthy(first)){
        const Value frame=ctx.get(array,key(u"frame"));
        const Value method=ctx.get(frame,key(u"push"));
        const auto* callable=std::get_if<ObjectRef>(&method.data);
        if(!callable||ctx.kind(*callable)!=ObjectKind::function)
            throw Exception("TypeError",u"array.frame.push is not a function");
        const Value pushedLine=ctx.get(work,key(u"0"));
        const std::array<Value,1> arguments{tuple(ctx,{Value(u"left"),pushedLine})};
        (void)ctx.call(*callable,frame,arguments);
    }
    const auto currentLength=ctx.arrayLength(workArray);
    const Value alignForCompare=state.get(u"calign");
    const auto alignLength=ctx.arrayLength(require_array(ctx,alignForCompare,u"calign"));
    if(currentLength>alignLength){
        const Value frame=ctx.get(array,key(u"frame"));
        const Value method=ctx.get(frame,key(u"push"));
        const auto* callable=std::get_if<ObjectRef>(&method.data);
        if(!callable||ctx.kind(*callable)!=ObjectKind::function)
            throw Exception("TypeError",u"array.frame.push is not a function");
        const Value last=pop_array(ctx,work);
        const std::array<Value,1> arguments{tuple(ctx,{Value(u"right"),last})};
        (void)ctx.call(*callable,frame,arguments);
    }else{
        const auto lessLength=ctx.arrayLength(workArray);
        const Value alignForLess=state.get(u"calign");
        const auto lessAlignLength=ctx.arrayLength(require_array(ctx,alignForLess,u"calign"));
        if(lessLength<lessAlignLength)push(ctx,work,u"none");
    }
    const auto workLength=ctx.arrayLength(workArray);
    if(workLength>1){
        const Value definition=ctx.get(array,key(u"arraydef"));
        const Value suffix=slice_array(ctx,work,1);
        const Value mapped=map_array(ctx,suffix,[](Value item){return truthy(item)?item:Value(u"none");});
        const auto joined=join_array(ctx,mapped,u" ");
        ctx.set(definition,key(u"columnlines"),joined);
    }
}
void ColumnParser::setPadding(ColumnState& state,Value array){
    Context& ctx=runtime_.context();
    const Value firstExtraArray=state.get(u"cextra");
    const Value firstExtra=read_index(ctx,firstExtraArray,0);
    if(!truthy(firstExtra)){
        const Value lastExtraArray=state.get(u"cextra");
        const Value alignForGuard=state.get(u"calign");
        const auto alignLength=ctx.arrayLength(require_array(ctx,alignForGuard,u"calign"));
        const Value lastExtra=read_index(ctx,lastExtraArray,static_cast<double>(alignLength)-1);
        if(!truthy(lastExtra))return;
    }
    const Value alignForI=state.get(u"calign");
    const double i=static_cast<double>(ctx.arrayLength(require_array(ctx,alignForI,u"calign")))-1;
    const Value extraForI=state.get(u"cextra");
    const Value spacing=state.get(u"cspace");
    const Value atEnd=read_index(ctx,extraForI,i);
    const Value space=truthy(atEnd)?read_index(ctx,spacing,i):Value(Null{});
    const Value definition=ctx.get(array,key(u"arraydef"));
    const Value start=read_index(ctx,spacing,0);
    const std::u16string left=truthy(start)?text_value(ctx,start):u".5em";
    const std::u16string right=truthy(space)?text_value(ctx,space):u".5em";
    ctx.set(definition,key(u"data-array-padding"),left+u" "+right);
}

void ColumnParser::getColumn(ColumnState& state,Value ralign,Value calign){
    Context& ctx=runtime_.context();
    const Value alignForSet=state.get(u"calign");
    const double alignIndex=number_value(ctx,state.get(u"j"));
    if(std::holds_alternative<Undefined>(calign.data))calign=Value(u"left");
    if(!truthy(calign)){
        const std::array<Value,1> arguments{state.record};
        calign=call_source_method(ctx,identity_,u"getAlign",arguments);
    }
    write_index(ctx,alignForSet,alignIndex,calign);

    const Value widthsForSet=state.get(u"cwidth");
    const double widthIndex=number_value(ctx,state.get(u"j"));
    const std::array<Value,1> dimensionArguments{state.record};
    const Value dimension=call_source_method(ctx,identity_,u"getDimen",dimensionArguments);
    write_index(ctx,widthsForSet,widthIndex,dimension);

    const Value ralignArray=state.get(u"ralign");
    const double ralignIndex=number_value(ctx,state.get(u"j"));
    const Value widthArrayForValue=state.get(u"cwidth");
    const double widthValueIndex=number_value(ctx,state.get(u"j"));
    const Value widthValue=read_index(ctx,widthArrayForValue,widthValueIndex);
    const Value alignArrayForValue=state.get(u"calign");
    const double alignValueIndex=number_value(ctx,state.get(u"j"));
    const Value alignValue=read_index(ctx,alignArrayForValue,alignValueIndex);
    write_index(ctx,ralignArray,ralignIndex,tuple(ctx,{ralign,widthValue,alignValue}));
    state.set(u"j",number_value(ctx,state.get(u"j"))+1);
}
std::u16string ColumnParser::getDimen(ColumnState& state){
    Context& ctx=runtime_.context();
    const std::array<Value,1> arguments{state.record};
    const std::u16string dim=text_value(ctx,call_source_method(ctx,identity_,u"getBraces",arguments));
    const auto match=UnitUtil::instance().matchDimen(Property(property_host::debugText(dim)));
    if(!property_host::truthy(match[0])){
        const std::array<Property,1> rest{Property(property_host::debugText(text_value(ctx,state.get(u"c"))))};
        throw TexError(u"MissingColumnDimOrUnits",u"Missing dimension or its units for %1 column declaration",rest);
    }
    return dim;
}
std::u16string ColumnParser::getAlign(ColumnState& state){
    Context& ctx=runtime_.context();
    const std::array<Value,1> arguments{state.record};
    const std::u16string align=js_case::lowerCase(text_value(ctx,call_source_method(ctx,identity_,u"getBraces",arguments)));
    if(align==u"l")return u"left";
    if(align==u"c")return u"center";
    if(align==u"r")return u"right";
    return {};
}
std::u16string ColumnParser::getBraces(ColumnState& state){
    Context& ctx=runtime_.context();
    for(;;){
        const std::u16string text=text_value(ctx,state.get(u"template"));
        const double i=number_value(ctx,state.get(u"i"));
        const bool indexed=std::isfinite(i)&&i>=0&&std::trunc(i)==i&&i<static_cast<double>(text.size());
        if(!indexed||text[static_cast<std::size_t>(i)]!=u' ')break;
        state.set(u"i",i+1);
    }
    double i=number_value(ctx,state.get(u"i"));
    const Value templateForLength=state.get(u"template");
    const Value lengthForMissing=ctx.get(templateForLength,key(u"length"));
    if(i>=number_value(ctx,lengthForMissing))
        throw_tex_arg(u"MissingArgForColumn",u"Missing argument for %1 column declaration",text_value(ctx,state.get(u"c")));
    std::u16string text=text_value(ctx,state.get(u"template"));
    i=number_value(ctx,state.get(u"i"));
    const bool indexed=std::isfinite(i)&&i>=0&&std::trunc(i)==i&&i<static_cast<double>(text.size());
    std::size_t position=indexed?static_cast<std::size_t>(i):text.size();
    if(!indexed||text[position]!=u'{'){
        const std::u16string result=indexed?std::u16string(1,text[position]):std::u16string{};
        state.set(u"i",i+1);
        return result;
    }
    i+=1;state.set(u"i",i);
    const double start=i;
    std::size_t braces=1;
    while(true){
        i=number_value(ctx,state.get(u"i"));
        const Value templateForLength=state.get(u"template");
        const Value currentLength=ctx.get(templateForLength,key(u"length"));
        if(!(i<number_value(ctx,currentLength)))break;
        text=text_value(ctx,state.get(u"template"));
        i=number_value(ctx,state.get(u"i"));
        const double charIndex=std::trunc(i);
        position=std::isfinite(charIndex)&&charIndex>=0?static_cast<std::size_t>(charIndex):text.size();
        if(!(i<static_cast<double>(text.size())))break;
        const char16_t ch=position<text.size()?text[position]:char16_t{};
        state.set(u"i",i+1);
        switch(ch){
        case u'\\':{
            const double after=number_value(ctx,state.get(u"i"));
            state.set(u"i",after+1);
            break;
        }
        case u'{':++braces;break;
        case u'}':
            if(--braces==0){
                const auto original=text_value(ctx,state.get(u"template"));
                const auto end=number_value(ctx,state.get(u"i"))-1;
                const auto begin=static_cast<std::size_t>(start);
                const auto stop=static_cast<std::size_t>(std::max(0.0,end));
                return original.substr(begin,stop>=begin?stop-begin:0);
            }
            break;
        }
    }
    throw_tex(u"MissingCloseBrace",u"Missing close brace");
}
void ColumnParser::macroColumn(ColumnState& state,std::u16string macro,double n){
    Context& ctx=runtime_.context();
    const auto args=ctx.array();
    while(n>0&&n--){
        const Value pushMethod=ctx.get(Value(args),key(u"push"));
        const auto* callable=std::get_if<ObjectRef>(&pushMethod.data);
        if(!callable||ctx.kind(*callable)!=ObjectKind::function)
            throw Exception("TypeError",u"args.push is not a function");
        const std::array<Value,1> braceArguments{state.record};
        const auto arg=text_value(ctx,call_source_method(ctx,identity_,u"getBraces",braceArguments));
        const std::array<Value,1> values{Value(arg)};
        (void)ctx.call(*callable,Value(args),values);
    }
    const auto parserValue=state.get(u"parser");
    const auto* parserObject=std::get_if<ObjectRef>(&parserValue.data);
    if(!parserObject)throw Exception("TypeError",u"Cannot read properties of undefined (reading 'configuration')");
    TexParser& parser=*runtime_.parserIdentities.resolve(*parserObject);
    const std::u16string substituted=ParseUtil::substituteArgs(parser,Value(args),std::move(macro));
    const std::u16string original=text_value(ctx,state.get(u"template"));
    const double i=number_value(ctx,state.get(u"i"));
    state.set(u"template",substituted+original.substr(substring_index(i,original.size())));
    state.set(u"i",0);
}
void ColumnParser::addRule(ColumnState& state,std::u16string rule){
    Context& ctx=runtime_.context();
    const Value linesForTest=state.get(u"clines");
    const double testIndex=number_value(ctx,state.get(u"j"));
    if(truthy(read_index(ctx,linesForTest,testIndex))){
        const std::array<Value,2> arguments{state.record,Value(u"\\,")};
        (void)call_source_method(ctx,identity_,u"addAt",arguments);
    }
    const Value linesForWrite=state.get(u"clines");
    const double writeIndex=number_value(ctx,state.get(u"j"));
    write_index(ctx,linesForWrite,writeIndex,rule);
    const Value spacing=state.get(u"cspace");
    const double spacingIndex=number_value(ctx,state.get(u"j"));
    if(read_index(ctx,spacing,spacingIndex)==Value(u"0")){
        const Value starts=state.get(u"cstart");
        const double startIndex=number_value(ctx,state.get(u"j"));
        write_index(ctx,starts,startIndex,u"\\hspace{.5em}");
    }
}
void ColumnParser::addAt(ColumnState& state,std::u16string macro){
    Context& ctx=runtime_.context();
    const Value starts=array_field(ctx,state,u"cstart"),spacing=array_field(ctx,state,u"cspace");
    const double j=number_value(ctx,state.get(u"j"));
    write_index(ctx,array_field(ctx,state,u"cextra"),j,true);
    write_index(ctx,array_field(ctx,state,u"calign"),j,u"center");
    if(truthy(read_index(ctx,array_field(ctx,state,u"clines"),j))){
        if(read_index(ctx,spacing,j)==Value(u".5em")){
            const Value old=read_index(ctx,starts,j-1);
            write_index(ctx,starts,j-1,text_value(ctx,old)+u"\\hspace{.25em}");
        }else if(!truthy(read_index(ctx,spacing,j))){
            const Value ends=array_field(ctx,state,u"cend");
            const Value old=read_index(ctx,ends,j-1);
            write_index(ctx,ends,j-1,(truthy(old)?text_value(ctx,old):u"")+u"\\hspace{.5em}");
        }
    }
    write_index(ctx,starts,j,std::move(macro));
    write_index(ctx,spacing,j,u"0");
    state.set(u"j",j+1);
    write_index(ctx,spacing,j+1,u"0");
}
void ColumnParser::addBang(ColumnState& state,std::u16string macro){
    Context& ctx=runtime_.context();
    const Value starts=array_field(ctx,state,u"cstart"),spacing=array_field(ctx,state,u"cspace");
    const double j=number_value(ctx,state.get(u"j"));
    write_index(ctx,array_field(ctx,state,u"cextra"),j,true);
    write_index(ctx,array_field(ctx,state,u"calign"),j,u"center");
    const bool before=read_index(ctx,spacing,j)==Value(u"0")&&truthy(read_index(ctx,array_field(ctx,state,u"clines"),j));
    write_index(ctx,starts,j,(before?u"\\hspace{.25em}":u"")+macro);
    if(!truthy(read_index(ctx,spacing,j)))write_index(ctx,spacing,j,u".5em");
    state.set(u"j",j+1);
    write_index(ctx,spacing,j+1,u".5em");
}
void ColumnParser::repeat(ColumnState& state){
    Context& ctx=runtime_.context();
    const std::array<Value,1> arguments{state.record};
    const std::u16string num=text_value(ctx,call_source_method(ctx,identity_,u"getBraces",arguments));
    const std::u16string cols=text_value(ctx,call_source_method(ctx,identity_,u"getBraces",arguments));
    const double count=js_input::parseInt(num);
    const std::u16string text=property_host::text(js_number::toString(count));
    if(text!=num)throw_tex_arg(u"ColArgNotNum",u"First argument to %1 column specifier must be a number",u"*");
    if(!std::isfinite(count)||count<0||count>4294967295.0||std::trunc(count)!=count)
        throw Exception("RangeError",u"Invalid array length");
    const ObjectRef repeatedArray=ctx.array(static_cast<std::uint32_t>(count));
    const Value fillMethod=ctx.get(Value(repeatedArray),key(u"fill"));
    const auto* fill=std::get_if<ObjectRef>(&fillMethod.data);
    if(!fill||ctx.kind(*fill)!=ObjectKind::function)
        throw Exception("TypeError",u"new Array(n).fill is not a function");
    const std::array<Value,1> fillArguments{Value(cols)};
    const Value filled=ctx.call(*fill,Value(repeatedArray),fillArguments);
    const Value joinMethod=ctx.get(filled,key(u"join"));
    const auto* join=std::get_if<ObjectRef>(&joinMethod.data);
    if(!join||ctx.kind(*join)!=ObjectKind::function)
        throw Exception("TypeError",u"filled.join is not a function");
    const std::array<Value,1> joinArguments{Value(std::u16string{})};
    const std::u16string repeated=text_value(ctx,ctx.call(*join,filled,joinArguments));
    const auto original=text_value(ctx,state.get(u"template"));
    const double i=number_value(ctx,state.get(u"i"));
    const auto position=substring_index(i,original.size());
    state.set(u"template",repeated+original.substr(position));
    state.set(u"i",0);
}
void ColumnParser::brace(ColumnState& state){
    const double i=number_value(runtime_.context(),state.get(u"i"));
    state.set(u"i",i-1);
    Context& ctx=runtime_.context();
    const auto process=source_method(ctx,identity_,u"processColumn");
    const std::array<Value,1> arguments{state.record};
    const Value column=call_source_method(ctx,identity_,u"getBraces",arguments);
    const std::array<Value,2> processArguments{state.record,column};
    (void)process.call(processArguments);
}

} // namespace folia::math::tex
