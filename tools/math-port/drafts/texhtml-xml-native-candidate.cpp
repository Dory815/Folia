// Four native observations for the locked CommonXmlNode/SvgXmlNode oracle.
// Every input DOM node, MML node, wrapper, BBox and SVG snapshot comes from the
// render session's canonical owners; no fixture geometry is substituted.
import std;
import folia.math.core.math_item;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.xml_dom_adaptor;
import folia.math.output.common;
import folia.math.output.scene;
import folia.math.output.svg.native_document;
import folia.math.output.svg.serializer;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.util.styles;
import folia.math.util.utf16;

namespace {
namespace opts=folia::math::options;
namespace core=folia::math::core;
namespace mml=folia::math::mml;
namespace common=folia::math::output::common;
namespace scene=folia::math::output::scene;
namespace svg=folia::math::output::svg;
using namespace folia::math;

common::FontLoadHost fontHost(){
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=[](const std::any&){};
    return host;
}

std::string quote(std::string_view value){
    std::string out="\"";constexpr char hex[]="0123456789abcdef";
    for(unsigned char c:value){
        switch(c){
        case '\\':out+="\\\\";break;
        case '"':out+="\\\"";break;
        case '\n':out+="\\n";break;
        case '\r':out+="\\r";break;
        case '\t':out+="\\t";break;
        default:if(c<0x20){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}else out+=static_cast<char>(c);
        }
    }
    out+='"';return out;
}
std::string encode(std::u16string_view value){return utf16::encode(value);}
std::u16string ascii16(std::string_view value){return std::u16string(value.begin(),value.end());}
std::string boolean(bool value){return value?"true":"false";}
std::string number(double value){
    if(std::isnan(value))return "{\"$\":\"NaN\"}";
    if(std::isinf(value))return value<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}";
    if(value==0&&std::signbit(value))return "{\"$\":\"-0\"}";
    std::ostringstream out;out<<std::setprecision(std::numeric_limits<double>::max_digits10)<<value;
    return out.str();
}
std::string property(const Property& value){
    if(value.missing())return "{\"$\":\"undefined\"}";
    if(std::holds_alternative<PropertyNull>(value.value))return "null";
    if(const auto* v=std::get_if<bool>(&value.value))return boolean(*v);
    if(const auto* v=std::get_if<double>(&value.value))return number(*v);
    if(const auto* v=std::get_if<std::string>(&value.value))return quote(*v);
    if(const auto* v=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);v&&*v){
        if((*v)->array){
            const auto array=PropertyArray::array(value).value();std::string out="[";
            for(std::size_t i=0;i<array.size();++i){if(i)out+=',';out+=array.has(i)?property(array.get(i)):"{\"$\":\"hole\"}";}
            return out+"]";
        }
        const auto record=PropertyList::record(value).value();std::string out="{";bool first=true;
        for(const auto& key:record.keys()){if(!first)out+=',';first=false;out+=quote(key)+":"+property(record.getOwn(key));}
        return out+"}";
    }
    return "null";
}
template<class F> std::string observed(F&& fn){
    try{return "{\"value\":"+fn()+",\"error\":null}";}
    catch(const opts::Exception& error){
        return "{\"value\":null,\"error\":{\"name\":"+quote(error.name)+
            ",\"message\":"+quote(encode(error.message))+",\"id\":null}}";
    }
    catch(const std::exception& error){
        return "{\"value\":null,\"error\":{\"name\":\"NativeException\",\"message\":"+
            quote(error.what())+",\"id\":null}}";
    }
}
opts::Value sourceMember(opts::Context& context,opts::Value receiver,std::u16string_view name){
    return context.get(receiver,opts::Key(std::u16string(name)));
}
opts::ObjectRef objectValue(const opts::Value& value,std::string_view where){
    if(const auto* object=std::get_if<opts::ObjectRef>(&value.data))return *object;
    throw std::runtime_error(std::string(where)+" is not a Context object");
}
void installOracleSvgProducer(opts::Context& context,opts::ObjectRef output){
    const auto svg=context.function(u"svg",
        [](opts::Context& cx,opts::Value receiver,std::span<const opts::Value> args)->opts::Value{
            const auto at=[&](std::size_t index){
                return index<args.size()?args[index]:opts::Value(opts::Undefined{});
            };
            const auto kind=at(0);
            auto definition=at(1);
            if(std::holds_alternative<opts::Undefined>(definition.data))definition=opts::Value(cx.object());
            const auto properties=cx.object();
            cx.set(opts::Value(properties),opts::Key(u"xmlns"),opts::Value(u"http://www.w3.org/2000/svg"));
            for(const auto& key:cx.enumerableStringKeys(definition))
                cx.set(opts::Value(properties),opts::Key(key),cx.get(definition,opts::Key(key)));
            auto children=at(2);
            if(std::holds_alternative<opts::Undefined>(children.data))children=opts::Value(cx.array());
            const auto html=cx.get(receiver,opts::Key(u"html"));
            const std::array<opts::Value,4> forwarded{kind,opts::Value(properties),children,
                opts::Value(u"http://www.w3.org/2000/svg")};
            return cx.callMember(html,receiver,forwarded,u"this.html");
        });
    context.set(opts::Value(output),opts::Key(u"svg"),opts::Value(svg));
}
std::string kinds(AbstractDOMAdaptor& adaptor,const DOMNodeArray& nodes){
    std::string out="[";
    for(std::size_t i=0;i<nodes.slots.size();++i){
        if(i)out+=',';
        const auto value=propertyNode(nodes.slots.get(i));
        const auto* node=std::get_if<DOMNodeRef>(&value);
        if(!node)throw std::runtime_error("Lite childNodes contains a non-node value");
        out+=quote(encode(adaptor.kind(*node)));
    }
    return out+"]";
}

