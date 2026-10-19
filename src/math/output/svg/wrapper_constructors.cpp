/*************************************************************
 * Native Source constructor/prototype graph for SVG wrappers.
 *************************************************************/
module folia.math.output.svg.wrapper_constructors;
import folia.math.core.mml_tree.mml_node;
import folia.math.util.object_types;
import folia.math.util.option_types;
import std;

namespace folia::math::output::svg {
using namespace options;
namespace {

struct ClassSpec {
    std::u16string_view key;       // empty for an abstract/mixin constructor
    std::u16string_view name;
    std::u16string_view parent;
    bool exportedKind;
    std::uint32_t arity=0;
};

// This is the Source constructor declaration order from output/svg/Wrappers.ts
// and its imported Common mixins. Parent names are constructor identities, not
// kind aliases. The public defaultNodes record below contains only the 38
// SvgWrappers entries and keeps their exact own-property order.
constexpr ClassSpec classes[] = {
    {u"",u"AbstractWrapper",u"",false},
    {u"",u"CommonWrapper",u"AbstractWrapper",false},
    {u"",u"SvgWrapper",u"CommonWrapper",false},
    {u"",u"CommonMathMixin",u"SvgWrapper",false},
    {u"math",u"SvgMath",u"CommonMathMixin",true},
    {u"",u"CommonMrowMixin",u"SvgWrapper",false},
    {u"",u"CommonInferredMrowMixin",u"SvgMrow",false},
    {u"mrow",u"SvgMrow",u"CommonMrowMixin",true},
    {u"inferredMrow",u"SvgInferredMrowNTD",u"CommonInferredMrowMixin",true},
    {u"",u"CommonMiMixin",u"SvgWrapper",false},
    {u"mi",u"SvgMi",u"CommonMiMixin",true},
    {u"",u"CommonMoMixin",u"SvgWrapper",false},
    {u"mo",u"SvgMo",u"CommonMoMixin",true},
    {u"",u"CommonMnMixin",u"SvgWrapper",false},
    {u"mn",u"SvgMn",u"CommonMnMixin",true},
    {u"",u"CommonMsMixin",u"SvgWrapper",false},
    {u"ms",u"SvgMs",u"CommonMsMixin",true},
    {u"",u"CommonMtextMixin",u"SvgWrapper",false},
    {u"mtext",u"SvgMtext",u"CommonMtextMixin",true},
    {u"merror",u"SvgMerror",u"SvgWrapper",true},
    {u"",u"CommonMspaceMixin",u"SvgWrapper",false},
    {u"mspace",u"SvgMspace",u"CommonMspaceMixin",true},
    {u"",u"CommonMpaddedMixin",u"SvgWrapper",false},
    {u"mpadded",u"SvgMpadded",u"CommonMpaddedMixin",true},
    {u"mphantom",u"SvgMphantom",u"SvgWrapper",true},
    {u"",u"CommonMfracMixin",u"SvgWrapper",false},
    {u"mfrac",u"SvgMfrac",u"CommonMfracMixin",true},
    {u"",u"CommonMsqrtMixin",u"SvgWrapper",false},
    {u"msqrt",u"SvgMsqrt",u"CommonMsqrtMixin",true},
    {u"",u"CommonMrootMixin",u"SvgMsqrt",false},
    {u"mroot",u"SvgMroot",u"CommonMrootMixin",true},
    {u"",u"CommonMfencedMixin",u"SvgWrapper",false},
    {u"mfenced",u"SvgMfenced",u"CommonMfencedMixin",true},
    {u"",u"CommonScriptbaseMixin",u"SvgWrapper",false},
    {u"",u"SvgScriptbase",u"CommonScriptbaseMixin",false},
    {u"",u"CommonMsubMixin",u"SvgScriptbase",false},
    {u"msub",u"SvgMsub",u"CommonMsubMixin",true},
    {u"",u"CommonMsupMixin",u"SvgScriptbase",false},
    {u"msup",u"SvgMsup",u"CommonMsupMixin",true},
    {u"",u"CommonMsubsupMixin",u"SvgScriptbase",false},
    {u"msubsup",u"SvgMsubsup",u"CommonMsubsupMixin",true},
    {u"",u"CommonMunderMixin",u"SvgMsub",false},
    {u"munder",u"SvgMunder",u"CommonMunderMixin",true},
    {u"",u"CommonMoverMixin",u"SvgMsup",false},
    {u"mover",u"SvgMover",u"CommonMoverMixin",true},
    {u"",u"CommonMunderoverMixin",u"SvgMsubsup",false},
    {u"munderover",u"SvgMunderover",u"CommonMunderoverMixin",true},
    {u"",u"CommonMmultiscriptsMixin",u"SvgMsubsup",false},
    {u"mmultiscripts",u"SvgMmultiscripts",u"CommonMmultiscriptsMixin",true},
    {u"",u"CommonMtableMixin",u"SvgWrapper",false},
    {u"mtable",u"SvgMtable",u"CommonMtableMixin",true,3},
    {u"",u"CommonMtrMixin",u"SvgWrapper",false},
    {u"",u"CommonMlabeledtrMixin",u"SvgMtr",false},
    {u"mtr",u"SvgMtr",u"CommonMtrMixin",true},
    {u"mlabeledtr",u"SvgMlabeledtr",u"CommonMlabeledtrMixin",true},
    {u"",u"CommonMtdMixin",u"SvgWrapper",false},
    {u"mtd",u"SvgMtd",u"CommonMtdMixin",true},
    {u"",u"CommonMactionMixin",u"SvgWrapper",false},
    {u"maction",u"SvgMaction",u"CommonMactionMixin",true},
    {u"",u"CommonMencloseMixin",u"SvgWrapper",false},
    {u"menclose",u"SvgMenclose",u"CommonMencloseMixin",true},
    {u"",u"CommonSemanticsMixin",u"SvgWrapper",false},
    {u"semantics",u"SvgSemantics",u"CommonSemanticsMixin",true},
    {u"annotation",u"SvgAnnotation",u"SvgWrapper",true},
    {u"annotation-xml",u"SvgAnnotationXML",u"SvgWrapper",true},
    {u"",u"CommonXmlNodeMixin",u"SvgWrapper",false},
    {u"XML",u"SvgXmlNode",u"CommonXmlNodeMixin",true},
    {u"",u"CommonMglyphMixin",u"SvgWrapper",false},
    {u"mglyph",u"SvgMglyph",u"CommonMglyphMixin",true},
    {u"",u"CommonTeXAtomMixin",u"SvgWrapper",false},
    {u"TeXAtom",u"SvgTeXAtom",u"CommonTeXAtomMixin",true},
    {u"",u"CommonTextNodeMixin",u"SvgWrapper",false},
    {u"text",u"SvgTextNode",u"CommonTextNodeMixin",true},
    {u"html",u"SvgHtmlNode",u"SvgXmlNode",true}
};

struct WrapperEntrySpec { std::u16string_view key, constructor; };
constexpr WrapperEntrySpec wrapperEntries[] = {
    {u"math",u"SvgMath"},{u"mrow",u"SvgMrow"},
    {u"inferredMrow",u"SvgInferredMrowNTD"},{u"mi",u"SvgMi"},
    {u"mo",u"SvgMo"},{u"mn",u"SvgMn"},{u"ms",u"SvgMs"},
    {u"mtext",u"SvgMtext"},{u"merror",u"SvgMerror"},
    {u"mspace",u"SvgMspace"},{u"mpadded",u"SvgMpadded"},
    {u"mphantom",u"SvgMphantom"},{u"mfrac",u"SvgMfrac"},
    {u"msqrt",u"SvgMsqrt"},{u"mroot",u"SvgMroot"},
    {u"mfenced",u"SvgMfenced"},{u"msub",u"SvgMsub"},
    {u"msup",u"SvgMsup"},{u"msubsup",u"SvgMsubsup"},
    {u"munder",u"SvgMunder"},{u"mover",u"SvgMover"},
    {u"munderover",u"SvgMunderover"},{u"mmultiscripts",u"SvgMmultiscripts"},
    {u"mtable",u"SvgMtable"},{u"mtr",u"SvgMtr"},
    {u"mlabeledtr",u"SvgMlabeledtr"},{u"mtd",u"SvgMtd"},
    {u"maction",u"SvgMaction"},{u"menclose",u"SvgMenclose"},
    {u"semantics",u"SvgSemantics"},{u"annotation",u"SvgAnnotation"},
    {u"annotation-xml",u"SvgAnnotationXML"},{u"XML",u"SvgXmlNode"},
    {u"mglyph",u"SvgMglyph"},{u"TeXAtom",u"SvgTeXAtom"},
    {u"text",u"SvgTextNode"},{u"html",u"SvgHtmlNode"},
    {u"unknown",u"SvgWrapper"}
};

struct ConstructorEntry {
    std::u16string name;
    ObjectRef constructor;
    ObjectRef prototype;
};

}

struct SvgWrapperConstructorBindings::State : std::enable_shared_from_this<State> {
    Context& context;
    math::tex::ParserRuntime& parser;
    common::CommonFactoryContextBindings& factories;
    std::vector<ConstructorEntry> entries;
    ObjectRef wrappers{};

