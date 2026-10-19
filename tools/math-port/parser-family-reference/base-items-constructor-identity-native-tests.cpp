// Native constructor-state and live-alias candidate for all Source BaseItems.
#include "../../../tests/support/folia_test.hpp"
import folia.math.input.tex.parser;
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
import folia.math.input.tex.stack_item_factory;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace tex=folia::math::tex;
namespace opts=folia::math::options;

struct Fixture {
    opts::Context context;
    OptionFunctionBindings functions{context};
    regexp_host::RegExpBindings regex{context};
    Unicode16Normalization unicode;
    core::InputJaxModule input{context};
    core::FindMathModule findMath{context};
    tex::FindTeXModule findTeX{context,findMath,regex};
    tex::ParserRuntime runtime{context,functions,regex,unicode,input,findTeX};
    std::shared_ptr<mml::MmlFactory> mml=std::make_shared<mml::MmlFactory>();
    tex::TeXRef jax;
    Fixture(){
        runtime.initializeParserRuntime();
        (void)tex::BasePackage::initializeModule(runtime);
        jax=tex::TeX::make(runtime);
        jax->setMmlFactory(mml);
    }
    tex::StackItemFactory& itemFactory(){
        const auto shared=jax->parseOptions();
        const auto* parse=std::get_if<std::shared_ptr<tex::ParseOptions>>(&shared);
        if(!parse||!*parse)throw opts::Exception("TypeError",u"ParseOptions unavailable");
        const auto* factory=std::get_if<std::shared_ptr<tex::StackItemFactory>>(&(**parse).itemFactory);
        if(!factory||!*factory)throw opts::Exception("TypeError",u"StackItemFactory unavailable");
        return **factory;
    }
};

bool distinctObject(const opts::Value& a,const opts::Value& b){
    const auto* left=std::get_if<opts::ObjectRef>(&a.data);
    const auto* right=std::get_if<opts::ObjectRef>(&b.data);
    return left&&right&&*left!=*right;
}

suite base_items_constructor_identity=[] {
    "array_constructor_records_are_fresh_per_item_and_preserve_defaults"_test=[] {
        Fixture fixture;
        auto& factory=fixture.itemFactory();
        auto* first=dynamic_cast<tex::ArrayItem*>(factory.create("array"));
        auto* second=dynamic_cast<tex::ArrayItem*>(factory.create("array"));
        expect(first&&second);
        if(!first||!second)return;
        const auto firstTable=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&first->table);
        const auto secondTable=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&second->table);
        const auto firstRow=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&first->row);
        const auto secondRow=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&second->row);
        expect(firstTable&&secondTable&&*firstTable&&*secondTable&&*firstTable!=*secondTable);
        expect(firstRow&&secondRow&&*firstRow&&*secondRow&&*firstRow!=*secondRow);
        expect(distinctObject(first->frame,second->frame));
        expect(distinctObject(first->hfill,second->hfill));
        expect(distinctObject(first->arraydef,second->arraydef));
        expect(distinctObject(first->cstart,second->cstart));
        expect(distinctObject(first->cend,second->cend));
        expect(distinctObject(first->cextra,second->cextra));
        expect(distinctObject(first->ralign,second->ralign));
        expect(distinctObject(first->breakAlign,second->breakAlign));
        expect(first->atEnd==Property(false));
        expect(first->templateSubs==Property(0));
        expect(std::holds_alternative<Missing>(first->parser));

        const auto breakAlign=std::get<opts::ObjectRef>(first->breakAlign.data);
        fixture.context.set(opts::Value(breakAlign),opts::Key(u"cell"),opts::Value(u"top"));
        const auto firstCell=fixture.context.get(first->breakAlign,opts::Key(u"cell"));
        const auto secondCell=fixture.context.get(second->breakAlign,opts::Key(u"cell"));
        expect(firstCell==opts::Value(u"top"));
        expect(secondCell==opts::Value(u""));
    };

    "mstyle_retains_the_supplied_attribute_property_identity"_test=[] {
        Fixture fixture;
        auto& factory=fixture.itemFactory();
        const Property attrs(PropertyList{{"mathcolor","red"}});
        auto* style=dynamic_cast<tex::MstyleItem*>(factory.create("mstyle",
            std::array<std::any,2>{attrs,Property("fixture-style")}));
        expect(style!=nullptr);
        if(!style)return;
        expect(style->attrList==attrs);
        PropertyList::record(attrs)->set("mathcolor",Property("blue"));
        expect((property_host::get(style->attrList,"mathcolor")==Property("blue"))==true);
        expect((tex::stack_property_host::scalar(style->getName())==Property("fixture-style"))==true);
    };
};
}
