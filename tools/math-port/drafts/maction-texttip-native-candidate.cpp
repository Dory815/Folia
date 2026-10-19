// Focused FoliaMathTextTipDump witness paired with the pinned Source profile.
// Run through the coherent shared build and raw-output capture tools.
import std;
import folia.math.output.svg.native_document;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
namespace opts=folia::math::options;
namespace svg=folia::math::output::svg;
namespace common=folia::math::output::common;
namespace scene=folia::math::output::scene;
constexpr std::array<std::u16string_view,38> packages={
    u"base",u"action",u"ams",u"amscd",u"bbm",u"bboldx",u"bbox",u"begingroup",
    u"boldsymbol",u"braket",u"bussproofs",u"cancel",u"cases",u"centernot",
    u"color",u"colortbl",u"configmacros",u"dsfont",u"empheq",u"enclose",
    u"extpfeil",u"fontsizev3",u"gensymb",u"html",u"mathtools",u"mhchem",
    u"newcommand",u"noundefined",u"physics",u"setoptions",u"tagformat",u"texhtml",
    u"textcomp",u"textmacros",u"unicode",u"units",u"upgreek",u"verb"};

common::FontLoadHost staticHost(){
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=[](const std::any&){
        throw opts::Exception("UnexpectedWarning",u"Unexpected font warning in text-tip witness");
    };
    host.syncLoad=[](common::FontData&,const common::DynamicFileRef&,std::string){
        throw opts::Exception("Unimplemented",u"Text-tip witness must not load a dynamic font");
    };
    host.asyncLoad=[](std::shared_ptr<common::FontData>,common::DynamicFileRef,std::string)
        ->async::PromiseRef{
        throw opts::Exception("Unimplemented",u"Text-tip witness must not load a dynamic font");
    };
    return host;
}

std::optional<scene::FrozenNodeRef> findMaction(scene::FrozenNodeRef node){
    if(!node)return {};
    for(const auto& [key,value]:node->attributes)
        if(key=="data-mml-node"&&value=="maction")return node;
    for(const auto& child:node->children)
        if(auto found=findMaction(child))return found;
    return {};
}
}

