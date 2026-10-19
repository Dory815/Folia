module folia.math.output.common.bbox_context_bindings;
import folia.math.util.native_identities;
import folia.math.util.option_types;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
using namespace options;

struct BBoxContextBindings::State : std::enable_shared_from_this<State> {
    Context& context;
    ContextPropertyBindings& properties;
    NativeIdentityBindings<BBox*> instances;
    // Type view for LineBBox over the same canonical BBox identity. The
    // NativeIdentityBindings payload remains the sole identity and owner.
    std::unordered_map<BBox*,std::weak_ptr<LineBBox>> lineOwners;
    std::optional<ObjectRef> ctor;
    std::optional<ObjectRef> lineCtor;

    State(Context& cx,ContextPropertyBindings& bindings)
        :context(cx),properties(bindings),instances(cx){
        if(&cx!=&bindings.context())
            throw Exception("TypeError",u"BBox bindings must share one Context");
    }
    std::shared_ptr<BBox> resolve(ObjectRef object)const{
        auto* identity=instances.resolve(object);
        const auto* payload=std::any_cast<NativeBindingPayload<BBox*>>(&context.payload(object));
        if(!payload||!payload->lifetime)
            throw Exception("Unimplemented",u"BBox identity has no retained actual owner");
        auto owner=std::static_pointer_cast<BBox>(payload->lifetime);
        if(owner.get()!=identity)
            throw Exception("Unimplemented",u"BBox Context identity does not match its actual owner");
        return owner;
    }
    double number(Value value)const{return property_host::number(properties.from(value));}
    Value indexArg(std::span<const Value> args,std::size_t index,double fallback)const{
        return index>=args.size()||std::holds_alternative<Undefined>(args[index].data)
            ?Value(fallback):args[index];
    }
    void method(ObjectRef prototype,std::u16string name,std::uint32_t length,
                NativeCall body){
        const auto function=context.function(name,std::move(body));
        context.define(function,Key(u"length"),Descriptor::data(
            Value(static_cast<double>(length)),false,false,true));
        context.set(Value(prototype),Key(std::move(name)),Value(function));
    }
    ObjectRef install(){
        if(ctor)return *ctor;
        auto self=shared_from_this();
        auto constructor=context.nativeConstructor(u"BBox",
            [self](Context&,std::span<const Value> args)->ObjectRef{
                BBoxData data;
                // BBox's default parameter applies only to omitted/undefined.
                // Explicit null reaches the first Source property Get and
                // therefore throws there instead of silently selecting the
                // default record.
                if(!args.empty()&&!std::holds_alternative<Undefined>(args.front().data)){
                    const auto def=args.front();
                    const auto w=self->context.get(def,Key(u"w"));
                    data.w=property_host::truthy(self->properties.from(w))?self->number(w):0.0;
                    if(self->context.has(def,Key(u"h")))data.h=self->number(self->context.get(def,Key(u"h")));
                    if(self->context.has(def,Key(u"d")))data.d=self->number(self->context.get(def,Key(u"d")));
                }
                return self->bind(std::make_shared<BBox>(data));
            });
        context.define(constructor,Key(u"length"),Descriptor::data(Value(0),false,false,true));
        const auto prototype=std::get<ObjectRef>(context.get(Value(constructor),Key(u"prototype")).data);
        const auto sides=context.array(4);
        constexpr std::array<std::array<std::u16string_view,3>,4> entries{{
            {{u"Top",u"0",u"h"}},{{u"Right",u"1",u"w"}},
            {{u"Bottom",u"2",u"d"}},{{u"Left",u"3",u"w"}}
        }};
        for(std::size_t i=0;i<entries.size();++i){
            const auto tuple=context.array(3);
            for(std::size_t j=0;j<3;++j)
                context.set(Value(tuple),Key(std::u16string(1,static_cast<char16_t>(u'0'+j))),
                            Value(std::u16string(entries[i][j])));
            context.set(Value(sides),Key(std::u16string(1,static_cast<char16_t>(u'0'+i))),Value(tuple));
        }
        context.set(Value(constructor),Key(u"fullWidth"),Value(u"100%"));
        context.set(Value(constructor),Key(u"boxSides"),Value(sides));
        method(prototype,u"empty",0,[self](Context&,Value receiver,std::span<const Value>)->Value{
            const auto object=std::get<ObjectRef>(receiver.data);
            auto value=self->resolve(object);value->empty();return Value(object);
        });
        method(prototype,u"clean",0,[self](Context&,Value receiver,std::span<const Value>)->Value{
            self->resolve(std::get<ObjectRef>(receiver.data))->clean();return Undefined{};
        });
        method(prototype,u"rescale",1,[self](Context&,Value receiver,std::span<const Value> args)->Value{
            if(args.empty())throw Exception("TypeError",u"BBox.rescale requires a scale");
            self->resolve(std::get<ObjectRef>(receiver.data))->rescale(self->number(args.front()));return Undefined{};
        });
        method(prototype,u"combine",3,[self](Context&,Value receiver,std::span<const Value> args)->Value{
            if(args.empty())throw Exception("TypeError",u"BBox.combine requires a box");
            const auto* child=std::get_if<ObjectRef>(&args.front().data);
            if(!child)throw Exception("TypeError",u"BBox.combine argument is not a BBox");
            auto target=self->resolve(std::get<ObjectRef>(receiver.data));
            auto source=self->resolve(*child);
            const double x=self->number(self->indexArg(args,1,0));
            const double y=self->number(self->indexArg(args,2,0));
            target->combine(*source,x,y);return Undefined{};
        });
        method(prototype,u"append",1,[self](Context&,Value receiver,std::span<const Value> args)->Value{
            if(args.empty())throw Exception("TypeError",u"BBox.append requires a box");
            const auto* child=std::get_if<ObjectRef>(&args.front().data);
            if(!child)throw Exception("TypeError",u"BBox.append argument is not a BBox");
            self->resolve(std::get<ObjectRef>(receiver.data))->append(*self->resolve(*child));return Undefined{};
        });
        method(prototype,u"updateFrom",1,[self](Context&,Value receiver,std::span<const Value> args)->Value{
            if(args.empty())throw Exception("TypeError",u"BBox.updateFrom requires a box");
            const auto* child=std::get_if<ObjectRef>(&args.front().data);
            if(!child)throw Exception("TypeError",u"BBox.updateFrom argument is not a BBox");
            self->resolve(std::get<ObjectRef>(receiver.data))->updateFrom(*self->resolve(*child));return Undefined{};
        });
        method(prototype,u"copy",0,[self](Context&,Value receiver,std::span<const Value>)->Value{
            return Value(self->bind(std::make_shared<BBox>(self->resolve(std::get<ObjectRef>(receiver.data))->copy())));
        });
        context.set(Value(constructor),Key(u"zero"),Value(context.function(u"zero",
            [self](Context&,Value,std::span<const Value>)->Value{
                return Value(self->bind(std::make_shared<BBox>(BBox::zero())));
            })));
        context.set(Value(constructor),Key(u"empty"),Value(context.function(u"empty",
            [self](Context&,Value,std::span<const Value>)->Value{
                return Value(self->bind(std::make_shared<BBox>(BBox::makeEmpty())));
            })));
        ctor=constructor;
        return constructor;
    }
    ObjectRef installLine(){
        if(lineCtor)return *lineCtor;
        const auto base=install();
        auto constructor=context.nativeConstructor(u"LineBBox",
            [self=shared_from_this()](Context&,std::span<const Value> args)->ObjectRef{
                BBoxData data;
                std::optional<std::array<double,2>> start;
                if(!args.empty()&&!std::holds_alternative<Undefined>(args.front().data)){
                    const Value def=args.front();
                    const auto w=self->context.get(def,Key(u"w"));
                    data.w=property_host::truthy(self->properties.from(w))?self->number(w):0.0;
                    if(self->context.has(def,Key(u"h")))data.h=self->number(self->context.get(def,Key(u"h")));
                    if(self->context.has(def,Key(u"d")))data.d=self->number(self->context.get(def,Key(u"d")));
                }
                if(args.size()>1&&!std::holds_alternative<Undefined>(args[1].data)&&
                   !std::holds_alternative<Null>(args[1].data))
                    throw Exception("Unimplemented",u"LineBBox.start array identity binding is unavailable");
                return self->bind(std::make_shared<LineBBox>(data,start));
            });
        context.define(constructor,Key(u"length"),Descriptor::data(
            Value(2.0),false,false,true));
        const auto proto=std::get<ObjectRef>(context.get(Value(constructor),Key(u"prototype")).data);
        const auto baseProto=std::get<ObjectRef>(context.get(Value(base),Key(u"prototype")).data);
        context.setPrototype(constructor,base);
        context.setPrototype(proto,baseProto);
        lineCtor=constructor;
        return constructor;
    }
    ObjectRef bind(std::shared_ptr<BBox> value){
        if(!value)throw Exception("TypeError",u"Cannot bind a null BBox");
        if(auto existing=instances.find(value.get()))return *existing;
        const auto object=instances.instance(value.get(),install(),std::static_pointer_cast<void>(value));
        auto lifetime=std::static_pointer_cast<void>(value);
        auto self=shared_from_this();
        auto bindNumber=[&](const char16_t* name,double BBox::*field){
            context.bindData(object,Key(name),std::make_shared<NativeDataField>(lifetime,
                [value,field]{return Value(value.get()->*field);},
                [self,value,field](Value input){value.get()->*field=self->number(std::move(input));}));
        };
        bindNumber(u"w",&BBox::w);bindNumber(u"h",&BBox::h);bindNumber(u"d",&BBox::d);
        bindNumber(u"scale",&BBox::scale);bindNumber(u"rscale",&BBox::rscale);
        bindNumber(u"L",&BBox::L);bindNumber(u"R",&BBox::R);bindNumber(u"ic",&BBox::ic);
        bindNumber(u"oc",&BBox::oc);bindNumber(u"sk",&BBox::sk);bindNumber(u"dx",&BBox::dx);
        context.bindData(object,Key(u"pwidth"),std::make_shared<NativeDataField>(lifetime,
            [value,self]{return self->properties.to(Property(value->pwidth));},
            [value,self](Value input){value->pwidth=property_host::debugText(property_host::string(self->properties.from(input)));}));
        return object;
    }
    ObjectRef bindLine(std::shared_ptr<LineBBox> value){
        if(!value)throw Exception("TypeError",u"Cannot bind a null LineBBox");
        auto* identity=static_cast<BBox*>(value.get());
        if(const auto found=lineOwners.find(identity);found!=lineOwners.end()){
            if(!found->second.expired()){
                if(const auto existing=instances.find(identity))return *existing;
            }
        }
        if(value->start||value->end)
            throw Exception("Unimplemented",u"LineBBox start/end arrays need canonical PropertyArray storage");
        const auto object=bind(std::static_pointer_cast<BBox>(value));
        lineOwners.insert_or_assign(identity,value);
        const auto constructor=installLine();
        const auto prototype=std::get<ObjectRef>(context.get(Value(constructor),Key(u"prototype")).data);
        context.setPrototype(object,prototype);
        auto lifetime=std::static_pointer_cast<void>(value);
        auto self=shared_from_this();
        if(value->lineLeading)context.bindData(object,Key(u"lineLeading"),std::make_shared<NativeDataField>(lifetime,
            [value]{return Value(*value->lineLeading);},
            [value,self](Value input){
                if(std::holds_alternative<Undefined>(input.data)){value->lineLeading.reset();return;}
                value->lineLeading=self->number(std::move(input));
            }));
        context.bindData(object,Key(u"originalL"),std::make_shared<NativeDataField>(lifetime,
            [value]{return Value(value->originalL);},
            [value,self](Value input){value->originalL=self->number(std::move(input));}));
        context.bindData(object,Key(u"isFirst"),std::make_shared<NativeDataField>(lifetime,
            [value]{return Value(value->isFirst);},
            [value](Value input){value->isFirst=truthy(input);}));
        context.bindData(object,Key(u"indentData"),std::make_shared<NativeDataField>(lifetime,
            [value,self]{return value->indentData?self->properties.to(Property(*value->indentData)):Value(Null{});},
            [value,self](Value input){
                if(std::holds_alternative<Null>(input.data)||std::holds_alternative<Undefined>(input.data)){
                    value->indentData.reset();return;
                }
                auto property=self->properties.from(input);
                auto array=PropertyArray::array(property);
                if(!array)throw Exception("TypeError",u"LineBBox.indentData must be an array");
                // PropertyArray is a handle into the same Property storage;
                // retain that canonical array rather than copying its slots.
                value->indentData=std::make_shared<PropertyArray>(std::move(*array));
            }));
        return object;
    }
    std::shared_ptr<LineBBox> resolveLine(ObjectRef object)const{
        const auto base=resolve(object);
        const auto found=lineOwners.find(base.get());
        if(found==lineOwners.end())
            throw Exception("TypeError",u"getBreakNode requires a LineBBox");
        auto line=found->second.lock();
        if(!line||static_cast<BBox*>(line.get())!=base.get())
            throw Exception("TypeError",u"getBreakNode requires a LineBBox");
        return line;
    }
};

BBoxContextBindings::BBoxContextBindings(Context& context,ContextPropertyBindings& properties)
    :state_(std::make_shared<State>(context,properties)){}
BBoxContextBindings::~BBoxContextBindings()=default;
Context& BBoxContextBindings::context()const{return state_->context;}
ObjectRef BBoxContextBindings::constructor(){return state_->install();}
ObjectRef BBoxContextBindings::lineConstructor(){return state_->installLine();}
ObjectRef BBoxContextBindings::bind(const std::shared_ptr<BBox>& value){state_->install();return state_->bind(value);}
ObjectRef BBoxContextBindings::bind(const std::shared_ptr<LineBBox>& value){state_->installLine();return state_->bindLine(value);}
std::shared_ptr<BBox> BBoxContextBindings::resolve(ObjectRef object)const{return state_->resolve(object);}
std::shared_ptr<LineBBox> BBoxContextBindings::resolveLine(ObjectRef object)const{return state_->resolveLine(object);}
}
