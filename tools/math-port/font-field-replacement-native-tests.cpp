#include "../../tests/support/folia_test.hpp"
import folia.math.output.common.font_data_surface;
import folia.math.util.context_property_bindings;
import folia.math.util.option_functions;
import folia.math.util.options;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace common=folia::math::output::common;
namespace opts=folia::math::options;

struct FontGraph {
    opts::Context context;
    OptionFunctionBindings functions{context};
    ContextPropertyBindings properties{context,functions};
    common::FontDataContextBindings identities{context};
    opts::MapContextBindings maps{context};
    std::shared_ptr<regexp_host::RegExpBindings> regexp=
        std::make_shared<regexp_host::RegExpBindings>(context);
    common::FontDataSourceSurface surface{context,identities,properties,maps,regexp.get()};
    std::shared_ptr<async::JobQueue> queue=std::make_shared<async::JobQueue>();
    std::shared_ptr<common::FontLoadHost> host=std::make_shared<common::FontLoadHost>();
    std::shared_ptr<common::FontDataClass> klass=std::make_shared<common::FontDataClass>(context);
    opts::ObjectRef constructor{};
    opts::ObjectRef instance{};
    std::shared_ptr<common::FontData> font;

    FontGraph(){
        host->queue=queue;
        host->regexp=regexp.get();
        constructor=surface.registerBase(klass,*host,std::static_pointer_cast<void>(host),
            [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
               opts::Context& context,opts::Value options){
                return std::make_shared<common::FontData>(owner,loadHost,context,std::move(options));
            });
        instance=context.construct(constructor);
        font=identities.resolve(instance);
    }
};

