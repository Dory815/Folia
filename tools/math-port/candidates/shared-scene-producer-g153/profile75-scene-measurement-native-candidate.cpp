import std;
import folia.math.output.svg.native_document;
import folia.math.output.svg.serializer;
import folia.math.util.property_host;

namespace {
namespace opts=folia::math::options;
namespace svg=folia::math::output::svg;
namespace scene=folia::math::output::scene;
namespace common=folia::math::output::common;

constexpr std::array<std::u16string_view,38> packages={
    u"base",u"action",u"ams",u"amscd",u"bbm",u"bboldx",u"bbox",u"begingroup",
    u"boldsymbol",u"braket",u"bussproofs",u"cancel",u"cases",u"centernot",
    u"color",u"colortbl",u"configmacros",u"dsfont",u"empheq",u"enclose",
    u"extpfeil",u"fontsizev3",u"gensymb",u"html",u"mathtools",u"mhchem",
    u"newcommand",u"noundefined",u"physics",u"setoptions",u"tagformat",u"texhtml",
    u"textcomp",u"textmacros",u"unicode",u"units",u"upgreek",u"verb"};

std::string quote(std::string_view text){
    std::string out="\"";constexpr char hex[]="0123456789abcdef";
    for(std::size_t i=0;i<text.size();){
        const auto c=static_cast<unsigned char>(text[i]);
        if(c=='\"'){out+="\\\"";++i;}
        else if(c=='\\'){out+="\\\\";++i;}
        else if(c=='\n'){out+="\\n";++i;}
        else if(c=='\r'){out+="\\r";++i;}
        else if(c=='\t'){out+="\\t";++i;}
        else if(c<0x20){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];++i;}
        else{out+=static_cast<char>(c);++i;}
    }
    return out+'"';
}
std::string decode64(std::string_view text){
    constexpr std::string_view digits="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;unsigned bits=0,buffer=0;
    for(const char c:text){if(c=='=')break;const auto digit=digits.find(c);
        if(digit==std::string_view::npos)throw std::runtime_error("Invalid base64 case input");
        buffer=(buffer<<6)|static_cast<unsigned>(digit);bits+=6;
        if(bits>=8){bits-=8;out.push_back(static_cast<char>((buffer>>bits)&255));}}
    return out;
}
std::vector<std::string_view> split(std::string_view row){
    std::vector<std::string_view> result;std::size_t start=0;
    while(true){const auto end=row.find('\t',start);result.push_back(row.substr(start,
        end==std::string_view::npos?row.size()-start:end-start));
        if(end==std::string_view::npos)break;start=end+1;}
    return result;
}
std::size_t sceneCount(const scene::FrozenNodeRef& node){
    if(!node)return 0;std::size_t result=1;
    for(const auto& child:node->children)result+=sceneCount(child);return result;
}
bool mmlError(std::string_view svgText){
    return svgText.find("data-mjx-error=")!=std::string_view::npos||
           svgText.find("data-mml-node=\"merror\"")!=std::string_view::npos;
}
bool sameDouble(double left,double right){
    return left==right||std::bit_cast<std::uint64_t>(left)==std::bit_cast<std::uint64_t>(right);
}
bool sameOutline(const std::shared_ptr<const scene::GlyphOutline>& left,
                 const std::shared_ptr<const scene::GlyphOutline>& right){
    if(!left||!right)return !left&&!right;
    if(left->sourcePath!=right->sourcePath||
       !sameDouble(left->coordinateUnitsPerEm,right->coordinateUnitsPerEm)||
       left->commands.size()!=right->commands.size())return false;
    for(std::size_t i=0;i<left->commands.size();++i){
        const auto& a=left->commands[i];const auto& b=right->commands[i];
        if(a.operation!=b.operation||a.arguments.size()!=b.arguments.size())return false;
        for(std::size_t j=0;j<a.arguments.size();++j)
            if(!sameDouble(a.arguments[j],b.arguments[j]))return false;
    }
    return true;
}
bool sameTransform(const scene::Transform& a,const scene::Transform& b){
    if(!sameDouble(a.translateX,b.translateX)||!sameDouble(a.translateY,b.translateY)||
       !sameDouble(a.scaleX,b.scaleX)||!sameDouble(a.scaleY,b.scaleY)||
       a.operations.size()!=b.operations.size())return false;
    for(std::size_t i=0;i<a.operations.size();++i){
        const auto& x=a.operations[i];const auto& y=b.operations[i];
        if(x.kind!=y.kind||!sameDouble(x.x,y.x)||!sameDouble(x.y,y.y)||
           !sameDouble(x.degrees,y.degrees)||!sameDouble(x.centerX,y.centerX)||
           !sameDouble(x.centerY,y.centerY)||!sameDouble(x.a,y.a)||
           !sameDouble(x.b,y.b)||!sameDouble(x.c,y.c)||!sameDouble(x.d,y.d)||
           !sameDouble(x.e,y.e)||!sameDouble(x.f,y.f)||x.scalarScale!=y.scalarScale||
           x.spaceSeparated!=y.spaceSeparated||x.scaleDigits!=y.scaleDigits||
           x.commaSpaceSeparated!=y.commaSpaceSeparated)return false;
    }
    return true;
}
bool sameGeometry(const scene::Geometry& a,const scene::Geometry& b){
    if(a.index()!=b.index())return false;
    return std::visit([&](const auto& x)->bool{
        using T=std::decay_t<decltype(x)>;
        const auto* y=std::get_if<T>(&b);if(!y)return false;
        if constexpr(std::is_same_v<T,std::monostate>)return true;
        else if constexpr(std::is_same_v<T,scene::RectGeometry>){
            return sameDouble(x.x,y->x)&&sameDouble(x.y,y->y)&&sameDouble(x.width,y->width)&&
                sameDouble(x.height,y->height)&&x.fields==y->fields&&x.fieldCount==y->fieldCount&&
                x.rx.has_value()==y->rx.has_value()&&(!x.rx||sameDouble(*x.rx,*y->rx))&&
                x.ry.has_value()==y->ry.has_value()&&(!x.ry||sameDouble(*x.ry,*y->ry));
        }else if constexpr(std::is_same_v<T,scene::ViewportGeometry>){
            return sameDouble(x.x,y->x)&&sameDouble(x.y,y->y)&&sameDouble(x.width,y->width)&&
                sameDouble(x.height,y->height)&&sameDouble(x.viewX,y->viewX)&&
                sameDouble(x.viewY,y->viewY)&&sameDouble(x.viewWidth,y->viewWidth)&&
                sameDouble(x.viewHeight,y->viewHeight)&&x.hasViewBox==y->hasViewBox&&
                x.emitRect==y->emitRect&&x.alignment==y->alignment&&x.fit==y->fit&&
                x.clipToViewport==y->clipToViewport&&x.widthRelative==y->widthRelative&&
                x.heightRelative==y->heightRelative&&x.emitFit==y->emitFit;
        }else if constexpr(std::is_same_v<T,scene::LineGeometry>){
            return sameDouble(x.from.x,y->from.x)&&sameDouble(x.from.y,y->from.y)&&
                sameDouble(x.to.x,y->to.x)&&sameDouble(x.to.y,y->to.y)&&
                sameDouble(x.width,y->width)&&x.emitStrokeWidth==y->emitStrokeWidth;
        }else if constexpr(std::is_same_v<T,scene::PolygonGeometry>){
            if(x.points.size()!=y->points.size())return false;
            for(std::size_t i=0;i<x.points.size();++i)
                if(!sameDouble(x.points[i].x,y->points[i].x)||
                   !sameDouble(x.points[i].y,y->points[i].y))return false;
            return true;
        }else if constexpr(std::is_same_v<T,scene::PathGeometry>){
            return sameOutline(x.outline,y->outline);
        }else if constexpr(std::is_same_v<T,scene::UseGeometry>){
            return x.href==y->href&&sameDouble(x.x,y->x)&&sameDouble(x.y,y->y)&&x.xlink==y->xlink;
        }else if constexpr(std::is_same_v<T,scene::ForeignObjectGeometry>){
            return sameDouble(x.x,y->x)&&sameDouble(x.y,y->y)&&sameDouble(x.width,y->width)&&
                sameDouble(x.height,y->height)&&sameTransform(x.transform,y->transform);
        }else if constexpr(std::is_same_v<T,scene::EllipseGeometry>){
            return sameDouble(x.cx,y->cx)&&sameDouble(x.cy,y->cy)&&
                sameDouble(x.rx,y->rx)&&sameDouble(x.ry,y->ry);
        }
        return false;
    },a);
}
bool sameHtml(const scene::HtmlNodePrimitive& a,const scene::HtmlNodePrimitive& b){
    if(a.kind!=b.kind||a.namespaceHint!=b.namespaceHint||a.attributes!=b.attributes||
       a.text!=b.text||a.children.size()!=b.children.size())return false;
    for(std::size_t i=0;i<a.children.size();++i)if(!sameHtml(a.children[i],b.children[i]))return false;
    return true;
}
bool sameSemantic(const scene::Semantic& a,const scene::Semantic& b){
    if(a.index()!=b.index())return false;
    return std::visit([&](const auto& x)->bool{
        using T=std::decay_t<decltype(x)>;const auto* y=std::get_if<T>(&b);if(!y)return false;
        if constexpr(std::is_same_v<T,std::monostate>||std::is_same_v<T,scene::SourceDomPrimitive>)return true;
        else if constexpr(std::is_same_v<T,scene::GlyphPrimitive>)
            return x.codepoint==y->codepoint&&x.variant==y->variant&&x.font==y->font&&
                sameOutline(x.outline,y->outline);
        else if constexpr(std::is_same_v<T,scene::TextRunPrimitive>)
            return x.text==y->text&&x.variant==y->variant&&x.fontFamily==y->fontFamily&&
                x.italic==y->italic&&x.bold==y->bold&&x.fontSize.has_value()==y->fontSize.has_value()&&
                (!x.fontSize||sameDouble(*x.fontSize,*y->fontSize));
        else if constexpr(std::is_same_v<T,scene::CommentRunPrimitive>)return x.text==y->text;
        else if constexpr(std::is_same_v<T,scene::SourceTextPrimitive>)
            return x.text==y->text&&x.comment==y->comment&&x.currentText()==y->currentText();
        else if constexpr(std::is_same_v<T,scene::ClipPrimitive>)return x.id==y->id;
        else if constexpr(std::is_same_v<T,scene::LinkPrimitive>||std::is_same_v<T,scene::ImagePrimitive>)
            return x.href==y->href;
        else if constexpr(std::is_same_v<T,scene::HtmlNodePrimitive>)return sameHtml(x,*y);
        else if constexpr(std::is_same_v<T,scene::InlineBreakPrimitive>)
            return sameDouble(x.space,y->space)&&x.forced==y->forced&&x.prebreak==y->prebreak;
        return false;
    },a);
}
bool samePaint(const std::optional<scene::ShapePaint>& a,const std::optional<scene::ShapePaint>& b){
    if(a.has_value()!=b.has_value())return false;if(!a)return true;
    const auto& x=*a;const auto& y=*b;
    if(x.fill!=y.fill||x.stroke!=y.stroke||x.lineCap!=y.lineCap||x.lineJoin!=y.lineJoin||
       x.emitAttributes!=y.emitAttributes||x.strokeWidth.has_value()!=y.strokeWidth.has_value()||
       (x.strokeWidth&&!sameDouble(*x.strokeWidth,*y.strokeWidth))||x.dash.has_value()!=y.dash.has_value())return false;
    if(!x.dash)return true;
    if(x.dash->lengths.size()!=y.dash->lengths.size()||
       !sameDouble(x.dash->offset,y.dash->offset)||x.dash->separator!=y.dash->separator)return false;
    for(std::size_t i=0;i<x.dash->lengths.size();++i)
        if(!sameDouble(x.dash->lengths[i],y.dash->lengths[i]))return false;
    return true;
}
bool sameNode(const scene::FrozenNodeRef& a,const scene::FrozenNodeRef& b){
    if(!a||!b)return !a&&!b;
    if(a->kind!=b->kind||a->attributes!=b->attributes||!sameGeometry(a->geometry,b->geometry)||
       !sameSemantic(a->semantic,b->semantic)||!samePaint(a->paint,b->paint)||
       a->transform.has_value()!=b->transform.has_value()||
       (a->transform&&!sameTransform(*a->transform,*b->transform))||
       a->transformAttributeIndex!=b->transformAttributeIndex||a->transformSuffix!=b->transformSuffix||
       a->children.size()!=b->children.size())return false;
    for(std::size_t i=0;i<a->children.size();++i)if(!sameNode(a->children[i],b->children[i]))return false;
    return true;
}
bool sameSceneData(const scene::Scene& a,const scene::Scene& b){
    if(a.roots().size()!=b.roots().size()||a.resources().size()!=b.resources().size())return false;
    for(std::size_t i=0;i<a.roots().size();++i)if(!sameNode(a.roots()[i],b.roots()[i]))return false;
    for(std::size_t i=0;i<a.resources().size();++i)
        if(!sameOutline(a.resources()[i],b.resources()[i]))return false;
    return true;
}
std::string errorJson(std::string_view name,std::string_view message){
    return "{\"name\":"+quote(name)+",\"message\":"+quote(message)+"}";
}
std::string currentError(){
    try{throw;}
    catch(const opts::Exception& error){
        return errorJson(error.name,folia::math::property_host::debugText(error.message));
    }catch(const std::exception& error){return errorJson("Error",error.what());}
    catch(...){return errorJson("Error","Unknown native exception");}
}
struct Row{std::size_t index=0;std::string name,tex;bool display=false;double em=16,width=1280;};
Row parseCase(std::string_view line){
    const auto fields=split(line);if(fields.size()!=6)throw std::runtime_error("Expected six case fields");
    return {static_cast<std::size_t>(std::stoull(std::string(fields[0]))),decode64(fields[1]),
        decode64(fields[2]),fields[3]=="1",std::stod(std::string(fields[4])),
        std::stod(std::string(fields[5]))};
}

