#include "../../tests/support/folia_test.hpp"
import folia.math.fonts.newcm_font_data;
import folia.math.fonts.newcm_font_data_surface;
import folia.math.fonts.newcm_font_module_provider;
import folia.math.output.common.font_data_surface;
import folia.math.output.svg.font_data_surface;
import folia.math.util.context_property_bindings;
import folia.math.util.option_functions;
import folia.math.util.regexp_host;
import folia.math.util.promise;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::options;
namespace common=folia::math::output::common;
namespace fonts=folia::math::fonts;

std::string sourceText(const Value& value){
    if(const auto* text=std::get_if<std::u16string>(&value.data))
        return property_host::debugText(*text);
    throw std::runtime_error("expected Source string in generated DynamicFile definition");
}

struct ProviderFixture {
    Context context;
    OptionFunctionBindings functions{context};
    ContextPropertyBindings properties{context,functions};
    common::FontDataContextBindings identities{context};
    MapContextBindings maps{context};
    std::shared_ptr<regexp_host::RegExpBindings> regexp=
        std::make_shared<regexp_host::RegExpBindings>(context);
    std::shared_ptr<async::JobQueue> queue=std::make_shared<async::JobQueue>();
    std::shared_ptr<common::FontLoadHost> host=
        std::make_shared<common::FontLoadHost>();
    common::FontDataSourceSurface commonSurface{
        context,identities,properties,maps,regexp.get()};
    std::shared_ptr<common::FontDataClass> baseClass=
        std::make_shared<common::FontDataClass>(context);
    std::shared_ptr<output::svg::SvgFontDataClass> svgClass=
        std::make_shared<output::svg::SvgFontDataClass>(context);
    std::shared_ptr<fonts::NewcmFontDataClass> fontClass=
        std::make_shared<fonts::NewcmFontDataClass>(context);
    ObjectRef constructor{};
    std::shared_ptr<fonts::NewcmFontModuleProvider> provider;

    ProviderFixture(){
        host->queue=queue;
        host->regexp=regexp.get();
        host->asyncIsSynchronous=[] {return false;};
        const auto hostAnchor=std::static_pointer_cast<void>(host);
        const auto baseConstructor=commonSurface.registerBase(baseClass,*host,hostAnchor,
            [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
               Context& cx,Value supplied){
                return std::make_shared<common::FontData>(owner,loadHost,cx,std::move(supplied));
            });
        output::svg::SvgFontDataSourceSurface svgSurface(context,identities,commonSurface);
        const auto svgConstructor=svgSurface.registerSvg(svgClass,*host,hostAnchor,
            [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
               Context& cx,Value supplied){
                return std::make_shared<output::svg::SvgFontData>(
                    static_cast<output::svg::SvgFontDataClass&>(owner),loadHost,cx,
                    std::move(supplied));
            },baseConstructor);
        fonts::NewcmFontDataSourceSurface newcmSurface(context,identities,commonSurface);
        constructor=newcmSurface.registerNewcm(fontClass,*host,hostAnchor,
            [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
               Context& cx,Value supplied){
                return std::make_shared<fonts::NewcmFontData>(
                    static_cast<fonts::NewcmFontDataClass&>(owner),loadHost,cx,
                    std::move(supplied));
            },svgConstructor);
        provider=std::make_shared<fonts::NewcmFontModuleProvider>(fontClass,queue);
        host->syncLoad=[loader=provider](common::FontData& font,
            const common::DynamicFileRef& file,std::string request){
            loader->loadSync(font,file,std::move(request));
        };
        host->asyncLoad=[loader=provider](std::shared_ptr<common::FontData> font,
            common::DynamicFileRef file,std::string request){
            return loader->loadAsync(std::move(font),std::move(file),std::move(request));
        };
    }

    std::shared_ptr<fonts::NewcmFontData> makeFont(){
        const auto instance=context.construct(constructor);
        return std::dynamic_pointer_cast<fonts::NewcmFontData>(identities.resolve(instance));
    }

