module folia.math.fonts.newcm_font_data_surface;
import folia.math.util.property_host;
import std;

namespace folia::math::fonts {
using namespace options;

NewcmFontDataSourceSurface::NewcmFontDataSourceSurface(Context& context,
    output::common::FontDataContextBindings& identities,
    output::common::FontDataSourceSurface& commonSurface)
    :context_(&context),identities_(&identities),commonSurface_(&commonSurface){
    if(&identities.context()!=&context)
        throw Exception("TypeError",u"NewCM FontData surface must share its Context");
}

ObjectRef NewcmFontDataSourceSurface::registerNewcm(
    std::shared_ptr<NewcmFontDataClass> owner,output::common::FontLoadHost& host,
    std::shared_ptr<void> hostAnchor,
    output::common::FontDataContextBindings::Allocator allocator,
    ObjectRef parentConstructor,
    output::common::FontDataContextBindings::InstanceInitializer initializeInstance){
    if(!owner)throw Exception("TypeError",u"NewCM FontData class owner is null");
    auto* commonSurface=commonSurface_;
    auto sourceInitialize=[commonSurface,initializeInstance=std::move(initializeInstance)](
        const std::shared_ptr<output::common::FontData>& font,ObjectRef instance){
        commonSurface->initializeInstanceFields(font,instance);
        if(initializeInstance)initializeInstance(font,instance);
    };
    const auto constructor=identities_->registerClass(u"MathJaxNewcmFont",owner,
        host,std::move(hostAnchor),std::move(allocator),parentConstructor,
        std::move(sourceInitialize));
    // The emitted derived class has the implicit zero-argument constructor;
    // the shared native allocator's extra options parameter is not reflected
    // in Source constructor metadata.
    context_->define(constructor,Key(u"length"),
        Descriptor::data(Value(0),false,false,true));
    auto classOwner=std::move(owner);
    context_->bindData(constructor,Key(u"NAME"),std::make_shared<NativeDataField>(
        std::static_pointer_cast<void>(classOwner),
        [classOwner]{return Value(property_host::text(classOwner->NAME));},
        [classOwner](Value value){
            if(const auto* text=std::get_if<std::u16string>(&value.data))
                classOwner->NAME=property_host::debugText(*text);
            else throw Exception("TypeError",u"MathJaxNewcmFont.NAME must be a string");
        }));
    // This is the source module's post-class-definition static initializer:
    // generated data and bundled loader recipes are attached only after the
    // actual canonical constructor identity exists.
    classOwner->initializeSourceData();
    // Bind the live class fields after their Source initializers have assigned
    // the real arrays. Binding before this phase would keep the empty native
    // placeholders rather than the canonical initialized storage.
    commonSurface->bindClassArrayFields(classOwner,constructor);
    return constructor;
}
}
