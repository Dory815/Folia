// Candidate-only projection of the actual core::AbstractMathItem owner.
// This exists only to compare the standalone Source TeX constructor path.
#pragma once

namespace standalone_math_item {
using namespace folia::math;
using options::Context;
using options::ObjectRef;
using options::Value;
using options::Key;
using options::Descriptor;
using options::Exception;

class Owner final : public core::AbstractMathItem,
                    public core::MathItemSourceIdentity {
    Context* context_;
    ObjectRef identity_;
    ObjectRef jaxIdentity_;
public:
    Owner(Context& context,ObjectRef identity,ObjectRef jaxIdentity,
          Property math,tex::TeX& jax,Property display,
          core::MathLocation start,core::MathLocation end)
        :core::AbstractMathItem(std::move(math),
             core::MathInputJaxReference{static_cast<core::MathInputJaxService*>(&jax)},
             std::move(display),std::move(start),std::move(end)),
         context_(&context),identity_(identity),jaxIdentity_(jaxIdentity){}
    Context& sourceContext()const override{return *context_;}
    ObjectRef sourceIdentity()const override{return identity_;}
    ObjectRef jaxIdentity()const noexcept{return jaxIdentity_;}
    void setJax(ObjectRef identity,tex::TeX& jax){jaxIdentity_=identity;inputJax=static_cast<core::MathInputJaxService*>(&jax);}
};

inline Owner& require(Context& context,Value receiver){
    const auto* identity=std::get_if<ObjectRef>(&receiver.data);
    if(!identity)throw Exception("TypeError",u"AbstractMathItem receiver is not an object");
    const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&context.payload(*identity));
    const auto owner=payload?std::dynamic_pointer_cast<Owner>(*payload):nullptr;
    if(!owner)throw Exception("TypeError",u"AbstractMathItem has no canonical native owner");
    return *owner;
}
inline Value arg(std::span<const Value> args,std::size_t index){
    return index<args.size()?args[index]:Value(options::Undefined{});
}

class Binding final {
    Context* context_;
    ContextPropertyBindings* properties_;
    tex::ParserRuntime* runtime_;
    ObjectRef constructor_{};
    ObjectRef prototype_{};