    common::DynamicFileRef arrows()const{
        const auto found=fontClass->dynamicFiles.find("arrows");
        return found==fontClass->dynamicFiles.end()?common::DynamicFileRef{}:found->second;
    }
};

suite newcmBundledFontModules=[] {
    "provider_rejects_uninitialized_source_class_before_catalog_access"_test=[] {
        Context context;
        auto queue=std::make_shared<async::JobQueue>();
        auto fontClass=std::make_shared<fonts::NewcmFontDataClass>(context);
        bool rejected=false;
        try{fonts::NewcmFontModuleProvider provider(fontClass,queue);}
        catch(const tree::FactoryError& error){
            rejected=error.name=="TypeError";
        }
        expect(rejected);
    };

    "sync_dynamic_request_evaluates_the_actual_bundled_module_and_setup_once"_test=[] {
        ProviderFixture fixture;
        auto font=fixture.makeFont();
        auto dynamic=fixture.arrows();
        expect(dynamic!=nullptr);
        if(!dynamic)return;
        expect(static_cast<bool>(dynamic->setup));
        const auto& placeholderType=dynamic->setup.target_type();
        const auto request=font->dynamicFileName(Value(*dynamic->sourceIdentity));
        expect(request==Value(u"@mathjax/mathjax-newcm-font/js/svg/dynamic/arrows.js"));

        fixture.host->asyncIsSynchronous=[] {return true;};
        const auto glyph=font->getChar("normal",0x219C);
        expect(glyph.has_value());
        expect(static_cast<bool>(dynamic->setup));
        expect(dynamic->setup.target_type()!=placeholderType);
        expect(!dynamic->failed);
        expect(dynamic->promise!=nullptr);
        expect(fixture.provider->loadedModuleCount()==1_u);
        const auto tuple=glyph?PropertyArray::array(*glyph):std::nullopt;
        expect(tuple.has_value());
        if(tuple){
            expect(tuple->get(0)==Property(0.47));
            expect(tuple->get(1)==Property(-0.09));
            expect(tuple->get(2)==Property(0.889));
            expect(PropertyList::record(tuple->get(3)).has_value());
        }
        (void)font->getChar("normal",0x219C);
        expect(fixture.provider->loadedModuleCount()==1_u);
    };

    "unknown_module_request_rejects_without_installing_setup"_test=[] {
        ProviderFixture fixture;
        auto font=fixture.makeFont();
        auto dynamic=fixture.arrows();
        expect(dynamic!=nullptr);
        if(!dynamic)return;
        bool rejected=false;
        try{fixture.provider->loadSync(*font,dynamic,"@mathjax/unknown/dynamic/arrows.js");}
        catch(const tree::FactoryError& error){
            rejected=error.name=="Error"&&
                error.message==u"Cannot find module '@mathjax/unknown/dynamic/arrows.js'";
        }
        expect(rejected);
        expect(static_cast<bool>(dynamic->setup));
        expect(fixture.provider->loadedModuleCount()==0_u);
    };

    "js_export_and_direct_cjs_request_share_one_resolved_module_cache_entry"_test=[] {
        ProviderFixture fixture;
        auto font=fixture.makeFont();
        auto dynamic=fixture.arrows();
        expect(dynamic!=nullptr);
        if(!dynamic)return;
        fixture.provider->loadSync(*font,dynamic,
            "@mathjax/mathjax-newcm-font/cjs/svg/dynamic/arrows.js");
        expect(static_cast<bool>(dynamic->setup));
        expect(fixture.provider->loadedModuleCount()==1_u);
        fixture.provider->loadSync(*font,dynamic,
            "@mathjax/mathjax-newcm-font/js/svg/dynamic/arrows.js");
        expect(static_cast<bool>(dynamic->setup));
        expect(fixture.provider->loadedModuleCount()==1_u);
    };

    "async_module_evaluation_precedes_font_setup_and_resolves_the_actual_file"_test=[] {
        ProviderFixture fixture;
        auto font=fixture.makeFont();
        auto dynamic=fixture.arrows();
        expect(dynamic!=nullptr);
        if(!dynamic)return;
        expect(static_cast<bool>(dynamic->setup));
        const auto completion=font->loadDynamicFile(dynamic);
        expect(completion->state()==async::State::pending);
        std::size_t jobs=0;
        while(fixture.queue->runOne()){
            ++jobs;
            if(jobs>1000)break;
        }
        expect(jobs>0_u);
        expect(static_cast<bool>(dynamic->setup));
        expect(completion->state()==async::State::fulfilled);
        expect(fixture.provider->loadedModuleCount()==1_u);
        const auto glyph=font->getChar("normal",0x219C);
        expect(glyph.has_value());
        expect(!dynamic->failed);
    };

    "source_package_aliases_resolve_to_the_same_cached_cjs_module"_test=[] {
        ProviderFixture fixture;
        auto font=fixture.makeFont();
        auto dynamic=fixture.arrows();
        expect(dynamic!=nullptr);
        if(!dynamic)return;
        const auto js="@mathjax/mathjax-newcm-font/js/svg/dynamic/arrows.js";
        const auto cjs="@mathjax/mathjax-newcm-font/cjs/svg/dynamic/arrows.js";
        const auto resolvedJs=fixture.provider->resolvedModuleName(js);
        const auto resolvedCjs=fixture.provider->resolvedModuleName(cjs);
        expect(resolvedJs.has_value());
        expect(resolvedCjs.has_value());
        if(!resolvedJs||!resolvedCjs)return;
        expect(*resolvedJs==*resolvedCjs);
        expect(*resolvedJs==cjs);
        expect(!fixture.provider->sameCachedModuleIdentity(js,cjs));
        const auto& placeholder=dynamic->setup.target_type();
        fixture.provider->loadSync(*font,dynamic,cjs);
        expect(dynamic->setup.target_type()!=placeholder);
        expect(fixture.provider->loadedModuleCount()==1_u);
        expect(fixture.provider->sameCachedModuleIdentity(js,cjs));
        std::size_t setupCalls=0;
        auto evaluated=dynamic->setup;
        dynamic->setup=[evaluated=std::move(evaluated),&setupCalls](common::FontData& actual){
            ++setupCalls;evaluated(actual);
        };
        const auto& wrapperType=dynamic->setup.target_type();
        fixture.provider->loadSync(*font,dynamic,js);
        expect(dynamic->setup.target_type()==wrapperType);
        expect(fixture.provider->loadedModuleCount()==1_u);
        expect(fixture.provider->sameCachedModuleIdentity(js,cjs));
        expect(eq(setupCalls,std::size_t{0}));
        dynamic->setup(*font);
        expect(eq(setupCalls,std::size_t{1}));
    };

    "thrown_module_evaluation_removes_the_commonjs_cache_entry"_test=[] {
        ProviderFixture fixture;
        auto font=fixture.makeFont();
        auto dynamic=fixture.arrows();
        expect(dynamic!=nullptr);
        if(!dynamic)return;
        const auto key=sourceText(dynamic->extension)+"\x1f"+sourceText(dynamic->file);
        fixture.fontClass->bundledDynamicSetups.insert_or_assign(key,
            [](common::FontDataClass&){
                throw tree::FactoryError("Error",u"module evaluation failed");
            });
        bool threw=false;
        try{
            fixture.provider->loadSync(*font,dynamic,
                "@mathjax/mathjax-newcm-font/cjs/svg/dynamic/arrows.js");
        }catch(const tree::FactoryError& error){
            threw=error.name=="Error"&&error.message==u"module evaluation failed";
        }
        expect(threw);
        expect(fixture.provider->loadedModuleCount()==0_u);
        expect(!fixture.provider->sameCachedModuleIdentity(
            "@mathjax/mathjax-newcm-font/js/svg/dynamic/arrows.js",
            "@mathjax/mathjax-newcm-font/cjs/svg/dynamic/arrows.js"));
        expect(static_cast<bool>(dynamic->setup));
    };
};
}
