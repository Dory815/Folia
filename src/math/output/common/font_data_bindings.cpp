module folia.math.output.common.font_data_bindings;
import std;

namespace folia::math::output::common {
struct FontDataContextBindings::State {
    options::Context& context;
    NativeIdentityBindings<FontDataClass*> classes;
    NativeIdentityBindings<FontData*> instances;
    std::unordered_map<FontDataClass*,std::weak_ptr<FontDataClass>> classOwners;
    std::optional<options::ObjectRef> baseConstructor;

    explicit State(options::Context& cx):context(cx),classes(cx),instances(cx){}

    std::shared_ptr<FontData> resolve(options::ObjectRef object)const{
        auto* identity=instances.resolve(object);
        const auto* payload=std::any_cast<NativeBindingPayload<FontData*>>(&context.payload(object));
        if(!payload||!payload->lifetime)
            throw options::Exception("Unimplemented",u"FontData Context identity has no retained actual owner");
        auto owner=std::static_pointer_cast<FontData>(payload->lifetime);
        if(owner.get()!=identity)
            throw options::Exception("Unimplemented",u"FontData Context identity does not match its owner");
        return owner;
    }

    std::shared_ptr<FontDataClass> resolveClass(options::ObjectRef constructor)const{
        auto* identity=classes.resolve(constructor);
        auto found=classOwners.find(identity);
        if(found==classOwners.end())
            throw options::Exception("Unimplemented",u"FontData class Context identity has no retained actual owner");
        auto owner=found->second.lock();
        if(!owner||owner.get()!=identity)
            throw options::Exception("Unimplemented",u"FontData class owner is no longer alive");
        return owner;
    }
};

FontDataContextBindings::FontDataContextBindings(options::Context& context)
    :state_(std::make_shared<State>(context)){}

options::ObjectRef FontDataContextBindings::registerClass(std::u16string name,
    std::shared_ptr<FontDataClass> classOwner,
    FontLoadHost& host,std::shared_ptr<void> hostAnchor,Allocator allocator,
    std::optional<options::ObjectRef> parentConstructor,
    InstanceInitializer initializeInstance){
    auto state=state_;
    if(!classOwner||!hostAnchor||!allocator)
        throw options::Exception("TypeError",u"FontData class binding requires actual owners and allocator");
    if(parentConstructor)state->context.kind(*parentConstructor);
    if(auto existing=state->classes.find(classOwner.get()))return *existing;

    // The constructor closure needs its own canonical constructor ref, which is
    // installed immediately after Context creates the function object.
    auto constructorSlot=std::make_shared<std::optional<options::ObjectRef>>();
    auto classAnchor=std::static_pointer_cast<void>(classOwner);
    auto nativeConstruct=[state,classOwner,classAnchor,hostPtr=&host,hostAnchor,
                          allocator=std::move(allocator),initializeInstance=std::move(initializeInstance),constructorSlot]
        (options::Context& context,std::span<const options::Value> arguments)->options::ObjectRef{
        if(!*constructorSlot)
            throw options::Exception("Unimplemented",u"FontData constructor was invoked before registration completed");
        // The pinned emitted CJS body also normalizes an explicitly
        // undefined first argument to null before userOptions runs.
        options::Value supplied=arguments.empty()||
            std::holds_alternative<options::Undefined>(arguments.front().data)
            ?options::Value(options::Null{}):arguments.front();
        auto font=allocator(*classOwner,*hostPtr,context,std::move(supplied));
        if(!font)throw options::Exception("TypeError",u"FontData constructor returned no instance");
        const auto instance=state->instances.instance(font.get(),**constructorSlot,
            std::static_pointer_cast<void>(font));
        context.bindData(instance,options::Key(u"options"),
            std::make_shared<options::NativeDataField>(std::static_pointer_cast<void>(font),
                [font]{return font->options();},
                [font](options::Value value){font->replaceOptions(std::move(value));}));
        if(initializeInstance)initializeInstance(font,instance);
        (void)classAnchor;(void)hostAnchor;
        return instance;
    };
    auto constructor=state->context.nativeConstructor(name,std::move(nativeConstruct));
    *constructorSlot=constructor;
    state->classes.bind(classOwner.get(),constructor,classAnchor);
    state->classOwners.insert_or_assign(classOwner.get(),classOwner);
    auto& context=state->context;
    if(!parentConstructor){
        if(state->baseConstructor&&*state->baseConstructor!=constructor)
            throw options::Exception("TypeError",u"FontData base class is already registered");
        state->baseConstructor=constructor;
    }

    if(parentConstructor){
        context.setPrototype(constructor,parentConstructor);
        const auto childPrototype=std::get<options::ObjectRef>(context.get(
            options::Value(constructor),options::Key(u"prototype")).data);
        const auto parentPrototype=std::get<options::ObjectRef>(context.get(
            options::Value(*parentConstructor),options::Key(u"prototype")).data);
        context.setPrototype(childPrototype,parentPrototype);
    }
    context.define(constructor,options::Key(u"length"),
        options::Descriptor::data(options::Value(1),false,false,true));
    context.define(constructor,options::Key(u"name"),
        options::Descriptor::data(options::Value(name),false,false,true));

    // OPTIONS is the live static class field read by separateOptions. Its
    // storage remains FontDataClass::OPTIONS, not a duplicate option record.
    context.bindData(constructor,options::Key(u"OPTIONS"),
        std::make_shared<options::NativeDataField>(classAnchor,
            [classOwner]{return classOwner->OPTIONS;},
            [classOwner](options::Value value){classOwner->OPTIONS=std::move(value);}));

    const auto prototype=std::get<options::ObjectRef>(context.get(
        options::Value(constructor),options::Key(u"prototype")).data);
    if(!parentConstructor){
        auto getter=context.function(u"get CLASS",
            [](options::Context& cx,options::Value receiver,
                             std::span<const options::Value>)->options::Value{
                return cx.get(receiver,options::Key(u"constructor"));
            });
        context.define(prototype,options::Key(u"CLASS"),
            options::Descriptor::access(getter,std::nullopt,false,true));
    }
    return constructor;
}

std::shared_ptr<FontData> FontDataContextBindings::resolve(options::ObjectRef instance)const{
    return state_->resolve(instance);
}
std::shared_ptr<FontDataClass> FontDataContextBindings::resolveClass(options::ObjectRef constructor)const{
    return state_->resolveClass(constructor);
}
std::optional<options::ObjectRef> FontDataContextBindings::identity(const FontData* instance)const{
    if(!instance)return std::nullopt;
    return state_->instances.find(const_cast<FontData*>(instance));
}
std::optional<options::ObjectRef> FontDataContextBindings::constructor(const FontDataClass* klass)const{
    if(!klass)return std::nullopt;
    return state_->classes.find(const_cast<FontDataClass*>(klass));
}
bool FontDataContextBindings::isFontData(options::Value value)const{
    auto* object=std::get_if<options::ObjectRef>(&value.data);
    if(!object)return false;
    if(!state_->baseConstructor)
        throw options::Exception("Unimplemented",u"FontData base constructor is not registered");
    auto prototype=state_->context.get(options::Value(*state_->baseConstructor),options::Key(u"prototype"));
    auto* target=std::get_if<options::ObjectRef>(&prototype.data);
    if(!target)throw options::Exception("TypeError",u"FontData constructor prototype is not an object");
    auto current=state_->context.prototype(*object);
    while(current){if(*current==*target)return true;current=state_->context.prototype(*current);}
    return false;
}
options::Context& FontDataContextBindings::context()const{return state_->context;}
}
