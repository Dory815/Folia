// Observe raw assignment and later method behavior on actual Context-bound
// DynamicFile owners. This candidate intentionally exposes any difference
// from font-dynamic-file-field-original-oracle.cjs.
import std;
import folia.math.output.common.font_data_surface;
import folia.math.util.context_property_bindings;
import folia.math.util.option_functions;
import folia.math.util.options;
import folia.math.util.property_host;
import folia.math.util.regexp_host;

namespace {
using namespace folia::math;
namespace common=folia::math::output::common;
namespace opts=folia::math::options;
struct Graph {
    opts::Context context;
    OptionFunctionBindings functions{context};
    ContextPropertyBindings properties{context,functions};
    common::FontDataContextBindings identities{context};
    opts::MapContextBindings maps{context};
    regexp_host::RegExpBindings regexp{context};
    common::FontDataSourceSurface surface{context,identities,properties,maps,&regexp};
    std::shared_ptr<async::JobQueue> queue=std::make_shared<async::JobQueue>();
    common::FontLoadHost host;
    std::shared_ptr<common::FontDataClass> klass=std::make_shared<common::FontDataClass>(context);
    std::shared_ptr<common::FontData> font;
    opts::ObjectRef instance{};
    Graph(){
        host.queue=queue;host.regexp=&regexp;host.asyncIsSynchronous=[] {return false;};host.warn=[](const std::any&){};
        auto ctor=surface.registerBase(klass,host,std::static_pointer_cast<void>(queue),
            [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
                opts::Context& cx,opts::Value options){
                return std::make_shared<common::FontData>(owner,loadHost,cx,std::move(options));
            });
        instance=context.construct(ctor);font=identities.resolve(instance);
    }
};
void quote(std::ostream& out,std::string_view s){out.put('"');for(char c:s){if(c=='"'||c=='\\')out.put('\\');if(c=='\n')out<<"\\n";else out.put(c);}out.put('"');}
void error(std::ostream& out,Graph& graph,const common::DynamicFileRef& file){
    try{
        const auto result=graph.font->dynamicFileName(opts::Value(*file->sourceIdentity));
        const auto* text=std::get_if<std::u16string>(&result.data);
        if(text)quote(out,property_host::debugText(*text));
        else out<<"{\"nonString\":true}";
    }
    catch(const opts::Exception& e){out<<"{\"error\":{\"name\":";quote(out,e.name);out<<",\"message\":";quote(out,property_host::debugText(e.message));out<<"}}";}
}
void observe(std::ostream& out,Graph& graph,opts::ObjectRef fileObject,
    const common::DynamicFileRef& file,std::string_view field,opts::ObjectRef replacement){
    const auto key=opts::Key(property_host::text(field));
    graph.context.set(opts::Value(fileObject),key,opts::Value(replacement));
    const auto after=graph.context.get(opts::Value(fileObject),key);
    const auto* same=std::get_if<opts::ObjectRef>(&after.data);
    const auto descriptor=graph.context.descriptor(opts::Value(fileObject),key);
    out<<"{\"sameValue\":"<<(same&&*same==replacement?"true":"false")<<",\"descriptor\":{"
        <<"\"writable\":"<<(descriptor&&descriptor->writable?"true":"false")<<','
        <<"\"enumerable\":"<<(descriptor&&descriptor->enumerable?"true":"false")<<','
        <<"\"configurable\":"<<(descriptor&&descriptor->configurable?"true":"false")<<"},\"dynamicFileName\":";
    error(out,graph,file);out<<'}';
}
}

