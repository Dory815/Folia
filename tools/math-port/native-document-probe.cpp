// Production document-path diagnostic. It uses the actual session, HTML
// document, TeX Jax, Common wrappers and completed Scene. This smoke probe
// supplies no substitute MathItem/MML/layout and is not the 75-case acceptance.
import std;
import folia.math.output.svg.native_document;
import folia.math.output.common;
import folia.math.core.mml_tree.mml_node;
import folia.math.util.property_host;
import folia.math.util.utf16;

namespace {
using namespace folia::math;
namespace opts=folia::math::options;
namespace svg=folia::math::output::svg;
namespace common=folia::math::output::common;
namespace scene=folia::math::output::scene;
namespace mml=folia::math::mml;

std::string quote(std::string_view value) {
    std::string out="\"";
    constexpr char hex[]="0123456789abcdef";
    for(const unsigned char c:value) {
        switch(c) {
        case '\\':out+="\\\\";break;
        case '"':out+="\\\"";break;
        case '\n':out+="\\n";break;
        case '\r':out+="\\r";break;
        case '\t':out+="\\t";break;
        default:
            if(c<0x20) {out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}
            else out+=static_cast<char>(c);
        }
    }
    return out+'"';
}
const char* boolean(bool value) {return value?"true":"false";}

struct Trace {
    std::vector<std::string> requests;
    std::size_t observedJobs=0,wrappers=0,sceneRoots=0;
    std::string rootKind;
};
common::FontLoadHost staticHost(const std::shared_ptr<Trace>& trace) {
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return false;};
    host.syncLoad=[trace](common::FontData&,const common::DynamicFileRef&,std::string name) {
        trace->requests.push_back(std::move(name));
        throw opts::Exception("Unimplemented",u"Unexpected dynamic font load in base document probe");
    };
    host.asyncLoad=[trace](std::shared_ptr<common::FontData>,common::DynamicFileRef,std::string name)->async::PromiseRef {
        trace->requests.push_back(std::move(name));
        throw opts::Exception("Unimplemented",u"Unexpected async font load in base document probe");
    };
    host.warn=[](const std::any&) {
        throw opts::Exception("Unimplemented",u"Unexpected font warning in base document probe");
    };
    return host;
}

