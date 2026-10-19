// Focused direct BaseItems reductions; boots only the real parser package.
#include "../../../tests/support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.stack_item;
import folia.math.input.tex.stack_property;
import folia.math.input.tex.node_util;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.base_configuration;
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

struct BaseReductionRuntime {
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

    BaseReductionRuntime(){
        runtime.initializeParserRuntime();
        (void)tex::BasePackage::initializeModule(runtime);
        jax=tex::TeX::make(runtime);
        jax->setMmlFactory(mml);
    }
    tex::StackItemFactory& factory(){
        auto value=jax->parseOptions();
        const auto* ptr=std::get_if<std::shared_ptr<tex::ParseOptions>>(&value);
        if(!ptr||!*ptr)throw opts::Exception("TypeError",u"ParseOptions unavailable");
        const auto* items=std::get_if<std::shared_ptr<tex::StackItemFactory>>(&(**ptr).itemFactory);
        if(!items||!*items)throw opts::Exception("TypeError",u"StackItemFactory unavailable");
        return **items;
    }
};

suite base_items_reductions_native=[] {
    "base_item_over_reduction_records_node_before_clearing_open_stack"_test=[] {
        BaseReductionRuntime fixture;
        auto& factory=fixture.factory();
        auto* open=dynamic_cast<tex::OpenItem*>(factory.create("open"));
        auto* over=dynamic_cast<tex::OverItem*>(factory.create("over"));
        expect(open!=nullptr&&over!=nullptr);
        if(!open||!over)return;
        const auto child=fixture.mml->create("mi",folia::math::tree::FactoryArgs{});
        const std::array<folia::math::mml::NodeValue,1> nodes{child};
        open->Push(nodes);
        const auto result=open->checkItem(over);
        const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&result);
        expect(tuple&&*tuple&&property_host::truthy((*tuple)->success));
        if(tuple&&*tuple)expect(std::holds_alternative<PropertyNull>((*tuple)->items));
        expect(open->Size()==0);
        const auto number=tex::stack_property_host::node(over->getProperty("num"));
        expect((number==child)==true);
        expect(tex::NodeUtil::instance().isType(number,"mi"));
    };

    "base_item_cell_linebreak_short_circuits_and_misplaced_reads_name_after_flag"_test=[] {
        BaseReductionRuntime fixture;
        auto& factory=fixture.factory();
        auto* open=dynamic_cast<tex::OpenItem*>(factory.create("open"));
        auto* cell=dynamic_cast<tex::CellItem*>(factory.create("cell"));
        expect(open!=nullptr&&cell!=nullptr);
        if(!open||!cell)return;
        (void)cell->setProperty("linebreak",tex::StackProperty(true));
        const auto shortCircuit=open->checkItem(cell);
        const auto* shortTuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&shortCircuit);
        expect(shortTuple&&*shortTuple&&!property_host::truthy((*shortTuple)->success));
        if(shortTuple&&*shortTuple)expect(std::holds_alternative<PropertyNull>((*shortTuple)->items));
        expect(open->Size()==0);

        auto* errorOpen=dynamic_cast<tex::OpenItem*>(factory.create("open"));
        auto* namedCell=dynamic_cast<tex::CellItem*>(factory.create("cell"));
        expect(errorOpen!=nullptr&&namedCell!=nullptr);
        if(!errorOpen||!namedCell)return;
        (void)namedCell->setProperty("linebreak",tex::StackProperty(false));
        (void)namedCell->setProperty("name",tex::StackProperty(Property("cell-name")));
        std::string error;
        try { (void)errorOpen->checkItem(namedCell); }
        catch(const tex::TexError& caught){
            error=property_host::debugText(caught.id)+"|"+property_host::debugText(caught.message);
        }
        expect(eq(error,std::string("Misplaced|Misplaced cell-name")));
        expect(errorOpen->Size()==0);
    };

    "base_item_final_reduction_pushes_first_node_and_returns_shared_failure"_test=[] {
        BaseReductionRuntime fixture;
        auto& factory=fixture.factory();
        auto* receiver=dynamic_cast<tex::NullItem*>(factory.create("null"));
        auto* final=dynamic_cast<tex::MmlItem*>(factory.create("mml"));
        expect(receiver!=nullptr&&final!=nullptr);
        if(!receiver||!final)return;
        const auto node=fixture.mml->create("mi",folia::math::tree::FactoryArgs{});
        const std::array<folia::math::mml::NodeValue,1> nodes{node};
        final->Push(nodes);
        const auto result=receiver->checkItem(final);
        const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&result);
        expect(tuple&&*tuple&&!property_host::truthy((*tuple)->success));
        if(tuple&&*tuple)expect(std::holds_alternative<PropertyNull>((*tuple)->items));
        expect(receiver->Size()==1);
        expect((receiver->First()==node)==true);
    };

    "base_item_open_stop_uses_derived_errors_record"_test=[] {
        BaseReductionRuntime fixture;
        auto* open=dynamic_cast<tex::OpenItem*>(fixture.factory().create("open"));
        auto* stop=dynamic_cast<tex::StopItem*>(fixture.factory().create("stop"));
        expect(open!=nullptr&&stop!=nullptr);
        if(!open||!stop)return;
        std::string error;
        try { (void)open->checkItem(stop); }
        catch(const tex::TexError& caught){
            error=property_host::debugText(caught.id)+"|"+property_host::debugText(caught.message);
        }
        expect(eq(error,std::string("ExtraOpenMissingClose|Extra open brace or missing close brace")));
        expect(open->Size()==0);
    };

    "left_item_right_reduction_builds_one_fenced_mml_item"_test=[] {
        BaseReductionRuntime fixture;
        auto& factory=fixture.factory();
        const std::array<std::any,1> leftArgs{Property("(")};
        const std::array<std::any,2> rightArgs{Property(")"),Property("blue")};
        auto* left=dynamic_cast<tex::LeftItem*>(factory.create("left",leftArgs));
        auto* right=dynamic_cast<tex::RightItem*>(factory.create("right",rightArgs));
        expect(left!=nullptr&&right!=nullptr);
        if(!left||!right)return;
        const auto result=left->checkItem(right);
        const auto* tuple=std::get_if<std::shared_ptr<tex::CheckTuple>>(&result);
        expect(tuple&&*tuple&&property_host::truthy((*tuple)->success));
        if(!tuple||!*tuple)return;
        const auto* list=std::get_if<tex::CheckList>(&(*tuple)->items);
        expect(list&&*list&&(*list)->size()==1);
        if(!list||!*list||(*list)->size()!=1)return;
        const auto* reference=std::get_if<tex::StackItemRef>(&(*list)->front());
        auto* item=reference&&*reference?dynamic_cast<tex::MmlItem*>(reference->get()):nullptr;
        expect(item!=nullptr);
        if(!item)return;
        const auto fenced=item->First();
        expect(tex::NodeUtil::instance().isType(fenced,"mrow"));
        expect((tex::NodeUtil::instance().getProperty(fenced,"open")==Property("("))==true);
        expect((tex::NodeUtil::instance().getProperty(fenced,"close")==Property(")"))==true);
        expect((tex::NodeUtil::instance().getAttribute(fenced,"data-latex-item")==Property("\\left"))==true);
    };

    "position_item_close_throws_missing_box_without_mutating_position"_test=[] {
        BaseReductionRuntime fixture;
        auto& factory=fixture.factory();
        auto* position=dynamic_cast<tex::PositionItem*>(factory.create("position"));
        auto* stop=dynamic_cast<tex::StopItem*>(factory.create("stop"));
        expect(position!=nullptr&&stop!=nullptr);
        if(!position||!stop)return;
        (void)position->setProperty("move",tex::StackProperty(Property("vertical")));
        std::string error;
        try { (void)position->checkItem(stop); }
        catch(const tex::TexError& caught){
            error=property_host::debugText(caught.id)+"|"+property_host::debugText(caught.message);
        }
        expect(eq(error,std::string("MissingBoxFor|Missing box for ")));
        expect((tex::stack_property_host::scalar(position->getProperty("move"))==Property("vertical"))==true);
        expect(position->Size()==0);
    };
};
}