class ProfileSession final{
    svg::NativeRenderSession session_;
    std::shared_ptr<folia::math::handlers::html::HTMLMathDocument> document_;
    std::shared_ptr<svg::SvgOutputJax> jax_;
    std::shared_ptr<folia::math::adaptors::lite::LiteAdaptor> adaptor_;
public:
    std::size_t materializeCalls=0,serializeXmlCalls=0;
    ProfileSession()
        :session_([]{common::FontLoadHost host;host.asyncIsSynchronous=[] {return true;};
            host.warn=[](const std::any&){};host.consoleWarn=[](const std::vector<std::any>&){};
            return host;}(),{svg::NewcmFontExtension::bboldx,svg::NewcmFontExtension::bbm,
                            svg::NewcmFontExtension::dsfont}){
        auto& context=session_.context();const auto outputOptions=context.object();
        context.set(outputOptions,opts::Key(u"fontCache"),opts::Value(u"none"));
        context.set(outputOptions,opts::Key(u"exFactor"),opts::Value(.5));
        context.set(outputOptions,opts::Key(u"displayOverflow"),opts::Value(u"linebreak"));
        const auto linebreaks=context.object();context.set(linebreaks,opts::Key(u"inline"),opts::Value(true));
        context.set(outputOptions,opts::Key(u"linebreaks"),opts::Value(linebreaks));
        jax_=session_.construct(opts::Value(outputOptions));
        const auto inputOptions=context.object();const auto selected=context.array(
            static_cast<std::uint32_t>(packages.size()));
        for(std::size_t index=0;index<packages.size();++index){
            const auto key=std::to_string(index);
            context.set(selected,opts::Key(std::u16string(key.begin(),key.end())),
                        opts::Value(std::u16string(packages[index])));
        }
        context.set(inputOptions,opts::Key(u"packages"),opts::Value(selected));
        document_=svg::createNativeSvgDocument(session_,opts::Undefined{},opts::Value(inputOptions));
        adaptor_=session_.adaptor();
        adaptor_->setSerializeXmlObserver([this]{++serializeXmlCalls;});
    }
    struct Output{
        std::shared_ptr<const scene::Scene> captured;
        const scene::Scene* completed=nullptr;
        folia::math::DOMNodeRef node{};
        std::size_t materialize=0,serializeXml=0;
        std::string svgText;
    };
    Output convert(const Row& row,bool sceneOnly){
        materializeCalls=0;serializeXmlCalls=0;
        auto previousObserver=svg::Serializer::exchangeMaterializationObserver(
            [this]{++materializeCalls;});
        struct RestoreObserver{
            svg::Serializer::MaterializationObserver previous;
            ~RestoreObserver(){(void)svg::Serializer::exchangeMaterializationObserver(std::move(previous));}
        } restoreObserver{std::move(previousObserver)};
        auto& context=session_.context();const auto settings=context.object();
        context.set(settings,opts::Key(u"display"),opts::Value(row.display));
        context.set(settings,opts::Key(u"em"),opts::Value(row.em));
        context.set(settings,opts::Key(u"ex"),opts::Value(row.em*.5));
        context.set(settings,opts::Key(u"containerWidth"),opts::Value(row.width));
        Output out;svg::SvgOutputJax::SceneOutputConsumer previous;
        if(sceneOnly){
            previous=jax_->exchangeSceneOutputConsumer([&out](
                std::shared_ptr<const scene::Scene> value,folia::math::DOMNodeRef){
                    out.captured=std::move(value);
                });
        }
        struct Restore{
            svg::SvgOutputJax* jax;svg::SvgOutputJax::SceneOutputConsumer previous;
            ~Restore(){if(jax)(void)jax->exchangeSceneOutputConsumer(std::move(previous));}
        } restore{sceneOnly?jax_.get():nullptr,std::move(previous)};
        const auto result=document_->convert(opts::Value(folia::math::property_host::text(row.tex)),opts::Value(settings));
        const auto returned=folia::math::propertyNode(document_->propertyBindings().from(result));
        const auto* node=std::get_if<folia::math::DOMNodeRef>(&returned);
        if(!node||!*node)throw opts::Exception("TypeError",u"Math document returned no DOM node");
        out.node=*node;
        const auto* completed=jax_->completedScene();
        if(!completed)throw opts::Exception("TypeError",u"Math conversion completed without a Scene");
        out.completed=completed;
        if(!sceneOnly)out.svgText=folia::math::property_host::debugText(
            document_->domAdaptor().serializeXML(out.node));
        out.materialize=materializeCalls;out.serializeXml=serializeXmlCalls;
        return out;
    }
    bool sameScene(const scene::Scene& left,const scene::Scene& right){
        return sameSceneData(left,right);
    }
};

