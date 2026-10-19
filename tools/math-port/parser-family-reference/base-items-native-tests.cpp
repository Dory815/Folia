// Runnable native candidate for direct BaseItems reductions. This fixture
// boots the actual parser runtime and package, then calls actual factory items.
#include "../../../tests/support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.stack_item;
import folia.math.input.tex.stack_property;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.physics.items;
import folia.math.input.tex.node_util;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.option_functions;
import folia.math.util.options;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace tex=folia::math::tex;
namespace opts=folia::math::options;

struct RuntimeFixture {
    opts::Context context;
    OptionFunctionBindings functions{context};
    folia::math::regexp_host::RegExpBindings regex{context};
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule inputJax{context};
    folia::math::core::FindMathModule findMath{context};
    tex::FindTeXModule findTeX{context,findMath,regex};
    tex::ParserRuntime runtime{context,functions,regex,unicode,inputJax,findTeX};
    std::shared_ptr<folia::math::mml::MmlFactory> mml=std::make_shared<folia::math::mml::MmlFactory>();
    tex::TeXRef jax;

    RuntimeFixture() {
        runtime.initializeParserRuntime();
        (void)tex::BasePackage::initializeModule(runtime);
        jax=tex::TeX::make(runtime);
        (void)runtime.bindMmlFactory(mml);
        jax->setMmlFactory(mml);
    }
    tex::ParseOptions& parseOptions() {
        auto binding=jax->parseOptions();
        const auto* value=std::get_if<std::shared_ptr<tex::ParseOptions>>(&binding);
        if(!value||!*value)throw opts::Exception("TypeError",u"TeX ParseOptions were not initialized");
        return **value;
    }
    tex::StackItemFactory& itemFactory() {
        const auto* value=std::get_if<std::shared_ptr<tex::StackItemFactory>>(&parseOptions().itemFactory);
        if(!value||!*value)throw opts::Exception("TypeError",u"ParseOptions item factory was not initialized");
        return **value;
    }
};

