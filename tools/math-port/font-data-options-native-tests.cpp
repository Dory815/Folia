#include "../../tests/support/folia_test.hpp"
import folia.math.output.common.font_data_options;
import folia.math.util.context_property_bindings;
import folia.math.util.options;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace opts=folia::math::options;

struct Graph {
    opts::Context context;
    OptionFunctionBindings functions{context};
    ContextPropertyBindings properties{context,functions};
};

struct ArrayOwner {
    Property field=PropertyArray{};
};

opts::ObjectRef makeArray(opts::Context& context,
                          std::initializer_list<opts::Value> values) {
    auto array=context.array(static_cast<std::uint32_t>(values.size()));
    std::size_t index=0;
    for(const auto& value:values)
        context.set(opts::Value(array),opts::Key(property_host::text(std::to_string(index++))),value);
    return array;
}

suite fontDataOptionsNative=[] {
    "font_data_merge_falsy_source_returns_live_receiver_value"_test=[] {
        Graph graph;
        auto receiver=graph.context.object();
        auto initial=makeArray(graph.context,{opts::Value(1),opts::Value(2)});
        graph.context.set(receiver,opts::Key(u"sizeVariants"),opts::Value(initial));
        const auto result=output::common::mergeSourceOption(
            graph.properties,receiver,u"sizeVariants",Property(Missing{}));
        expect(graph.properties.to(result)==opts::Value(initial));
        expect(graph.context.get(receiver,opts::Key(u"sizeVariants"))==opts::Value(initial));
    };

    "font_data_merge_append_uses_real_default_options_and_spreads_holes"_test=[] {
        Graph graph;
        auto receiver=graph.context.object();
        auto initial=makeArray(graph.context,{opts::Value(1)});
        auto additions=graph.context.array(2);
        graph.context.set(opts::Value(additions),opts::Key(u"1"),opts::Value(9));
        graph.context.set(receiver,opts::Key(u"sizeVariants"),opts::Value(initial));
        Property patch(PropertyList{{"[+]",graph.properties.from(additions)}});
        const auto merged=output::common::mergeSourceOption(
            graph.properties,receiver,u"sizeVariants",patch);
        auto result=PropertyArray::array(merged);
        expect(result.has_value());
        expect(result->size()==3_u);
        expect(result->has(0));expect(result->get(0)==Property(1));
        expect(result->has(1));expect(result->get(1).missing());
        expect(result->has(2));expect(result->get(2)==Property(9));
        const auto assigned=graph.context.get(receiver,opts::Key(u"sizeVariants"));
        expect(graph.properties.to(merged)==assigned);
        expect(assigned!=opts::Value(initial));
    };

    "font_data_merge_remove_uses_the_original_context_object_identity"_test=[] {
        Graph graph;
        auto receiver=graph.context.object();
        auto retained=graph.context.object();
        auto removed=graph.context.object();
        auto initial=makeArray(graph.context,{opts::Value(retained),opts::Value(removed)});
        auto removals=makeArray(graph.context,{opts::Value(removed)});
        graph.context.set(receiver,opts::Key(u"sizeVariants"),opts::Value(initial));
        Property patch(PropertyList{{"[-]",graph.properties.from(removals)}});
        const auto merged=output::common::mergeSourceOption(
            graph.properties,receiver,u"sizeVariants",patch);
        auto result=PropertyArray::array(merged);
        expect(result.has_value());expect(result->size()==1_u);
        const auto value=graph.properties.to(result->get(0));
        expect(value==opts::Value(retained));
        expect(graph.properties.to(merged)==graph.context.get(receiver,opts::Key(u"sizeVariants")));
    };

    "font_data_merge_plain_array_replacement_uses_options_deep_insert"_test=[] {
        Graph graph;
        auto receiver=graph.context.object();
        auto initial=makeArray(graph.context,{opts::Value(1)});
        auto leaf=graph.context.object();
        graph.context.set(leaf,opts::Key(u"value"),opts::Value(8));
        auto nestedArray=makeArray(graph.context,{opts::Value(leaf)});
        auto nested=graph.context.object();
        graph.context.set(nested,opts::Key(u"nested"),opts::Value(nestedArray));
        auto source=graph.context.array(3);
        graph.context.set(opts::Value(source),opts::Key(u"1"),opts::Value(nested));
        graph.context.set(receiver,opts::Key(u"sizeVariants"),opts::Value(initial));
        const auto merged=output::common::mergeSourceOption(
            graph.properties,receiver,u"sizeVariants",graph.properties.from(source));
        const auto value=graph.properties.to(merged);
        const auto* result=std::get_if<opts::ObjectRef>(&value.data);
        expect(result!=nullptr);
        expect(*result!=source);expect(*result!=initial);
        // Options.insert copies enumerable array keys, not the source array's
        // non-enumerable length. Only index 1 is an own enumerable key, so
        // the replacement has length 2.
        expect(graph.context.arrayLength(*result)==2_u);
        expect(!graph.context.hasOwn(*result,opts::Key(u"0")));
        expect(!graph.context.hasOwn(*result,opts::Key(u"2")));
        const auto clonedNested=graph.context.get(*result,opts::Key(u"1"));
        expect(std::get<opts::ObjectRef>(clonedNested.data)!=nested);
        const auto clonedArray=graph.context.get(std::get<opts::ObjectRef>(clonedNested.data),opts::Key(u"nested"));
        expect(std::get<opts::ObjectRef>(clonedArray.data)!=nestedArray);
        const auto clonedLeaf=graph.context.get(std::get<opts::ObjectRef>(clonedArray.data),opts::Key(u"0"));
        expect(std::get<opts::ObjectRef>(clonedLeaf.data)!=leaf);
    };

    "font_data_list_patch_preserves_replace_append_and_remove_array_handles"_test=[] {
        auto replaced=PropertyArray{Property(3),Property(4)};
        auto appended=PropertyArray{Property(5)};
        auto removed=PropertyArray{Property(6)};
        const auto replaceValue=output::common::listPatchSource(output::common::ListPatch{replaced,{},{}});
        expect(replaceValue==Property(replaced));
        expect(PropertyArray::array(replaceValue)->get(1)==Property(4));
        expect(PropertyArray::array(replaceValue)->size()==2_u);

        const auto patchValue=output::common::listPatchSource(output::common::ListPatch{{},appended,removed});
        const auto patch=PropertyList::record(patchValue);
        expect(patch.has_value());
        expect(patch->get("[+]")==Property(appended));
        expect(patch->get("[-]")==Property(removed));
        expect(PropertyArray::array(patch->get("[+]"))->get(0)==Property(5));
        expect(PropertyArray::array(patch->get("[-]"))->get(0)==Property(6));
        expect(PropertyArray::array(patch->get("[+]"))->size()==1_u);
    };

    "font_data_option_array_field_reads_and_replacements_share_the_owner"_test=[] {
        Graph graph;
        auto owner=std::make_shared<ArrayOwner>();
        owner->field=PropertyArray{Property(1)};
        const auto receiver=graph.context.object();
        output::common::bindSourceOptionArrayField(graph.properties,receiver,
            std::static_pointer_cast<void>(owner),u"sizeVariants",owner->field);

        const auto first=graph.context.get(receiver,opts::Key(u"sizeVariants"));
        expect(std::get<opts::ObjectRef>(first.data)==
               std::get<opts::ObjectRef>(graph.properties.to(owner->field).data));
        const auto descriptor=graph.context.descriptor(opts::Value(receiver),opts::Key(u"sizeVariants"));
        expect(descriptor.has_value());
        expect(descriptor->writable);expect(descriptor->enumerable);expect(descriptor->configurable);
        const auto replacement=makeArray(graph.context,{opts::Value(7),opts::Value(8)});
        graph.context.set(opts::Value(receiver),opts::Key(u"sizeVariants"),opts::Value(replacement));
        auto ownerArray=PropertyArray::array(owner->field);
        expect(ownerArray.has_value());
        expect(ownerArray->size()==2_u);
        expect(ownerArray->get(0)==Property(7));
        expect(graph.context.get(receiver,opts::Key(u"sizeVariants"))==opts::Value(replacement));
        ownerArray->set(1,Property(9));
        expect(graph.context.get(opts::Value(replacement),opts::Key(u"1"))==opts::Value(9));
        graph.context.erase(receiver,opts::Key(u"sizeVariants"));
        expect(!graph.context.hasOwn(opts::Value(receiver),opts::Key(u"sizeVariants")));
        const auto afterDelete=makeArray(graph.context,{opts::Value(11)});
        graph.context.set(opts::Value(receiver),opts::Key(u"sizeVariants"),opts::Value(afterDelete));
        const auto restored=PropertyArray::array(owner->field);
        expect(restored.has_value());
        expect(restored->size()==1_u);expect(restored->get(0)==Property(11));
        expect(graph.context.get(opts::Value(receiver),opts::Key(u"sizeVariants"))==opts::Value(afterDelete));
    };

    "font_option_array_field_keeps_raw_assignment_and_delete_rebind_on_one_owner"_test=[] {
        Graph graph;
        auto owner=std::make_shared<ArrayOwner>();
        owner->field=PropertyArray{Property(1)};
        const auto receiver=graph.context.object();
        output::common::bindSourceOptionArrayField(graph.properties,receiver,
            std::static_pointer_cast<void>(owner),u"sizeVariants",owner->field);
        const auto oldValue=graph.context.get(receiver,opts::Key(u"sizeVariants"));
        const auto oldArray=std::get<opts::ObjectRef>(oldValue.data);

        graph.context.set(receiver,opts::Key(u"sizeVariants"),opts::Value(opts::Null{}));
        expect(owner->field==Property(PropertyNull{}));
        expect(graph.context.get(receiver,opts::Key(u"sizeVariants"))==opts::Value(opts::Null{}));
        expect(graph.context.arrayLength(oldArray)==1_u);

        graph.context.erase(receiver,opts::Key(u"sizeVariants"));
        expect(!graph.context.hasOwn(opts::Value(receiver),opts::Key(u"sizeVariants")));
        expect(owner->field.missing());
        const auto replacement=makeArray(graph.context,{opts::Value(8),opts::Value(9)});
        graph.context.set(opts::Value(receiver),opts::Key(u"sizeVariants"),opts::Value(replacement));
        expect(std::get<opts::ObjectRef>(graph.properties.to(owner->field).data)==replacement);
        expect(graph.context.get(receiver,opts::Key(u"sizeVariants"))==opts::Value(replacement));
    };
};
}