std::string record(const Row& row,std::string_view group,ProfileSession& sceneSession,
                   ProfileSession& svgSession){
    bool equal=false,hasMmlError=false;std::size_t nodes=0;
    std::string sceneError="null",svgError="null",svgText="null";
    ProfileSession::Output sceneOutput,svgOutput;
    try{sceneOutput=sceneSession.convert(row,true);}catch(...){sceneError=currentError();}
    const auto noMaterialize=sceneSession.materializeCalls;
    const auto noSerializeXml=sceneSession.serializeXmlCalls;
    try{svgOutput=svgSession.convert(row,false);}catch(...){svgError=currentError();}
    const auto svgMaterialize=svgSession.materializeCalls;
    const auto svgSerializeXml=svgSession.serializeXmlCalls;
    if(sceneError=="null"&&svgError=="null"){
        if(!sceneOutput.captured||!sceneOutput.completed)
            sceneError=errorJson("Error","Scene-only route did not retain its produced Scene");
        else if(sceneOutput.completed!=sceneOutput.captured.get())
            sceneError=errorJson("Error","Scene-only route captured a different Scene owner");
        else if(!svgOutput.completed)
            svgError=errorJson("Error","SVG route did not retain its produced Scene");
        else equal=sceneSession.sameScene(*sceneOutput.captured,*svgOutput.completed);
    }
    if(svgError=="null"){
        hasMmlError=mmlError(svgOutput.svgText);
        svgText=quote(svgOutput.svgText);
    }
    if(sceneOutput.captured)
        for(const auto& root:sceneOutput.captured->roots())nodes+=sceneCount(root);
    const auto error=(sceneError=="null"&&svgError=="null")?"null":
        "{\"scene\":"+sceneError+",\"svg\":"+svgError+"}";
    return "{\"group\":"+quote(group)+",\"index\":"+std::to_string(row.index)+
        ",\"name\":"+quote(row.name)+",\"tex\":"+quote(row.tex)+
        ",\"display\":"+(row.display?"true":"false")+
        ",\"em\":"+std::to_string(row.em)+",\"ex\":"+std::to_string(row.em*.5)+
        ",\"width\":"+std::to_string(row.width)+
        ",\"sceneEqual\":"+(equal?"true":"false")+
        ",\"sceneOnlyMaterializeCalls\":"+std::to_string(noMaterialize)+
        ",\"sceneOnlySerializeXmlCalls\":"+std::to_string(noSerializeXml)+
        ",\"svgMaterializeCalls\":"+std::to_string(svgMaterialize)+
        ",\"svgSerializeXmlCalls\":"+std::to_string(svgSerializeXml)+
        ",\"sceneNodes\":"+std::to_string(nodes)+",\"svg\":"+svgText+
        ",\"mmlError\":"+(hasMmlError?"true":"false")+
        ",\"error\":"+error+"}";
}
}

