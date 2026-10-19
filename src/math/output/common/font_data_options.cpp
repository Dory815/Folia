module folia.math.output.common.font_data_options;
import folia.math.util.options;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
Property mergeSourceOption(ContextPropertyBindings& bindings,options::ObjectRef receiver,
                           std::u16string_view key,const Property& source){
    auto& context=bindings.context();
    // FontData.mergeOptions returns obj[dst] directly for a falsy source.
    // In particular this branch must not call defaultOptions or assign back:
    // either would expose extra accessors and alter alias/descriptor behavior.
    if(!property_host::truthy(source)){
        return bindings.from(context.get(options::Value(receiver),options::Key(std::u16string(key))));
    }
    const auto keyText=property_host::debugText(key);
    const PropertyList sourceRecord{{keyText,source}};
    const std::array<options::Value,1> definitions{bindings.to(Property(sourceRecord))};
    (void)options::defaultOptions(context,options::Value(receiver),definitions);
    return bindings.from(context.get(options::Value(receiver),options::Key(std::u16string(key))));
}

Property listPatchSource(const ListPatch& patch){
    // Options.insert receives an actual array for the replacement branch.
    // Append/remove are Source's ordinary object-patch keys; retaining the
    // PropertyArray handles here preserves their values, holes and aliases.
    if(patch.replace)return Property(*patch.replace);
    PropertyList source;
    if(patch.append)source.set("[+]",Property(*patch.append));
    if(patch.remove)source.set("[-]",Property(*patch.remove));
    return Property(std::move(source));
}

void bindSourceOptionArrayField(ContextPropertyBindings& bindings,options::ObjectRef receiver,
    std::shared_ptr<void> owner,std::u16string_view key,Property& field){
    if(!owner)throw options::Exception("TypeError",u"FontData option array requires its actual owner");
    auto cell=std::make_shared<options::NativeDataField>(std::move(owner),
        [&bindings,&field](){return bindings.to(Property(field));},
        [&bindings,&field](options::Value value){field=bindings.from(std::move(value));},
        [&field](){field=Missing{};});
    bindings.context().bindData(receiver,options::Key(std::u16string(key)),std::move(cell));
}
}
