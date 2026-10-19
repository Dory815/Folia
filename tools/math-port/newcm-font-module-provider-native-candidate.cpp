// Native observation of the same pinned NewCM dynamic module requests as
// newcm-font-module-provider-original-oracle.cjs. Root owns the executable.
import std;
import folia.math.fonts.newcm_font_data;
import folia.math.fonts.newcm_font_module_provider;
import folia.math.util.regexp_binding;
import folia.math.util.option_functions;
import folia.math.util.property_host;
import folia.math.util.context_property_bindings;
import folia.math.output.common.font_data_bindings;
import folia.math.output.common.font_data_options;
import folia.math.util.map_bindings;

namespace {
using namespace folia::math;
using namespace folia::math::options;
namespace common=folia::math::output::common;
namespace fonts=folia::math::fonts;

void quote(std::ostream& out,std::string_view value){
    out.put('"');
    for(const unsigned char c:value){
        switch(c){
        case '"':out<<"\\\"";break;
        case '\\':out<<"\\\\";break;
        case '\n':out<<"\\n";break;
        case '\r':out<<"\\r";break;
        case '\t':out<<"\\t";break;
        default:if(c<0x20)out<<"\\u00"<<std::hex<<std::setw(2)<<std::setfill('0')<<static_cast<unsigned>(c)<<std::dec;
                else out.put(static_cast<char>(c));
        }
    }
    out.put('"');
}
void number(std::ostream& out,double value){
    if(std::isnan(value)){out<<"{\"$\":\"NaN\"}";return;}
    if(std::isinf(value)){out<<(value<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}");return;}
    if(value==0&&std::signbit(value)){out<<"{\"$\":\"-0\"}";return;}
    out<<std::setprecision(std::numeric_limits<double>::max_digits10)<<value;
}
void property(std::ostream& out,const Property& value);
void array(std::ostream& out,const PropertyArray& value){
    out.put('[');
    for(std::size_t i=0;i<value.size();++i){
        if(i)out.put(',');
        if(!value.has(i))out<<"{\"$\":\"hole\"}";
        else property(out,value.get(i));
    }
    out.put(']');
}
void record(std::ostream& out,const PropertyList& value){
    auto keys=value.keys();std::sort(keys.begin(),keys.end());out.put('{');
    for(std::size_t i=0;i<keys.size();++i){
        if(i)out.put(',');quote(out,keys[i]);out.put(':');property(out,value.getOwn(keys[i]));
    }
    out.put('}');
}
void property(std::ostream& out,const Property& value){
    if(value.missing()){out<<"{\"$\":\"undefined\"}";return;}
    if(std::holds_alternative<PropertyNull>(value.value)){out<<"null";return;}
    if(const auto* v=std::get_if<bool>(&value.value)){out<<(*v?"true":"false");return;}
    if(const auto* v=std::get_if<double>(&value.value)){number(out,*v);return;}
    if(const auto* v=std::get_if<std::string>(&value.value)){quote(out,*v);return;}
    if(PropertyArray::array(value)){array(out,*PropertyArray::array(value));return;}
    if(const auto object=PropertyList::record(value)){record(out,*object);return;}
    throw std::runtime_error("unsupported Property in native dynamic glyph observation");
}
struct Event {std::string phase,request,setup;};
void events(std::ostream& out,const std::vector<Event>& values){
    out.put('[');
    for(std::size_t i=0;i<values.size();++i){
        if(i)out.put(',');const auto& value=values[i];
        out<<"{\"phase\":";quote(out,value.phase);out<<",\"request\":";quote(out,value.request);
        if(!value.setup.empty()){out<<",\"setup\":";quote(out,value.setup);}out.put('}');
    }
    out.put(']');
}
void statuses(std::ostream& out,const common::DynamicFileList& files,bool after){
    out.put('[');std::size_t i=0;
    for(const auto&[key,file]:files){
        if(i++)out.put(',');out<<"{\"key\":";quote(out,key);
        out<<",\"setup\":";quote(out,file&&file->setup?"function":"undefined");
        if(after)out<<",\"promise\":"<<(file&&file->promise?"true":"false")
                    <<",\"failed\":"<<(file&&file->failed?"true":"false");
        else out<<",\"promise\":"<<(file&&file->promise?"true":"false");
        out.put('}');
    }
    out.put(']');
}
void sample(std::ostream& out,common::FontData& font,std::string_view name,std::uint32_t cp){
    quote(out,name);out.put(':');
    auto value=font.getChar("normal",cp);
    if(value)property(out,*value);else out<<"{\"$\":\"undefined\"}";
}
options::ObjectRef registerNewcmSourceClass(options::Context& context,
    common::FontDataContextBindings& identities,ContextPropertyBindings& properties,
    options::MapContextBindings& maps,
    const std::shared_ptr<fonts::NewcmFontDataClass>& owner,
    const std::shared_ptr<common::FontLoadHost>& hostAnchor){
    auto constructor=identities.registerClass(u"MathJaxNewcmFont",owner,*hostAnchor,
        std::static_pointer_cast<void>(hostAnchor),
        [](common::FontDataClass& actual,common::FontLoadHost& host,
           options::Context& cx,options::Value supplied){
            return std::make_shared<fonts::NewcmFontData>(
                static_cast<fonts::NewcmFontDataClass&>(actual),host,cx,
                std::move(supplied));
        },{},[&properties](const std::shared_ptr<common::FontData>& font,
                          options::ObjectRef instance){
            font->bindSourceIdentity(properties,instance);
        });
    // The Source class identity must exist before its static data and dynamic
    // options fields are initialized, as in the production FontData surface.
    owner->bindSourceIdentity(properties,constructor,maps);
    owner->dynamicExtensions=options::Value(maps.create());
    context.bindData(constructor,options::Key(u"dynamicExtensions"),
        std::make_shared<options::NativeDataField>(std::static_pointer_cast<void>(owner),
            [owner]{return owner->dynamicExtensions;},
            [owner](options::Value value){owner->dynamicExtensions=std::move(value);}));
    owner->initializeSourceData();
    common::bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
        u"defaultVariants",owner->defaultVariants);
    common::bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
        u"defaultSizeVariants",owner->defaultSizeVariants);
    common::bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
        u"defaultStretchVariants",owner->defaultStretchVariants);
    return constructor;
}
struct AliasObservation {
    std::string first,before,afterFirst,afterSecond,module;
    bool changedAfterFirst=false,stableAfterSecond=false,resolvedSame=false;
    bool moduleIdentitySame=false,cached=false;
};
std::string setupType(const common::DynamicFileRef& dynamic){
    return dynamic&&dynamic->setup?"function":"undefined";
}
AliasObservation aliasObservation(std::string first){
    Context context;
    auto fontClass=std::make_shared<fonts::NewcmFontDataClass>(context);
    regexp_host::RegExpBindings regexp(context);
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    common::FontDataContextBindings identities(context);
    options::MapContextBindings maps(context);
    auto hostAnchor=std::make_shared<common::FontLoadHost>();
    hostAnchor->regexp=&regexp;
    auto classConstructor=registerNewcmSourceClass(context,identities,properties,maps,
        fontClass,hostAnchor);
    auto queue=std::make_shared<async::JobQueue>();
    auto provider=std::make_shared<fonts::NewcmFontModuleProvider>(fontClass,queue);
    hostAnchor->queue=queue;
    auto font=identities.resolve(context.construct(classConstructor));
    const auto found=fontClass->dynamicFiles.find("arrows");
    if(found==fontClass->dynamicFiles.end()||!found->second)
        throw std::runtime_error("NewCM arrows module is absent");
    const auto dynamic=found->second;
    const auto js="@mathjax/mathjax-newcm-font/js/svg/dynamic/arrows.js";
    const auto cjs="@mathjax/mathjax-newcm-font/cjs/svg/dynamic/arrows.js";
    const auto firstRequest=first=="js"?js:cjs;
    const auto secondRequest=first=="js"?cjs:js;
    const auto jsModule=provider->resolvedModuleName(js);
    const auto cjsModule=provider->resolvedModuleName(cjs);
    if(!jsModule||!cjsModule)throw std::runtime_error("NewCM alias did not resolve");
    const auto before=setupType(dynamic);
    const auto& placeholder=dynamic->setup.target_type();
    provider->loadSync(*font,dynamic,firstRequest);
    const auto afterFirst=setupType(dynamic);
    const auto& firstSetupType=dynamic->setup.target_type();
    std::size_t setupCalls=0;
    auto evaluated=dynamic->setup;
    dynamic->setup=[evaluated=std::move(evaluated),&setupCalls](common::FontData& actual){
        ++setupCalls;evaluated(actual);
    };
    const auto& wrapperType=dynamic->setup.target_type();
    provider->loadSync(*font,dynamic,secondRequest);
    const auto afterSecond=setupType(dynamic);
    const bool changedAfterFirst=firstSetupType!=placeholder;
    const bool stableAfterSecond=dynamic->setup.target_type()==wrapperType&&setupCalls==0;
    const bool moduleIdentitySame=provider->sameCachedModuleIdentity(js,cjs);
    return {std::move(first),before,afterFirst,afterSecond,*jsModule,
        changedAfterFirst,stableAfterSecond,*jsModule==*cjsModule,
        moduleIdentitySame,provider->loadedModuleCount()==1};
}
void aliases(std::ostream& out){
    out.put('[');
    bool separator=false;
    for(const auto& first:{std::string("js"),std::string("cjs")}){
        if(separator)out.put(',');
        separator=true;
        const auto item=aliasObservation(first);
        out<<"{\"first\":";quote(out,item.first);
        out<<",\"before\":";quote(out,item.before);
        out<<",\"afterFirst\":";quote(out,item.afterFirst);
        out<<",\"afterSecond\":";quote(out,item.afterSecond);
        out<<",\"changedAfterFirst\":"<<(item.changedAfterFirst?"true":"false");
        out<<",\"stableAfterSecond\":"<<(item.stableAfterSecond?"true":"false");
        out<<",\"resolvedSame\":"<<(item.resolvedSame?"true":"false");
        out<<",\"moduleIdentitySame\":"<<(item.moduleIdentitySame?"true":"false");
        out<<",\"canonicalModule\":";quote(out,item.module);
        out<<",\"cached\":"<<(item.cached?"true":"false")<<'}';
    }
    out.put(']');
}
}