suite fontFieldReplacementNative=[] {
    "font_extension_plain_array_replacement_keeps_old_alias_and_copies_nested_values"_test=[] {
        FontGraph graph;
        const auto fieldKey=opts::Key(u"sizeVariants");
        const auto oldValue=graph.context.get(graph.instance,fieldKey);
        const auto oldArray=std::get<opts::ObjectRef>(oldValue.data);
        auto nested=graph.context.object();
        graph.context.set(nested,opts::Key(u"value"),opts::Value(9));
        auto nestedArray=graph.context.array(1);
        graph.context.set(opts::Value(nestedArray),opts::Key(u"0"),opts::Value(nested));
        auto input=graph.context.array(3);
        graph.context.set(opts::Value(input),opts::Key(u"0"),opts::Value(u"replacement"));
        graph.context.set(opts::Value(input),opts::Key(u"2"),opts::Value(nestedArray));

        common::FontExtensionData extension;
        extension.name="field-replacement";
        extension.sizeVariants=common::ListPatch{
            PropertyArray::array(graph.properties.from(opts::Value(input))),std::nullopt,std::nullopt};
        graph.font->addExtension(extension);

        const auto currentValue=graph.context.get(graph.instance,fieldKey);
        const auto currentArray=std::get<opts::ObjectRef>(currentValue.data);
        expect(currentArray!=oldArray);
        expect(graph.context.arrayLength(oldArray)==0_u);
        expect(graph.context.arrayLength(currentArray)==3_u);
        expect(graph.context.get(opts::Value(currentArray),opts::Key(u"0"))==opts::Value(u"replacement"));
        expect(!graph.context.hasOwn(opts::Value(currentArray),opts::Key(u"1")));
        const auto copiedNestedArray=graph.context.get(opts::Value(currentArray),opts::Key(u"2"));
        expect(std::holds_alternative<opts::ObjectRef>(copiedNestedArray.data));
        if(const auto* copied=std::get_if<opts::ObjectRef>(&copiedNestedArray.data)){
            expect(*copied!=nestedArray);
            const auto copiedNested=graph.context.get(opts::Value(*copied),opts::Key(u"0"));
            expect(std::get<opts::ObjectRef>(copiedNested.data)!=nested);
        }
    };

    "font_instance_data_field_accepts_raw_assignment_until_source_use"_test=[] {
        FontGraph graph;
        const auto key=opts::Key(u"sizeVariants");
        const auto oldAlias=graph.context.get(graph.instance,key);
        bool assignmentThrew=false;
        try { graph.context.set(opts::Value(graph.instance),key,opts::Value(opts::Null{})); }
        catch(...) { assignmentThrew=true; }
        expect(!assignmentThrew);
        expect(graph.context.get(graph.instance,key)==opts::Value(opts::Null{}));
        expect(graph.context.arrayLength(std::get<opts::ObjectRef>(oldAlias.data))==0_u);
    };

    "font_add_extension_reads_raw_array_field_lengths_in_source_order"_test=[] {
        FontGraph graph;
        std::vector<std::string> reads;
        const auto sizeValue=graph.context.object();
        const auto stretchValue=graph.context.object();
        const auto sizeLength=graph.context.function(u"",[&reads](opts::Context&,opts::Value,
            std::span<const opts::Value>)->opts::Value{
            reads.push_back("size.length");return opts::Value(4);
        });
        const auto stretchLength=graph.context.function(u"",[&reads](opts::Context&,opts::Value,
            std::span<const opts::Value>)->opts::Value{
            reads.push_back("stretch.length");return opts::Value(6);
        });
        graph.context.define(sizeValue,opts::Key(u"length"),
            opts::Descriptor::access(sizeLength,{},true,true));
        graph.context.define(stretchValue,opts::Key(u"length"),
            opts::Descriptor::access(stretchLength,{},true,true));
        graph.context.set(opts::Value(graph.instance),opts::Key(u"sizeVariants"),
            opts::Value(sizeValue));
        graph.context.set(opts::Value(graph.instance),opts::Key(u"stretchVariants"),
            opts::Value(stretchValue));
        common::FontExtensionData extension;extension.name="raw-length-getters";
        (void)graph.font->addExtension(extension);
        expect(reads==std::vector<std::string>{"size.length","stretch.length"});
        const auto mapValue=graph.context.get(opts::Value(graph.constructor),opts::Key(u"dynamicExtensions"));
        const auto map=std::get<opts::ObjectRef>(mapValue.data);
        const auto extensionValue=graph.maps.get(map,opts::Value(u"raw-length-getters"));
        const auto extensionObject=std::get<opts::ObjectRef>(extensionValue.data);
        expect(graph.context.get(opts::Value(extensionObject),opts::Key(u"sizeN"))==opts::Value(4));
        expect(graph.context.get(opts::Value(extensionObject),opts::Key(u"stretchN"))==opts::Value(6));
    };

    "font_dynamic_extension_map_replacement_keeps_map_order_and_replaces_value_identity"_test=[] {
        FontGraph graph;
        const auto mapValue=graph.context.get(opts::Value(graph.constructor),opts::Key(u"dynamicExtensions"));
        const auto map=std::get<opts::ObjectRef>(mapValue.data);
        const auto first=graph.context.object();graph.context.set(opts::Value(first),opts::Key(u"name"),opts::Value(u"first"));
        const auto old=graph.context.object();graph.context.set(opts::Value(old),opts::Key(u"name"),opts::Value(u"old"));
        const auto last=graph.context.object();graph.context.set(opts::Value(last),opts::Key(u"name"),opts::Value(u"last"));
        graph.maps.set(map,opts::Value(u"first"),opts::Value(first));
        graph.maps.set(map,opts::Value(u"replace-me"),opts::Value(old));
        graph.maps.set(map,opts::Value(u"last"),opts::Value(last));
        common::FontExtensionData extension;extension.name="replace-me";
        graph.font->addExtension(extension);
        const auto entries=graph.maps.resolve(map)->entries();
        std::vector<opts::Value> liveKeys,liveValues;
        for(const auto& entry:entries)if(entry.live){liveKeys.push_back(entry.key);liveValues.push_back(entry.value);}
        expect(liveKeys.size()==3_u);
        expect(liveKeys[0]==opts::Value(u"first"));
        expect(liveKeys[1]==opts::Value(u"replace-me"));
        expect(liveKeys[2]==opts::Value(u"last"));
        expect(liveValues[0]==opts::Value(first));
        expect(liveValues[1]!=opts::Value(old));
        expect(liveValues[2]==opts::Value(last));
    };

    "font_dynamic_map_member_calls_do_not_read_function_call"_test=[] {
        FontGraph graph;
        const auto mapValue=graph.context.get(opts::Value(graph.constructor),opts::Key(u"dynamicExtensions"));
        const auto map=std::get<opts::ObjectRef>(mapValue.data);
        std::vector<std::u16string> reads;
        std::string stage="poison Map method call properties";
        for(const auto name:{std::u16string(u"set"),std::u16string(u"get"),std::u16string(u"values")}){
            try {
                const auto method=graph.context.get(opts::Value(map),opts::Key(name));
                const auto function=std::get<opts::ObjectRef>(method.data);
                const auto getter=graph.context.function(u"get call",[&reads,name](opts::Context&,
                    opts::Value,std::span<const opts::Value>)->opts::Value{
                    reads.push_back(name+u".call");
                    throw opts::Exception("Error",u"unexpected Function.call lookup");
                });
                graph.context.define(function,opts::Key(u"call"),
                    opts::Descriptor::access(getter,{},true,true));
            } catch(const opts::Exception& error){
                expect(false) << stage << ": " << error.name << ": "
                    << std::string(error.message.begin(),error.message.end());
                return;
            }
        }

        common::FontExtensionData extension;
        extension.name="direct-map-call";
        extension.ranges=std::vector<common::DynamicFileDef>{{"arrow.js",{}, {}}};
        stage="addExtension";
        try { (void)graph.font->addExtension(extension,"direct-map-call"); }
        catch(const opts::Exception& error){
            expect(false) << stage << ": " << error.name << ": "
                << std::string(error.message.begin(),error.message.end());
            return;
        }
        const auto stored=graph.maps.get(map,opts::Value(u"direct-map-call"));
        const auto extensionObject=std::get<opts::ObjectRef>(stored.data);
        const auto files=graph.context.get(opts::Value(extensionObject),opts::Key(u"files"));
        const auto fileValue=graph.context.get(files,opts::Key(u"arrow.js"));
        const auto fileObject=std::get<opts::ObjectRef>(fileValue.data);
        const auto* file=std::any_cast<common::DynamicFileRef>(&graph.context.payload(fileObject));
        expect(file!=nullptr);
        if(file&&*file){
            stage="dynamicFileName";
            try { expect(graph.font->dynamicFileName(opts::Value(*(*file)->sourceIdentity))==
                opts::Value(u"direct-map-call/arrow.js")); }
            catch(const opts::Exception& error){
                expect(false) << stage << ": " << error.name << ": "
                    << std::string(error.message.begin(),error.message.end());
                return;
            }
        }
        stage="loadDynamicFiles";
        folia::math::async::PromiseRef loads;
        try { loads=graph.font->loadDynamicFiles(); }
        catch(const opts::Exception& error){
            expect(false) << stage << ": " << error.name << ": "
                << std::string(error.message.begin(),error.message.end());
            return;
        }
        expect(loads!=nullptr);
        expect(reads.empty());
    };
};
}