int main(){
    try{
    svg::NativeRenderSession session(staticHost());
    auto& context=session.context();
    const auto jaxOptions=context.object();
    context.set(opts::Value(jaxOptions),opts::Key(u"fontCache"),opts::Value(u"none"));
    context.set(opts::Value(jaxOptions),opts::Key(u"exFactor"),opts::Value(0.5));
    context.set(opts::Value(jaxOptions),opts::Key(u"displayOverflow"),opts::Value(u"linebreak"));
    const auto linebreaks=context.object();
    context.set(linebreaks,opts::Key(u"inline"),opts::Value(true));
    context.set(jaxOptions,opts::Key(u"linebreaks"),opts::Value(linebreaks));
    auto jax=session.construct(opts::Value(jaxOptions));
    bool checkedMactionWrapper=false;
    jax->setLayoutObserver([&](const common::CommonWrapper& root,const common::LayoutJob&){
        const_cast<common::CommonWrapper&>(root).walkTree(
            [&](common::CommonWrapper* wrapper,const std::any&){
                if(wrapper->kind()!="maction")return;
                if(!wrapper->bbox||!std::isfinite(wrapper->bbox->scale))
                    throw opts::Exception("TextTipMismatch",u"maction parent bbox/scale was not initialized");
                if(!wrapper->childNodes||!*wrapper->childNodes||(*wrapper->childNodes)->empty())
                    throw opts::Exception("TextTipMismatch",u"maction children were not initialized");
                const auto selected=(*wrapper->childNodes)->get(0);
                if(!selected.isNode()||selected.nodeOrNull()->parent!=wrapper)
                    throw opts::Exception("TextTipMismatch",u"maction selected child lost its parent wrapper");
                const auto identity=wrapper->factory().objectFor(*wrapper);
                const auto factoryIdentity=context.get(opts::Value(identity),opts::Key(u"factory"));
                if(factoryIdentity!=opts::Value(context.get(opts::Value(jax->instanceIdentity()),
                    opts::Key(u"factory"))))
                    throw opts::Exception("TextTipMismatch",u"maction factory is not the canonical factory");
                const auto childrenA=context.get(opts::Value(identity),opts::Key(u"childNodes"));
                const auto childrenB=context.get(opts::Value(identity),opts::Key(u"childNodes"));
                if(childrenA!=childrenB||
                   context.get(childrenA,opts::Key(u"0"))!=
                       opts::Value(selected.nodeOrNull()->factory().objectFor(*selected.nodeOrNull())))
                    throw opts::Exception("TextTipMismatch",u"maction child array lost its live owner identity");
                checkedMactionWrapper=true;
            });
    });
    const auto inputOptions=context.object();
    const auto selected=context.array(static_cast<std::uint32_t>(packages.size()));
    for(std::size_t index=0;index<packages.size();++index){
        const auto key=std::to_string(index);
        context.set(opts::Value(selected),opts::Key(std::u16string(key.begin(),key.end())),
            opts::Value(std::u16string(packages[index])));
    }
    context.set(opts::Value(inputOptions),opts::Key(u"packages"),opts::Value(selected));
    auto document=svg::createNativeSvgDocument(session,opts::Undefined{},opts::Value(inputOptions));
    auto& cx=document->sourceContext();
    const auto settings=cx.object();
    cx.set(opts::Value(settings),opts::Key(u"display"),opts::Value(false));
    cx.set(opts::Value(settings),opts::Key(u"em"),opts::Value(16.0));
    cx.set(opts::Value(settings),opts::Key(u"ex"),opts::Value(8.0));
    cx.set(opts::Value(settings),opts::Key(u"containerWidth"),opts::Value(1280.0));
    std::cerr<<"texttip: starting Source document conversion\n";
    const auto result=document->convert(opts::Value(u"\\texttip{x}{tip}"),opts::Value(settings));
    if(!checkedMactionWrapper)
        throw opts::Exception("TextTipMismatch",u"Text-tip layout did not visit a maction wrapper");
    std::cerr<<"texttip: document conversion returned\n";
    const auto dom=propertyNode(document->propertyBindings().from(result));
    const auto* node=std::get_if<DOMNodeRef>(&dom);
    if(!node||!*node)throw opts::Exception("TypeError",u"Text-tip conversion returned no DOM node");
    const auto xml=document->domAdaptor().serializeXML(*node);
    std::cerr<<"texttip svg: "<<property_host::debugText(xml)<<'\n';
    const auto title=xml.find(u"<title>tip</title>");
    const auto rect=xml.find(u"<rect");
    if(title==std::u16string::npos||rect==std::u16string::npos||title>=rect)
        throw opts::Exception("TextTipMismatch",u"SVG title must precede the existing hitbox rect");
    const auto* completed=jax->completedScene();
    if(!completed)throw opts::Exception("TypeError",u"Text-tip conversion did not complete a Scene");
    std::optional<scene::FrozenNodeRef> maction;
    for(const auto& root:completed->roots())if((maction=findMaction(root)))break;
    if(!maction||(*maction)->children.size()<2||(*maction)->children[0]->kind!=u"title"||
       (*maction)->children[1]->kind!=u"rect")
        throw opts::Exception("TextTipMismatch",u"Scene must retain title-before-rect parent/child order");
    if(!std::holds_alternative<scene::RectGeometry>((*maction)->children[1]->geometry))
        throw opts::Exception("TextTipMismatch",u"Existing hitbox rectangle lost its typed geometry");
    std::cout<<"text-tip source path retained: title before hitbox; geometry remains typed\n";
    return 0;
    }catch(const opts::Exception& error){
        std::cerr<<error.name<<": "<<property_host::debugText(error.message)<<'\n';
        return 1;
    }catch(const std::exception& error){
        std::cerr<<error.what()<<'\n';
        return 1;
    }
}
