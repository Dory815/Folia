// MathJax 4.1.3 core/MathDocument.ts RenderList implementation.
module folia.math.core.math_render_actions;
import folia.math.util.property_host;
import std;

namespace folia::math::core {
namespace {
using namespace options;

Value indexed(Context& context,Value object,std::size_t index){
    auto ascii=std::to_string(index);
    return context.get(std::move(object),Key(std::u16string(ascii.begin(),ascii.end())));
}
Value argument(std::span<const Value> args,std::size_t index){
    return index<args.size()?args[index]:Value(Undefined{});
}
bool isString(const Value& value){return std::holds_alternative<std::u16string>(value.data);}
bool isBoolean(const Value& value){return std::holds_alternative<bool>(value.data);}
bool isFunction(Context& context,const Value& value){
    const auto* object=std::get_if<ObjectRef>(&value.data);
    return object&&context.kind(*object)==ObjectKind::function;
}
std::u16string stringValue(const Value& value){
    if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
    throw Exception("TypeError",u"RenderList action method name is not a string");
}
std::optional<double> numberValue(const Value& value){
    if(const auto* number=std::get_if<double>(&value.data))return *number;
    return {};
}
RenderDataRef renderData(const PrioritizedListItemHandle<RenderDataRef>& record){
    if(!record.native()||!record.nativeRecord())
        throw Exception("TypeError",u"RenderList item is not a native RenderData record");
    const auto item=record.readItem();
    if(const auto* data=std::get_if<RenderDataRef>(&item);data&&*data)return *data;
    throw Exception("TypeError",u"RenderList item is not a RenderData value");
}
}

RenderActions::RenderActions(Context& context,ObjectRef identity)
    :context_(context),identity_(identity){
    (void)context_.kind(identity_);
}

RenderDataRef RenderActions::makeData(std::u16string id,Value renderDoc,
                                       Value renderMath,Value convert){
    auto data=std::make_shared<RenderData>();
    data->identity=context_.object();
    context_.payload(data->identity)=data;
    context_.set(data->identity,Key(u"id"),Value(std::move(id)));
    context_.set(data->identity,Key(u"renderDoc"),std::move(renderDoc));
    context_.set(data->identity,Key(u"renderMath"),std::move(renderMath));
    context_.set(data->identity,Key(u"convert"),std::move(convert));
    return data;
}

ObjectRef RenderActions::methodAction(std::u16string method,bool documentMethod){
    return context_.function(documentMethod?u"renderDoc":u"renderMath",
        [method=std::move(method),documentMethod](Context& context,Value,
                                                 std::span<const Value> args)->Value{
            // Source's methodActions wrapper tests the captured method name at
            // call time, then dynamically gets the member from the actual
            // document or math object and calls it with that same receiver.
            if(method.empty())return Value(false);
            const Value target=argument(args,documentMethod?0:0);
            const Value function=context.get(target,Key(method));
            if(documentMethod){
                (void)context.callMember(function,target,{} ,u"RenderList document action");
            }else{
                const Value document=argument(args,1);
                const std::array<Value,1> callArgs{document};
                (void)context.callMember(function,target,callArgs,u"RenderList math action");
            }
            return Value(false);
        });
}

std::pair<RenderDataRef,Value> RenderActions::action(std::u16string id,
                                                     Value definition){
    // The access pattern intentionally keeps each `length`, index and slice
    // operation at its Source evaluation point. Accessors and mutable action
    // tuples therefore remain observable in the same order.
    Value priority=indexed(context_,definition,0);
    Value renderDoc(Undefined{}),renderMath(Undefined{}),convert(true);
    const Value lengthForSingle=context_.get(definition,Key(u"length"));
    const bool single=numberValue(lengthForSingle)==1;
    Value second(Undefined{});
    if(!single)second=indexed(context_,definition,1);
    if(single||isBoolean(second)){
        const Value lengthForConvert=context_.get(definition,Key(u"length"));
        if(numberValue(lengthForConvert)==2)convert=indexed(context_,definition,1);
        renderDoc=Value(methodAction(id,true));
        renderMath=Value(methodAction(id,false));
    }else if(isString(second)){
        const Value third=indexed(context_,definition,2);
        if(isString(third)){
            const Value lengthForConvert=context_.get(definition,Key(u"length"));
            if(numberValue(lengthForConvert)==4)convert=indexed(context_,definition,3);
            const Value slice=context_.get(definition,Key(u"slice"));
            const std::array<Value,1> sliceArgs{Value(1)};
            const Value suffix=context_.callMember(slice,definition,sliceArgs,
                                                   u"RenderList action.slice");
            const Value values=context_.readIterable(suffix,2);
            const auto method1=context_.get(values,Key(u"0"));
            const auto method2=context_.get(values,Key(u"1"));
            renderDoc=Value(methodAction(stringValue(method1),true));
            renderMath=Value(methodAction(stringValue(method2),false));
        }else{
            const Value lengthForConvert=context_.get(definition,Key(u"length"));
            if(numberValue(lengthForConvert)==3)convert=indexed(context_,definition,2);
            const auto method=stringValue(second);
            renderDoc=Value(methodAction(method,true));
            renderMath=Value(methodAction(method,false));
        }
    }else{
        const Value lengthForConvert=context_.get(definition,Key(u"length"));
        if(numberValue(lengthForConvert)==4)convert=indexed(context_,definition,3);
        const Value slice=context_.get(definition,Key(u"slice"));
        const std::array<Value,1> sliceArgs{Value(1)};
        const Value suffix=context_.callMember(slice,definition,sliceArgs,
                                               u"RenderList action.slice");
        const Value values=context_.readIterable(suffix,2);
        renderDoc=context_.get(values,Key(u"0"));
        renderMath=context_.get(values,Key(u"1"));
    }
    auto data=makeData(std::move(id),std::move(renderDoc),std::move(renderMath),
                       std::move(convert));
    return {std::move(data),std::move(priority)};
}

void RenderActions::create(Value actions){
    for(const auto& id:context_.enumerableStringKeys(actions)){
        const Value definition=context_.get(actions,Key(id));
        auto [data,priority]=action(id,definition);
        // This is Source's `if (priority)` guard. PrioritizedList stores its
        // typed number priority, as its own Source contract specifies.
        if(truthy(priority))add(std::move(data),context_.toNumber(priority));
    }
}

bool RenderActions::invoke(const RenderDataRef& data,bool documentMethod,
                           MathItem* math,MathDocumentService& document,
                           bool& result){
    if(!data)throw Exception("TypeError",u"RenderList item is null");
    const auto callback=context_.get(Value(data->identity),
        Key(documentMethod?u"renderDoc":u"renderMath"));
    std::array<Value,2> args{Value(document.sourceIdentity()),Value(Undefined{})};
    std::size_t count=1;
    if(!documentMethod){
        if(!math)throw Exception("TypeError",u"RenderList math action has no MathItem");
        const auto* identity=dynamic_cast<const MathItemSourceIdentity*>(math);
        if(!identity)throw Exception("Unimplemented",u"MathItem has no canonical Source Context identity");
        if(&identity->sourceContext()!=&context_)
            throw Exception("TypeError",u"MathItem and RenderList belong to different Contexts");
        args[0]=Value(identity->sourceIdentity());
        args[1]=Value(document.sourceIdentity());
        count=2;
    }
    if(&document.sourceContext()!=&context_)
        throw Exception("TypeError",u"MathDocument and RenderList belong to different Contexts");
    const Value called=context_.callMember(callback,Value(data->identity),
        std::span<const Value>(args.data(),count),u"RenderList callback");
    result=truthy(called);
    return true;
}

void RenderActions::renderDoc(MathDocumentService& document,Property start){
    const double threshold=start.missing()
        ?property_host::number(STATE.get("UNPROCESSED"))
        :property_host::number(start);
    for(auto iterator=begin(),last=end();iterator!=last;++iterator){
        const auto current=*iterator;
        if(current.readPriority()>=threshold){
            bool stop=false;
            (void)invoke(renderData(current),true,nullptr,document,stop);
            if(stop)return;
        }
    }
}

void RenderActions::renderMath(MathItem& math,MathDocumentReference documentValue,
                               Property start){
    auto* document=std::get_if<MathDocumentService*>(&documentValue);
    if(!document||!*document)
        throw Exception("TypeError",u"Cannot read properties of null (reading 'sourceIdentity')");
    const double threshold=start.missing()
        ?property_host::number(STATE.get("UNPROCESSED"))
        :property_host::number(start);
    for(auto iterator=begin(),last=end();iterator!=last;++iterator){
        const auto current=*iterator;
        if(current.readPriority()>=threshold){
            bool stop=false;
            (void)invoke(renderData(current),false,&math,**document,stop);
            if(stop)return;
        }
    }
}

void RenderActions::renderConvert(MathItem& math,MathDocumentReference documentValue,
                                  Property endValue){
    auto* document=std::get_if<MathDocumentService*>(&documentValue);
    if(!document||!*document)
        throw Exception("TypeError",u"Cannot read properties of null (reading 'sourceIdentity')");
    const double limit=endValue.missing()
        ?property_host::number(STATE.get("LAST"))
        :property_host::number(endValue);
    for(auto iterator=begin(),last=end();iterator!=last;++iterator){
        const auto current=*iterator;
        if(current.readPriority()>limit)return;
        const auto data=renderData(current);
        if(truthy(context_.get(Value(data->identity),Key(u"convert")))){
            bool stop=false;
            (void)invoke(data,false,&math,**document,stop);
            if(stop)return;
        }
    }
}

Value RenderActions::findID(std::u16string_view id){
    for(auto iterator=begin(),last=end();iterator!=last;++iterator){
        const auto current=*iterator;
        const auto data=renderData(current);
        if(context_.get(Value(data->identity),Key(u"id"))==Value(std::u16string(id)))
            return Value(data->identity);
    }
    return Value(Null{});
}
}
