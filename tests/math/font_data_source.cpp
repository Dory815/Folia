#include "support/folia_test.hpp"
import folia.math.fonts.newcm_font_data;
import folia.math.fonts.newcm_font_data_surface;
import folia.math.fonts.svg_extensions;
import folia.math.output.common.font_data_surface;
import folia.math.output.svg.font_data_surface;
import folia.math.util.context_property_bindings;
import folia.math.util.option_functions;
import folia.math.util.options;
import folia.math.util.property_host;
import folia.math.core.tree.factory_types;
import folia.math.output.common.font_data_bindings;

namespace {
using namespace boost::ut;
namespace common=folia::math::output::common;
namespace fonts=folia::math::fonts;
namespace math_options=folia::math::options;
using folia::math::Property;
using folia::math::PropertyArray;
using folia::math::PropertyList;

std::size_t glyphCount(const common::CharMapMap& maps){
    std::size_t count=0;
    for(const auto&[_,map]:maps)count+=map.size();
    return count;
}

struct RegisteredNewcmFixture {
    math_options::Context context;
    folia::math::OptionFunctionBindings functions{context};
    folia::math::ContextPropertyBindings properties{context,functions};
    common::FontDataContextBindings identities{context};
    math_options::MapContextBindings maps{context};
    std::shared_ptr<folia::math::regexp_host::RegExpBindings> regexp=
        std::make_shared<folia::math::regexp_host::RegExpBindings>(context);
    common::FontDataSourceSurface commonSurface{context,identities,properties,maps,regexp.get()};
    std::shared_ptr<folia::math::async::JobQueue> queue=std::make_shared<folia::math::async::JobQueue>();
    std::shared_ptr<common::FontLoadHost> host=std::make_shared<common::FontLoadHost>();
    std::shared_ptr<common::FontDataClass> base=std::make_shared<common::FontDataClass>(context);
    std::shared_ptr<folia::math::output::svg::SvgFontDataClass> svg=
        std::make_shared<folia::math::output::svg::SvgFontDataClass>(context);
    std::shared_ptr<fonts::NewcmFontDataClass> newcm=std::make_shared<fonts::NewcmFontDataClass>(context);
    math_options::ObjectRef constructor{};

    RegisteredNewcmFixture(){
        host->queue=queue;
        host->regexp=regexp.get();
        host->asyncIsSynchronous=[] {return false;};
        auto anchor=std::static_pointer_cast<void>(host);
        const auto baseConstructor=commonSurface.registerBase(base,*host,anchor,
            [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
               math_options::Context& cx,math_options::Value options){
                return std::make_shared<common::FontData>(owner,loadHost,cx,std::move(options));
            });
        folia::math::output::svg::SvgFontDataSourceSurface svgSurface(context,identities,commonSurface);
        const auto svgConstructor=svgSurface.registerSvg(svg,*host,anchor,
            [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
               math_options::Context& cx,math_options::Value options){
                return std::make_shared<folia::math::output::svg::SvgFontData>(
                    static_cast<folia::math::output::svg::SvgFontDataClass&>(owner),
                    loadHost,cx,std::move(options));
            },baseConstructor);
        fonts::NewcmFontDataSourceSurface newcmSurface(context,identities,commonSurface);
        constructor=newcmSurface.registerNewcm(newcm,*host,anchor,
            [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
               math_options::Context& cx,math_options::Value options){
                return std::make_shared<fonts::NewcmFontData>(
                    static_cast<fonts::NewcmFontDataClass&>(owner),loadHost,cx,std::move(options));
            },svgConstructor);
    }

    std::shared_ptr<fonts::NewcmFontData> makeFont(){
        const auto instance=context.construct(constructor);
        return std::dynamic_pointer_cast<fonts::NewcmFontData>(identities.resolve(instance));
    }
};
}