// Optional actual-layout observation mode. Keep all helpers and short names
// isolated so the existing lifetime diagnostic above remains unchanged.
namespace layout_observation {
constexpr std::array<std::u16string_view,38> packages={
    u"base",u"action",u"ams",u"amscd",u"bbm",u"bboldx",u"bbox",u"begingroup",
    u"boldsymbol",u"braket",u"bussproofs",u"cancel",u"cases",u"centernot",
    u"color",u"colortbl",u"configmacros",u"dsfont",u"empheq",u"enclose",
    u"extpfeil",u"fontsizev3",u"gensymb",u"html",u"mathtools",u"mhchem",
    u"newcommand",u"noundefined",u"physics",u"setoptions",u"tagformat",u"texhtml",
    u"textcomp",u"textmacros",u"unicode",u"units",u"upgreek",u"verb"};

struct Snapshot {std::string json,mmlJson;std::size_t wrappers=0;std::string rootKind;};
std::string quote(std::string_view value){
    std::string out="\"";constexpr char hex[]="0123456789abcdef";
    for(const unsigned char c:value)switch(c){case '\\':out+="\\\\";break;case '"':out+="\\\"";break;case '\n':out+="\\n";break;case '\r':out+="\\r";break;case '\t':out+="\\t";break;default:if(c<0x20){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}else out+=static_cast<char>(c);}
    return out+'"';
}
std::string number(double value){if(!std::isfinite(value))return "null";char buffer[64];const auto [end,error]=std::to_chars(buffer,buffer+sizeof buffer,value,std::chars_format::general,17);return error==std::errc{}?std::string(buffer,end):"null";}
std::string property(const Property& value){return quote(property_host::debugText(property_host::string(value)));}
std::string readFile(const std::filesystem::path& path){std::ifstream input(path,std::ios::binary);if(!input)throw std::runtime_error("cannot open TeX input");return {std::istreambuf_iterator<char>(input),{}};}

// Diagnostic-only MML preorder. It reads the MML tree's canonical child
// owner independently of wrapper children; mtable rows have wrapper-specific
// exposure and must not be inferred from the wrapper tree.
std::string mmlTree(const mml::MmlNode& root){
    std::vector<const mml::MmlNode*> nodes;
    std::vector<std::size_t> parents;
    std::unordered_map<const mml::MmlNode*,std::size_t> ids;
    auto walk=[&](auto&& self,const mml::MmlNode* node,std::size_t parent)->void{
        if(!node||ids.contains(node))return;
        const auto id=nodes.size();ids.emplace(node,id);nodes.push_back(node);parents.push_back(parent);
        const auto children=node->childArrayOwner();
        if(!children)return;
        for(std::size_t i=0;i<children->size();++i){
            const auto child=children->get(i);
            if(child.isNode())self(self,child.nodeOrNull(),id);
        }
    };
    walk(walk,&root,std::numeric_limits<std::size_t>::max());
    std::string out="[";
    for(std::size_t i=0;i<nodes.size();++i){
        if(i)out+=',';
        const auto& node=*nodes[i];
        out+="{\"id\":"+std::to_string(i)+",\"parent\":"+
            (parents[i]==std::numeric_limits<std::size_t>::max()?"null":std::to_string(parents[i]))+
            ",\"kind\":"+quote(node.kind())+",\"arity\":"+number(node.arity())+
            ",\"isInferred\":"+std::string(boolean(node.isInferred()))+
            ",\"isEmbellished\":"+std::string(boolean(node.isEmbellished()))+
            ",\"childCount\":";
        const auto children=node.childArrayOwner();
        out+=std::to_string(children?children->size():0)+",\"children\":[";
        bool first=true;
        if(children)for(std::size_t c=0;c<children->size();++c){
            const auto child=children->get(c);if(!child.isNode())continue;
            const auto found=ids.find(child.nodeOrNull());if(found==ids.end())continue;
            if(!first)out+=',';
            first=false;out+=std::to_string(found->second);
        }
        out+="],\"attributes\":";
        if(const auto attrs=node.attributes()){
            const auto explicitAttrs=attrs->getAllAttributes();
            out+="{";bool firstAttr=true;
            for(const auto& key:explicitAttrs.keys()){
                if(!firstAttr)out+=',';
                firstAttr=false;out+=quote(key)+":"+property(explicitAttrs.get(key));
            }
            out+='}';
        }else out+="null";
        auto& mutableNode=const_cast<mml::MmlNode&>(node);
        out+=",\"texClass\":"+property(mutableNode.texClass())+
            ",\"prevClass\":"+property(mutableNode.prevClass())+
            ",\"prevLevel\":"+property(mutableNode.prevLevel())+
            ",\"hasSpacingAttributes\":"+std::string(boolean(mutableNode.hasSpacingAttributes()))+
            ",\"texSpacing\":"+property(mutableNode.texSpacing())+"}";
    }
    out+=']';return out;
}

void observe(const common::CommonWrapper& root,const common::LayoutJob&,
             Snapshot& snapshot){
    // One uniform preorder over wrapper-owned layout children. All pointers
    // remain local to this synchronous callback.
    std::vector<const common::CommonWrapper*> nodes;
    std::vector<std::size_t> parents;
    std::unordered_map<const common::CommonWrapper*,std::size_t> ids;
    auto walk=[&](auto&& self,const common::CommonWrapper* wrapper,
                  std::size_t parent)->void{
        if(!wrapper||ids.contains(wrapper))return;
        const auto id=nodes.size();ids.emplace(wrapper,id);nodes.push_back(wrapper);parents.push_back(parent);
        if(!wrapper->childNodes||!*wrapper->childNodes)return;
        const auto children=*wrapper->childNodes;
        for(std::size_t i=0;i<children->size();++i){const auto child=children->get(i);if(!child.isNode())continue;auto* next=dynamic_cast<const common::CommonWrapper*>(child.nodeOrNull());if(next)self(self,next,id);}
    };
    walk(walk,&root,std::numeric_limits<std::size_t>::max());
    snapshot.wrappers=nodes.size();snapshot.rootKind=root.node->kind();
    const auto bbox=[](const std::shared_ptr<BBox>& box){if(!box)return std::string("null");return std::string("{\"h\":")+number(box->h)+",\"d\":"+number(box->d)+",\"w\":"+number(box->w)+",\"L\":"+number(box->L)+",\"R\":"+number(box->R)+",\"scale\":"+number(box->scale)+",\"rscale\":"+number(box->rscale)+"}";};
    std::string output="[";
    for(std::size_t i=0;i<nodes.size();++i){
        if(i)output+=',';
        const auto& wrapper=*nodes[i];auto& mutableWrapper=const_cast<common::CommonWrapper&>(wrapper);
        const auto attrs=wrapper.node->attributes();
        output+="{\"id\":"+std::to_string(i)+",\"parent\":"+
            (parents[i]==std::numeric_limits<std::size_t>::max()?"null":std::to_string(parents[i]))+
            ",\"kind\":"+quote(wrapper.node->kind())+",\"attrs\":";
        if(!attrs)output+="null";
        else{output+="{";constexpr std::array<std::string_view,6> keys={"width","height","depth","lspace","voffset","style"};for(std::size_t k=0;k<keys.size();++k){if(k)output+=',';output+=quote(keys[k])+":"+property(attrs->get(keys[k]));}output+='}';}
        output+=",\"bbox\":"+bbox(mutableWrapper.getBBox())+",\"outerBBox\":"+bbox(mutableWrapper.getOuterBBox())+",\"children\":[";
        bool first=true;if(wrapper.childNodes&&*wrapper.childNodes){const auto children=*wrapper.childNodes;for(std::size_t c=0;c<children->size();++c){const auto child=children->get(c);if(!child.isNode())continue;auto* next=dynamic_cast<const common::CommonWrapper*>(child.nodeOrNull());if(!next)continue;const auto found=ids.find(next);if(found==ids.end())continue;if(!first)output+=',';first=false;output+=std::to_string(found->second);}}
        output+=']';
        // Source-state diagnostic for the spacing divergence. These are
        // read-only snapshots of the real MML node and its core operator.
        auto* mmlNode=wrapper.node;
        const Property explicitTexClass=mmlNode->getProperty("texClass");
        const auto coreValue=mmlNode->coreMO();
        auto* coreMo=coreValue.nodeOrNull();
        const auto coreAttributes=coreMo?coreMo->attributes():std::shared_ptr<Attributes>{};
        const auto parentValue=mmlNode->getParent();
        auto* parentNode=parentValue.nodeOrNull();
        const bool topEmbellished=mmlNode->isEmbellished()&&
            (!parentNode||!parentNode->isEmbellished());
        auto& jax=wrapper.jax();
        const bool mathmlSpacing=opts::truthy(jax.optionsContext().get(jax.options,opts::Key(u"mathmlSpacing")));
        output+=std::string(",\"mmlSpacingState\":{\"texClassPropertyMissing\":")+
            boolean(explicitTexClass.missing())+",\"texClassProperty\":"+property(explicitTexClass)+
            ",\"texClass\":"+property(mmlNode->texClass())+",\"prevClass\":"+property(mmlNode->prevClass())+
            ",\"prevLevel\":"+property(mmlNode->prevLevel())+",\"isEmbellished\":"+
            boolean(mmlNode->isEmbellished())+",\"hasSpacingAttributes\":"+
            boolean(mmlNode->hasSpacingAttributes())+",\"isTopEmbellished\":"+boolean(topEmbellished)+
            ",\"texSpacing\":"+property(mmlNode->texSpacing())+",\"mathmlSpacing\":"+
            boolean(mathmlSpacing)+",\"coreLspaceIsSet\":"+
            boolean(coreAttributes&&coreAttributes->isSet("lspace"))+
            ",\"coreRspaceIsSet\":"+boolean(coreAttributes&&coreAttributes->isSet("rspace"))+
            ",\"coreLspace\":"+(coreAttributes?property(coreAttributes->get("lspace")):"null")+
            ",\"coreRspace\":"+(coreAttributes?property(coreAttributes->get("rspace")):"null")+"}";
        if(auto* padded=dynamic_cast<const common::CommonMpadded*>(&wrapper)){const auto dimensions=const_cast<common::CommonMpadded*>(padded)->getDimens();output+=",\"mpaddedGetDimens\":[";for(std::size_t n=0;n<dimensions.size();++n){if(n)output+=',';output+=number(dimensions[n]);}output+=']';}
        if(auto* under=dynamic_cast<const common::CommonMunder*>(&wrapper)){auto* base=under->baseChild();auto* script=under->scriptChild();const auto baseBox=base->getOuterBBox(),underBox=script->getOuterBBox();const double delta=under->isLineBelow?0:under->getDelta(*script,true);const auto dw=under->getDeltaW({*baseBox,*underBox},{0,-delta});output+=",\"munderWidths\":{\"base\":"+number(baseBox->w)+",\"under\":"+number(underBox->w)+",\"deltaW\":[";for(std::size_t n=0;n<dw.size();++n){if(n)output+=',';output+=number(dw[n]);}output+="]}";}
        output+='}';
    }
    output+=']';snapshot.json=std::move(output);
    snapshot.mmlJson=mmlTree(*root.node);
}

common::FontLoadHost fontHost(){common::FontLoadHost host;host.asyncIsSynchronous=[] {return true;};host.syncLoad=[](common::FontData&,const common::DynamicFileRef&,std::string name){throw opts::Exception("Unimplemented",u"Unexpected dynamic font load: "+std::u16string(name.begin(),name.end()));};host.asyncLoad=[](std::shared_ptr<common::FontData>,common::DynamicFileRef,std::string name)->async::PromiseRef{throw opts::Exception("Unimplemented",u"Unexpected async font load: "+std::u16string(name.begin(),name.end()));};host.warn=[](const std::any&){};host.consoleWarn=[](const std::vector<std::any>&){};return host;}

int run(const std::filesystem::path& inputPath,const std::filesystem::path& outputPath,
        bool display){
    Snapshot snapshot;std::string svgText="null",error="null",phase="session";bool succeeded=false;
    try{
        const auto tex=utf16::decode(readFile(inputPath)).text;
        svg::NativeRenderSession session(fontHost(),{svg::NewcmFontExtension::bboldx,svg::NewcmFontExtension::bbm,svg::NewcmFontExtension::dsfont});
        auto& context=session.context();const auto outputOptions=context.object();context.set(outputOptions,opts::Key(u"fontCache"),opts::Value(u"none"));context.set(outputOptions,opts::Key(u"exFactor"),opts::Value(.5));context.set(outputOptions,opts::Key(u"displayOverflow"),opts::Value(u"linebreak"));const auto linebreaks=context.object();context.set(linebreaks,opts::Key(u"inline"),opts::Value(true));context.set(outputOptions,opts::Key(u"linebreaks"),opts::Value(linebreaks));(void)session.construct(opts::Value(outputOptions));
        const auto inputOptions=context.object();const auto selected=context.array(static_cast<std::uint32_t>(packages.size()));for(std::size_t i=0;i<packages.size();++i){const auto key=std::to_string(i);context.set(selected,opts::Key(std::u16string(key.begin(),key.end())),opts::Value(std::u16string(packages[i])));}context.set(inputOptions,opts::Key(u"packages"),opts::Value(selected));auto document=svg::createNativeSvgDocument(session,opts::Undefined{},opts::Value(inputOptions));auto jax=session.outputJax();jax->setLayoutObserver([&snapshot](const common::CommonWrapper& root,const common::LayoutJob& job){observe(root,job,snapshot);});
        phase="convert";auto& documentContext=document->sourceContext();const auto settings=documentContext.object();documentContext.set(settings,opts::Key(u"display"),opts::Value(display));documentContext.set(settings,opts::Key(u"em"),opts::Value(16));documentContext.set(settings,opts::Key(u"ex"),opts::Value(8));documentContext.set(settings,opts::Key(u"containerWidth"),opts::Value(1280));const auto converted=document->convert(opts::Value(tex),opts::Value(settings));const auto domValue=propertyNode(document->propertyBindings().from(converted));const auto* node=std::get_if<DOMNodeRef>(&domValue);if(!node||!*node)throw opts::Exception("TypeError",u"Actual document conversion did not return a DOM node");svgText=quote(property_host::debugText(document->domAdaptor().serializeXML(*node)));succeeded=true;
    }catch(const opts::Exception& caught){error="{\"name\":"+quote(caught.name)+",\"message\":"+quote(property_host::debugText(caught.message))+"}";}catch(const std::exception& caught){error="{\"name\":\"std::exception\",\"message\":"+quote(caught.what())+"}";}
    std::ofstream output(outputPath,std::ios::binary);if(!output)return 2;output<<"{\"phase\":"<<quote(phase)<<",\"error\":"<<error<<",\"settings\":{\"display\":"<<(display?"true":"false")<<",\"em\":16,\"ex\":8,\"containerWidth\":1280,\"packageCount\":"<<packages.size()<<"},\"svg\":"<<svgText<<",\"wrapperCount\":"<<snapshot.wrappers<<",\"rootKind\":"<<quote(snapshot.rootKind)<<",\"wrappers\":"<<(snapshot.json.empty()?"[]":snapshot.json)<<",\"mmlNodes\":"<<(snapshot.mmlJson.empty()?"[]":snapshot.mmlJson)<<"}\n";return succeeded?0:1;
}
}
}