    Property from(Value value)const{return properties_->from(std::move(value));}
    Value to(const Property& value)const{return properties_->to(value);}
    core::MathDocumentReference document(Context& context,Value value)const{
        const auto* identity=std::get_if<ObjectRef>(&value.data);
        if(!identity)return core::MathDocumentReference{Missing{}};
        const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&context.payload(*identity));
        const auto owner=payload?std::dynamic_pointer_cast<core::MathDocument>(*payload):nullptr;
        return owner?core::MathDocumentReference{owner.get()}:core::MathDocumentReference{Missing{}};
    }
    void defineMethod(std::u16string_view name,std::uint32_t length,options::NativeCall callback){
        const auto function=context_->function(u"",std::move(callback));
        context_->define(function,Key(u"length"),Descriptor::data(Value(static_cast<double>(length)),false,false,true));
        context_->define(prototype_,Key(std::u16string(name)),Descriptor::data(Value(function),true,true,true));
    }
    ObjectRef bindOwner(const std::shared_ptr<Owner>& owner){
        Context& cx=*context_;const ObjectRef identity=owner->sourceIdentity();
        cx.setPrototype(identity,prototype_);
        cx.payload(identity)=std::static_pointer_cast<NativeObject>(owner);
        const auto bindProperty=[&](std::u16string_view name,Property& field){
            auto* value=&field;
            cx.bindData(identity,Key(std::u16string(name)),std::make_shared<options::NativeDataField>(owner,
                [this,value]{return to(*value);},[this,value](Value next){*value=from(std::move(next));},
                [value]{*value=Missing{};}));
        };
        // Match Source own-key order exactly.
        cx.bindData(identity,Key(u"root"),std::make_shared<options::NativeDataField>(owner,
            [this,owner]{if(owner->root.isNode()){
                const auto found=runtime_->mmlNodeIdentities.find(owner->root.requireNode());return found?Value(*found):Value(options::Null{});}
                return Value(options::Null{});},
            [this,owner](Value next){if(std::holds_alternative<options::Null>(next.data)||std::holds_alternative<options::Undefined>(next.data)){owner->root=PropertyNull{};return;}
                const auto* id=std::get_if<ObjectRef>(&next.data);if(!id)throw Exception("TypeError",u"AbstractMathItem.root must be an MML node");owner->root=&runtime_->resolveMmlNode(*id);}));
        cx.bindData(identity,Key(u"typesetRoot"),std::make_shared<options::NativeDataField>(owner,
            [this,owner]{return core::sourceOutputValue(owner->typesetRoot,*properties_);},
            [this,owner](Value next){const Property value=from(std::move(next));const auto projected=propertyNode(value);
                if(const auto* node=std::get_if<DOMNodeRef>(&projected))owner->typesetRoot=*node;
                else if(std::holds_alternative<Missing>(projected))owner->typesetRoot=Missing{};
                else if(std::holds_alternative<PropertyNull>(projected))owner->typesetRoot=PropertyNull{};
                else throw Exception("TypeError",u"AbstractMathItem.typesetRoot must be an adaptor node");}));
        bindProperty(u"metrics",owner->metrics);bindProperty(u"inputData",owner->inputData);bindProperty(u"outputData",owner->outputData);
        cx.bindData(identity,Key(u"_state"),std::make_shared<options::NativeDataField>(owner,
            [this,owner]{return to(owner->sourceStateField());},
            [this,owner](Value next){owner->setSourceStateField(from(std::move(next)));},
            [owner]{owner->setSourceStateField(Missing{});}));
        bindProperty(u"math",owner->math);
        cx.bindData(identity,Key(u"inputJax"),std::make_shared<options::NativeDataField>(owner,
            [owner]{return Value(owner->jaxIdentity());},
            [this,owner](Value next){const auto* id=std::get_if<ObjectRef>(&next.data);if(!id)throw Exception("TypeError",u"AbstractMathItem.inputJax must be an InputJax");
                auto jax=runtime_->texIdentities.resolve(*id);owner->setJax(*id,*jax);}));
        bindProperty(u"display",owner->display);
        const auto location=[&](std::u16string_view name,core::MathLocation& field){
            auto* value=&field;
            cx.bindData(identity,Key(std::u16string(name)),std::make_shared<options::NativeDataField>(owner,
                [this,value]{const auto* prop=std::get_if<Property>(value);return prop?to(*prop):Value(options::Undefined{});},
                [this,value](Value next){*value=from(std::move(next));},[value]{*value=Property(Missing{});}));
        };
        location(u"start",owner->start);location(u"end",owner->end);
        return identity;
    }
    void installPrototype(){
        const auto getter=context_->function(u"get",[](Context& cx,Value receiver,std::span<const Value>)->Value{
            return Value(require(cx,std::move(receiver)).isEscaped());});
        context_->define(prototype_,Key(u"isEscaped"),Descriptor::access(getter,std::nullopt,false,true));
        defineMethod(u"render",1,[this](Context& cx,Value receiver,std::span<const Value> a)->Value{require(cx,std::move(receiver)).render(document(cx,arg(a,0)));return options::Undefined{};});
        defineMethod(u"rerender",2,[this](Context& cx,Value receiver,std::span<const Value> a)->Value{auto& i=require(cx,std::move(receiver));i.rerender(document(cx,arg(a,0)),from(arg(a,1)));return options::Undefined{};});
        defineMethod(u"convert",2,[this](Context& cx,Value receiver,std::span<const Value> a)->Value{auto& i=require(cx,std::move(receiver));i.convert(document(cx,arg(a,0)),from(arg(a,1)));return options::Undefined{};});
        defineMethod(u"compile",1,[this](Context& cx,Value receiver,std::span<const Value> a)->Value{require(cx,std::move(receiver)).compile(document(cx,arg(a,0)));return options::Undefined{};});
        defineMethod(u"typeset",1,[this](Context& cx,Value receiver,std::span<const Value> a)->Value{require(cx,std::move(receiver)).typeset(document(cx,arg(a,0)));return options::Undefined{};});
        defineMethod(u"updateDocument",1,[this](Context& cx,Value receiver,std::span<const Value> a)->Value{require(cx,std::move(receiver)).updateDocument(document(cx,arg(a,0)));return options::Undefined{};});
        defineMethod(u"removeFromDocument",1,[this](Context&,Value receiver,std::span<const Value> a)->Value{require(*context_,std::move(receiver)).removeFromDocument(from(arg(a,0)));return options::Undefined{};});
        defineMethod(u"setMetrics",4,[this](Context&,Value receiver,std::span<const Value> a)->Value{auto& i=require(*context_,std::move(receiver));i.setMetrics(from(arg(a,0)),from(arg(a,1)),from(arg(a,2)),from(arg(a,3)));return options::Undefined{};});
        defineMethod(u"state",2,[this](Context&,Value receiver,std::span<const Value> a)->Value{auto& i=require(*context_,std::move(receiver));return to(i.state(from(arg(a,0)),from(arg(a,1))));});
        defineMethod(u"reset",1,[this](Context&,Value receiver,std::span<const Value> a)->Value{require(*context_,std::move(receiver)).reset(from(arg(a,0)));return options::Undefined{};});
        defineMethod(u"clear",0,[this](Context&,Value receiver,std::span<const Value>)->Value{require(*context_,std::move(receiver)).clear();return options::Undefined{};});
    }
public:
    Binding(Context& context,ContextPropertyBindings& properties,tex::ParserRuntime& runtime)
        :context_(&context),properties_(&properties),runtime_(&runtime){
        constructor_=context.nativeConstructor(u"AbstractMathItem",
            [this](Context& cx,std::span<const Value> args)->ObjectRef{
                if(args.size()<2)throw Exception("TypeError",u"AbstractMathItem requires math and inputJax");
                const auto* jaxId=std::get_if<ObjectRef>(&args[1].data);
                if(!jaxId)throw Exception("TypeError",u"AbstractMathItem constructor arguments are invalid");
                auto jax=runtime_->texIdentities.resolve(*jaxId);const auto identity=cx.object();
                return bindOwner(std::make_shared<Owner>(cx,identity,*jaxId,from(args[0]),*jax,
                    from(arg(args,2)),core::MathLocation(from(arg(args,3))),
                    core::MathLocation(from(arg(args,4)))));
            });
        context.define(constructor_,Key(u"length"),Descriptor::data(Value(5),false,false,true));
        prototype_=std::get<ObjectRef>(context.get(Value(constructor_),Key(u"prototype")).data);
        context.define(prototype_,Key(u"constructor"),Descriptor::data(Value(constructor_),true,false,true));
        installPrototype();
    }
    ObjectRef constructor()const noexcept{return constructor_;}
    ObjectRef prototype()const noexcept{return prototype_;}
};
}