suite base_items_native=[] {
    "stack_item_class_metadata_preserves_own_and_inherited_error_tables"_test=[] {
        RuntimeFixture fixture;
        auto& context=fixture.context;
        const auto module=fixture.runtime.baseItemsModule;
        const auto base=context.get(fixture.runtime.stackItemModule,opts::Key(u"BaseItem"));
        const auto baseErrors=context.get(base,opts::Key(u"errors"));
        for(const auto name:std::array{u"OpenItem",u"SubsupItem",u"LeftItem"}) {
            const auto constructor=context.get(module,opts::Key(name));
            expect(context.hasOwn(constructor,opts::Key(u"errors")));
            const auto errors=context.get(constructor,opts::Key(u"errors"));
            expect(context.prototype(std::get<opts::ObjectRef>(errors.data))==
                std::get<opts::ObjectRef>(baseErrors.data));
        }
        const auto start=context.get(module,opts::Key(u"StartItem"));
        expect(!context.hasOwn(start,opts::Key(u"errors")));
        expect(context.get(start,opts::Key(u"errors"))==baseErrors);

        const auto type=tex::AutoOpen::initializeModule(fixture.runtime);
        const auto constructor=fixture.runtime.itemClassIdentities.find(type);
        expect(constructor.has_value());
        if(!constructor)return;
        expect(context.hasOwn(opts::Value(*constructor),opts::Key(u"errors")));
        const auto errors=context.get(opts::Value(*constructor),opts::Key(u"errors"));
        expect(context.prototype(std::get<opts::ObjectRef>(errors.data))==
            std::get<opts::ObjectRef>(baseErrors.data));
        const auto stop=context.get(errors,opts::Key(u"stop"));
        expect(context.get(stop,opts::Key(u"0"))==opts::Value(u"ExtraOrMissingDelims"));
        expect(context.get(stop,opts::Key(u"1"))==
            opts::Value(u"Extra open or missing close delimiter"));
    };
    "all_twenty_six_base_items_use_the_registered_source_classes"_test=[] {
        RuntimeFixture fixture;
        struct Expected { std::string_view className,kind; double arity; bool open; bool close; bool final; bool copyEnv; };
        constexpr Expected expected[]={
            {"StartItem","start",2,true,false,false,true},{"StopItem","stop",0,false,true,false,true},
            {"OpenItem","open",0,true,false,false,true},{"CloseItem","close",0,false,true,false,true},
            {"NullItem","null",0,false,false,false,true},{"PrimeItem","prime",0,false,false,false,true},
            {"SubsupItem","subsup",0,false,false,false,true},{"OverItem","over",1,false,true,false,true},
            {"LeftItem","left",2,true,false,false,true},{"Middle","middle",3,false,true,false,true},
            {"RightItem","right",3,false,true,false,true},{"BreakItem","break",3,false,false,false,true},
            {"BeginItem","begin",0,true,false,false,true},{"EndItem","end",0,false,true,false,true},
            {"StyleItem","style",0,false,false,false,true},{"PositionItem","position",0,false,false,false,true},
            {"CellItem","cell",0,false,true,false,true},{"MmlItem","mml",0,false,false,true,true},
            {"FnItem","fn",0,false,false,false,true},{"NotItem","not",0,false,false,false,true},
            {"NonscriptItem","nonscript",0,false,false,false,true},{"DotsItem","dots",0,false,false,false,true},
            {"ArrayItem","array",0,true,false,false,false},{"EqnArrayItem","eqnarray",1,true,false,false,false},
            {"EquationItem","equation",1,true,false,false,true},{"MstyleItem","mstyle",3,true,false,false,true}
        };
        for(const auto& row:expected){
            const std::u16string className(row.className.begin(),row.className.end());
            const auto constructor=fixture.context.get(fixture.runtime.baseItemsModule,opts::Key(className));
            const auto* classObject=std::get_if<opts::ObjectRef>(&constructor.data);
            expect(classObject!=nullptr);
            if(classObject){
                const auto length=fixture.context.get(constructor,opts::Key(u"length"));
                expect((length==opts::Value(row.arity))==true);
                const auto name=fixture.context.get(constructor,opts::Key(u"name"));
                expect((name==opts::Value(className))==true);
                const auto prototype=fixture.context.get(constructor,opts::Key(u"prototype"));
                const auto* prototypeObject=std::get_if<opts::ObjectRef>(&prototype.data);
                expect(prototypeObject!=nullptr);
                if(prototypeObject){
                    const auto kind=fixture.context.get(prototype,opts::Key(u"kind"));
                    expect((kind==opts::Value(std::u16string(row.kind.begin(),row.kind.end())))==true);
                    const auto kindDescriptor=fixture.context.descriptor(*prototypeObject,opts::Key(u"kind"));
                    expect(kindDescriptor&&kindDescriptor->accessor&&!kindDescriptor->enumerable&&kindDescriptor->configurable);
                }
            }
            auto* item=fixture.itemFactory().create(row.kind);
            expect(item!=nullptr);
            if(!item)continue;
            expect(eq(item->kind(),std::string(row.kind)));
            expect(item->isOpen()==row.open);
            expect(item->isClose()==row.close);
            expect(item->isFinal()==row.final);
            expect(item->copyEnv()==row.copyEnv);
        }
    };

    "base_item_source_class_reflection_uses_live_context_identities"_test=[] {
        std::string stage="fixture construction";
        try {
        RuntimeFixture fixture;
        // StackItem.ts owns BaseItem; BaseItems.ts exports its concrete
        // subclasses from a separate package module.
        stage="BaseItem constructor/module lookup";
        const auto module=fixture.runtime.stackItemModule;
        const auto baseCtor=fixture.context.get(module,opts::Key(u"BaseItem"));
        const auto basePrototype=fixture.context.get(baseCtor,opts::Key(u"prototype"));
        const auto* basePrototypeObject=std::get_if<opts::ObjectRef>(&basePrototype.data);
        expect(basePrototypeObject!=nullptr);
        if(basePrototypeObject){
            const auto constructorDescriptor=fixture.context.descriptor(*basePrototypeObject,opts::Key(u"constructor"));
            expect(constructorDescriptor&&constructorDescriptor->enumerable&&constructorDescriptor->writable&&constructorDescriptor->configurable);
            expect((fixture.context.get(basePrototype,opts::Key(u"constructor"))==baseCtor)==true);
            for(const auto member:{u"checkItem",u"getProperty",u"setProperty",u"setProperties",u"clearEnv",u"getErrors",u"addLatexItem"}){
                const auto descriptor=fixture.context.descriptor(*basePrototypeObject,opts::Key(member));
                expect(descriptor&&!descriptor->accessor&&std::holds_alternative<opts::ObjectRef>(descriptor->value.data))
                    << "BaseItem Source prototype callable is missing";
            }
            const auto env=fixture.context.descriptor(*basePrototypeObject,opts::Key(u"env"));
            expect(env&&env->accessor&&!env->enumerable&&env->configurable&&env->getter&&env->setter);
        }
        stage="BaseItem prototype and method descriptors";
        const auto packageModule=fixture.runtime.baseItemsModule;
        for(const auto name:{u"OpenItem",u"SubsupItem",u"LeftItem"}){
            const auto constructor=fixture.context.get(packageModule,opts::Key(name));
            const auto errors=fixture.context.get(constructor,opts::Key(u"errors"));
            const auto* errorObject=std::get_if<opts::ObjectRef>(&errors.data);
            expect(errorObject!=nullptr);
            if(errorObject){
                const auto parent=fixture.context.prototype(*errorObject);
                const auto baseErrors=fixture.context.get(baseCtor,opts::Key(u"errors"));
                const auto* baseErrorObject=std::get_if<opts::ObjectRef>(&baseErrors.data);
                expect(parent&&baseErrorObject&&*parent==*baseErrorObject)
                    << "subclass errors must inherit the canonical BaseItem errors object";
            }
        }
        stage="StartItem factory construction";
        const auto constructor=fixture.context.get(packageModule,opts::Key(u"StartItem"));
        const auto* constructorObject=std::get_if<opts::ObjectRef>(&constructor.data);
        expect(constructorObject!=nullptr);
        if(!constructorObject)return;
        const Property initial=PropertyList{{"isInner",false}};
        const std::array<std::any,1> initialArguments{initial};
        auto* item=dynamic_cast<tex::StartItem*>(fixture.itemFactory().create("start",initialArguments));
        expect(item!=nullptr);
        if(!item)return;
        const auto identity=fixture.runtime.stackItemIdentities.find(std::static_pointer_cast<tex::StackItemContext>(fixture.itemFactory().reference(item)));
        expect(identity.has_value());
        if(identity){
            stage="live global field and factory identity";
            const auto global=fixture.context.get(*identity,opts::Key(u"global"));
            expect((global==fixture.runtime.properties.to(item->global))==true)
                << "the Context global data property must view the actual BaseItem field";
            expect(fixture.context.get(global,opts::Key(u"isInner"))==opts::Value(false));
            const auto factory=fixture.context.get(*identity,opts::Key(u"factory"));
            expect(std::holds_alternative<opts::ObjectRef>(factory.data));
            const auto startI=fixture.context.get(*identity,opts::Key(u"startI"));
            expect(startI==opts::Value(0));
        }
        } catch(const opts::Exception& error) {
            std::cerr<<"base_item_source_class_reflection stage="<<stage
                     <<" error="<<error.name<<":"<<property_host::debugText(error.message)<<"\n";
            throw;
        } catch(const std::exception& error) {
            std::cerr<<"base_item_source_class_reflection stage="<<stage
                     <<" std-error="<<error.what()<<"\n";
            throw;
        } catch(...) {
            std::cerr<<"base_item_source_class_reflection stage="<<stage<<" unknown error\n";
            throw;
        }
    };

    "base_item_factory_args_preserve_native_scalar_and_nullish_slots"_test=[] {
        RuntimeFixture fixture;
        auto& factory=fixture.itemFactory();
        const auto createStart=[&](std::any value){
            const std::array<std::any,1> args{std::move(value)};
            auto* item=dynamic_cast<tex::StartItem*>(factory.create("start",tree::FactoryArgs(args.data(),args.size())));
            expect(item!=nullptr);
            return item;
        };
        auto* omitted=dynamic_cast<tex::StartItem*>(factory.create("start"));
        expect(omitted!=nullptr);
        if(omitted)expect(omitted->global.missing());
        if(auto* item=createStart(std::any(true)))expect(item->global==Property(true));
        if(auto* item=createStart(std::any(7.0)))expect(item->global==Property(7.0));
        if(auto* item=createStart(std::any(std::string("source"))))expect(item->global==Property("source"));
        if(auto* item=createStart(std::any(std::u16string(u"source"))))expect(item->global==Property("source"));
        if(auto* item=createStart(std::any(std::u16string(u"\u03BB\U0001D11E"))))
            expect(item->global==property_host::debugText(u"\u03BB\U0001D11E"));
        if(auto* item=createStart(std::any(Property("property"))))expect(item->global==Property("property"));
        if(auto* item=createStart(std::any(opts::Value(false))))expect(item->global==Property(false));
        const auto object=fixture.context.object();
        fixture.context.set(object,opts::Key(u"marker"),opts::Value(17));
        if(auto* item=createStart(std::any(opts::Value(object))))
            expect(fixture.runtime.properties.to(item->global)==opts::Value(object))
                << "Context-backed FactoryArgs values must retain their original object identity";
        if(auto* item=createStart(std::any(opts::Undefined{})))expect(item->global.missing());
        if(auto* item=createStart(std::any(opts::Null{})))expect(std::holds_alternative<PropertyNull>(item->global.value));
        if(auto* item=createStart(std::any(PropertyNull{})))expect(std::holds_alternative<PropertyNull>(item->global.value));

        bool opaqueRejected=false;
        try{(void)createStart(std::any(std::array<int,1>{1}));}
        catch(const opts::Exception& error){opaqueRejected=error.name=="TypeError";}
        expect(opaqueRejected)<<"opaque FactoryArgs values must not be stringified or accepted";
    };
    "base_item_constructor_callbacks_preserve_source_arguments_and_defaults"_test=[] {
        RuntimeFixture fixture;
        auto& factory=fixture.itemFactory();
        const Property global=PropertyList{{"isInner",false}};
        const std::array<std::any,1> startArgs{global};
        auto* start=dynamic_cast<tex::StartItem*>(factory.create("start",startArgs));
        expect(start!=nullptr);
        if(start){
            expect(start->global==global);
            expect((property_host::get(start->global,"isInner")==Property(false))==true);
            expect(start->isOpen());
        }
        const std::array<std::any,1> leftArgs{Property("(")};
        auto* left=dynamic_cast<tex::LeftItem*>(factory.create("left",leftArgs));
        expect(left!=nullptr);
        if(left)expect((tex::stack_property_host::scalar(left->getProperty("delim"))==Property("("))==true);
        const std::array<std::any,2> middleArgs{Property("|"),Property("red")};
        auto* middle=dynamic_cast<tex::Middle*>(factory.create("middle",middleArgs));
        expect(middle!=nullptr);
        if(middle){
            expect((tex::stack_property_host::scalar(middle->getProperty("delim"))==Property("|"))==true);
            expect((tex::stack_property_host::scalar(middle->getProperty("color"))==Property("red"))==true);
        }
        const std::array<std::any,2> rightArgs{Property(")"),Property("blue")};
        auto* right=dynamic_cast<tex::RightItem*>(factory.create("right",rightArgs));
        expect(right!=nullptr);
        if(right){
            expect((tex::stack_property_host::scalar(right->getProperty("delim"))==Property(")"))==true);
            expect((tex::stack_property_host::scalar(right->getProperty("color"))==Property("blue"))==true);
        }
        const std::array<std::any,2> breakArgs{Property("newline"),Property(true)};
        auto* linebreak=dynamic_cast<tex::BreakItem*>(factory.create("break",breakArgs));
        expect(linebreak!=nullptr);
        if(linebreak){
            expect((tex::stack_property_host::scalar(linebreak->getProperty("linebreak"))==Property("newline"))==true);
            expect((tex::stack_property_host::scalar(linebreak->getProperty("insert"))==Property(true))==true);
        }
        const Property styleAttributes=PropertyList{{"mathcolor","red"}};
        const std::array<std::any,2> styleArgs{styleAttributes,Property("fixture-style")};
        auto* style=dynamic_cast<tex::MstyleItem*>(factory.create("mstyle",styleArgs));
        expect(style!=nullptr);
        if(style){
            expect(style->attrList==styleAttributes);
            expect((tex::stack_property_host::scalar(style->getProperty("name"))==Property("fixture-style"))==true);
        }
        const std::array<std::any,3> eqnArgs{Property("fixture-tag"),Property(true),Property(false)};
        auto* eqn=dynamic_cast<tex::EqnArrayItem*>(factory.create("eqnarray",eqnArgs));
        expect(eqn!=nullptr);
        if(eqn)expect(eqn->maxrow==Property(0));
        const std::array<std::any,1> equationArgs{Property("fixture-tag")};
        auto* equation=dynamic_cast<tex::EquationItem*>(factory.create("equation",equationArgs));
        expect(equation!=nullptr);
        if(equation)expect(equation->isOpen());
        auto* over=dynamic_cast<tex::OverItem*>(factory.create("over"));
        expect(over!=nullptr);
        if(over)expect((tex::stack_property_host::scalar(over->getProperty("name"))==Property("\\over"))==true);
    };
    "base_item_checkitem_fallbacks_match_source_return_shapes"_test=[] {
        RuntimeFixture fixture;
        auto& factory=fixture.itemFactory();
        auto* nullItem=factory.create("null");
        auto* openItem=factory.create("open");
        auto isSuccessful=[](const tex::CheckType& result){
            const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&result);
            return tuple&&*tuple&&property_host::truthy((*tuple)->success);
        };
        const auto startResult=factory.create("start")->checkItem(nullItem);
        const auto* commonSuccess=std::get_if<std::shared_ptr<tex::CheckTuple>>(&startResult);
        expect(isSuccessful(startResult));
        const std::array<tex::CheckType,4> fallbacks{
            openItem->checkItem(nullItem),factory.create("begin")->checkItem(nullItem),
            factory.create("style")->checkItem(nullItem),factory.create("position")->checkItem(nullItem)};
        for(const auto& result:fallbacks){
            const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&result);
            expect(isSuccessful(result));
            expect(commonSuccess&&tuple&&*commonSuccess==*tuple);
        }
        const auto nonscriptResult=factory.create("nonscript")->checkItem(nullItem);
        expect(isSuccessful(nonscriptResult));
        const auto* nonscriptTuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&nonscriptResult);
        expect(commonSuccess&&nonscriptTuple&&*commonSuccess!=*nonscriptTuple);
        auto* fn=factory.create("fn");
        const std::array<folia::math::mml::NodeValue,1> fnTop{fixture.mml->create("mi",folia::math::tree::FactoryArgs{})};
        fn->Push(fnTop);
        const std::array<tex::CheckType,4> shortCircuits{
            fn->checkItem(openItem),factory.create("not")->checkItem(openItem),
            factory.create("dots")->checkItem(openItem),factory.create("subsup")->checkItem(openItem)};
        for(const auto& result:shortCircuits){
            const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&result);
            expect(isSuccessful(result));
            expect(commonSuccess&&tuple&&*commonSuccess==*tuple);
        }

        auto* linebreak=dynamic_cast<tex::BreakItem*>(factory.create("break",std::array<std::any,2>{Property("newline"),Property(true)}));
        expect(linebreak!=nullptr);
        if(!linebreak)return;
        const auto inserted=linebreak->checkItem(nullItem);
        const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&inserted);
        expect(tuple&&*tuple&&property_host::truthy((*tuple)->success));
        if(!tuple||!*tuple)return;
        const auto* entries=std::get_if<tex::CheckList>(&(*tuple)->items);
        expect(entries&&*entries&&(*entries)->size()==2);
        if(!entries||!*entries||(*entries)->size()!=2)return;
        const auto* wrapped=std::get_if<tex::StackItemRef>(&(*entries)->at(0));
        const auto* retained=std::get_if<tex::StackItemRef>(&(*entries)->at(1));
        auto* wrappedItem=wrapped&&*wrapped?dynamic_cast<tex::BaseItem*>((*wrapped).get()):nullptr;
        expect(wrappedItem&&wrappedItem->kind()=="mml");
        expect(retained&&*retained&&retained->get()==nullItem);
        if(wrappedItem){
            const auto first=wrappedItem->First();
            expect(tex::NodeUtil::instance().isType(first,"mspace"));
            expect((tex::NodeUtil::instance().getAttribute(first,"linebreak")==Property("newline"))==true);
        }
    };
    "base_items_classes_are_reached_by_real_texparser_inputs"_test=[] {
        RuntimeFixture fixture;
        struct Case { std::u16string_view source; std::string_view rootKind; std::string_view innerKind; };
        constexpr Case cases[]={
            {u"x'","msup",{}},{u"x''","msup",{}},{u"x_1","msubsup",{}},
            {u"\\not=","mo",{}},{u"a+\\ldots+b+\\cdots+c","inferredMrow",{}},
            {u"\\sin x","inferredMrow",{}},{u"\\displaystyle x","mstyle",{}},
            {u"\\raise{2pt}{x}+\\lower{1pt}{y}","inferredMrow",{}},
            {u"x\\nonscript\\,y","inferredMrow",{}},{u"\\left(x\\middle|y\\right)","mrow",{}},
            {u"{a\\over b}","TeXAtom","mfrac"},
            {u"\\begin{array}{cc}a&b\\\\c&d\\end{array}","mstyle","mtable"},
            {u"\\begin{array}{|c|}a\\\\b\\end{array}","mstyle","mtable"},
            {u"\\begin{array}{c@{\\quad}c}a&b\\end{array}","mstyle","mtable"}
        };
        const auto hasKind=[&](auto&& self,folia::math::mml::NodeValue node,std::string_view kind)->bool{
            if(!node.isNode())return false;
            if(node.requireNode()->kind()==kind)return true;
            const auto children=node.requireNode()->childNodes;
            if(children)for(std::size_t i=0;i<children->size();++i)
                if(self(self,children->get(i),kind))return true;
            return false;
        };
        for(const auto& row:cases){
            try {
                auto parser=tex::TexParser::make(std::u16string(row.source),PropertyList{},fixture.parseOptions());
                parser->Parse();
                const auto result=parser->mml();
                expect(result.isNode()) << "input=" << property_host::debugText(row.source);
                const std::string actualKind=result.isNode()?result.requireNode()->kind():"<non-node>";
                expect(tex::NodeUtil::instance().isType(result,row.rootKind))
                    << "input=" << property_host::debugText(row.source)
                    << " expected-root=" << row.rootKind << " actual-root=" << actualKind;
                if(!row.innerKind.empty())
                    expect(hasKind(hasKind,result,row.innerKind))
                        << "input=" << property_host::debugText(row.source)
                        << " expected descendant=" << row.innerKind;
            } catch(const opts::Exception& error) {
                expect(false) << "input=" << property_host::debugText(row.source) << " exception="
                              << error.name << ":" << property_host::debugText(error.message);
            } catch(const tex::TexError& error) {
                expect(false) << "input=" << property_host::debugText(row.source) << " TexError="
                              << property_host::debugText(error.id) << ":" << property_host::debugText(error.message);
            }
        }
    };
    "mstyle_and_equation_items_preserve_their_environment_branches"_test=[] {
        RuntimeFixture fixture;
        auto& factory=fixture.itemFactory();
        const Property styleAttributes=PropertyList{{"mathcolor","red"}};
        const std::array<std::any,2> styleArgs{styleAttributes,Property("fixture-style")};
        auto* style=dynamic_cast<tex::MstyleItem*>(factory.create("mstyle",styleArgs));
        auto* end=factory.create("end");
        expect(style!=nullptr&&end!=nullptr) << "Mstyle/End factory products";
        if(!style||!end)return;
        (void)end->setProperty("name",tex::StackProperty(Property("fixture-style")));
        const std::array<folia::math::mml::NodeValue,1> contents{fixture.mml->create("mi",folia::math::tree::FactoryArgs{})};
        style->Push(contents);
        const auto wrapped=style->checkItem(end);
        const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&wrapped);
        expect(tuple&&*tuple&&property_host::truthy((*tuple)->success)) << "Mstyle matching-end reduction";
        if(!tuple||!*tuple)return;
        const auto* items=std::get_if<tex::CheckList>(&(*tuple)->items);
        expect(items&&*items&&(*items)->size()==1) << "Mstyle reduction item count";
        if(!items||!*items||(*items)->size()!=1)return;
        const auto* nodeValue=std::get_if<folia::math::mml::NodeValue>(&(*items)->front());
        expect(nodeValue&&nodeValue->isNode()) << "Mstyle reduction returns the direct MML node";
        if(nodeValue&&nodeValue->isNode()){
            expect(tex::NodeUtil::instance().isType(*nodeValue,"mstyle")) << "Mstyle generated node kind";
            expect((tex::NodeUtil::instance().getAttribute(*nodeValue,"mathcolor")==Property("red"))==true) << "Mstyle generated attribute";
            expect((*nodeValue).requireNode()->childNodes->size()==1) << "Mstyle generated child count";
        }

        const std::array<std::any,1> equationArgs{Property("fixture-tag")};
        auto* equation=dynamic_cast<tex::EquationItem*>(factory.create("equation",equationArgs));
        auto* stop=factory.create("stop");
        expect(equation!=nullptr&&stop!=nullptr);
        if(!equation||!stop)return;
        std::string error;
        try { (void)equation->checkItem(stop); }
        catch(const tex::TexError& caught){error=property_host::debugText(caught.id)+"|"+property_host::debugText(caught.message);}
        expect(error=="EnvMissingEnd|Missing \\end{}");
    };
    "array_rowspacing_and_create_mml_preserve_source_side_effects"_test=[] {
        RuntimeFixture fixture;
        auto* item=dynamic_cast<tex::ArrayItem*>(fixture.itemFactory().create("array"));
        expect(item!=nullptr);
        if(!item)return;
        const auto* table=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->table);
        expect(table&&*table);
        if(!table||!*table)return;
        (*table)->push_back(fixture.mml->create("mtr",folia::math::tree::FactoryArgs{}));
        (*table)->push_back(fixture.mml->create("mtr",folia::math::tree::FactoryArgs{}));
        const auto definitions=std::get<opts::ObjectRef>(item->arraydef.data);
        fixture.context.set(opts::Value(definitions),opts::Key(u"rowspacing"),opts::Value(u"1"));
        item->addRowSpacing(u"0.5em");
        expect(fixture.context.get(opts::Value(definitions),opts::Key(u"rowspacing"))==opts::Value(u"1 0.6em"));
        fixture.context.set(opts::Value(definitions),opts::Key(u"scriptlevel"),opts::Value(1));
        fixture.context.set(opts::Value(std::get<opts::ObjectRef>(item->breakAlign.data)),opts::Key(u"table"),opts::Value(u"top"));
        (void)item->setProperty("arrayPadding",tex::StackProperty(Property("1em")));
        const auto result=item->createMml();
        expect(tex::NodeUtil::instance().isType(result,"mstyle"));
        expect(!fixture.context.hasOwn(definitions,opts::Key(u"scriptlevel")));
        const auto outerChildren=result.requireNode("childNodes")->childNodes;
        expect(outerChildren&&outerChildren->size()==1);
        if(!outerChildren||outerChildren->size()!=1)return;
        const auto inferred=outerChildren->getProperty("0");
        expect(tex::NodeUtil::instance().isType(inferred,"inferredMrow"));
        const auto inferredChildren=inferred.requireNode("childNodes")->childNodes;
        expect(inferredChildren&&inferredChildren->size()==1);
        if(!inferredChildren||inferredChildren->size()!=1)return;
        const auto tableNode=inferredChildren->getProperty("0");
        expect(tex::NodeUtil::instance().isType(tableNode,"mtable"));
        expect(property_host::truthy(tex::NodeUtil::instance().getProperty(tableNode,"smallmatrix")));
        expect((tex::NodeUtil::instance().getAttribute(tableNode,"data-break-align")==Property("top"))==true);
        expect((tex::NodeUtil::instance().getAttribute(tableNode,"framespacing")==Property("1em"))==true);
        expect((tex::NodeUtil::instance().getAttribute(tableNode,"rowspacing")==Property("1 0.6em"))==true);
    };
    "array_handle_frame_covers_solid_and_mixed_source_paths"_test=[] {
        RuntimeFixture fixture;
        auto makeFrame=[&](std::initializer_list<std::pair<std::u16string_view,std::u16string_view>> sides){
            auto* item=dynamic_cast<tex::ArrayItem*>(fixture.itemFactory().create("array"));
            if(!item)throw opts::Exception("TypeError",u"Array factory returned a non-ArrayItem");
            const auto frame=std::get<opts::ObjectRef>(item->frame.data);
            for(const auto& [side,style]:sides){
                const auto tuple=fixture.context.array(2);
                fixture.context.set(opts::Value(tuple),opts::Key(u"0"),opts::Value(std::u16string(side)));
                fixture.context.set(opts::Value(tuple),opts::Key(u"1"),opts::Value(std::u16string(style)));
                (void)fixture.context.arrayPush(frame,std::array<opts::Value,1>{opts::Value(tuple)});
            }
            return item->createMml();
        };
        const auto partial=makeFrame({{u"top",u"solid"},{u"right",u"solid"}});
        expect(tex::NodeUtil::instance().isType(partial,"menclose"));
        expect((tex::NodeUtil::instance().getAttribute(partial,"notation")==Property("top right"))==true);
        expect((tex::NodeUtil::instance().getAttribute(partial,"data-padding")==Property(0))==true);
        const auto allSolid=makeFrame({{u"top",u"solid"},{u"right",u"solid"},{u"bottom",u"solid"},{u"left",u"solid"}});
        expect(tex::NodeUtil::instance().isType(allSolid,"mtable"));
        expect((tex::NodeUtil::instance().getAttribute(allSolid,"frame")==Property("solid"))==true);
        expect(property_host::nullish(tex::NodeUtil::instance().getAttribute(allSolid,"data-frame-styles")));
        const auto mixed=makeFrame({{u"top",u"dashed"},{u"left",u"solid"}});
        expect(tex::NodeUtil::instance().isType(mixed,"mtable"));
        expect((tex::NodeUtil::instance().getAttribute(mixed,"data-frame-styles")==Property("dashed none none solid"))==true);
    };
    "eqnarray_endentry_missing_columnalign_throws_before_base_reduction"_test=[] {
        RuntimeFixture fixture;
        const std::array<std::any,3> args{Property("fixture-tag"),Property(true),Property(false)};
        auto* item=dynamic_cast<tex::EqnArrayItem*>(fixture.itemFactory().create("eqnarray",args));
        expect(item!=nullptr);
        if(!item)return;
        std::string error;
        try { item->EndEntry(); }
        catch(const opts::Exception& caught){error=caught.name+"|"+property_host::debugText(caught.message);}
        const auto* row=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->row);
        expect(error=="TypeError|Cannot read properties of undefined (reading 'split')");
        expect(row&&*row&&(*row)->size()==0);
        expect(item->Size()==0);
    };
    "stack_item_factory_binds_the_actual_parse_options_service"_test=[] {
        RuntimeFixture fixture;
        const auto* owner=std::get_if<std::shared_ptr<tex::StackItemFactory>>(&fixture.parseOptions().itemFactory);
        expect(owner!=nullptr&&*owner);
        if(!owner||!*owner)return;
        const auto constructor=std::get_if<opts::ObjectRef>(&fixture.runtime.stackItemFactoryClass.data);
        expect(constructor!=nullptr);
        if(!constructor)return;
        const auto object=fixture.runtime.stackItemFactoryIdentities.find(owner->get());
        expect(object.has_value());
        if(!object)return;
        const auto prototype=fixture.context.get(opts::Value(*constructor),opts::Key(u"prototype"));
        const auto* prototypeObject=std::get_if<opts::ObjectRef>(&prototype.data);
        expect(prototypeObject!=nullptr);
        if(!prototypeObject)return;
        const auto actualPrototype=fixture.context.prototype(*object);
        expect(actualPrototype&&prototypeObject&&*actualPrototype==*prototypeObject);
        expect(fixture.runtime.stackItemFactoryIdentities.resolve(*object)==owner->get());
        const auto defaults=fixture.context.get(opts::Value(*constructor),opts::Key(u"DefaultStackItems"));
        expect(static_cast<bool>(fixture.context.enumerableStringKeys(defaults)==std::vector<std::u16string>({u"base"})));
    };
    "null_item_constructor_call_initializes_the_supplied_receiver"_test=[] {
        std::string stage="fixture construction";
        try {
        RuntimeFixture fixture;
        stage="NullItem constructor lookup";
        const auto constructorValue=fixture.context.get(fixture.runtime.baseItemsModule,opts::Key(u"NullItem"));
        const auto* constructor=std::get_if<opts::ObjectRef>(&constructorValue.data);
        expect(constructor!=nullptr);
        if(!constructor)return;
        stage="NullItem prototype and receiver construction";
        const auto prototypeValue=fixture.context.get(constructorValue,opts::Key(u"prototype"));
        const auto* prototype=std::get_if<opts::ObjectRef>(&prototypeValue.data);
        expect(prototype!=nullptr);
        if(!prototype)return;
        const auto receiver=fixture.context.objectWithPrototype(*prototype);
        opts::Value result(opts::Undefined{});
        std::string callError;
        stage="NullItem ordinary constructor call";
        try { result=fixture.context.call(*constructor,opts::Value(receiver)); }
        catch(const opts::Exception& error) {
            callError=error.name+":"+property_host::debugText(error.message);
        }
        expect(callError.empty())<<"Source constructor call failed: "<<callError;
        expect(result==opts::Value(receiver));
        stage="NullItem bound instance fields";
        const auto reference=fixture.runtime.stackItemIdentities.resolve(receiver);
        const auto* item=dynamic_cast<const tex::NullItem*>(reference.get());
        expect(item!=nullptr);
        if(!item)return;
        expect(item->kind()==std::string("null"));
        expect(!item->isOpen()&&!item->isClose()&&!item->isFinal());
        expect(item->copyEnv());
        const auto firstIdentity=reference;
        expect(fixture.context.call(*constructor,opts::Value(receiver))==opts::Value(receiver));
        const auto secondIdentity=fixture.runtime.stackItemIdentities.resolve(receiver);
        expect(firstIdentity==secondIdentity);
        } catch(const opts::Exception& error) {
            std::cerr<<"null_item_constructor_call stage="<<stage
                     <<" error="<<error.name<<":"<<property_host::debugText(error.message)<<"\n";
            throw;
        } catch(const std::exception& error) {
            std::cerr<<"null_item_constructor_call stage="<<stage
                     <<" std-error="<<error.what()<<"\n";
            throw;
        } catch(...) {
            std::cerr<<"null_item_constructor_call stage="<<stage<<" unknown error\n";
            throw;
        }
    };
    "null_item_constructor_preserves_source_order_on_readonly_start_index"_test=[] {
        RuntimeFixture fixture;
        const auto constructorValue=fixture.context.get(fixture.runtime.baseItemsModule,opts::Key(u"NullItem"));
        const auto* constructor=std::get_if<opts::ObjectRef>(&constructorValue.data);
        expect(constructor!=nullptr);
        if(!constructor)return;
        const auto prototypeValue=fixture.context.get(constructorValue,opts::Key(u"prototype"));
        const auto* prototype=std::get_if<opts::ObjectRef>(&prototypeValue.data);
        expect(prototype!=nullptr);
        if(!prototype)return;
        const auto receiver=fixture.context.objectWithPrototype(*prototype);
        fixture.context.define(receiver,opts::Key(u"startI"),
            opts::Descriptor::data(opts::Value(11.0),false,true,true));
        fixture.context.define(receiver,opts::Key(u"stopI"),
            opts::Descriptor::data(opts::Value(22.0),true,true,true));
        std::string name;
        try { (void)fixture.context.call(*constructor,opts::Value(receiver)); }
        catch(const opts::Exception& error) { name=error.name; }
        expect(eq(name,std::string("TypeError")));
        expect(fixture.context.get(opts::Value(receiver),opts::Key(u"startStr"))==opts::Value(u""));
        expect(fixture.context.get(opts::Value(receiver),opts::Key(u"startI"))==opts::Value(11.0));
        expect(fixture.context.get(opts::Value(receiver),opts::Key(u"stopI"))==opts::Value(22.0));
        expect(!fixture.context.hasOwn(opts::Value(receiver),opts::Key(u"factory")));
        expect(!fixture.context.hasOwn(opts::Value(receiver),opts::Key(u"global")));
    };
    "null_item_constructor_uses_existing_accessors_in_source_order"_test=[] {
        RuntimeFixture fixture;
        const auto constructorValue=fixture.context.get(fixture.runtime.baseItemsModule,opts::Key(u"NullItem"));
        const auto* constructor=std::get_if<opts::ObjectRef>(&constructorValue.data);
        expect(constructor!=nullptr);
        if(!constructor)return;
        const auto prototypeValue=fixture.context.get(constructorValue,opts::Key(u"prototype"));
        const auto* prototype=std::get_if<opts::ObjectRef>(&prototypeValue.data);
        expect(prototype!=nullptr);
        if(!prototype)return;
        const auto receiver=fixture.context.objectWithPrototype(*prototype);
        std::vector<std::u16string> writes;
        for(const auto field:{u"startStr",u"startI",u"stopI",u"factory",u"global"}){
            const std::u16string name(field);
            const auto setter=fixture.context.function(u"set",[&writes,name,receiver](opts::Context&,opts::Value thisValue,std::span<const opts::Value>)->opts::Value{
                writes.push_back(name);
                expect(thisValue==opts::Value(receiver));
                return opts::Undefined{};
            });
            fixture.context.define(receiver,opts::Key(name),opts::Descriptor::access(std::nullopt,setter,true,true));
        }
        (void)fixture.context.call(*constructor,opts::Value(receiver));
        expect(static_cast<bool>(writes==std::vector<std::u16string>({u"startStr",u"startI",u"stopI",u"factory",u"global"})));
    };
    "open_stop_uses_derived_class_error_record"_test=[] {
        RuntimeFixture fixture;
        auto* open=dynamic_cast<tex::OpenItem*>(fixture.itemFactory().create("open"));
        auto* stop=fixture.itemFactory().create("stop");
        expect(open!=nullptr&&stop!=nullptr);
        if(!open||!stop)return;
        std::string id,message;
        try { (void)open->checkItem(stop); }
        catch(const tex::TexError& error) {
            id=property_host::debugText(error.id);
            message=property_host::debugText(error.message);
        }
        expect(eq(id+"|"+message,std::string("ExtraOpenMissingClose|Extra open brace or missing close brace")));
    };
    "subsup_error_spreads_trailing_class_error_arguments"_test=[] {
        RuntimeFixture fixture;
        const auto constructor=fixture.context.get(fixture.runtime.baseItemsModule,opts::Key(u"SubsupItem"));
        const auto errors=fixture.context.get(constructor,opts::Key(u"errors"));
        const auto* errorObject=std::get_if<opts::ObjectRef>(&errors.data);
        expect(errorObject!=nullptr);
        if(!errorObject)return;
        const auto original=fixture.context.get(errors,opts::Key(u"sup"));
        const auto replacement=fixture.context.array(3);
        fixture.context.set(opts::Value(replacement),opts::Key(u"0"),opts::Value(u"MissingOpenForSup"));
        fixture.context.set(opts::Value(replacement),opts::Key(u"1"),opts::Value(u"Missing open brace for superscript %1"));
        fixture.context.set(opts::Value(replacement),opts::Key(u"2"),opts::Value(u"extra-argument"));
        fixture.context.set(errors,opts::Key(u"sup"),opts::Value(replacement));
        auto* item=dynamic_cast<tex::SubsupItem*>(fixture.itemFactory().create("subsup"));
        auto* incoming=fixture.itemFactory().create("null");
        expect(item!=nullptr&&incoming!=nullptr);
        if(!item||!incoming){fixture.context.set(errors,opts::Key(u"sup"),original);return;}
        (void)item->setProperty("position",tex::StackProperty(2));
        std::string id,message;
        try { (void)item->checkItem(incoming); }
        catch(const tex::TexError& error) {
            id=property_host::debugText(error.id);
            message=property_host::debugText(error.message);
        }
        const auto mutated=fixture.context.get(errors,opts::Key(u"sup"));
        const auto* mutatedArray=std::get_if<opts::ObjectRef>(&mutated.data);
        expect(mutatedArray&&fixture.context.get(mutated,opts::Key(u"length"))==opts::Value(2.0));
        fixture.context.set(errors,opts::Key(u"sup"),original);
        expect(eq(id+"|"+message,std::string("MissingOpenForSup|Missing open brace for superscript extra-argument")));
    };
    "begin_end_name_mismatch_uses_source_environment_error"_test=[] {
        RuntimeFixture fixture;
        auto& factory=fixture.itemFactory();
        auto* begin=dynamic_cast<tex::BeginItem*>(factory.create("begin"));
        auto* end=factory.create("end");
        expect(begin!=nullptr && end!=nullptr);
        if(!begin||!end)return;
        (void)begin->setProperty("name",tex::StackProperty("alpha"));
        (void)end->setProperty("name",tex::StackProperty("beta"));
        std::string id,message;
        try { (void)begin->checkItem(end); }
        catch(const tex::TexError& error) {
            id=property_host::debugText(error.id);
            message=property_host::debugText(error.message);
        }
        expect(eq(id+"|"+message,std::string("EnvBadEnd|\\begin{alpha} ended with \\end{beta}")));
    };
    "open_close_wraps_actual_content_in_tex_atom"_test=[] {
        RuntimeFixture fixture;
        auto& factory=fixture.itemFactory();
        auto* open=dynamic_cast<tex::OpenItem*>(factory.create("open"));
        auto* close=factory.create("close");
        expect(open!=nullptr && close!=nullptr);
        if(!open||!close)return;
        const std::array<folia::math::mml::NodeValue,1> content{
            fixture.mml->create("mi",folia::math::tree::FactoryArgs{})};
        open->Push(content);
        const auto checked=open->checkItem(close);
        const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&checked);
        expect(tuple&&*tuple&&property_host::truthy((*tuple)->success));
        if(!tuple||!*tuple)return;
        const auto* items=std::get_if<tex::CheckList>(&(*tuple)->items);
        expect(items&&*items&&(*items)->size()==1);
        if(!items||!*items||(*items)->empty())return;
        const auto* itemRef=std::get_if<tex::StackItemRef>(&(**items)[0]);
        auto* mmlItem=itemRef&&*itemRef?dynamic_cast<tex::MmlItem*>(itemRef->get()):nullptr;
        expect(mmlItem!=nullptr);
        if(!mmlItem)return;
        const auto atom=mmlItem->First();
        expect(folia::math::tex::NodeUtil::instance().isType(atom,"TeXAtom"));
        expect(mmlItem->Size()==1);
        const auto atomChildren=atom.requireNode("childNodes")->childNodes;
        expect(atomChildren->size()==1);
        if(atomChildren->size()!=1)return;
        const auto inferred=atomChildren->getProperty("0");
        expect(folia::math::tex::NodeUtil::instance().isType(inferred,"inferredMrow"));
        expect(inferred.requireNode("childNodes")->childNodes->size()==1);
    };
    "over_double_over_uses_real_items_and_error"_test=[] {
        RuntimeFixture fixture;
        auto& factory=fixture.itemFactory();
        auto* first=factory.create("over");
        auto* second=factory.create("over");
        expect(first!=nullptr && second!=nullptr);
        if(!first||!second)return;
        expect(first->kind()=="over" && first->isClose());
        const auto name=first->getName();
        const auto* property=std::get_if<Property>(&name.value);
        expect(property!=nullptr && *property==Property("\\over"));
        std::string id,message;
        try { (void)first->checkItem(second); }
        catch(const tex::TexError& error) {
            id=property_host::debugText(error.id);
            message=property_host::debugText(error.message);
        }
        expect(eq(id+"|"+message,std::string("AmbiguousUseOf|Ambiguous use of \\over")));
    };
    "array_item_initial_state_keeps_actual_typed_storage"_test=[] {
        RuntimeFixture fixture;
        auto& factory=fixture.itemFactory();
        auto* item=dynamic_cast<tex::ArrayItem*>(factory.create("array"));
        expect(item!=nullptr);
        if(!item)return;
        const auto* table=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->table);
        const auto* row=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->row);
        expect(table&&*table&&(*table)->size()==0);
        expect(row&&*row&&(*row)->size()==0);
        const auto* frame=std::get_if<opts::ObjectRef>(&item->frame.data);
        const auto* hfill=std::get_if<opts::ObjectRef>(&item->hfill.data);
        const auto* definition=std::get_if<opts::ObjectRef>(&item->arraydef.data);
        const auto* align=std::get_if<opts::ObjectRef>(&item->breakAlign.data);
        expect(frame&&fixture.context.kind(*frame)==opts::ObjectKind::array);
        expect(hfill&&fixture.context.kind(*hfill)==opts::ObjectKind::array);
        expect(definition&&fixture.context.kind(*definition)==opts::ObjectKind::object);
        expect(align&&fixture.context.kind(*align)==opts::ObjectKind::object);
        expect(fixture.context.get(opts::Value(*align),opts::Key(u"cell"))==opts::Value(u""));
        expect(fixture.context.get(opts::Value(*align),opts::Key(u"row"))==opts::Value(u""));
        expect(fixture.context.get(opts::Value(*align),opts::Key(u"table"))==opts::Value(u""));
        const auto* factoryBinding=std::get_if<std::shared_ptr<tex::StackItemFactory>>(&fixture.parseOptions().itemFactory);
        expect(factoryBinding&&*factoryBinding);
        if(!factoryBinding||!*factoryBinding)return;
        const auto constructor=fixture.runtime.itemClassIdentities.find((*factoryBinding)->getNodeClass("array"));
        const auto identity=fixture.runtime.stackItemIdentities.find((*factoryBinding)->reference(item));
        expect(constructor.has_value()&&identity.has_value());
        if(!constructor||!identity)return;
        expect(fixture.context.get(opts::Value(*constructor),opts::Key(u"length"))==opts::Value(0));
        expect(fixture.context.get(opts::Value(*identity),opts::Key(u"kind"))==opts::Value(u"array"));
        const auto prototype=fixture.context.get(opts::Value(*constructor),opts::Key(u"prototype"));
        const auto* prototypeObject=std::get_if<opts::ObjectRef>(&prototype.data);
        expect(prototypeObject!=nullptr);
        if(!prototypeObject)return;
        const auto kindDescriptor=fixture.context.descriptor(*prototypeObject,opts::Key(u"kind"));
        expect(kindDescriptor&&kindDescriptor->accessor&&kindDescriptor->getter.has_value());
    };
    "array_end_entry_empty_uses_real_mml_factory_and_clears_stack"_test=[] {
        RuntimeFixture fixture;
        auto& factory=fixture.itemFactory();
        auto* item=dynamic_cast<tex::ArrayItem*>(factory.create("array"));
        expect(item!=nullptr);
        if(!item)return;
        item->EndEntry();
        const auto* row=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->row);
        expect(row&&*row&&(*row)->size()==1);
        if(!row||!*row||(*row)->size()!=1)return;
        const auto cell=(*row)->get(0);
        expect(tex::NodeUtil::instance().isType(cell,"mtd"));
        expect(cell.requireNode("childNodes")->childNodes->size()==1);
        expect(item->Size()==0);
        const auto* hfill=std::get_if<opts::ObjectRef>(&item->hfill.data);
        expect(hfill&&fixture.context.kind(*hfill)==opts::ObjectKind::array);
        expect(hfill&&fixture.context.arrayLength(*hfill)==0);
    };
    "array_end_entry_ralign_builds_mpadded_and_preserves_source_values"_test=[] {
        RuntimeFixture fixture;
        auto& factory=fixture.itemFactory();
        auto* item=dynamic_cast<tex::ArrayItem*>(factory.create("array"));
        expect(item!=nullptr);
        if(!item)return;
        const auto tuple=fixture.context.array(3);
        fixture.context.set(opts::Value(tuple),opts::Key(u"0"),opts::Value(u"middle"));
        fixture.context.set(opts::Value(tuple),opts::Key(u"1"),opts::Value(u"60%"));
        fixture.context.set(opts::Value(tuple),opts::Key(u"2"),opts::Value(u"center"));
        const auto alignments=fixture.context.array(1);
        fixture.context.set(opts::Value(alignments),opts::Key(u"0"),opts::Value(tuple));
        item->ralign=opts::Value(alignments);
        item->EndEntry();
        const auto* row=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->row);
        expect(row&&*row&&(*row)->size()==1);
        if(!row||!*row||(*row)->size()!=1)return;
        const auto cell=(*row)->get(0);
        expect(tex::NodeUtil::instance().isType(cell,"mtd"));
        const auto cellChildren=cell.requireNode("childNodes")->childNodes;
        expect(cellChildren->size()==1);
        if(cellChildren->size()!=1)return;
        const auto inferred=cellChildren->getProperty("0");
        const auto inferredChildren=inferred.requireNode("childNodes")->childNodes;
        expect(inferredChildren->size()==1);
        if(inferredChildren->size()!=1)return;
        const auto box=inferredChildren->getProperty("0");
        expect(tex::NodeUtil::instance().isType(box,"mpadded"));
        expect(eq(property_host::debugText(property_host::string(tex::NodeUtil::instance().getAttribute(box,"width"))),std::string("60%")));
        expect(eq(property_host::debugText(property_host::string(tex::NodeUtil::instance().getAttribute(box,"data-align"))),std::string("center")));
        expect(eq(property_host::debugText(property_host::string(tex::NodeUtil::instance().getAttribute(box,"data-vertical-align"))),std::string("middle")));
        expect(property_host::debugText(property_host::string(tex::stack_property_host::scalar(tex::NodeUtil::instance().getProperty(box,"vbox"))))==std::string("middle"));
        expect(box.requireNode("childNodes")->childNodes->size()==1);
        expect(item->Size()==0);
    };
    "array_end_row_consumes_label_and_break_alignment"_test=[] {
        RuntimeFixture fixture;
        auto* item=dynamic_cast<tex::ArrayItem*>(fixture.itemFactory().create("array"));
        expect(item!=nullptr);
        if(!item)return;
        const auto* row=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->row);
        expect(row&&*row);
        if(!row||!*row)return;
        (*row)->push_back(fixture.mml->create("mi",folia::math::tree::FactoryArgs{}));
        (void)item->setProperty("isLabeled",tex::StackProperty(true));
        item->atEnd=true;
        fixture.context.set(std::get<opts::ObjectRef>(item->breakAlign.data),opts::Key(u"row"),opts::Value(u"baseline"));
        item->EndRow();
        const auto* table=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->table);
        expect(table&&*table&&(*table)->size()==1);
        if(!table||!*table||(*table)->size()!=1)return;
        const auto result=(*table)->get(0);
        expect(tex::NodeUtil::instance().isType(result,"mlabeledtr"));
        expect(eq(property_host::debugText(property_host::string(tex::NodeUtil::instance().getAttribute(result,"data-break-align"))),std::string("baseline")));
        const auto* resetRow=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->row);
        expect(resetRow&&*resetRow&&(*resetRow)->size()==0);
        expect(item->atEnd==false);
        expect(!property_host::truthy(tex::stack_property_host::scalar(item->getProperty("isLabeled"))));
        expect(fixture.context.get(opts::Value(std::get<opts::ObjectRef>(item->breakAlign.data)),opts::Key(u"row"))==opts::Value(u""));
    };
    "array_check_lines_updates_frame_and_row_spacing_records"_test=[] {
        RuntimeFixture fixture;
        auto* item=dynamic_cast<tex::ArrayItem*>(fixture.itemFactory().create("array"));
        expect(item!=nullptr);
        if(!item)return;
        const auto* table=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->table);
        expect(table&&*table);
        if(!table||!*table)return;
        (*table)->push_back(fixture.mml->create("mtr",folia::math::tree::FactoryArgs{}));
        (*table)->push_back(fixture.mml->create("mtr",folia::math::tree::FactoryArgs{}));
        const auto definitions=std::get<opts::ObjectRef>(item->arraydef.data);
        fixture.context.set(opts::Value(definitions),opts::Key(u"rowlines"),opts::Value(u"solid none"));
        fixture.context.set(opts::Value(definitions),opts::Key(u"rowspacing"),opts::Value(u"1"));
        (void)item->setProperty("rowspacing",tex::StackProperty(0.5));
        item->checkLines();
        expect(fixture.context.get(opts::Value(definitions),opts::Key(u"rowlines"))==opts::Value(u"solid"));
        expect(fixture.context.get(opts::Value(definitions),opts::Key(u"rowspacing"))==opts::Value(u"1 0.5em"));
        const auto frame=std::get<opts::ObjectRef>(item->frame.data);
        expect(fixture.context.arrayLength(frame)==1);
        const auto pair=std::get<opts::ObjectRef>(fixture.context.get(opts::Value(frame),opts::Key(u"0")).data);
        expect(fixture.context.get(opts::Value(pair),opts::Key(u"0"))==opts::Value(u"bottom"));
        expect(fixture.context.get(opts::Value(pair),opts::Key(u"1"))==opts::Value(u"none"));
    };
    "array_check_lines_reloads_rowlines_before_split"_test=[] {
        RuntimeFixture fixture;
        auto* item=dynamic_cast<tex::ArrayItem*>(fixture.itemFactory().create("array"));
        expect(item!=nullptr);
        if(!item)return;
        const auto* table=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->table);
        expect(table&&*table);
        if(!table||!*table)return;
        (*table)->push_back(fixture.mml->create("mtr",folia::math::tree::FactoryArgs{}));
        const auto definitions=std::get<opts::ObjectRef>(item->arraydef.data);
        std::vector<int> trace;
        auto getter=fixture.context.function(u"rowlines.get",[&trace](opts::Context&,opts::Value,std::span<const opts::Value>){
            trace.push_back(static_cast<int>(trace.size()+1));
            return trace.size()==1?opts::Value(u"first"):opts::Value(opts::Undefined{});
        });
        fixture.context.define(definitions,opts::Key(u"rowlines"),opts::Descriptor::access(getter,std::nullopt,true,true));
        std::string error;
        try { item->checkLines(); }
        catch(const opts::Exception& caught) { error=caught.name+"|"+property_host::debugText(caught.message); }
        expect(error=="TypeError|Cannot read properties of undefined (reading 'split')");
        expect(trace==std::vector<int>({1,2}));
        expect(fixture.context.arrayLength(std::get<opts::ObjectRef>(item->frame.data))==0);
    };
    "array_start_entry_preserves_cextra_short_circuit_and_live_reads"_test=[] {
        RuntimeFixture fixture;
        auto* item=dynamic_cast<tex::ArrayItem*>(fixture.itemFactory().create("array"));
        expect(item!=nullptr);
        if(!item)return;
        const auto parserOwner=tex::TexParser::make(u"x",PropertyList{},fixture.parseOptions());
        item->parser=parserOwner.get();
        const auto starts=std::get<opts::ObjectRef>(item->cstart.data);
        fixture.context.set(opts::Value(starts),opts::Key(u"0"),opts::Value(u"prefix"));
        const auto extras=std::get<opts::ObjectRef>(item->cextra.data);
        std::vector<int> trace;
        auto zero=fixture.context.function(u"cextra[0].get",[&trace](opts::Context&,opts::Value,std::span<const opts::Value>){trace.push_back(0);return opts::Value(opts::Undefined{});});
        auto one=fixture.context.function(u"cextra[1].get",[&trace](opts::Context&,opts::Value,std::span<const opts::Value>){trace.push_back(1);return opts::Value(true);});
        fixture.context.define(extras,opts::Key(u"0"),opts::Descriptor::access(zero,std::nullopt,true,true));
        fixture.context.define(extras,opts::Key(u"1"),opts::Descriptor::access(one,std::nullopt,true,true));
        item->StartEntry();
        expect(trace==std::vector<int>({0,1,0}));
        expect(property_host::truthy(item->atEnd));
        const auto* parser=std::get_if<tex::TexParser*>(&item->parser);
        expect(parser&&*parser&&(*parser)->string()==u"x");
    };
    "array_end_entry_null_hfill_throws_before_row_or_stack_mutation"_test=[] {
        RuntimeFixture fixture;
        auto& factory=fixture.itemFactory();
        auto* item=dynamic_cast<tex::ArrayItem*>(factory.create("array"));
        expect(item!=nullptr);
        if(!item)return;
        const std::array<folia::math::mml::NodeValue,1> child{
            fixture.mml->create("mi",folia::math::tree::FactoryArgs{})};
        item->Push(child);
        item->hfill=opts::Null{};
        std::string error;
        try { item->EndEntry(); }
        catch(const opts::Exception& caught) {
            error=caught.name+"|"+property_host::debugText(caught.message);
        }
        const auto* row=std::get_if<std::shared_ptr<folia::math::tree::ChildArray<folia::math::mml::MmlNode>>>(&item->row);
        expect(error=="TypeError|Cannot read properties of null (reading 'length')");
        expect(row&&*row&&(*row)->size()==0);
        expect(item->Size()==1);
    };
    "over_array_boundary_uses_real_parser_and_mml_factory"_test=[] {
        RuntimeFixture fixture;
        const auto parser=tex::TexParser::make(u"{a\\over b}",PropertyList{},fixture.parseOptions());
        const auto result=parser->mml();
        auto& nodeUtil=tex::NodeUtil::instance();
        expect(nodeUtil.isType(result,"TeXAtom"));
        const auto atomChild=result.requireNode("childNodes")->childNodes->getProperty("0");
        expect(nodeUtil.isType(atomChild,"inferredMrow"));
        const auto fraction=atomChild.requireNode("childNodes")->childNodes->getProperty("0");
        expect(nodeUtil.isType(fraction,"mfrac"));
        expect(fraction.requireNode("childNodes")->childNodes->size()==2);
    };
};
}