int main(){
    try{
        Graph graph;
        common::FontExtensionData data;data.name="field-probe";
        data.ranges=std::vector<common::DynamicFileDef>{{"arrow.js",{}, {}}};
        (void)graph.font->addExtension(data);
        const auto mapValue=graph.context.get(opts::Value(*graph.klass->sourceIdentity),opts::Key(u"dynamicExtensions"));
        const auto map=std::get<opts::ObjectRef>(mapValue.data);
        const auto dynamicValue=graph.maps.get(map,opts::Value(u"field-probe"));
        const auto dynamic=std::get<opts::ObjectRef>(dynamicValue.data);
        const auto files=graph.context.get(opts::Value(dynamic),opts::Key(u"files"));
        const auto fileObject=std::get<opts::ObjectRef>(graph.context.get(files,opts::Key(u"arrow.js")).data);
        const auto* filePayload=std::any_cast<common::DynamicFileRef>(&graph.context.payload(fileObject));
        if(!filePayload||!*filePayload)throw std::runtime_error("DynamicFile Context owner is missing");
        const auto extensionReplacement=graph.context.object();
        const auto fileReplacement=graph.context.object();

        std::vector<std::string> receiverOrder;
        std::size_t fileReads=0;
        const auto matchingFile=graph.context.object();
        const auto matchGetter=graph.context.function(u"get match",
            [&graph,&receiverOrder,matchingFile](opts::Context&,opts::Value,
                std::span<const opts::Value>)->opts::Value{
                receiverOrder.push_back("match.get");
                return opts::Value(graph.context.function(u"match",
                    [&receiverOrder,matchingFile](opts::Context&,opts::Value receiver,
                        std::span<const opts::Value>)->opts::Value{
                        receiverOrder.push_back("match.call");
                        const auto* object=std::get_if<opts::ObjectRef>(&receiver.data);
                        return opts::Value(object&&*object==matchingFile);
                    }));
            });
        const auto replaceGetter=graph.context.function(u"get replace",
            [](opts::Context&,opts::Value,std::span<const opts::Value>)->opts::Value{
                throw opts::Exception("Error",u"replace must not be read");
            });
        graph.context.define(matchingFile,opts::Key(u"match"),
            opts::Descriptor::access(matchGetter,{},true,true));
        graph.context.define(matchingFile,opts::Key(u"replace"),
            opts::Descriptor::access(replaceGetter,{},true,true));
        const auto extensionGetter=graph.context.function(u"get extension",
            [&receiverOrder](opts::Context&,opts::Value,std::span<const opts::Value>)->opts::Value{
                receiverOrder.push_back("extension.get");
                return opts::Value(u"field-probe");
            });
        const auto fileGetter=graph.context.function(u"get file",
            [&graph,&receiverOrder,&fileReads,matchingFile](opts::Context&,opts::Value,
                std::span<const opts::Value>)->opts::Value{
                receiverOrder.push_back("file.get."+std::to_string(++fileReads));
                return opts::Value(matchingFile);
            });
        graph.context.define(fileObject,opts::Key(u"extension"),
            opts::Descriptor::access(extensionGetter,{},true,true));
        graph.context.define(fileObject,opts::Key(u"file"),
            opts::Descriptor::access(fileGetter,{},true,true));
        bool resultIsInput=false;
        try{
            const auto result=graph.font->dynamicFileName(opts::Value(fileObject));
            resultIsInput=result==opts::Value(matchingFile);
        }catch(const opts::Exception&){}

        std::cout<<"{\"ordinaryFields\":{";
        std::cout<<"\"extension\":";observe(std::cout,graph,fileObject,*filePayload,"extension",extensionReplacement);
        std::cout<<",\"file\":";observe(std::cout,graph,fileObject,*filePayload,"file",fileReplacement);
        std::cout<<",\"matchingFileOrder\":{\"resultIsInput\":"
            <<(resultIsInput?"true":"false")<<",\"receiverOrder\":[";
        for(std::size_t i=0;i<receiverOrder.size();++i){
            if(i)std::cout.put(',');quote(std::cout,receiverOrder[i]);
        }
        std::cout<<"]}";
        std::cout<<"}}\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<"DynamicFile observer: "<<e.what()<<'\n';return 1;}
}