int main(){
    std::vector<std::string> records;std::unique_ptr<ProfileSession> continuousScene,continuousSvg;
    std::string group;
    try{
        for(std::string line;std::getline(std::cin,line);){if(line.empty())continue;
            if(line.starts_with("@group\t")){group=decode64(std::string_view(line).substr(7));
                if(group=="continuous"){continuousScene=std::make_unique<ProfileSession>();
                    continuousSvg=std::make_unique<ProfileSession>();}
                else if(group=="fresh-session-per-case"){continuousScene.reset();continuousSvg.reset();}
                else throw std::runtime_error("Unknown render group: "+group);continue;}
            if(group.empty())throw std::runtime_error("Missing @group header");
            const auto row=parseCase(line);
            if(group=="continuous")records.push_back(record(row,group,*continuousScene,*continuousSvg));
            else {ProfileSession sceneSession;ProfileSession svgSession;
                records.push_back(record(row,group,sceneSession,svgSession));}
        }
        std::cout<<"{\"schema\":1,\"renderer\":\"NativeRenderSession/CommonWrapper/BBox/Scene producer\","
            "\"groups\":[\"continuous\",\"fresh-session-per-case\"],\"records\":[";
        for(std::size_t i=0;i<records.size();++i){if(i)std::cout<<',';std::cout<<records[i];}
        std::cout<<"]}\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
