// Source-visible class members for the actual FontData Context constructor.
// The typed FontData owner remains the sole font-state authority.
module folia.math.output.common.font_data_surface;
import folia.math.util.js_number;
import folia.math.util.js_numeric_input;
import folia.math.util.property_host;
import folia.math.output.common.font_data_options;
import std;

namespace folia::math::output::common {
using namespace options;

namespace {
ObjectRef objectReceiver(Context& context,const Value& receiver){
    const auto* object=std::get_if<ObjectRef>(&receiver.data);
    if(!object)throw Exception("TypeError",u"FontData method called on an incompatible receiver");
    (void)context.kind(*object);
    return *object;
}
Value argument(std::span<const Value> args,std::size_t index){
    return index<args.size()?args[index]:Value(Undefined{});
}
std::u16string sourceString(ContextPropertyBindings& properties,const Value& value){
    return property_host::string(properties.from(value));
}
double sourceNumber(ContextPropertyBindings& properties,const Value& value){
    return property_host::number(properties.from(value));
}
Value optionalString(std::optional<std::string> value){
    if(!value)return Undefined{};
    return Value(property_host::text(*value));
}
PropertyList sourceRecord(ContextPropertyBindings& properties,const Value& value){
    auto record=PropertyList::record(properties.from(value));
    if(!record)throw Exception("TypeError",u"FontData argument is not an object record");
    return *record;
}
std::optional<std::uint32_t> characterKey(std::string_view key){
    if(key.empty())return {};
    const auto value=js_input::parseInt(property_host::text(key),js_input::IntegerRadix::Automatic);
    if(!std::isfinite(value)||value<0||value>std::numeric_limits<std::uint32_t>::max())return {};
    return static_cast<std::uint32_t>(value);
}
PropertyArray sourceArray(ContextPropertyBindings& properties,const Value& value){
    auto array=PropertyArray::array(properties.from(value));
    if(!array)throw Exception("TypeError",u"FontData argument is not an array");
    return *array;
}
class FunctionPropertyRecord final:public PropertyRecordBinding {
    std::shared_ptr<void> owner_;
    std::function<Property(std::string_view)> get_;
    std::function<bool(std::string_view)> has_;
    std::function<void(std::string_view,Property)> set_;
    std::function<void(std::string_view)> erase_;
    std::function<std::vector<std::string>()> keys_;
public:
    FunctionPropertyRecord(std::shared_ptr<void> owner,
        std::function<Property(std::string_view)> get,
        std::function<bool(std::string_view)> has,
        std::function<void(std::string_view,Property)> set,
        std::function<void(std::string_view)> erase,
        std::function<std::vector<std::string>()> keys)
        :owner_(std::move(owner)),get_(std::move(get)),has_(std::move(has)),
         set_(std::move(set)),erase_(std::move(erase)),keys_(std::move(keys)){
        if(!owner_||!get_||!has_||!set_||!erase_||!keys_)
            throw Exception("TypeError",u"FontData view requires a live owner and complete record operations");
    }
    Property get(std::string_view key)override{return has_(key)?get_(key):Property(Missing{});}
    Property getOwn(std::string_view key)override{return has_(key)?get_(key):Property(Missing{});}
    bool has(std::string_view key)const override{return has_(key);}
    bool hasOwn(std::string_view key)const override{return has_(key);}
    void set(std::string_view key,Property value)override{set_(key,std::move(value));}
    void setOwn(std::string_view key,Property value)override{set_(key,std::move(value));}
    void erase(std::string_view key)override{erase_(key);}
    std::vector<std::string> keys()const override{return keys_();}
    Property prototype()const override{return Missing{};}
    Property primitive(PrimitiveHint)override{return BuiltinProperty::toString;}
};
template<class Map,class KeyReader,class ValueReader,class ValueWriter,class KeyWriter>
Property liveMap(std::shared_ptr<void> owner,Map& map,KeyReader keyReader,
    ValueReader valueReader,ValueWriter valueWriter,KeyWriter keyWriter){
    using KeyType=typename Map::key_type;
    auto get=[&map,keyReader,valueReader](std::string_view key)->Property{
        const auto parsed=keyReader(key);if(!parsed)return Missing{};
        auto found=map.find(*parsed);return found==map.end()?Property(Missing{}):valueReader(found->second);
    };
    auto has=[&map,keyReader](std::string_view key){const auto parsed=keyReader(key);return parsed&&map.contains(*parsed);};
    auto set=[&map,keyReader,valueWriter](std::string_view key,Property value){
        const auto parsed=keyReader(key);if(!parsed)return;
        valueWriter(map,*parsed,std::move(value));
    };
    auto erase=[&map,keyReader](std::string_view key){const auto parsed=keyReader(key);if(parsed)map.erase(*parsed);};
    auto keys=[&map,keyWriter]{std::vector<std::string> result;result.reserve(map.size());for(const auto& entry:map)result.push_back(keyWriter(entry.first));return result;};
    auto binding=std::make_shared<FunctionPropertyRecord>(std::move(owner),std::move(get),std::move(has),
        std::move(set),std::move(erase),std::move(keys));
    return Property(PropertyList::live(std::move(binding)));
}
std::optional<std::uint32_t> numericPropertyKey(std::string_view key){
    const auto number=js_input::parseInt(property_host::text(key),js_input::IntegerRadix::Automatic);
    if(!std::isfinite(number)||number<0||number>std::numeric_limits<std::uint32_t>::max())return {};
    return static_cast<std::uint32_t>(number);
}
std::optional<std::string> stringPropertyKey(std::string_view key){return std::string(key);}
std::string uintKey(std::uint32_t key){return std::to_string(key);}
template<class Value> Property numberProperty(Value value){return Property(static_cast<double>(value));}
std::uint32_t uintProperty(Property value){
    const auto number=property_host::number(value);
    if(!std::isfinite(number)||number<0||number>std::numeric_limits<std::uint32_t>::max())return 0;
    return static_cast<std::uint32_t>(number);
}
template<class Map,class KeyReader,class ValueReader,class ValueWriter,class KeyWriter>
void replaceMap(Map& target,PropertyList source,KeyReader keyReader,ValueWriter valueWriter){
    target.clear();
    for(const auto& key:source.keys()){
        const auto parsed=keyReader(key);if(!parsed)continue;
        valueWriter(target,*parsed,source.getOwn(key));
    }
}
template<class Map,class KeyReader,class ValueReader,class ValueWriter,class KeyWriter>
void bindLiveMapField(Context& context,ContextPropertyBindings& properties,ObjectRef receiver,
    const Key& name,std::shared_ptr<void> owner,Map& target,KeyReader keyReader,
    ValueReader valueReader,ValueWriter valueWriter,KeyWriter keyWriter){
    Property view=liveMap(owner,target,keyReader,valueReader,valueWriter,keyWriter);
    auto retainedView=view;
    auto* propertyBindings=&properties;
    context.bindData(receiver,name,std::make_shared<NativeDataField>(std::move(owner),
        [propertyBindings,retainedView]{return propertyBindings->to(retainedView);},
        [propertyBindings,&target,keyReader,valueReader,valueWriter,keyWriter](Value value){
            auto source=PropertyList::record(propertyBindings->from(value));
            if(!source)throw Exception("TypeError",u"FontData map field must be an object");
            replaceMap(target,*source,keyReader,valueReader,valueWriter,keyWriter);
        }));
}
[[noreturn]] void missingSourceAdapter(std::u16string_view method){
    throw Exception("Unimplemented",std::u16string(u"FontData.")+std::u16string(method)+
        u" requires a canonical source-record view");
}
}

FontDataSourceSurface::FontDataSourceSurface(Context& context,
    FontDataContextBindings& identities,ContextPropertyBindings& properties,
    MapContextBindings& maps,regexp_host::RegExpBindings* regexp)
    :context_(&context),regexp_(regexp),identities_(&identities),properties_(&properties),maps_(&maps){
    if(&identities.context()!=&context)
        throw Exception("TypeError",u"FontData surface and identities must share one Context");
    if(&maps.context()!=&context)
        throw Exception("TypeError",u"FontData surface and Map bindings must share one Context");
}

ObjectRef FontDataSourceSurface::method(ObjectRef target,std::u16string_view name,
    std::uint32_t length,NativeCall body){
    auto& context=*context_;
    auto function=context.function(u"",std::move(body));
    context.define(function,Key(u"length"),Descriptor::data(Value(static_cast<double>(length)),false,false,true));
    context.define(target,Key(std::u16string(name)),Descriptor::data(Value(function),true,true,true));
    return function;
}

ObjectRef FontDataSourceSurface::registerBase(std::shared_ptr<FontDataClass> owner,
    FontLoadHost& host,std::shared_ptr<void> hostAnchor,
    FontDataContextBindings::Allocator allocator,
    FontDataContextBindings::InstanceInitializer initializeInstance){
    if(!owner)throw Exception("TypeError",u"FontData class owner is null");
    bindHost(host);
    auto sourceInitialize=[this,initializeInstance=std::move(initializeInstance)](
        const std::shared_ptr<FontData>& font,ObjectRef instance){
        initializeInstanceFields(font,instance);
        if(initializeInstance)initializeInstance(font,instance);
    };
    const auto constructor=identities_->registerClass(u"FontData",owner,host,
        std::move(hostAnchor),std::move(allocator),{},std::move(sourceInitialize));
    // The emitted pinned CJS constructor has one formal argument; its length
    // is 1 even though the TypeScript declaration uses a default parameter.
    context_->define(constructor,Key(u"length"),
        Descriptor::data(Value(1),false,false,true));
    auto classOwner=owner;
    auto& context=*context_;
    auto* properties=properties_;
    auto ownerLease=std::static_pointer_cast<void>(classOwner);
    context.bindData(constructor,Key(u"JAX"),std::make_shared<NativeDataField>(ownerLease,
        [classOwner]{return Value(property_host::text(classOwner->JAX));},
        [classOwner,properties](Value value){classOwner->JAX=property_host::debugText(property_host::string(properties->from(value)));}));
    context.bindData(constructor,Key(u"NAME"),std::make_shared<NativeDataField>(ownerLease,
        [classOwner]{return Value(property_host::text(classOwner->NAME));},
        [classOwner,properties](Value value){classOwner->NAME=property_host::debugText(property_host::string(properties->from(value)));}));
    bindClassArrayFields(classOwner,constructor);
    installBase(constructor);
    return constructor;
}

ObjectRef FontDataSourceSurface::registerSvg(std::shared_ptr<FontDataClass> owner,
    FontLoadHost& host,std::shared_ptr<void> hostAnchor,
    FontDataContextBindings::Allocator allocator,ObjectRef parentConstructor,
    FontDataContextBindings::InstanceInitializer initializeInstance){
    if(!owner)throw Exception("TypeError",u"SVG FontData class owner is null");
    bindHost(host);
    auto sourceInitialize=[this,initializeInstance=std::move(initializeInstance)](
        const std::shared_ptr<FontData>& font,ObjectRef instance){
        initializeInstanceFields(font,instance);
        if(initializeInstance)initializeInstance(font,instance);
    };
    auto constructor=identities_->registerClass(u"SvgFontData",std::move(owner),host,
        std::move(hostAnchor),std::move(allocator),parentConstructor,std::move(sourceInitialize));
    auto classOwner=identities_->resolveClass(constructor);
    bindClassArrayFields(classOwner,constructor);
    // The emitted CJS class has a zero-arity constructor even though the
    // shared native allocator accepts Source options for all derived classes.
    context_->define(constructor,Key(u"length"),Descriptor::data(Value(0),false,false,true));
    installSvgOverrides(constructor);
    return constructor;
}

ObjectRef FontDataSourceSurface::registerNewcm(std::shared_ptr<FontDataClass> owner,
    FontLoadHost& host,std::shared_ptr<void> hostAnchor,
    FontDataContextBindings::Allocator allocator,ObjectRef parentConstructor,
    FontDataContextBindings::InstanceInitializer initializeInstance){
    if(!owner)throw Exception("TypeError",u"NewCM FontData class owner is null");
    bindHost(host);
    auto sourceInitialize=[this,initializeInstance=std::move(initializeInstance)](
        const std::shared_ptr<FontData>& font,ObjectRef instance){
        initializeInstanceFields(font,instance);
        if(initializeInstance)initializeInstance(font,instance);
    };
    auto constructor=identities_->registerClass(u"MathJaxNewcmFont",std::move(owner),host,
        std::move(hostAnchor),std::move(allocator),parentConstructor,std::move(sourceInitialize));
    auto classOwner=identities_->resolveClass(constructor);
    bindClassArrayFields(classOwner,constructor);
    return constructor;
}

void FontDataSourceSurface::initializeInstanceFields(
    const std::shared_ptr<FontData>& font,ObjectRef instance){
    if(!font)throw Exception("TypeError",u"FontData instance owner is null");
    font->bindSourceIdentity(*properties_,instance);
    auto& context=*context_;
    auto& properties=*properties_;
    const auto owner=std::static_pointer_cast<void>(font);
    auto bindString=[&](std::u16string_view name,std::string& field){
        context.bindData(instance,Key(std::u16string(name)),std::make_shared<NativeDataField>(owner,
            [&field]{return Value(property_host::text(field));},
            [&field,&properties](Value value){field=property_host::debugText(
                property_host::string(properties.from(value)));}));
    };
    auto bindNumber=[&](std::u16string_view name,double& field){
        context.bindData(instance,Key(std::u16string(name)),std::make_shared<NativeDataField>(owner,
            [&field]{return Value(field);},
            [&context,&field](Value value){field=context.toNumber(value);}));
    };
    bindString(u"cssFamilyPrefix",font->cssFamilyPrefix);
    bindString(u"cssFontPrefix",font->cssFontPrefix);
    bindNumber(u"skewIcFactor",font->skewIcFactor);
    bindSourceOptionArrayField(*properties_,instance,owner,u"sizeVariants",font->sizeVariants());
    bindSourceOptionArrayField(*properties_,instance,owner,u"stretchVariants",font->stretchVariants());
}

void FontDataSourceSurface::bindClassArrayFields(
    const std::shared_ptr<FontDataClass>& owner,ObjectRef constructor){
    if(!owner)throw Exception("TypeError",u"FontData class owner is null");
    owner->bindSourceIdentity(*properties_,constructor,*maps_);
    owner->dynamicExtensions=Value(maps_->create());
    context_->bindData(constructor,Key(u"dynamicExtensions"),
        std::make_shared<NativeDataField>(std::static_pointer_cast<void>(owner),
            [owner]{return owner->dynamicExtensions;},
            [owner](Value value){owner->dynamicExtensions=std::move(value);}));
    // Generated static class data creates real DynamicFile records before
    // this binding phase. Give each record its single Source identity so
    // dynamicFileName/load/setup observe the same writable fields as CJS.
    for(const auto& [_,file]:owner->dynamicFiles)
        (void)bindDynamicFileSourceObject(*owner,file);
    bindSourceOptionArrayField(*properties_,constructor,
        std::static_pointer_cast<void>(owner),u"defaultVariants",owner->defaultVariants);
    bindSourceOptionArrayField(*properties_,constructor,
        std::static_pointer_cast<void>(owner),u"defaultSizeVariants",owner->defaultSizeVariants);
    bindSourceOptionArrayField(*properties_,constructor,
        std::static_pointer_cast<void>(owner),u"defaultStretchVariants",owner->defaultStretchVariants);
}

void FontDataSourceSurface::installBase(ObjectRef constructor){
    auto& cx=*context_;
    auto& properties=*properties_;
    const auto prototypeValue=cx.get(Value(constructor),Key(u"prototype"));
    const auto* prototype=std::get_if<ObjectRef>(&prototypeValue.data);
    if(!prototype)throw Exception("TypeError",u"FontData constructor has no prototype object");
    const auto proto=*prototype;

    auto charOptions=cx.function(u"",[](Context& context,Value,std::span<const Value> args)->Value{
        const auto font=args.empty()?Value(Undefined{}):args[0];
        const auto nValue=args.size()>1?args[1]:Value(Undefined{});
        const auto n=context.toNumber(nValue);
        const auto key=property_host::text(js_number::toString(n));
        const auto tuple=context.get(font,Key(key));
        const auto* array=std::get_if<ObjectRef>(&tuple.data);
        if(!array||context.kind(*array)!=ObjectKind::array){
            const auto integer=std::isfinite(n)&&n>=0&&n<=std::numeric_limits<std::uint32_t>::max()
                ?static_cast<std::uint32_t>(n):0;
            std::ostringstream message;
            message<<"Character data hasn't been loaded for 0x"<<std::uppercase<<std::hex<<integer;
            throw Exception("Error",property_host::text(message.str()));
        }
        if(context.arrayLength(*array)==3)context.set(Value(*array),Key(u"3"),Value(context.object()));
        return context.get(Value(*array),Key(u"3"));
    });
    cx.define(charOptions,Key(u"length"),Descriptor::data(Value(2),false,false,true));
    cx.define(constructor,Key(u"charOptions"),Descriptor::data(Value(charOptions),true,true,true));

    auto adjustArrayIndices=cx.function(u"",[](Context& context,Value,
        std::span<const Value> args)->Value{
        const auto list=argument(args,0);
        const auto pivot=context.toNumber(argument(args,1));
        const auto map=context.get(list,Key(u"map"));
        const auto callback=context.function(u"",[pivot](Context& cx,Value,
            std::span<const Value> values)->Value{
            const auto item=values.empty()?Value(Undefined{}):values[0];
            return Value(FontDataClass::adjustArrayIndex(cx.toNumber(item),pivot));
        });
        const std::array<Value,1> mapArgs{Value(callback)};
        const auto* callable=std::get_if<ObjectRef>(&map.data);
        if(!callable)throw Exception("TypeError",u"list.map is not callable");
        return context.call(*callable,list,mapArgs);
    });
    cx.define(adjustArrayIndices,Key(u"length"),Descriptor::data(Value(2),false,false,true));
    cx.define(constructor,Key(u"adjustArrayIndices"),Descriptor::data(Value(adjustArrayIndices),true,true,true));

    // Static algorithm methods are installed by the shared native method
    // owner; this registrar only owns canonical Source-visible identity and
    // descriptor adaptation.

    auto* contextOwner=context_;
    auto* identityOwner=identities_;
    auto instance=[contextOwner,identityOwner](Context& context,const Value& receiver){
        return identityOwner->resolve(objectReceiver(context,receiver));
    };
    auto add=[&](std::u16string_view name,std::uint32_t length,NativeCall body){
        return method(proto,name,length,std::move(body));
    };

    add(u"setOptions",1,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        instance(context,receiver)->setOptions(argument(args,0));
        return Undefined{};
    });
    add(u"createVariant",3,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        const auto name=property_host::debugText(sourceString(properties,argument(args,0)));
        const auto inherit=argument(args,1);
        const auto link=argument(args,2);
        std::optional<std::string> inheritText,linkText;
        if(property_host::truthy(properties.from(inherit)))
            inheritText=property_host::debugText(sourceString(properties,inherit));
        if(property_host::truthy(properties.from(link)))
            linkText=property_host::debugText(sourceString(properties,link));
        font->createVariant(name,std::move(inheritText),std::move(linkText));
        return Undefined{};
    });
    add(u"createVariants",1,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        const auto definitions=argument(args,0);
        context.forEachIterable(definitions,[&](Value variantValue){
            const auto variant=sourceArray(properties,variantValue);
            const auto name=property_host::debugText(property_host::string(variant.get(0)));
            const auto inherit=variant.get(1),link=variant.get(2);
            std::optional<std::string> inheritText,linkText;
            if(property_host::truthy(inherit))inheritText=property_host::debugText(property_host::string(inherit));
            if(property_host::truthy(link))linkText=property_host::debugText(property_host::string(link));
            font->createVariant(name,std::move(inheritText),std::move(linkText));
        });
        return Undefined{};
    });
    add(u"defineChars",2,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        const auto name=property_host::debugText(sourceString(properties,argument(args,0)));
        auto source=sourceRecord(properties,argument(args,1));
        CharMap chars;
        const auto keys=source.keys();
        for(const auto& key:keys)if(auto cp=characterKey(key))chars.insert_or_assign(*cp,source.get(key));
        font->defineChars(name,chars);
        return Undefined{};
    });
    add(u"defineCssFonts",1,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        auto source=sourceRecord(properties,argument(args,0));
        CssFontMap fonts;
        const auto keys=source.keys();
        for(const auto& key:keys)fonts.insert_or_assign(key,sourceArray(properties,properties.to(source.get(key))));
        font->defineCssFonts(fonts);
        return Undefined{};
    });
    add(u"defineRemap",2,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        const auto name=property_host::debugText(sourceString(properties,argument(args,0)));
        const auto raw=properties.from(argument(args,1));
        if(property_host::nullish(raw))return Undefined{};
        auto source=PropertyList::record(raw);
        if(!source)throw Exception("TypeError",u"Remap map is not an object");
        RemapMap map;
        const auto keys=source->keys();
        for(const auto& key:keys)if(auto cp=characterKey(key))
            map.insert_or_assign(*cp,property_host::string(source->get(key)));
        font->defineRemap(name,&map);
        return Undefined{};
    });
    add(u"getChar",2,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        const auto name=property_host::debugText(sourceString(properties,argument(args,0)));
        auto value=font->getChar(name,sourceNumber(properties,argument(args,1)));
        return value?properties.to(*value):Value(Undefined{});
    });
    add(u"getCssFont",1,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        const auto name=property_host::debugText(sourceString(properties,argument(args,0)));
        return properties.to(Property(font->getCssFont(name)));
    });
    add(u"getFamily",1,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        const auto name=property_host::debugText(sourceString(properties,argument(args,0)));
        return Value(property_host::text(font->getFamily(name)));
    });
    add(u"getSizeVariant",2,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        return optionalString(font->getSizeVariant(sourceNumber(properties,argument(args,0)),
                                                   sourceNumber(properties,argument(args,1))));
    });
    add(u"getStretchVariant",2,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        return optionalString(font->getStretchVariant(sourceNumber(properties,argument(args,0)),
                                                      sourceNumber(properties,argument(args,1))));
    });
    add(u"getStretchVariants",1,[contextOwner,instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        auto values=font->getStretchVariants(sourceNumber(properties,argument(args,0)));
        const auto result=contextOwner->array(static_cast<std::uint32_t>(values.size()));
        for(std::size_t i=0;i<values.size();++i)
            contextOwner->set(Value(result),Key(property_host::text(std::to_string(i))),optionalString(values[i]));
        return result;
    });
    add(u"getRemappedChar",2,[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        auto font=instance(context,receiver);
        const auto name=property_host::debugText(sourceString(properties,argument(args,0)));
        const auto value=font->getRemappedChar(name,sourceNumber(properties,argument(args,1)));
        return value?Value(*value):Value(Undefined{});
    });
    add(u"smpChar",1,[](Context& context,Value,std::span<const Value> args)->Value{
        const auto number=argument(args,0);
        const auto result=context.array(4);
        const auto options=context.object();
        context.set(Value(options),Key(u"smp"),number);
        context.set(Value(result),Key(u"3"),Value(options));
        return result;
    });
    add(u"loadDynamicFilesSync",0,[instance](Context& context,Value receiver,std::span<const Value>)->Value{
        instance(context,receiver)->loadDynamicFilesSync();
        return Undefined{};
    });
    add(u"addDynamicFontCss",2,[](Context&,Value,std::span<const Value>)->Value{
        // The pinned common FontData implementation is intentionally empty.
        return Undefined{};
    });
    add(u"dynamicFileName",1,[instance](Context& context,Value receiver,
        std::span<const Value> args)->Value{
        return instance(context,receiver)->dynamicFileName(argument(args,0));
    });

    auto unsupported=[&](std::u16string_view name,std::uint32_t length){
        add(name,length,[name](Context&,Value,std::span<const Value>)->Value{
            missingSourceAdapter(name);
        });
    };
    unsupported(u"remapSmpChars",2);
    unsupported(u"defineDelimiters",1);
    unsupported(u"defineDynamicCharacters",1);
    unsupported(u"flattenRanges",2);
    unsupported(u"loadDynamicFile",1);
    unsupported(u"loadDynamicFiles",0);
    unsupported(u"loadDynamicFileSync",1);
    unsupported(u"getDelimiter",1);
    unsupported(u"getVariant",1);

    method(proto,u"addExtension",2,[identities=identities_,properties=properties_](Context& context,
        Value receiver,std::span<const Value> args)->Value{
        auto font=identities->resolve(objectReceiver(context,receiver));
        const auto extensionValue=argument(args,0);
        const auto* extensionObject=std::get_if<ObjectRef>(&extensionValue.data);
        if(!extensionObject)throw Exception("TypeError",u"FontData extension must be an object");
        const auto* extension=std::any_cast<std::shared_ptr<FontExtensionData>>(
            &context.payload(*extensionObject));
        if(!extension||!*extension)
            throw Exception("Unimplemented",u"FontData extension has no canonical generated Source owner");
        const auto prefixValue=argument(args,1);
        const auto prefix=std::holds_alternative<Undefined>(prefixValue.data)
            ?std::string{}:property_host::debugText(property_host::string(properties->from(prefixValue)));
        const auto rules=font->addExtension(**extension,prefix);
        const auto result=context.array(rules.size());
        for(std::size_t index=0;index<rules.size();++index)
            context.set(Value(result),Key(property_host::text(std::to_string(index))),
                        Value(property_host::text(rules[index])));
        return result;
    });

    method(constructor,u"addExtension",2,[identities=identities_,properties=properties_](Context& context,
        Value receiver,std::span<const Value> args)->Value{
        const auto* object=std::get_if<ObjectRef>(&receiver.data);
        if(!object)throw Exception("TypeError",u"FontData.addExtension called on an incompatible receiver");
        auto owner=identities->resolveClass(*object);
        const auto extensionValue=argument(args,0);
        const auto* extensionObject=std::get_if<ObjectRef>(&extensionValue.data);
        if(!extensionObject)throw Exception("TypeError",u"FontData extension must be an object");
        const auto* extension=std::any_cast<std::shared_ptr<FontExtensionData>>(
            &context.payload(*extensionObject));
        if(!extension||!*extension)
            throw Exception("Unimplemented",u"FontData extension has no canonical generated Source owner");
        const auto prefixValue=argument(args,1);
        const auto prefix=std::holds_alternative<Undefined>(prefixValue.data)
            ?std::string{}:property_host::debugText(property_host::string(properties->from(prefixValue)));
        owner->addExtension(**extension,prefix);
        return Undefined{};
    });

    auto stylesGetter=cx.function(u"get",[instance,&properties](Context& context,Value receiver,std::span<const Value>)->Value{
        return properties.to(instance(context,receiver)->styles());
    });
    auto stylesSetter=cx.function(u"set",[instance,&properties](Context& context,Value receiver,std::span<const Value> args)->Value{
        instance(context,receiver)->setStyles(properties.from(argument(args,0)));
        return Undefined{};
    });
    cx.define(stylesGetter,Key(u"length"),Descriptor::data(Value(0),false,false,true));
    cx.define(stylesSetter,Key(u"length"),Descriptor::data(Value(1),false,false,true));
    cx.define(proto,Key(u"styles"),Descriptor::access(stylesGetter,stylesSetter,false,true));
}