int main(){
    std::string phase="construct NewCM module graph";
    try{
        Context context;
        auto fontClass=std::make_shared<fonts::NewcmFontDataClass>(context);
        regexp_host::RegExpBindings regexp(context);
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        common::FontDataContextBindings identities(context);
        options::MapContextBindings maps(context);
        auto hostAnchor=std::make_shared<common::FontLoadHost>();
        auto& host=*hostAnchor;
        host.regexp=&regexp;
        auto queue=std::make_shared<async::JobQueue>();
        host.queue=queue;
        host.asyncIsSynchronous=[] {return true;};
        host.warn=[](const std::any& reason){
            const auto* failure=std::any_cast<std::exception_ptr>(&reason);
            if(!failure){std::cerr<<"font load warning: "<<reason.type().name()<<'\n';return;}
            try{std::rethrow_exception(*failure);}
            catch(const options::Exception& error){
                std::cerr<<"font load warning: "<<error.name<<": "
                         <<property_host::debugText(error.message)<<'\n';
            }catch(const std::exception& error){
                std::cerr<<"font load warning: "<<error.what()<<'\n';
            }catch(...){std::cerr<<"font load warning: non-standard exception\n";}
        };
        phase="register canonical NewCM Source class";
        const auto classConstructor=registerNewcmSourceClass(context,identities,
            properties,maps,fontClass,hostAnchor);
        auto provider=std::make_shared<fonts::NewcmFontModuleProvider>(fontClass,queue);
        phase="wire NewCM module provider";
        std::vector<Event> trace;
        host.syncLoad=[provider,&trace](common::FontData& actualFont,
            const common::DynamicFileRef& dynamic,std::string request){
            trace.push_back({"request",request,{}});
            provider->loadSync(actualFont,dynamic,request);
            trace.push_back({"evaluated",request,dynamic&&dynamic->setup?"function":"undefined"});
        };
        phase="construct canonical NewCM Source instance";
        auto fontOwner=identities.resolve(context.construct(classConstructor));
        auto& font=*fontOwner;
        const auto options=std::get<ObjectRef>(fontClass->OPTIONS.data);
        const auto sourcePrefix=property_host::debugText(std::get<std::u16string>(
            context.get(Value(options),Key(u"dynamicPrefix")).data));

        std::ostringstream beforeOut,afterOut;
        statuses(beforeOut,fontClass->dynamicFiles,false);
        phase="load all dynamic font files synchronously";
        font.loadDynamicFilesSync();
        statuses(afterOut,fontClass->dynamicFiles,true);
        std::size_t glyphCount=0;
        for(const auto&[name,_]:fontClass->defaultChars){
            if(const auto variant=font.getVariant(name);variant&&variant->get().chars)
                glyphCount+=variant->get().chars->own.size();
        }
        std::cout<<"{\"source\":\"MathJax 4.1.3 / fb987178c2d279a99b0db5ea02e751691703955e\",\"sourcePrefix\":";
        quote(std::cout,sourcePrefix);std::cout<<",\"initialModuleCount\":"<<fontClass->dynamicFiles.size()<<",\"before\":";
        std::cout<<beforeOut.str()<<",\"requests\":";events(std::cout,trace);
        std::cout<<",\"after\":"<<afterOut.str()<<",\"loadedGlyphCount\":"<<glyphCount<<",\"samples\":{";
        sample(std::cout,font,"arrows",0x219C);std::cout<<',';
        sample(std::cout,font,"symbols",0x2200);std::cout<<',';
        sample(std::cout,font,"monospace",0x1D400);std::cout<<',';
        sample(std::cout,font,"sans-serif",0x1D5D4);std::cout<<"},\"aliasCache\":";
        phase="observe dynamic module alias cache";
        aliases(std::cout);std::cout<<"}\n";
        return 0;
    }catch(const Exception& error){
        std::cerr<<"newcm-font-module-provider candidate failed during "<<phase
                 <<": "<<error.name<<":"<<property_host::debugText(error.message)<<'\n';
        return 1;
    }catch(const std::exception& error){
        std::cerr<<"newcm-font-module-provider candidate failed during "<<phase<<": "<<error.what()<<'\n';
        return 1;
    }catch(...){
        std::cerr<<"newcm-font-module-provider candidate failed during "<<phase<<": unknown native failure\n";
        return 1;
    }
}