    State(Context& cx,math::tex::ParserRuntime& runtime,
          common::CommonFactoryContextBindings& bindings)
        :context(cx),parser(runtime),factories(bindings){
        if(&context!=&factories.context()||&context!=&parser.context())
            throw Exception("TypeError",u"SVG wrapper constructors must share the parser and Common Context");
    }

    ConstructorEntry* find(std::u16string_view name){
        auto it=std::find_if(entries.begin(),entries.end(),[&](const auto& e){return e.name==name;});
        return it==entries.end()?nullptr:&*it;
    }
    const ClassSpec* spec(std::u16string_view name)const{
        auto it=std::find_if(std::begin(classes),std::end(classes),[&](const auto& s){return s.name==name;});
        return it==std::end(classes)?nullptr:it;
    }
    ObjectRef allocate(std::u16string_view name,std::span<const Value> args){
        auto* entry=find(name);
        if(!entry)throw Exception("TypeError",u"SVG wrapper constructor is not registered");
        if(args.size()<2||!std::holds_alternative<ObjectRef>(args[0].data)||
           !std::holds_alternative<ObjectRef>(args[1].data))
            throw Exception("TypeError",u"SVG wrapper construction requires factory and MML node objects");
        auto factory=factories.resolve(std::get<ObjectRef>(args[0].data));
        auto& node=parser.resolveMmlNode(std::get<ObjectRef>(args[1].data));
        common::CommonWrapper* parent=nullptr;
        if(args.size()>2&&!std::holds_alternative<Undefined>(args[2].data)&&
           !std::holds_alternative<Null>(args[2].data)){
            const auto* parentObject=std::get_if<ObjectRef>(&args[2].data);
            if(!parentObject)throw Exception("TypeError",u"SVG wrapper parent is not a wrapper instance");
            parent=&factories.resolveWrapper(*parentObject);
        }
        const auto descriptor=factories.resolveWrapperClass(entry->constructor);
        // A direct `new SvgMrow(factory, node, parent)` invokes this exact
        // constructor body even when the factory's current defaultNodes map
        // selects a different class for that kind. Factory-driven wrapping
        // reaches the same descriptor through its map; constructor dispatch
        // itself must not perform a second kind lookup.
        const std::array<std::any,2> constructorArgs{std::any(&node),std::any(parent)};
        if(!descriptor||!descriptor->construct)
            throw Exception("TypeError",u"Svg wrapper constructor has no native class body");
        auto* product=descriptor->construct(*factory,constructorArgs);
        if(!product)throw Exception("TypeError",u"Svg wrapper constructor returned no product");
        return factory->objectFor(*product);
    }
    ObjectRef install(){
        if(wrappers!=ObjectRef{})return wrappers;
        auto self=shared_from_this();
        wrappers=context.object();
        entries.reserve(std::size(classes));
        for(const auto& item:classes){
            const auto name=std::u16string(item.name);
            auto entry=std::make_unique<ConstructorEntry>();
            entry->name=name;
            const auto weak=std::weak_ptr<State>(self);
            auto ctor=context.nativeConstructor(name,
                [weak,name](Context&,std::span<const Value> args)->ObjectRef{
                    auto state=weak.lock();
                    if(!state)throw Exception("Unimplemented",u"SVG wrapper constructor module has been released");
                    return state->allocate(name,args);
                });
            context.define(ctor,Key(u"name"),Descriptor::data(Value(name),false,false,true));
            context.define(ctor,Key(u"length"),Descriptor::data(
                Value(static_cast<double>(item.arity)),false,false,true));
            const auto proto=std::get<ObjectRef>(context.get(Value(ctor),Key(u"prototype")).data);
            entry->constructor=ctor;
            entry->prototype=proto;
            entries.push_back(std::move(*entry));
        }
        // Constructor and prototype inheritance are set only after all real
        // constructor objects exist, preserving forward references such as
        // SvgHtmlNode -> SvgXmlNode.
        for(const auto& item:classes){
            auto* current=find(item.name);
            if(!current)throw Exception("Unimplemented",u"SVG constructor graph has an unresolved class");
            if(item.parent.empty())continue;
            const auto* base=find(item.parent);
            if(!base)throw Exception("Unimplemented",u"SVG constructor graph has an unresolved parent");
            context.setPrototype(current->constructor,base->constructor);
            context.setPrototype(current->prototype,base->prototype);
        }
        for(const auto& item:wrapperEntries){
            auto* entry=find(item.constructor);
            if(!entry)throw Exception("Unimplemented",u"SVG wrapper map references a missing constructor");
            context.define(entry->constructor,Key(u"kind"),
                Descriptor::data(Value(std::u16string(item.key)),true,true,true));
            context.set(Value(wrappers),Key(std::u16string(item.key)),Value(entry->constructor));
        }
        return wrappers;
    }
};

SvgWrapperConstructorBindings::SvgWrapperConstructorBindings(
    options::Context& context,math::tex::ParserRuntime& parser,
    common::CommonFactoryContextBindings& factories)
    :state_(std::make_shared<State>(context,parser,factories)){}
SvgWrapperConstructorBindings::~SvgWrapperConstructorBindings()=default;
options::Context& SvgWrapperConstructorBindings::context()const{return state_->context;}
SvgWrapperConstructorSet SvgWrapperConstructorBindings::initialize(){
    SvgWrapperConstructorSet result;
    result.wrappers=state_->install();
    result.constructors.reserve(state_->entries.size());
    for(const auto& entry:state_->entries)
        result.constructors.emplace_back(entry.name,entry.constructor);
    return result;
}

SvgWrapperConstructorSet loadSvgWrapperConstructors(
    util::RuntimeModules& modules,math::tex::ParserRuntime& parser,
    common::CommonFactoryContextBindings& factories){
    auto owner=modules.getOrCreate<SvgWrapperConstructorBindings>(u"output/svg/Wrappers",
        [&]{
            auto created=std::make_shared<SvgWrapperConstructorBindings>(
                modules.context(),parser,factories);
            (void)created->initialize();
            return created;
        });
    return owner->initialize();
}
}