struct CaseInput {const char* name;const char* option;const char* hdw;};

std::string observe(const CaseInput& input){
    svg::NativeRenderSession session(fontHost());
    auto& context=session.context();
    const auto outputOptions=context.object();
    context.set(outputOptions,opts::Key(u"fontCache"),opts::Value(u"none"));
    context.set(outputOptions,opts::Key(u"htmlHDW"),opts::Value(ascii16(input.option)));
    context.set(outputOptions,opts::Key(u"scale"),opts::Value(1));
    const auto output=session.construct(opts::Value(outputOptions));
    // Match the locked CJS oracle's custom jax.svg stub. The pinned SVG.svg
    // method itself only passes SVGNS as the fourth html() argument; the
    // oracle stub additionally materializes xmlns as an own property.
    installOracleSvgProducer(context,output->instanceIdentity());
    auto document=svg::createNativeSvgDocument(session);
    auto adaptor=session.adaptor();
    if(!adaptor)throw std::runtime_error("Native session has no actual Lite adaptor");

    PropertyList xmlAttributes;
    if(input.hdw)xmlAttributes.setOwn("data-mjx-hdw",Property(input.hdw));
    const auto text=adaptor->text(u"xml & html");
    const auto bold=adaptor->node(u"b",PropertyList{{"class",Property("probe")},
        {"style",Property("font-weight: bold")}},PropertyArray{Property(nodeProperty(text))});
    const auto xmlDom=adaptor->node(u"section",xmlAttributes,
        PropertyArray{Property(nodeProperty(bold))});
    auto xmlAdaptor=mml::XmlDomAdaptor::create(adaptor);
    auto factory=session.mathFactory();
    if(!factory)throw std::runtime_error("Native session has no canonical MmlFactory");
    const std::span<const std::any> noArguments{};
    auto* math=factory->create("math",noArguments);
    auto* xml=dynamic_cast<mml::XMLNode*>(factory->create("XML",noArguments));
    if(!math||!xml)throw std::runtime_error("Canonical MmlFactory did not construct math/XML nodes");
    xml->setXML(xmlAdaptor->fromDOM(xmlDom),xmlAdaptor.get());
    (void)math->appendChild(mml::NodeValue(xml));

    const opts::Value documentValue(document->sourceIdentity());
    if(&document->sourceContext()!=&context)
        throw std::runtime_error("MathDocument does not retain the session Context");
    auto& sourceContext=context;
    const auto documentOptions=sourceMember(sourceContext,documentValue,u"options");
    const auto itemConstructor=sourceMember(sourceContext,documentOptions,u"MathItem");
    const auto inputJaxArray=sourceMember(sourceContext,documentValue,u"inputJax");
    const auto inputJax=sourceContext.get(inputJaxArray,opts::Key(u"0"));
    const std::array<opts::Value,3> itemArguments{opts::Value(u"<xml fixture>"),inputJax,opts::Value(true)};
    const auto itemIdentity=sourceContext.construct(objectValue(itemConstructor,"MathItem constructor"),itemArguments);
    const auto* itemPayload=std::any_cast<std::shared_ptr<NativeObject>>(&sourceContext.payload(itemIdentity));
    auto item=itemPayload?std::dynamic_pointer_cast<core::MathItem>(*itemPayload):nullptr;
    if(!item)throw std::runtime_error("Canonical MathItem payload is unavailable");
    item->root=mml::NodeValue(math);
    item->setMetrics(Property(16),Property(8),Property(640),Property(2));
    auto metrics=PropertyList::record(item->metrics).value();
    metrics.setOwn("family",Property("SourceFont"));
    item->metrics=Property(metrics);
    const auto start=adaptor->node(u"div");
    item->start=Property(PropertyList{{"node",nodeProperty(start)},
        {"n",Property(0)}, {"delim",PropertyNull{}}});
    const auto container=adaptor->node(u"div");

    auto* commonJax=static_cast<common::CommonOutputJax*>(output.get());
    const auto fixed=context.function(u"fixed",[](opts::Context&,opts::Value,
        std::span<const opts::Value> arguments)->opts::Value{
        const auto* value=arguments.empty()?nullptr:std::get_if<double>(&arguments[0].data);
        if(!value)throw opts::Exception("TypeError",u"Number.toFixed requires a numeric value");
        unsigned digits=3;
        if(arguments.size()>1){
            const auto* requested=std::get_if<double>(&arguments[1].data);
            if(!requested||!std::isfinite(*requested))throw opts::Exception("RangeError",u"Invalid toFixed digits");
            digits=static_cast<unsigned>(std::clamp(std::trunc(*requested),0.0,100.0));
        }
        const auto text=js_number::toFixed(*value,digits);
        return opts::Value(std::u16string(text.begin(),text.end()));
    });
    context.set(opts::Value(commonJax->instanceIdentity()),opts::Key(u"fixed"),fixed);
    const auto actualOptions=objectValue(commonJax->options,"OutputJax options");
    context.set(actualOptions,opts::Key(u"htmlHDW"),opts::Value(ascii16(input.option)));
    commonJax->container=container;
    const auto job=commonJax->beginLayoutJob(*item,core::MathDocumentReference{document.get()},container);
    auto* mathWrapper=commonJax->factory->wrap(math);
    if(!mathWrapper)throw std::runtime_error("Actual CommonWrapperFactory returned no math wrapper");
    // The locked Source oracle supplies a parent wrapper whose live Styles
    // owner returns ParentFont. Install that same value on this real wrapper.
    mathWrapper->styles=std::make_unique<styles::Styles>();
    mathWrapper->styles->set(u"font-family",styles::StyleValue(u"ParentFont"));
    // A rejected Lite measurement is itself one of the locked outcomes. Keep
    // the actual job alive on that path so its already-created XML wrapper can
    // still be observed exactly as the Source call sequence does.
    try{(void)mathWrapper->getBBox();}catch(const opts::Exception&){}
    auto* xmlWrapperBase=job->nodeMap->find(xml);
    auto* xmlWrapper=dynamic_cast<common::CommonXmlNode*>(xmlWrapperBase);
    if(!xmlWrapper)throw std::runtime_error("Actual XML wrapper is absent from the active canonical nodeMap");
    if(xmlWrapper->parent){
        xmlWrapper->parent->styles=std::make_unique<styles::Styles>();
        xmlWrapper->parent->styles->set(u"font-family",styles::StyleValue(u"ParentFont"));
        xmlWrapper->parent->variant="normal";
    }

    const auto bbox=observed([&]{
        const auto value=xmlWrapper->getBBox();
        return std::string("{\"w\":")+number(value->w)+",\"h\":"+number(value->h)+
            ",\"d\":"+number(value->d)+",\"scale\":"+number(value->scale)+
            ",\"rscale\":"+number(xmlWrapper->rscale)+"}";
    });
    const auto styles=observed([&]{return property(xmlWrapper->getFontStyles());});
    const auto html=observed([&]{
        const auto node=xmlWrapper->getHTML();
        const auto child=adaptor->firstChild(node);
        const auto* childNode=std::get_if<DOMNodeRef>(&child);
        if(!childNode)throw opts::Exception("TypeError",u"Cannot read properties of undefined (reading 'children')");
        return std::string("{\"kind\":")+quote(encode(adaptor->kind(node)))+
            ",\"serialized\":"+quote(encode(adaptor->outerHTML(node)))+
            ",\"childKind\":"+quote(encode(adaptor->kind(*childNode)))+
            ",\"childSerialized\":"+quote(encode(adaptor->outerHTML(*childNode)))+"}";
    });
    const auto svgRecord=observed([&]{
        scene::Builder builder;
        auto root=builder.node(u"svg",PropertyList{{"xmlns",Property("http://www.w3.org/2000/svg")}});
        scene::XmlNodePlacement placement;
        scene::WrapperFactory placements;
        placements.exchangeXmlSvgNodeCreator(
            [output](scene::Builder& target,scene::NodeRef shell,
                const PropertyList& attributes,DOMNodeRef html){
                return output->createXmlSceneNode(target,std::move(shell),attributes,std::move(html));
            });
        const std::array<scene::NodeRef,1> parents{root};
        (void)placement.build(*xmlWrapper,builder,placements,parents);
        builder.addRoot(root);
        auto completed=std::move(builder).finish();
        svg::Serializer serializer(*adaptor);
        const auto roots=serializer.materialize(completed);
        if(roots.size()!=1)throw std::runtime_error("XML scene did not materialize one SVG root");
        return std::string("{\"svg\":")+quote(encode(adaptor->outerHTML(roots.front())))+
            ",\"children\":"+kinds(*adaptor,adaptor->childNodes(roots.front()))+"}";
    });
    const auto xmlRecord=std::string("{\"kind\":")+quote(encode(adaptor->kind(xmlDom)))+
        ",\"serialized\":"+quote(encode(adaptor->outerHTML(xmlDom)))+
        ",\"textContent\":"+quote(encode(adaptor->textContent(xmlDom)))+"}";
    const auto measurement=std::string("{\"containerChildren\":")+kinds(*adaptor,adaptor->childNodes(container))+
        ",\"containerParent\":";
    const auto parent=adaptor->parent(container);
    const auto* parentNode=std::get_if<DOMNodeRef>(&parent);
    const auto measurementRecord=measurement+(parentNode?quote(encode(adaptor->kind(*parentNode))):"null")+"}";
    commonJax->finishLayoutJob();
    return std::string("{\"name\":")+quote(input.name)+",\"option\":"+quote(input.option)+
        ",\"definition\":"+(input.hdw?std::string("{\"data-mjx-hdw\":")+quote(input.hdw)+"}":"{}")+
        ",\"xml\":"+xmlRecord+",\"bbox\":"+bbox+",\"styles\":"+styles+
        ",\"html\":"+html+",\"svg\":"+svgRecord+
        ",\"measurementNodesAfterCalls\":"+measurementRecord+"}";
}
}

int main(){
    try{
        const std::array<CaseInput,4> inputs{{
            {"xml-hdw-use","use","1em .25em 2em"},
            {"xml-hdw-force","force","1.5em .5em 3em"},
            {"xml-hdw-disabled","ignore","1em .25em 2em"},
            {"xml-measured-lite-dom","use",nullptr},
        }};
        std::cout<<"{\"cases\":[";
        for(std::size_t i=0;i<inputs.size();++i){if(i)std::cout<<',';std::cout<<observe(inputs[i]);}
        std::cout<<"]}\n";
        return 0;
    }catch(const opts::Exception& error){
        std::cerr<<"XML native candidate "<<error.name<<": "<<encode(error.message)<<'\n';return 2;
    }catch(const std::exception& error){
        std::cerr<<"XML native candidate NativeException: "<<error.what()<<'\n';return 2;
    }catch(...){std::cerr<<"XML native candidate unclassified exception\n";return 2;}
}
