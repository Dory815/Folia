// Source FontData option-array merge bridge.  This is deliberately a narrow
// adapter over the existing Options algorithms and canonical Property graph.
export module folia.math.output.common.font_data_options;
export import folia.math.util.context_property_bindings;
export import folia.math.core.tree.property;
export import folia.math.output.common.font_data;
import folia.math.util.option_types;
import std;

export namespace folia::math::output::common {
// Apply one FontData mergeOptions(this, key, source) operation to the actual
// Context receiver. Falsy sources return the current field directly; truthy
// sources use real defaultOptions/Options.insert. Property arrays and records
// are projected through the caller's existing ContextPropertyBindings, so
// mutations reach the same source graph and actual Context operations.
Property mergeSourceOption(ContextPropertyBindings&,options::ObjectRef receiver,
                           std::u16string_view key,const Property& source);
// Convert the native tagged patch record back into the exact Source value
// shape consumed by Options.insert: a plain array for replacement, otherwise
// an ordinary object with the original [+]/[-] operands.
Property listPatchSource(const ListPatch&);
// Bind one raw Source property value as a real writable Source data field.
// It may be an array, null, undefined, or another value; coercion belongs at
// the Source operation that later reads length/indices.
void bindSourceOptionArrayField(ContextPropertyBindings&,options::ObjectRef receiver,
    std::shared_ptr<void> owner,std::u16string_view key,Property& field);
}
