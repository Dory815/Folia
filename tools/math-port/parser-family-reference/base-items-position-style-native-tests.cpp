// Direct PositionItem and StyleItem reductions through the real Base package.
#include "../../../tests/support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.stack_item;
import folia.math.input.tex.stack_property;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.node_util;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.option_functions;
import folia.math.util.options;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace tex=folia::math::tex;
namespace opts=folia::math::options;

struct PositionStyleRuntime {
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
    PositionStyleRuntime(){
        runtime.initializeParserRuntime();
        (void)tex::BasePackage::initializeModule(runtime);
        jax=tex::TeX::make(runtime);
        jax->setMmlFactory(mml);
    }
    tex::StackItemFactory& factory(){
        const auto binding=jax->parseOptions();
        const auto* options=std::get_if<std::shared_ptr<tex::ParseOptions>>(&binding);
        if(!options||!*options)throw opts::Exception("TypeError",u"ParseOptions unavailable");
        const auto* items=std::get_if<std::shared_ptr<tex::StackItemFactory>>(&(**options).itemFactory);
        if(!items||!*items)throw opts::Exception("TypeError",u"StackItemFactory unavailable");
        return **items;
    }
};

suite base_items_position_style_native=[] {
    "position_item_horizontal_final_expands_around_original_item"_test=[] {
        PositionStyleRuntime fixture;
        auto& factory=fixture.factory();
        auto* position=dynamic_cast<tex::PositionItem*>(factory.create("position"));
        auto* final=dynamic_cast<tex::MmlItem*>(factory.create("mml"));
        expect(position!=nullptr&&final!=nullptr);
        if(!position||!final)return;
        const auto left=fixture.mml->create("mi",folia::math::tree::FactoryArgs{});
        const auto center=fixture.mml->create("mn",folia::math::tree::FactoryArgs{});
        const auto right=fixture.mml->create("mo",folia::math::tree::FactoryArgs{});
        (void)position->setProperty("move",tex::StackProperty(Property("horizontal")));
        (void)position->setProperty("left",tex::StackProperty(left));
        (void)position->setProperty("right",tex::StackProperty(right));
        const std::array<folia::math::mml::NodeValue,1> centerNodes{center};
        final->Push(centerNodes);
        const auto result=position->checkItem(final);
        const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&result);
        expect(tuple&&*tuple&&property_host::truthy((*tuple)->success));
        if(!tuple||!*tuple)return;
        const auto* entries=std::get_if<tex::CheckList>(&(*tuple)->items);
        expect(entries&&*entries&&(*entries)->size()==3);
        if(!entries||!*entries||(*entries)->size()!=3)return;
        auto* first=std::get_if<tex::StackItemRef>(&(*entries)->at(0));
        auto* middle=std::get_if<tex::StackItemRef>(&(*entries)->at(1));
        auto* last=std::get_if<tex::StackItemRef>(&(*entries)->at(2));
        expect(first&&middle&&last&&*first&&*middle&&*last);
        if(!first||!middle||!last||!*first||!*middle||!*last)return;
        auto* leftItem=dynamic_cast<tex::MmlItem*>(first->get());
        auto* rightItem=dynamic_cast<tex::MmlItem*>(last->get());
        expect(leftItem!=nullptr&&rightItem!=nullptr);
        expect(middle->get()==final);
        if(leftItem&&rightItem){
            expect((leftItem->First()==left)==true);
            expect((rightItem->First()==right)==true);
        }
        expect((final->First()==center)==true);
        expect(position->Size()==0);
    };

    "style_item_close_wraps_its_content_and_retains_closing_item"_test=[] {
        PositionStyleRuntime fixture;
        auto& factory=fixture.factory();
        auto* style=dynamic_cast<tex::StyleItem*>(factory.create("style"));
        auto* stop=dynamic_cast<tex::StopItem*>(factory.create("stop"));
        expect(style!=nullptr&&stop!=nullptr);
        if(!style||!stop)return;
        (void)style->setProperty("styles",tex::StackProperty(PropertyList{{"mathcolor","red"},{"displaystyle",true}}));
        const auto child=fixture.mml->create("mi",folia::math::tree::FactoryArgs{});
        const std::array<folia::math::mml::NodeValue,1> nodes{child};
        style->Push(nodes);
        const auto result=style->checkItem(stop);
        const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&result);
        expect(tuple&&*tuple&&property_host::truthy((*tuple)->success));
        if(!tuple||!*tuple)return;
        const auto* entries=std::get_if<tex::CheckList>(&(*tuple)->items);
        expect(entries&&*entries&&(*entries)->size()==2);
        if(!entries||!*entries||(*entries)->size()!=2)return;
        const auto* wrapped=std::get_if<tex::StackItemRef>(&(*entries)->at(0));
        const auto* closing=std::get_if<tex::StackItemRef>(&(*entries)->at(1));
        auto* mml=wrapped&&*wrapped?dynamic_cast<tex::MmlItem*>(wrapped->get()):nullptr;
        expect(mml!=nullptr&&closing&&*closing&&closing->get()==stop);
        if(mml){
            const auto node=mml->First();
            expect(tex::NodeUtil::instance().isType(node,"mstyle"));
            expect((tex::NodeUtil::instance().getAttribute(node,"mathcolor")==Property("red"))==true);
            expect((tex::NodeUtil::instance().getAttribute(node,"displaystyle")==Property(true))==true);
            const auto children=node.requireNode("childNodes")->childNodes;
            expect(children&&children->size()==1);
            if(children&&children->size()==1)expect(tex::NodeUtil::instance().isType(children->getProperty("0"),"inferredMrow"));
        }
        expect(style->Size()==1);
    };
};
}
