module folia.math.output.svg.font_data_surface;
import folia.math.util.js_number;
import folia.math.util.property_host;
import std;

namespace folia::math::output::svg {
using namespace options;

SvgFontDataSourceSurface::SvgFontDataSourceSurface(Context& context,
    common::FontDataContextBindings& identities,
    common::FontDataSourceSurface& commonSurface)
    :context_(&context),identities_(&identities),commonSurface_(&commonSurface){
    if(&identities.context()!=&context)
        throw Exception("TypeError",u"SVG FontData surface must share the common Context");
}

ObjectRef SvgFontDataSourceSurface::registerSvg(
    std::shared_ptr<SvgFontDataClass> owner,common::FontLoadHost& host,
    std::shared_ptr<void> hostAnchor,common::FontDataContextBindings::Allocator allocator,
    ObjectRef parentConstructor,
    common::FontDataContextBindings::InstanceInitializer initializeInstance){
    if(!owner)throw Exception("TypeError",u"SvgFontData class owner is null");
    auto* commonSurface=commonSurface_;
    auto sourceInitialize=[commonSurface,initializeInstance=std::move(initializeInstance)](
        const std::shared_ptr<common::FontData>& font,ObjectRef instance){
        commonSurface->initializeInstanceFields(font,instance);
        if(initializeInstance)initializeInstance(font,instance);
    };
    const auto constructor=identities_->registerClass(u"SvgFontData",owner,host,
        std::move(hostAnchor),std::move(allocator),parentConstructor,
        std::move(sourceInitialize));
    installSvg(constructor);
    return constructor;
}

void SvgFontDataSourceSurface::installSvg(ObjectRef constructor){
    auto& context=*context_;

    // The emitted CJS constructor is declared with no formal arguments even
    // though the TypeScript source forwards its optional argument to super.
    context.define(constructor,Key(u"length"),Descriptor::data(Value(0),false,false,true));
    // Install the common static overrides on this actual derived constructor.
    // Their `super` dispatch stays dynamic and the shared surface owns their
    // sole callback bodies.
    commonSurface_->installSvgOverrides(constructor);
    const auto addExtension=context.function(u"",[identities=identities_,constructor](Context& cx,
        Value receiver,std::span<const Value> args)->Value{
        const auto parent=cx.prototype(constructor);
        if(!parent)throw Exception("TypeError",u"SvgFontData has no FontData constructor parent");
        const auto baseMethod=cx.get(Value(*parent),Key(u"addExtension"));
        const auto* baseFunction=std::get_if<ObjectRef>(&baseMethod.data);
        if(!baseFunction||cx.kind(*baseFunction)!=ObjectKind::function)
            throw Exception("TypeError",u"FontData.addExtension is not a function");
        const auto result=cx.call(*baseFunction,receiver,args);
        const auto* classObject=std::get_if<ObjectRef>(&receiver.data);
        if(!classObject)throw Exception("TypeError",u"SvgFontData.addExtension called on an incompatible receiver");
        auto owner=std::static_pointer_cast<SvgFontDataClass>(identities->resolveClass(*classObject));
        const auto data=args.empty()?Value(Undefined{}):args[0];
        const auto cacheIds=cx.get(data,Key(u"cacheIds"));
        if(!std::holds_alternative<Undefined>(cacheIds.data)&&!std::holds_alternative<Null>(cacheIds.data)){
            for(const auto& key:cx.enumerableStringKeys(cacheIds)){
                const auto value=cx.get(cacheIds,Key(key));
                if(const auto* text=std::get_if<std::u16string>(&value.data))
                    owner->variantCacheIds.insert_or_assign(property_host::debugText(key),
                                                             property_host::debugText(*text));
                else throw Exception("Unimplemented",u"SVG cacheIds values must use supported string values");
            }
        }
        return result;
    });
    context.define(addExtension,Key(u"length"),Descriptor::data(Value(2),false,false,true));
    context.define(constructor,Key(u"addExtension"),Descriptor::data(Value(addExtension),true,true,true));
    context.set(Value(constructor),Key(u"JAX"),Value(u"SVG"));
}
}