suite native_math_font_data_source=[] {
    "newcm_generated_static_tables_match_pinned_counts_and_samples"_test=[] {
        math_options::Context context;
        fonts::NewcmFontDataClass font(context);
        font.initializeSourceData();
        expect(std::string_view(font.NAME)==std::string_view("MathJaxNewcm"));
        expect(font.JAX==std::string_view("SVG"));
        expect(eq(PropertyArray::array(font.defaultVariants)->size(),std::size_t{33}));
        expect(eq(glyphCount(font.defaultChars),std::size_t{2315}));
        expect(eq(font.defaultDelimiters.size(),std::size_t{222}));
        expect(eq(font.dynamicFiles.size(),std::size_t{40}));
        const auto normal=font.defaultChars.find("normal");
        expect(normal!=font.defaultChars.end());
        if(normal!=font.defaultChars.end()){
            const auto a=normal->second.find(65);
            expect(a!=normal->second.end());
            const auto* glyph=a==normal->second.end()?nullptr:std::get_if<Property>(&a->second);
            const auto tuple=glyph?PropertyArray::array(*glyph):std::nullopt;
            expect(tuple.has_value());
            if(tuple){
                expect(tuple->size()==4);
                expect(tuple->get(0)==Property(0.716));
                expect(tuple->get(1)==Property(0.0));
                expect(tuple->get(2)==Property(0.75));
                const auto options=PropertyList::record(tuple->get(3));
                expect(options&&options->getOwn("p")==Property(std::string("583 3L717 0L717 39L695 39C670 39 653 40 646 42C631 47 621 58 615 75L401 694C397 707 390 716 375 716C362 716 353 709 348 694L143 102C129 61 92 39 32 38L32 0L137 3L254 0L254 39C205 39 180 54 180 85C180 92 181 96 182 98L227 225L465 225L516 77C518 71 519 67 519 64C519 47 492 39 437 39L437 0C479 3 528 4 583 3M345 569L451 264L240 264")));
            }
        }
        expect(font.variantCacheIds.contains("normal"));
        expect(font.VariantSmp.contains("double-struck"));
        const auto defaults=PropertyArray::array(font.defaultVariants).value();
        const auto normalVariant=PropertyArray::array(defaults.get(0));
        const auto boldVariant=PropertyArray::array(defaults.get(1));
        expect(normalVariant.has_value()&&eq(normalVariant->size(),std::size_t{1}));
        expect(boldVariant.has_value()&&eq(boldVariant->size(),std::size_t{2}));
        const auto* italicSmp=std::get_if<PropertyArray>(&font.VariantSmp["italic"]);
        expect(italicSmp!=nullptr);
        if(italicSmp){
            expect(eq(italicSmp->size(),std::size_t{4}));
            expect(italicSmp->has(0)&&italicSmp->has(3));
            expect(!italicSmp->has(4));
        }
        const auto* boldSmp=std::get_if<PropertyArray>(&font.VariantSmp["bold"]);
        expect(boldSmp!=nullptr);
        if(boldSmp){
            expect(eq(boldSmp->size(),std::size_t{6}));
            expect(boldSmp->has(5));
        }
        expect(eq(font.defaultCssFonts["normal"].size(),std::size_t{3}));
        expect(font.defaultParams.axis_height>0);
        expect(font.defaultParams.rule_thickness>0);
    };
    "bundled_svg_extensions_preserve_source_variants_and_shared_glyph_records"_test=[] {
        RegisteredNewcmFixture fixture;
        auto& fontClass=*fixture.newcm;
        fontClass.addExtension(fonts::BbmFontExtensionData());
        fontClass.addExtension(fonts::BboldxFontExtensionData());
        fontClass.addExtension(fonts::DsfontFontExtensionData());
        expect(fontClass.defaultChars.contains("-bbm-normal"));
        expect(fontClass.defaultChars.contains("-bbm-bold"));
        expect(fontClass.defaultChars.contains("-bboldx"));
        expect(fontClass.defaultChars.contains("-bboldx-light"));
        expect(fontClass.defaultChars.contains("-ds-rm"));
        expect(fontClass.defaultChars.contains("-ds-ss"));
        expect(eq(PropertyArray::array(fontClass.defaultVariants)->size(),std::size_t{43}));
        expect(eq(glyphCount(fontClass.defaultChars),std::size_t{3056}));
        expect(fontClass.variantCacheIds.contains("-bboldx"));
        expect(fontClass.variantCacheIds.contains("-ds-ss"));
        const auto ds=fontClass.defaultChars.find("-ds-rm");
        expect(ds!=fontClass.defaultChars.end());
        if(ds!=fontClass.defaultChars.end()){
            const auto ch=ds->second.find(0x2115);
            expect(ch!=ds->second.end());
            const auto* glyph=ch==ds->second.end()?nullptr:std::get_if<Property>(&ch->second);
            expect(glyph&&PropertyArray::array(*glyph).has_value());
        }
    };
    "font_extension_array_options_use_live_class_and_instance_receivers"_test=[] {
        RegisteredNewcmFixture fixture;
        const auto classBefore=fixture.context.get(math_options::Value(fixture.constructor),
            math_options::Key(u"defaultSizeVariants"));
        const auto* classBeforeRef=std::get_if<math_options::ObjectRef>(&classBefore.data);
        expect(classBeforeRef!=nullptr);
        const auto classBeforeLength=classBeforeRef?fixture.context.arrayLength(*classBeforeRef):0;
        folia::math::output::svg::SvgFontExtensionData classExtension;
        classExtension.name="source-class-array-probe";
        common::ListPatch classPatch;
        classPatch.append=PropertyArray{Property(std::string("class-added"))};
        classExtension.sizeVariants=classPatch;
        fixture.newcm->addExtension(classExtension);
        const auto classAfter=fixture.context.get(math_options::Value(fixture.constructor),
            math_options::Key(u"defaultSizeVariants"));
        const auto classOwnerValue=fixture.properties.to(Property(fixture.newcm->defaultSizeVariants));
        expect(classAfter==classOwnerValue);
        if(classBeforeRef){
            const auto* classAfterRef=std::get_if<math_options::ObjectRef>(&classAfter.data);
            expect(classAfterRef&&*classAfterRef!=*classBeforeRef);
            expect(classAfterRef&&fixture.context.arrayLength(*classAfterRef)==classBeforeLength+1);
        }

        auto font=fixture.makeFont();
        expect(font!=nullptr);
        if(!font)return;
        const auto instance=fixture.identities.identity(font.get());
        expect(instance.has_value());
        if(!instance)return;
        const auto instanceBefore=fixture.context.get(math_options::Value(*instance),
            math_options::Key(u"sizeVariants"));
        const auto* instanceBeforeRef=std::get_if<math_options::ObjectRef>(&instanceBefore.data);
        expect(instanceBeforeRef!=nullptr);
        const auto instanceBeforeLength=instanceBeforeRef?fixture.context.arrayLength(*instanceBeforeRef):0;
        folia::math::output::svg::SvgFontExtensionData instanceExtension;
        instanceExtension.name="source-instance-array-probe";
        common::ListPatch instancePatch;
        instancePatch.append=PropertyArray{Property(std::string("instance-added"))};
        instanceExtension.sizeVariants=instancePatch;
        font->addExtension(instanceExtension);
        const auto instanceAfter=fixture.context.get(math_options::Value(*instance),
            math_options::Key(u"sizeVariants"));
        const auto instanceOwnerValue=fixture.properties.to(Property(font->sizeVariants()));
        expect(instanceAfter==instanceOwnerValue);
        if(instanceBeforeRef){
            const auto* instanceAfterRef=std::get_if<math_options::ObjectRef>(&instanceAfter.data);
            expect(instanceAfterRef&&*instanceAfterRef!=*instanceBeforeRef);
            expect(instanceAfterRef&&fixture.context.arrayLength(*instanceAfterRef)==instanceBeforeLength+1);
        }
    };
    "generated_bbm_extension_appends_to_its_registered_class_receiver"_test=[] {
        RegisteredNewcmFixture fixture;
        const auto beforeVariants=PropertyArray::array(fixture.newcm->defaultVariants);
        const auto beforeSizes=PropertyArray::array(fixture.newcm->defaultSizeVariants);
        const auto beforeStretch=PropertyArray::array(fixture.newcm->defaultStretchVariants);
        const auto extension=fonts::BbmFontExtensionData();
        expect(extension.variants&&extension.variants->append.has_value()) << "generated BBM variant patch is present";
        expect(extension.sizeVariants&&extension.sizeVariants->append.has_value()) << "generated BBM size patch is present";
        expect(extension.stretchVariants&&extension.stretchVariants->append.has_value()) << "generated BBM stretch patch is present";
        fixture.newcm->addExtension(extension);
        const auto afterVariants=PropertyArray::array(fixture.newcm->defaultVariants);
        const auto afterSizes=PropertyArray::array(fixture.newcm->defaultSizeVariants);
        const auto afterStretch=PropertyArray::array(fixture.newcm->defaultStretchVariants);
        expect(beforeVariants&&afterVariants&&afterVariants->size()==beforeVariants->size()+5)
            << "the real generated five-variant append is visible on the native owner";
        expect(beforeSizes&&afterSizes&&afterSizes->size()==beforeSizes->size()+1)
            << "the real generated size append is visible on the native owner";
        expect(beforeStretch&&afterStretch&&afterStretch->size()==beforeStretch->size()+1)
            << "the real generated stretch append is visible on the native owner";
        const auto ctxVariants=fixture.context.get(math_options::Value(fixture.constructor),math_options::Key(u"defaultVariants"));
        const auto ctxSizes=fixture.context.get(math_options::Value(fixture.constructor),math_options::Key(u"defaultSizeVariants"));
        const auto ctxStretch=fixture.context.get(math_options::Value(fixture.constructor),math_options::Key(u"defaultStretchVariants"));
        const auto* variantRef=std::get_if<math_options::ObjectRef>(&ctxVariants.data);
        const auto* sizeRef=std::get_if<math_options::ObjectRef>(&ctxSizes.data);
        const auto* stretchRef=std::get_if<math_options::ObjectRef>(&ctxStretch.data);
        expect(variantRef&&fixture.context.arrayLength(*variantRef)==afterVariants->size())
            << "Context observes the exact owner-backed variant array";
        expect(sizeRef&&fixture.context.arrayLength(*sizeRef)==afterSizes->size())
            << "Context observes the exact owner-backed size array";
        expect(stretchRef&&fixture.context.arrayLength(*stretchRef)==afterStretch->size())
            << "Context observes the exact owner-backed stretch array";
    };
    "character_option_accessor_preserves_tuple_identity_and_slot_presence"_test=[] {
        common::CharMap map;
        PropertyArray three{Property(1.0),Property(0.0),Property(0.5)};
        map.emplace(0x41,Property(three));
        const auto before=std::get<Property>(map.at(0x41));
        const auto options=common::FontDataClass::charOptions(map,0x41);
        const auto after=std::get<Property>(map.at(0x41));
        const auto tuple=PropertyArray::array(after);
        expect(before==after);
        expect(tuple.has_value()&&eq(tuple->size(),std::size_t{4})&&tuple->has(3));
        expect(PropertyList::object(options).has_value());

        PropertyArray explicitUndefined;explicitUndefined.resize(4);explicitUndefined.set(3,Property(folia::math::Missing{}));
        map.insert_or_assign(0x42,Property(explicitUndefined));
        expect(std::holds_alternative<folia::math::Missing>(common::FontDataClass::charOptions(map,0x42).value));
        PropertyArray explicitNull;explicitNull.resize(4);explicitNull.set(3,Property(folia::math::PropertyNull{}));
        map.insert_or_assign(0x43,Property(explicitNull));
        expect(std::holds_alternative<folia::math::PropertyNull>(common::FontDataClass::charOptions(map,0x43).value));
        PropertyList invalidOptions;
        map.insert_or_assign(0x44,Property(invalidOptions));
        bool rejected=false;
        try{(void)common::FontDataClass::charOptions(map,0x44);}
        catch(const folia::math::tree::FactoryError& error){
            rejected=error.name=="Error"&&error.message==u"Character data hasn't been loaded for 0x44";
        }
        expect(rejected);
    };
    "font_instance_resolves_metrics_outline_remaps_and_delimiters_from_same_data"_test=[] {
        RegisteredNewcmFixture fixture;
        auto& fontClass=*fixture.newcm;
        fontClass.addExtension(fonts::BboldxFontExtensionData());
        auto font=fixture.makeFont();
        expect(font!=nullptr);
        if(!font)return;
        const auto extensionResult=font->addExtension(fonts::BbmFontExtensionData());
        expect(extensionResult.empty());
        expect(font->getVariant("-bbm-normal").has_value());
        const auto normalZero=font->getChar("normal",48);
        const auto boldZero=font->getChar("bold",48);
        expect(normalZero.has_value()&&boldZero.has_value());
        if(normalZero&&boldZero)expect(*normalZero!=*boldZero);
        const auto normalHorizontalBar=font->getChar("normal",8213);
        const auto boldHorizontalBar=font->getChar("bold",8213);
        expect(normalHorizontalBar.has_value()&&boldHorizontalBar.has_value());
        if(normalHorizontalBar&&boldHorizontalBar)expect(*normalHorizontalBar==*boldHorizontalBar);
        const auto a=font->getChar("normal",65);
        expect(a.has_value());
        if(a){
            const auto tuple=PropertyArray::array(*a);
            expect(tuple.has_value());
            if(tuple){
                expect(eq(std::get<double>(tuple->get(2).value),0.75));
                const auto options=PropertyList::record(tuple->get(3));
                expect(options&&options->getOwn("p")==Property(std::string("583 3L717 0L717 39L695 39C670 39 653 40 646 42C631 47 621 58 615 75L401 694C397 707 390 716 375 716C362 716 353 709 348 694L143 102C129 61 92 39 32 38L32 0L137 3L254 0L254 39C205 39 180 54 180 85C180 92 181 96 182 98L227 225L465 225L516 77C518 71 519 67 519 64C519 47 492 39 437 39L437 0C479 3 528 4 583 3M345 569L451 264L240 264")));
            }
        }
        const auto remapped=font->getChar("double-struck",65);
        expect(remapped.has_value()&&PropertyArray::array(*remapped).has_value());
        if(remapped){
            const auto tuple=PropertyArray::array(*remapped);
            expect(tuple.has_value());
            if(tuple){expect(eq(tuple->size(),std::size_t{4}));expect(!tuple->has(0)&&!tuple->has(1)&&!tuple->has(2));expect(tuple->has(3));}
        }
        expect(font->getVariant("-bboldx").has_value());
        const auto extended=font->getChar("-bboldx",65);
        expect(extended.has_value()&&PropertyArray::array(*extended).has_value());
        const auto delim=font->getDelimiter(0x28);
        expect(delim.has_value()&&std::holds_alternative<common::DelimiterDataRef>(*delim));
        expect(font->getSizeVariant(0x28,0).has_value());
        expect(font->getStretchVariants(0x28)[0].has_value());
        expect(std::get<std::string>(font->getCssFont("missing").get(0).value)==std::string("serif"));
        expect(font->getFamily("sans-serif")==std::string("sans-serif"));
    };
    "bundled_dynamic_setup_is_activated_only_after_successful_provider_load"_test=[] {
        RegisteredNewcmFixture fixture;
        auto& fontClass=*fixture.newcm;
        auto queue=fixture.queue;
        auto provider=folia::math::async::Promise::create(queue);
        auto& host=*fixture.host;
        host.asyncLoad=[provider,&fontClass,dynamic=fontClass.dynamicFiles.begin()->second](
            std::shared_ptr<common::FontData>,common::DynamicFileRef,std::string){
            return provider->then([&fontClass,dynamic](const std::any& value)->std::any{
                fontClass.activateBundledDynamicSetup(dynamic);
                return value;
            });
        };
        auto font=fixture.makeFont();
        const auto dynamic=fontClass.dynamicFiles.begin()->second;
        expect(dynamic&&static_cast<bool>(dynamic->setup));
        const auto& placeholder=dynamic->setup.target_type();
        const auto completion=font->loadDynamicFile(dynamic);
        expect(dynamic&&dynamic->setup.target_type()==placeholder);
        provider->resolve();
        while(queue->runOne()){}
        expect(dynamic&&static_cast<bool>(dynamic->setup)&&dynamic->setup.target_type()!=placeholder);
        expect(completion->state()==folia::math::async::State::fulfilled);
    };
    "failed_dynamic_provider_preserves_source_setup_placeholder_and_sync_retry_is_a_noop"_test=[] {
        RegisteredNewcmFixture fixture;
        auto& fontClass=*fixture.newcm;
        auto queue=fixture.queue;
        std::size_t loads=0,warnings=0;
        auto& host=*fixture.host;
        host.asyncIsSynchronous=[] {return true;};
        host.syncLoad=[&](common::FontData&,const common::DynamicFileRef&,std::string){++loads;throw 1;};
        host.warn=[&](const std::any&){++warnings;};
        auto font=fixture.makeFont();
        const auto dynamic=fontClass.dynamicFiles.begin()->second;
        expect(dynamic&&dynamic->sourceIdentity.has_value())
            << "generated class DynamicFile has its canonical Source object before loading";
        if(dynamic&&dynamic->sourceIdentity){
            expect(fixture.context.get(math_options::Value(*dynamic->sourceIdentity),
                math_options::Key(u"file"))==math_options::Value(
                    folia::math::property_host::text(fontClass.dynamicFiles.begin()->first)))
                << "the first generated file retains its Source file value";
            expect(fixture.context.get(math_options::Value(*dynamic->sourceIdentity),
                math_options::Key(u"extension"))==math_options::Value(u""))
                << "the first generated file retains its Source extension value";
        }
        const auto& placeholder=dynamic->setup.target_type();
        font->loadDynamicFileSync(dynamic);
        expect(dynamic&&dynamic->failed&&dynamic->setup.target_type()==placeholder)
            << "a rejected synchronous provider marks the same file failed and keeps its placeholder setup";
        font->loadDynamicFileSync(dynamic);
        expect(eq(loads,std::size_t{1})) << "a failed file does not request its provider again";
        expect(eq(warnings,std::size_t{1})) << "the failed provider warning is emitted once";
    };
    "missing_sync_provider_uses_source_setup_placeholder"_test=[] {
        RegisteredNewcmFixture fixture;
        auto& fontClass=*fixture.newcm;
        auto queue=fixture.queue;
        std::size_t warnings=0;
        auto& host=*fixture.host;
        host.asyncIsSynchronous=[] {return true;};
        host.warn=[&](const std::any&){++warnings;};
        auto font=fixture.makeFont();
        const auto dynamic=fontClass.dynamicFiles.begin()->second;
        const auto& placeholder=dynamic->setup.target_type();
        font->loadDynamicFileSync(dynamic);
        expect(dynamic&&dynamic->failed&&dynamic->setup.target_type()==placeholder);
        expect(eq(warnings,std::size_t{1}));
    };
    "provider_installed_setup_is_preserved_and_setup_errors_remain_retryable"_test=[] {
        RegisteredNewcmFixture fixture;
        auto& fontClass=*fixture.newcm;
        const auto dynamic=fontClass.dynamicFiles.begin()->second;
        auto queue=fixture.queue;
        std::size_t setupCalls=0,warnings=0;
        auto& host=*fixture.host;
        host.asyncIsSynchronous=[] {return true;};
        host.syncLoad=[dynamic,&setupCalls](common::FontData&,const common::DynamicFileRef&,std::string){
            dynamic->setup=[&setupCalls](common::FontData&){++setupCalls;throw 7;};
        };
        host.warn=[&](const std::any&){++warnings;};
        auto font=fixture.makeFont();
        for(int i=0;i<2;++i){bool failed=false;try{font->loadDynamicFileSync(dynamic);}catch(int value){failed=value==7;}expect(failed==(i==0));}
        expect(dynamic&&dynamic->setup&&!dynamic->failed);
        expect(eq(setupCalls,std::size_t{1}));
        expect(eq(warnings,std::size_t{0}));
    };
    "missing_async_provider_is_warned_then_runs_source_setup_placeholder"_test=[] {
        RegisteredNewcmFixture fixture;
        auto& fontClass=*fixture.newcm;
        auto queue=fixture.queue;
        std::size_t warnings=0;
        auto& host=*fixture.host;
        host.warn=[&](const std::any&){++warnings;};
        auto font=fixture.makeFont();
        const auto dynamic=fontClass.dynamicFiles.begin()->second;
        const auto completion=font->loadDynamicFile(dynamic);
        while(queue->runOne()){}
        expect(completion->state()==folia::math::async::State::fulfilled);
        expect(dynamic&&dynamic->failed&&static_cast<bool>(dynamic->setup));
        expect(eq(warnings,std::size_t{1}));
    };
    "async_provider_installed_setup_throw_rejects_without_marking_file_failed"_test=[] {
        RegisteredNewcmFixture fixture;
        auto& fontClass=*fixture.newcm;
        const auto dynamic=fontClass.dynamicFiles.begin()->second;
        auto queue=fixture.queue;
        std::size_t setupCalls=0;
        auto& host=*fixture.host;
        host.asyncLoad=[queue,dynamic,&setupCalls](std::shared_ptr<common::FontData>,common::DynamicFileRef,std::string){
            dynamic->setup=[&setupCalls](common::FontData&){++setupCalls;throw 9;};
            auto provider=folia::math::async::Promise::create(queue);
            provider->resolve();
            return provider;
        };
        auto font=fixture.makeFont();
        const auto first=font->loadDynamicFile(dynamic);
        while(queue->runOne()){}
        expect(first->state()==folia::math::async::State::rejected);
        expect(dynamic&&dynamic->setup&&!dynamic->failed);
        const auto second=font->loadDynamicFile(dynamic);
        while(queue->runOne()){}
        expect(second->state()==folia::math::async::State::rejected);
        expect(eq(setupCalls,std::size_t{2}));
        expect(dynamic&&!dynamic->failed);
    };
    "failed_async_dynamic_provider_runs_source_setup_placeholder"_test=[] {
        RegisteredNewcmFixture fixture;
        auto& fontClass=*fixture.newcm;
        auto queue=fixture.queue;
        std::size_t warnings=0;
        auto& host=*fixture.host;
        host.asyncLoad=[queue](std::shared_ptr<common::FontData>,common::DynamicFileRef,std::string){
            auto provider=folia::math::async::Promise::create(queue);
            provider->reject(std::string("provider failed"));
            return provider;
        };
        host.warn=[&](const std::any&){++warnings;};
        auto font=fixture.makeFont();
        const auto dynamic=fontClass.dynamicFiles.begin()->second;
        const auto first=font->loadDynamicFile(dynamic);
        while(queue->runOne()){}
        // NewCM's real defineDynamicFiles record starts with Source's setup
        // placeholder. A rejected provider is caught, then that placeholder
        // marks the file failed while the chained load fulfills.
        expect(first->state()==folia::math::async::State::fulfilled) << "first async load fulfills after provider rejection is caught";
        expect(dynamic&&dynamic->failed&&static_cast<bool>(dynamic->setup)) << "rejected provider marks the file failed and retains its real setup function";
        const auto second=font->loadDynamicFile(dynamic);
        expect(second->state()==folia::math::async::State::rejected) << "a later load rejects immediately for a failed dynamic file";
        expect(eq(warnings,std::size_t{1})) << "the rejected provider warns exactly once";
    };
    "newcm_source_data_initializes_after_canonical_constructor_registration"_test=[] {
        math_options::Context context;
        folia::math::OptionFunctionBindings functions(context);
        folia::math::ContextPropertyBindings properties(context,functions);
        common::FontDataContextBindings identities(context);
        math_options::MapContextBindings maps(context);
        common::FontDataSourceSurface commonSurface(context,identities,properties,maps);
        auto queue=std::make_shared<folia::math::async::JobQueue>();
        common::FontLoadHost host;host.queue=queue;host.asyncIsSynchronous=[] {return false;};
        auto anchor=std::make_shared<int>(1);

        auto base=std::make_shared<common::FontDataClass>(context);
        const auto baseConstructor=commonSurface.registerBase(base,host,anchor,
            [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
               math_options::Context& cx,math_options::Value options){
                return std::make_shared<common::FontData>(owner,loadHost,cx,std::move(options));
            });
        auto svg=std::make_shared<folia::math::output::svg::SvgFontDataClass>(context);
        folia::math::output::svg::SvgFontDataSourceSurface svgSurface(context,identities,commonSurface);
        const auto svgConstructor=svgSurface.registerSvg(svg,host,anchor,
            [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
               math_options::Context& cx,math_options::Value options){
                return std::make_shared<folia::math::output::svg::SvgFontData>(
                    static_cast<folia::math::output::svg::SvgFontDataClass&>(owner),
                    loadHost,cx,std::move(options));
            },baseConstructor);
        auto newcm=std::make_shared<fonts::NewcmFontDataClass>(context);
        fonts::NewcmFontDataSourceSurface newcmSurface(context,identities,commonSurface);
        const auto constructor=newcmSurface.registerNewcm(newcm,host,anchor,
            [](common::FontDataClass& owner,common::FontLoadHost& loadHost,
               math_options::Context& cx,math_options::Value options){
                return std::make_shared<fonts::NewcmFontData>(
                    static_cast<fonts::NewcmFontDataClass&>(owner),loadHost,cx,std::move(options));
            },svgConstructor);

        expect(identities.constructor(newcm.get())==constructor);
        expect(newcm->defaultChars.size()==33_u);
        expect(newcm->dynamicFiles.size()==40_u);
        const auto variantsValue=context.get(math_options::Value(constructor),math_options::Key(u"defaultVariants"));
        const auto* variantsObject=std::get_if<math_options::ObjectRef>(&variantsValue.data);
        expect(variantsObject!=nullptr);
        if(variantsObject){
            expect(context.kind(*variantsObject)==math_options::ObjectKind::array);
            expect(context.arrayLength(*variantsObject)==33_u);
            const auto replacement=context.array(1);
            context.set(math_options::Value(replacement),math_options::Key(u"0"),math_options::Value(u"source-value"));
            context.set(math_options::Value(constructor),math_options::Key(u"defaultVariants"),math_options::Value(replacement));
            const auto ownerProperty=properties.to(Property(newcm->defaultVariants));
            expect(std::get<math_options::ObjectRef>(ownerProperty.data)==replacement);
            expect(context.get(math_options::Value(constructor),math_options::Key(u"defaultVariants"))==
                   math_options::Value(replacement));
        }
    };
};