ObjectRef FontDataSourceSurface::extensionObject(const FontExtensionData& data){
    auto payload=std::make_shared<FontExtensionData>(data);
    const auto object=context_->object();
    context_->payload(object)=payload;
    auto setValue=[&](std::string_view name,Value value){
        context_->set(Value(object),Key(property_host::text(name)),std::move(value));
    };
    auto setProperty=[&](std::string_view name,Property value){
        setValue(name,properties_->to(value));
    };
    auto stringRecord=[](const auto& source){
        PropertyList result;
        for(const auto&[key,value]:source)result.setOwn(key,Property(value));
        return Property(result);
    };
    setValue("name",Value(property_host::text(payload->name)));
    if(payload->options)setValue("options",*payload->options);
    if(payload->variants)setProperty("variants",listPatchSource(*payload->variants));
    if(payload->variantSmp){
        PropertyList record;
        for(const auto&[key,value]:*payload->variantSmp){
            if(const auto* array=std::get_if<PropertyArray>(&value))record.setOwn(key,Property(*array));
            else record.setOwn(key,Property(std::get<std::string>(value)));
        }
        setProperty("variantSmp",Property(record));
    }
    if(payload->cssFonts){
        PropertyList record;
        for(const auto&[key,value]:*payload->cssFonts)record.setOwn(key,Property(value));
        setProperty("cssFonts",Property(record));
    }
    auto remapRecord=[](const RemapMap& source){
        PropertyList record;
        for(const auto&[key,value]:source)
            record.setOwn(std::to_string(key),Property(property_host::debugText(value)));
        return Property(record);
    };
    if(payload->accentMap)setProperty("accentMap",remapRecord(*payload->accentMap));
    if(payload->moMap)setProperty("moMap",remapRecord(*payload->moMap));
    if(payload->mnMap)setProperty("mnMap",remapRecord(*payload->mnMap));
    if(payload->parameters)setProperty("parameters",stringRecord(*payload->parameters));
    if(payload->cacheIds)setProperty("cacheIds",stringRecord(*payload->cacheIds));
    if(payload->delimiters){
        PropertyList record;
        for(const auto&[key,value]:*payload->delimiters){
            const auto* delimiter=std::get_if<DelimiterDataRef>(&value);
            if(!delimiter||!*delimiter)continue;
            PropertyList fields;fields.setOwn("dir",Property((*delimiter)->dir));
            if((*delimiter)->sizes){PropertyArray a;for(const auto n:*(*delimiter)->sizes)a.push(Property(n));fields.setOwn("sizes",Property(a));}
            if((*delimiter)->variants){PropertyArray a;for(const auto n:*(*delimiter)->variants)a.push(Property(n));fields.setOwn("variants",Property(a));}
            if((*delimiter)->schar){PropertyArray a;for(const auto n:*(*delimiter)->schar)a.push(Property(n));fields.setOwn("schar",Property(a));}
            if((*delimiter)->stretch){PropertyArray a;for(const auto n:*(*delimiter)->stretch)a.push(Property(n));fields.setOwn("stretch",Property(a));}
            if((*delimiter)->stretchv){PropertyArray a;for(const auto n:*(*delimiter)->stretchv)a.push(Property(n));fields.setOwn("stretchv",Property(a));}
            record.setOwn(std::to_string(key),Property(fields));
        }
        setProperty("delimiters",Property(record));
    }
    if(payload->chars){
        PropertyList variants;
        for(const auto&[variant,chars]:*payload->chars){
            PropertyList entries;
            for(const auto&[cp,value]:chars){
                if(const auto* data=std::get_if<Property>(&value))entries.setOwn(std::to_string(cp),*data);
                else throw Exception("Unimplemented",u"Dynamic placeholders cannot be projected into generated font chars");
            }
            variants.setOwn(variant,Property(entries));
        }
        setProperty("chars",Property(variants));
    }
    auto rangesProperty=[](const std::vector<DynamicFileDef>& ranges){
        PropertyArray files;
        for(const auto& file:ranges){
            PropertyList variants;
            for(const auto&[name,items]:file.variants){
                PropertyArray values;
                for(const auto& range:items){
                    PropertyArray tuple;tuple.push(Property(static_cast<double>(range.first)));
                    if(range.isRange)tuple.push(Property(static_cast<double>(range.last)));
                    values.push(Property(tuple));
                }
                variants.setOwn(name,Property(values));
            }
            PropertyArray delimiters;
            for(const auto& range:file.delimiters){
                PropertyArray tuple;tuple.push(Property(static_cast<double>(range.first)));
                if(range.isRange)tuple.push(Property(static_cast<double>(range.last)));
                delimiters.push(Property(tuple));
            }
            PropertyArray entry;entry.push(Property(file.file));entry.push(Property(variants));
            entry.push(Property(delimiters));files.push(Property(entry));
        }
        return Property(files);
    };
    if(payload->sizeVariants)setProperty("sizeVariants",listPatchSource(*payload->sizeVariants));
    if(payload->stretchVariants)setProperty("stretchVariants",listPatchSource(*payload->stretchVariants));
    if(payload->ranges)setProperty("ranges",rangesProperty(*payload->ranges));
    return object;
}