int main(int argc,char** argv) {
    if((argc==4||argc==5)&&std::string_view(argv[1])=="--layout-observation") {
        bool display=false;
        if(argc==5){if(std::string_view(argv[4])=="1")display=true;else if(std::string_view(argv[4])!="0"){std::cerr<<"display must be 0 or 1\n";return 2;}}
        return layout_observation::run(argv[2],argv[3],display);
    }
    const std::filesystem::path output=argc>1?std::filesystem::path(argv[1])
        :std::filesystem::path("build/math-port/native-document-probe.json");
    auto trace=std::make_shared<Trace>();
    std::weak_ptr<Trace> weakTrace=trace;
    std::weak_ptr<opts::Context> weakContext;
    std::weak_ptr<svg::SvgRenderServiceGraph> weakGraph;
    std::optional<scene::Scene> completed;
    std::string phase="session",error="null",svgText="null";
    bool publicDocumentRetainedContext=false,publicDocumentRetainedGraph=false;
    bool conversionSucceeded=false;
    try {
        std::shared_ptr<handlers::html::HTMLMathDocument> document;
        {
            svg::NativeRenderSession session(staticHost(trace));
            weakContext=session.contextOwner();
            weakGraph=session.services().shared_from_this();
            auto& context=session.context();
            const auto options=context.object();
            context.set(options,opts::Key(u"fontCache"),opts::Value(u"none"));
            context.set(options,opts::Key(u"exFactor"),opts::Value(.5));
            context.set(options,opts::Key(u"displayOverflow"),opts::Value(u"linebreak"));
            const auto linebreaks=context.object();
            context.set(linebreaks,opts::Key(u"inline"),opts::Value(true));
            context.set(options,opts::Key(u"linebreaks"),opts::Value(linebreaks));
            const auto jax=session.construct(opts::Value(options));
            jax->setLayoutObserver([trace](const common::CommonWrapper& root,
                                          const common::LayoutJob& job) {
                ++trace->observedJobs;
                trace->wrappers=job.wrapperCount();
                trace->rootKind=root.node->kind();
            });
            phase="document";
            document=svg::createNativeSvgDocument(session);
        }
        publicDocumentRetainedContext=!weakContext.expired();
        publicDocumentRetainedGraph=!weakGraph.expired();
        phase="convert";
        auto& context=document->sourceContext();
        const auto settings=context.object();
        context.set(settings,opts::Key(u"display"),opts::Value(false));
        context.set(settings,opts::Key(u"em"),opts::Value(16));
        context.set(settings,opts::Key(u"ex"),opts::Value(8));
        context.set(settings,opts::Key(u"containerWidth"),opts::Value(1280));
        const auto result=document->convert(opts::Value(u"x+y"),opts::Value(settings));
        const auto domValue=propertyNode(document->propertyBindings().from(result));
        const auto* node=std::get_if<DOMNodeRef>(&domValue);
        if(!node||!*node)
            throw opts::Exception("TypeError",u"Actual document conversion did not return a DOM node");
        phase="serialize";
        svgText=quote(property_host::debugText(document->domAdaptor().serializeXML(*node)));
        const auto outputService=document->outputJaxReference();
        const auto* actualService=std::get_if<core::MathOutputJaxService*>(&outputService);
        const auto* jax=actualService?dynamic_cast<svg::SvgOutputJax*>(*actualService):nullptr;
        if(!jax)throw opts::Exception("TypeError",u"Document has no actual SVG output Jax");
        if(const auto* finished=jax->completedScene()) {
            completed.emplace(*finished);
            trace->sceneRoots=completed->roots().size();
        } else {
            throw opts::Exception("TypeError",u"Actual SVG Jax did not finish a Scene");
        }
        phase="release";
        document.reset();
        conversionSucceeded=true;
    } catch(const opts::Exception& caught) {
        error="{\"name\":"+quote(caught.name)+",\"message\":"+
            quote(property_host::debugText(caught.message))+"}";
    } catch(const std::exception& caught) {
        error="{\"name\":\"std::exception\",\"message\":"+quote(caught.what())+"}";
    }
    const bool contextExpired=weakContext.expired(),graphExpired=weakGraph.expired();
    std::string requests="[";
    for(std::size_t i=0;i<trace->requests.size();++i) {
        if(i)requests+=',';
        requests+=quote(trace->requests[i]);
    }
    requests+=']';
    const auto observedJobs=trace->observedJobs,wrappers=trace->wrappers,sceneRoots=trace->sceneRoots;
    const auto rootKind=trace->rootKind;
    trace.reset();
    std::ofstream stream(output,std::ios::binary);
    if(!stream)return 2;
    stream<<"{\"tex\":\"x+y\",\"phase\":"<<quote(phase)
        <<",\"error\":"<<error<<",\"svg\":"<<svgText
        <<",\"observedJobs\":"<<observedJobs<<",\"wrappers\":"<<wrappers
        <<",\"rootKind\":"<<quote(rootKind)<<",\"sceneRoots\":"<<sceneRoots
        <<",\"loads\":"<<requests
        <<",\"publicDocumentRetainedContext\":"<<boolean(publicDocumentRetainedContext)
        <<",\"publicDocumentRetainedGraph\":"<<boolean(publicDocumentRetainedGraph)
        <<",\"contextExpiredAfterRelease\":"<<boolean(contextExpired)
        <<",\"graphExpiredAfterRelease\":"<<boolean(graphExpired)
        <<",\"hostExpiredAfterRelease\":"<<boolean(weakTrace.expired())
        <<",\"sceneSurvivesRelease\":"<<boolean(completed&&completed->roots().size()==sceneRoots)
        <<"}\n";
    return conversionSucceeded&&contextExpired&&graphExpired&&weakTrace.expired()?0:1;
}
