/*************************************************************
 * Source-ordered SVG wrapper-class registration.
 *************************************************************/
module folia.math.output.svg.wrapper_classes;
import folia.math.output.common;
import folia.math.output.svg.xml_wrapper;
import folia.math.output.svg.wrappers.math;
import folia.math.output.svg.menclose_surface;
import folia.math.output.svg.output_jax;
import folia.math.util.property_host;
import std;

namespace folia::math::output::svg {
namespace {
using namespace options;

template<class Wrapper>
void add(common::CommonWrapperClassMap& result,
         common::CommonFactoryContextBindings& bindings,
         std::u16string_view kind,ObjectRef constructor){
    common::CommonWrapperClassData metadata;
    metadata.kind=property_host::debugText(kind);
    auto descriptor=common::registerCommonWrapperClass<Wrapper>(
        bindings,constructor,std::move(metadata));
    result.emplace_back(property_host::debugText(kind),std::move(descriptor));
}

void addPendingSourceClass(common::CommonWrapperClassMap& result,
                           common::CommonFactoryContextBindings& bindings,
                           std::u16string_view kind,ObjectRef constructor,
                           std::u16string_view sourceMember){
    // Keep the real Source constructor/prototype in the complete defaultNodes
    // catalog, but do not pretend an unrelated Common wrapper implements it.
    // The unsupported body fails only if this exact descriptor is invoked.
    common::CommonWrapperClassData metadata;
    metadata.kind=property_host::debugText(kind);
    metadata.sourceConstructor=constructor;
    auto descriptor=std::make_shared<tree::FactoryClass<common::CommonWrapper>>();
    descriptor->nativeType=typeid(void);
    descriptor->staticData=std::move(metadata);
    const auto member=std::u16string(sourceMember);
    descriptor->construct=[member](tree::FactoryServices<common::CommonWrapper>&,
                                   tree::FactoryArgs)->common::CommonWrapper*{
        throw Exception("Unimplemented",u"SVG wrapper source member is not ported: "+member);
    };
    descriptor->isInstance=[](const common::CommonWrapper*){return false;};
    bindings.bindWrapperClass(descriptor,constructor);
    result.emplace_back(property_host::debugText(kind),std::move(descriptor));
}

void addForKind(common::CommonWrapperClassMap& result,
                common::CommonFactoryContextBindings& bindings,
                std::u16string_view kind,ObjectRef constructor,
                Context& context,ContextPropertyBindings& properties){
#define FOLIA_SVG_WRAPPER_CASE(name, type) \
    if(kind==u##name){add<common::type>(result,bindings,kind,constructor);return;}
    if(kind==u"math"){
        add<SvgMath>(result,bindings,kind,constructor);
        return;
    }
    FOLIA_SVG_WRAPPER_CASE("mrow",CommonMrow)
    FOLIA_SVG_WRAPPER_CASE("inferredMrow",CommonInferredMrow)
    FOLIA_SVG_WRAPPER_CASE("mi",CommonMi)
    FOLIA_SVG_WRAPPER_CASE("mo",CommonMo)
    FOLIA_SVG_WRAPPER_CASE("mn",CommonMn)
    FOLIA_SVG_WRAPPER_CASE("ms",CommonMs)
    FOLIA_SVG_WRAPPER_CASE("mtext",CommonMtext)
    // These Source SVG constructors extend SvgWrapper directly. Each still
    // gets its own canonical descriptor and Context constructor identity; the
    // shared C++ measurement base is not a kind-based fallback.
    FOLIA_SVG_WRAPPER_CASE("merror",CommonWrapper)
    FOLIA_SVG_WRAPPER_CASE("mspace",CommonMspace)
    FOLIA_SVG_WRAPPER_CASE("mpadded",CommonMpadded)
    FOLIA_SVG_WRAPPER_CASE("mphantom",CommonWrapper)
    FOLIA_SVG_WRAPPER_CASE("mfrac",CommonMfrac)
    FOLIA_SVG_WRAPPER_CASE("msqrt",CommonMsqrt)
    FOLIA_SVG_WRAPPER_CASE("mroot",CommonMroot)
    FOLIA_SVG_WRAPPER_CASE("mfenced",CommonMfenced)
    FOLIA_SVG_WRAPPER_CASE("msub",CommonMsub)
    FOLIA_SVG_WRAPPER_CASE("msup",CommonMsup)
    FOLIA_SVG_WRAPPER_CASE("msubsup",CommonMsubsup)
    FOLIA_SVG_WRAPPER_CASE("munder",CommonMunder)
    FOLIA_SVG_WRAPPER_CASE("mover",CommonMover)
    FOLIA_SVG_WRAPPER_CASE("munderover",CommonMunderover)
    FOLIA_SVG_WRAPPER_CASE("mmultiscripts",CommonMmultiscripts)
    if(kind==u"mtable"){
        common::CommonWrapperClassData metadata;
        metadata.kind=property_host::debugText(kind);
        auto descriptor=common::registerCommonWrapperClass<common::CommonMtable>(
            bindings,constructor,std::move(metadata));
        // SvgMtable's constructor calls this.svg('g', def) after the Common
        // constructor phase. Keep that call on the same canonical Jax and
        // publish the returned Lite DOM node on the already-bound wrapper.
        auto sourceConstruct=descriptor->construct;
        descriptor->construct=[sourceConstruct=std::move(sourceConstruct),
            &context,&properties](tree::FactoryServices<common::CommonWrapper>& services,
                                  tree::FactoryArgs args)->common::CommonWrapper*{
            auto* wrapper=sourceConstruct(services,std::move(args));
            auto* table=dynamic_cast<common::CommonMtable*>(wrapper);
            if(!table)throw Exception("TypeError",u"SvgMtable constructed a non-Mtable wrapper");
            auto* jax=dynamic_cast<SvgOutputJax*>(&table->jax());
            if(!jax)throw Exception("TypeError",u"SvgMtable requires its actual SVG output Jax");
            PropertyList attributes{{"data-labels",Property(true)}};
            if(table->isTop)attributes.set("transform",Property("matrix(1 0 0 -1 0 0)"));
            const auto labels=jax->sourceSvgNode(u"g",std::move(attributes));
            const auto identity=table->factory().objectFor(*table);
            context.set(Value(identity),Key(u"labels"),properties.to(Property(nodeProperty(labels))));
            return wrapper;
        };
        result.emplace_back(property_host::debugText(kind),std::move(descriptor));
        return;
    }
    FOLIA_SVG_WRAPPER_CASE("mtr",CommonMtr)
    FOLIA_SVG_WRAPPER_CASE("mlabeledtr",CommonMlabeledtr)
    FOLIA_SVG_WRAPPER_CASE("mtd",CommonMtd)
    FOLIA_SVG_WRAPPER_CASE("maction",CommonMaction)
    if(kind==u"menclose"){
        auto descriptor=registerSvgMencloseClass(bindings,constructor);
        result.emplace_back(property_host::debugText(kind),std::move(descriptor));
        return;
    }
    FOLIA_SVG_WRAPPER_CASE("semantics",CommonSemantics)
    FOLIA_SVG_WRAPPER_CASE("annotation",CommonWrapper)
    FOLIA_SVG_WRAPPER_CASE("annotation-xml",CommonWrapper)
    FOLIA_SVG_WRAPPER_CASE("mglyph",CommonMglyph)
    FOLIA_SVG_WRAPPER_CASE("TeXAtom",CommonTeXAtom)
    FOLIA_SVG_WRAPPER_CASE("text",CommonTextNode)
    if(kind==u"XML"){
        auto descriptor=registerSvgXmlNodeClass(bindings,constructor);
        result.emplace_back(property_host::debugText(kind),std::move(descriptor));
        return;
    }
    if(kind==u"html"){
        auto descriptor=registerSvgHtmlNodeClass(bindings,constructor);
        result.emplace_back(property_host::debugText(kind),std::move(descriptor));
        return;
    }
    FOLIA_SVG_WRAPPER_CASE("unknown",CommonWrapper)
#undef FOLIA_SVG_WRAPPER_CASE
    throw Exception("Unimplemented",u"SVG wrapper kind has no implemented canonical native class descriptor: "+
        std::u16string(kind));
}
}

common::CommonWrapperClassMap registerImplementedWrapperClasses(
    Context& context,ObjectRef actualSvgWrappers,
    common::CommonFactoryContextBindings& bindings,ContextPropertyBindings& properties){
    if(&context!=&bindings.context())
        throw Exception("TypeError",u"SVG wrapper class registration must share the Common Context");
    if(&context!=&properties.context())
        throw Exception("TypeError",u"SVG wrapper registration properties must share the Context");
    context.kind(actualSvgWrappers);
    common::CommonWrapperClassMap result;
    const auto keys=context.enumerableStringKeys(Value(actualSvgWrappers));
    result.reserve(keys.size());
    for(const auto& key:keys){
        const auto value=context.get(Value(actualSvgWrappers),Key(key));
        const auto* constructor=std::get_if<ObjectRef>(&value.data);
        if(!constructor)
            throw Exception("TypeError",u"SvgWrappers entry is not a constructor object");
        addForKind(result,bindings,key,*constructor,context,properties);
    }
    return result;
}
}