void FontDataSourceSurface::installSvgOverrides(ObjectRef constructor){
    // SvgFontData owns only its two static overrides.  Its prototype inherits
    // the complete FontData prototype; no parallel instance method table is
    // installed here.
    auto& cx=*context_;
    const auto charOptions=cx.function(u"",[constructor](Context& context,Value receiver,
        std::span<const Value> args)->Value{
        const auto parent=context.prototype(constructor);
        if(!parent)throw Exception("TypeError",u"SvgFontData has no FontData constructor parent");
        const auto baseMethod=context.get(Value(*parent),Key(u"charOptions"));
        const auto* callable=std::get_if<ObjectRef>(&baseMethod.data);
        if(!callable)throw Exception("TypeError",u"FontData.charOptions is not callable");
        return context.call(*callable,std::move(receiver),args);
    });
    cx.define(charOptions,Key(u"length"),Descriptor::data(Value(2),false,false,true));
    cx.define(constructor,Key(u"charOptions"),Descriptor::data(Value(charOptions),true,true,true));
    const auto addExtension=cx.function(u"",[](Context&,Value,std::span<const Value>)->Value{
        missingSourceAdapter(u"SvgFontData.addExtension");
    });
    cx.define(addExtension,Key(u"length"),Descriptor::data(Value(2),false,false,true));
    cx.define(constructor,Key(u"addExtension"),Descriptor::data(Value(addExtension),true,true,true));
}
}
