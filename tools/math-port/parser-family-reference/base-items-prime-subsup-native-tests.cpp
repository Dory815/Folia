// Direct native witnesses for prime construction and missing-script errors.
#include "../../../tests/support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.stack_item;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.node_util;
import folia.math.input.tex.tex_error;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.tree.property;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.option_functions;
import folia.math.util.option_functions;
import folia.math.util.options;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace tex=folia::math::tex;
namespace opts=folia::math::options;

struct Fixture {
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

    Fixture() {
        runtime.initializeParserRuntime();
        (void)tex::BasePackage::initializeModule(runtime);
        jax=tex::TeX::make(runtime);
        jax->setMmlFactory(mml);
    }
    tex::StackItemFactory& factory() {
        auto binding=jax->parseOptions();
        auto parseOptions=std::get<std::shared_ptr<tex::ParseOptions>>(binding);
        return **std::get_if<std::shared_ptr<tex::StackItemFactory>>(&parseOptions->itemFactory);
    }
};

suite base_items_prime_subsup=[] {
    "inherited_base_constructor_forwards_original_mml_child"_test=[] {
        Fixture fixture;
        auto& factory=fixture.factory();
        auto* child=fixture.mml->create("mi",folia::math::tree::FactoryArgs{});
        expect(child!=nullptr);
        if(!child)return;
        const std::any argument{folia::math::mml::NodeValue(child)};
        auto* item=dynamic_cast<tex::MmlItem*>(factory.create("mml",folia::math::tree::FactoryArgs(&argument,1)));
        expect(item!=nullptr);
        if(!item)return;
        expect(item->Size()==1);
        expect(item->First()==folia::math::mml::NodeValue(child));
    };
    "prime_item_wraps_its_captured_two_nodes_and_retains_input_item"_test=[] {
        Fixture fixture;
        auto& factory=fixture.factory();
        auto* prime=dynamic_cast<tex::PrimeItem*>(factory.create("prime"));
        auto* input=factory.create("mml");
        expect(prime!=nullptr&&input!=nullptr);
        if(!prime||!input)return;
        const std::array<folia::math::mml::NodeValue,2> nodes{
            fixture.mml->create("mi",folia::math::tree::FactoryArgs{}),
            fixture.mml->create("mn",folia::math::tree::FactoryArgs{})};
        prime->Push(nodes);
        const auto result=prime->checkItem(input);
        const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&result);
        expect(tuple&&*tuple&&property_host::truthy((*tuple)->success));
        if(!tuple||!*tuple)return;
        const auto* entries=std::get_if<tex::CheckList>(&(*tuple)->items);
        expect(entries&&*entries&&(*entries)->size()==2);
        if(!entries||!*entries||(*entries)->size()!=2)return;
        const auto* wrapped=std::get_if<folia::math::mml::NodeValue>(&(**entries)[0]);
        const auto* retained=std::get_if<tex::StackItemRef>(&(**entries)[1]);
        expect(wrapped&&wrapped->isNode()&&retained&&*retained&&retained->get()==input);
        if(!wrapped||!wrapped->isNode())return;
        const auto resultNode=*wrapped;
        expect(tex::NodeUtil::instance().isType(resultNode,"msup"));
        const auto children=resultNode.requireNode("childNodes")->childNodes;
        expect(children&&children->size()==2);
        if(children&&children->size()==2){
            expect(children->get(0)==nodes[0]);
            expect(children->get(1)==nodes[1]);
        }
    };
    "subsup_position_two_uses_source_missing_open_sup_error"_test=[] {
        Fixture fixture;
        auto& factory=fixture.factory();
        auto* item=factory.create("subsup");
        expect(item!=nullptr);
        if(!item)return;
        (void)item->setProperty("position",tex::StackProperty(Property(2)));
        std::string error;
        try { (void)item->checkItem(factory.create("null")); }
        catch(const tex::TexError& caught){
            error=property_host::debugText(caught.id)+"|"+property_host::debugText(caught.message);
        }
        expect(error=="MissingOpenForSup|Missing open brace for superscript");
        expect((item->getProperty("position")==tex::StackProperty(Property(2)))==true);
        expect(item->Size()==0);
    };
    "subsup_movesupsub_uses_source_nullish_guard"_test=[] {
        const std::array<std::pair<bool,Property>,4> cases{{
            {false,Property(Missing{})},{true,Property(PropertyNull{})},
            {true,Property(0)},{true,Property(false)}}};
        const std::array<Property,4> expected{Property("prior"),Property("prior"),Property(0),Property(false)};
        for(std::size_t index=0;index<cases.size();++index){
            Fixture fixture;
            auto& factory=fixture.factory();
            auto* top=fixture.mml->create("mrow",folia::math::tree::FactoryArgs{});
            auto* sub=dynamic_cast<tex::SubsupItem*>(factory.create("subsup"));
            auto* incoming=factory.create("mml");
            auto* child=fixture.mml->create("mn",folia::math::tree::FactoryArgs{});
            expect(top!=nullptr&&sub!=nullptr&&incoming!=nullptr&&child!=nullptr);
            if(!top||!sub||!incoming||!child)continue;
            (void)sub->setProperty("position",tex::StackProperty(Property(1)));
            if(cases[index].first)(void)sub->setProperty("movesupsub",tex::StackProperty(cases[index].second));
            tex::NodeUtil::instance().setProperty(folia::math::mml::NodeValue(top),"movesupsub",Property("prior"));
            sub->Push(std::array<folia::math::mml::NodeValue,1>{top});
            incoming->Push(std::array<folia::math::mml::NodeValue,1>{child});
            (void)sub->checkItem(incoming);
            expect((tex::NodeUtil::instance().getProperty(folia::math::mml::NodeValue(top),"movesupsub")==expected[index])==true);
        }
    };
    "over_thickness_uses_source_nullish_guard"_test=[] {
        const std::array<std::pair<bool,Property>,4> cases{{
            {false,Property(Missing{})},{true,Property(PropertyNull{})},
            {true,Property(0)},{true,Property(false)}}};
        const std::array<Property,4> expected{Property("medium"),Property("medium"),Property(0),Property(false)};
        for(std::size_t index=0;index<cases.size();++index){
            Fixture fixture;
            auto& factory=fixture.factory();
            auto* over=dynamic_cast<tex::OverItem*>(factory.create("over"));
            auto* denominator=fixture.mml->create("mi",folia::math::tree::FactoryArgs{});
            auto* numerator=fixture.mml->create("mn",folia::math::tree::FactoryArgs{});
            auto* close=factory.create("close");
            expect(over!=nullptr&&denominator!=nullptr&&numerator!=nullptr&&close!=nullptr);
            if(!over||!denominator||!numerator||!close)continue;
            (void)over->setProperty("num",tex::StackProperty(folia::math::mml::NodeValue(numerator)));
            if(cases[index].first)(void)over->setProperty("thickness",tex::StackProperty(cases[index].second));
            over->Push(std::array<folia::math::mml::NodeValue,1>{denominator});
            const auto checked=over->checkItem(close);
            const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&checked);
            expect(tuple&&*tuple);
            if(!tuple||!*tuple)continue;
            const auto* items=std::get_if<tex::CheckList>(&(*tuple)->items);
            expect(items&&*items&&(*items)->size()==2);
            if(!items||!*items||(*items)->size()!=2)continue;
            const auto* item=std::get_if<tex::StackItemRef>(&(**items)[0]);
            auto* result=item&&*item?dynamic_cast<tex::MmlItem*>((*item).get()):nullptr;
            expect(result!=nullptr);
            if(result)expect((tex::NodeUtil::instance().getAttribute(result->First(),"linethickness")==expected[index])==true);
        }
    };
    "not_item_explicit_overlay_mutates_existing_token_in_place"_test=[] {
        Fixture fixture;
        auto& factory=fixture.factory();
        auto* item=factory.create("mml");
        auto* notItem=factory.create("not");
        auto* token=fixture.mml->create("mo",folia::math::tree::FactoryArgs{});
        auto* text=dynamic_cast<folia::math::mml::TextNode*>(fixture.mml->create("text",folia::math::tree::FactoryArgs{}));
        expect(item!=nullptr&&notItem!=nullptr&&token!=nullptr&&text!=nullptr);
        if(!item||!notItem||!token||!text)return;
        (void)text->setText(u"x");
        token->appendChild(folia::math::mml::NodeValue(text));
        const std::array<folia::math::mml::NodeValue,1> stackNodes{token};
        item->Push(stackNodes);
        const auto result=notItem->checkItem(item);
        const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&result);
        expect(tuple&&*tuple&&property_host::truthy((*tuple)->success));
        if(!tuple||!*tuple)return;
        const auto* entries=std::get_if<tex::CheckList>(&(*tuple)->items);
        expect(entries&&*entries&&(*entries)->size()==1);
        if(!entries||!*entries||(*entries)->size()!=1)return;
        const auto* retained=std::get_if<tex::StackItemRef>(&(**entries)[0]);
        expect(retained&&*retained&&retained->get()==item);
        const auto children=token->childNodes;
        expect(children&&children->size()==2);
        if(children&&children->size()==2){
            expect(children->get(0)==folia::math::mml::NodeValue(text));
            const auto overlay=children->get(1);
            expect(overlay.isNode());
            auto* overlayText=dynamic_cast<folia::math::mml::TextNode*>(overlay.nodeOrNull());
            expect(overlayText!=nullptr);
            if(overlayText)expect(overlayText->getText()==u"\u0338");
        }
    };
};
}
