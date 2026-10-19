// Native candidate for the real TextMacros package graph. This is kept out of
// the automatic test glob until Root wires the package owner into its gate.
#include "../../../tests/support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.textmacros.configuration;
import folia.math.input.tex.node_util;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace tex=folia::math::tex;
namespace opts=folia::math::options;

opts::Value strings(opts::Context& context,std::initializer_list<std::u16string_view> values){
    const auto array=context.array(static_cast<std::uint32_t>(values.size()));
    std::uint32_t index=0;
    for(const auto value:values){
        const std::string digits=std::to_string(index++);
        context.set(array,opts::Key(std::u16string(digits.begin(),digits.end())),opts::Value(std::u16string(value)));
    }
    return opts::Value(array);
}

struct TextMacrosRuntime {
    opts::Context context;
    OptionFunctionBindings functions{context};
    regexp_host::RegExpBindings regex{context};
    Unicode16Normalization unicode;
    core::InputJaxModule inputJax{context};
    core::FindMathModule findMath{context};
    tex::FindTeXModule findTeX{context,findMath,regex};
    tex::ParserRuntime runtime{context,functions,regex,unicode,inputJax,findTeX};
    std::shared_ptr<mml::MmlFactory> mml=std::make_shared<mml::MmlFactory>();
    tex::TeXRef jax;

    TextMacrosRuntime(){
        runtime.initializeParserRuntime();
        tex::BasePackage::initializeModule(runtime);
        tex::TextMacrosConfiguration::initializeModule(runtime);
        const auto options=context.object();
        context.set(options,opts::Key(u"packages"),strings(context,{u"base",u"textmacros"}));
        jax=tex::TeX::make(runtime,opts::Value(options));
        (void)runtime.bindMmlFactory(mml);
        jax->setMmlFactory(mml);
    }

    tex::ParseOptionsRef mainOptions(){
        const auto value=jax->parseOptions();
        const auto* result=std::get_if<tex::ParseOptionsRef>(&value);
        if(!result||!*result)throw opts::Exception("TypeError",u"TeX ParseOptions are missing");
        return *result;
    }
    std::shared_ptr<folia::math::SourceStringMap<opts::Value>> packageData(){
        const auto value=mainOptions()->packageData;
        const auto* result=std::get_if<std::shared_ptr<folia::math::SourceStringMap<opts::Value>>>(&value);
        if(!result||!*result)throw opts::Exception("TypeError",u"TeX packageData is missing");
        return *result;
    }
    core::InputJaxFilterDataRef filterData(tex::ParseOptionsRef options){
        auto filter=std::make_shared<core::InputJaxFilterData>();
        filter->initialize(core::MathItemReference{Missing{}},core::MathDocumentReference{Missing{}},std::any(options));
        filter->encodeData=[this](const std::any& value)->opts::Value{
            if(const auto* contextValue=std::any_cast<opts::Value>(&value))return *contextValue;
            if(const auto* owner=std::any_cast<tex::ParseOptionsRef>(&value);owner&&*owner){
                const auto identity=runtime.parseOptionsIdentities.find(*owner);
                if(!identity)throw opts::Exception("TypeError",u"fixture ParseOptions lacks its canonical identity");
                return opts::Value(*identity);
            }
            throw opts::Exception("TypeError",u"fixture filter data has no canonical Context value");
        };
        filter->decodeData=[this](opts::Value value)->std::any{
            const auto* identity=std::get_if<opts::ObjectRef>(&value.data);
            if(!identity)throw opts::Exception("TypeError",u"fixture filter data is not ParseOptions");
            return std::any(runtime.parseOptionsIdentities.resolve(*identity));
        };
        filter->bindSourceRecord(context,filter,runtime.executionLifetime());
        return filter;
    }
    void runSourcePreprocessors(){
        auto filter=filterData(mainOptions());
        std::array<std::any,1> arguments{std::any(filter)};
        try{jax->preFilters->execute(arguments);}
        catch(const opts::Exception& error){
            throw opts::Exception(error.name,
                u"TextMacros.preFilters.execute: "+error.message);
        }
    }
};

suite textmacros_native_candidate=[] {
    "parse_options_parsers_is_the_live_context_array"_test=[] {
        TextMacrosRuntime fixture;
        const auto options=fixture.mainOptions();
        const auto identity=fixture.runtime.parseOptionsIdentities.find(options);
        expect(identity.has_value());
        if(!identity)return;
        const auto before=fixture.context.get(*identity,opts::Key(u"parsers"));
        expect(before==options->parsers);
        const auto* beforeArray=std::get_if<opts::ObjectRef>(&before.data);
        expect(beforeArray&&eq(fixture.context.arrayLength(*beforeArray),std::uint32_t{0}));
        options->clear();
        const auto after=fixture.context.get(*identity,opts::Key(u"parsers"));
        expect(after==options->parsers);
        expect(after!=before);
        const auto* afterArray=std::get_if<opts::ObjectRef>(&after.data);
        expect(afterArray&&eq(fixture.context.arrayLength(*afterArray),std::uint32_t{0}));
    };

    "parse_options_column_parser_reads_the_live_context_field"_test=[] {
        TextMacrosRuntime fixture;
        const auto options=fixture.mainOptions();
        const auto identity=fixture.runtime.parseOptionsIdentities.find(options);
        expect(identity.has_value());
        if(!identity)return;
        const auto initial=fixture.context.get(*identity,opts::Key(u"columnParser"));
        expect(initial==options->columnParser);
        const auto* initialObject=std::get_if<opts::ObjectRef>(&initial.data);
        expect(initialObject!=nullptr);
        if(!initialObject)return;
        expect(fixture.runtime.columnParserIdentities.resolve(*initialObject)==
               fixture.runtime.columnParserIdentities.resolve(std::get<opts::ObjectRef>(options->columnParser.data)));

        const opts::ObjectRef replacement=fixture.context.construct(
            std::get<opts::ObjectRef>(fixture.runtime.columnParserClass.data));
        fixture.context.set(*identity,opts::Key(u"columnParser"),opts::Value(replacement));
        expect(fixture.context.get(*identity,opts::Key(u"columnParser"))==opts::Value(replacement));
        expect(options->columnParserOwner().sourceIdentity()==replacement);

        fixture.context.set(*identity,opts::Key(u"columnParser"),opts::Value(opts::Null{}));
        expect(throws<opts::Exception>([&]{(void)options->columnParserOwner();}));
    };

    "parse_options_node_lists_expose_the_same_mml_child_array"_test=[] {
        TextMacrosRuntime fixture;
        const auto options=fixture.mainOptions();
        const auto parser=tex::TexParser::make(u"x",PropertyList{},*options);
        const auto node=parser->mml();
        options->root=node;
        options->addNode(Property("observed"),node);
        const auto identity=fixture.runtime.parseOptionsIdentities.find(options);
        expect(identity.has_value());
        if(!identity)return;
        const auto lists=fixture.context.get(*identity,opts::Key(u"nodeLists"));
        const auto slots=fixture.context.get(lists,opts::Key(u"observed"));
        const auto* array=std::get_if<opts::ObjectRef>(&slots.data);
        expect(array!=nullptr);
        if(!array)return;
        expect(eq(fixture.context.arrayLength(*array),std::uint32_t{1}));
        const auto nodeIdentity=fixture.runtime.mmlNodeIdentities.find(node.requireNode());
        expect(nodeIdentity.has_value());
        if(!nodeIdentity)return;
        expect(fixture.context.get(slots,opts::Key(u"0"))==opts::Value(*nodeIdentity));
        fixture.context.set(slots,opts::Key(u"0"),opts::Value(opts::Null{}));
        expect(fixture.context.get(fixture.context.get(lists,opts::Key(u"observed")),opts::Key(u"0"))==opts::Value(opts::Null{}));
        const auto sameSlots=fixture.context.get(lists,opts::Key(u"observed"));
        const auto* sameArray=std::get_if<opts::ObjectRef>(&sameSlots.data);
        expect(sameArray&&*sameArray==*array);
        expect(eq(fixture.context.arrayLength(*array),std::uint32_t{1}));
    };

    "tex_parser_runs_the_real_base_package_to_mml_factory"_test=[] {
        TextMacrosRuntime fixture;
        struct Case { std::u16string_view source; std::string_view kind; double consumed; };
        constexpr Case cases[]{
            {u"", "inferredMrow", 0}, {u"x", "mi", 1},
            {u"x+1", "inferredMrow", 3}, {u"\\frac{1}{2}", "mfrac", 11},
            {u"😀", "mo", 2}, {u"x^2", "msubsup", 4},
        };
        for(const auto& item:cases){
            auto& options=*fixture.mainOptions();options.clear();
            const auto parser=tex::TexParser::make(std::u16string(item.source),PropertyList{},options);
            const auto result=parser->mml();
            expect(eq(result.requireNode("kind")->kind(),std::string(item.kind)));
            expect(eq(parser->i,item.consumed));
            if(item.source==u"x+1"){
                expect(result.requireNode("attributes")->attributes()->get("data-latex")==Property("x+1"));
                const auto children=result.requireNode("childNodes")->childNodes;
                expect(children&&eq(children->size(),std::size_t{3}));
                if(children&&children->size()==3){
                    expect(eq(children->get(0).requireNode("kind")->kind(),std::string("mi")));
                    expect(eq(children->get(1).requireNode("kind")->kind(),std::string("mo")));
                    expect(eq(children->get(2).requireNode("kind")->kind(),std::string("mn")));
                }
            }
        }
    };

    "textmacros_mapping_methods_execute_through_the_real_parser_graph"_test=[] {
        TextMacrosRuntime fixture;
        const auto main=fixture.mainOptions();
        main->clear();
        fixture.runSourcePreprocessors();
        const auto outer=tex::TexParser::make(u"",PropertyList{},*main);
        const auto data=fixture.packageData()->get(u"textmacros");
        expect(data.has_value());if(!data)return;
        const auto* record=std::get_if<opts::ObjectRef>(&data->data);
        expect(record!=nullptr);if(!record)return;
        fixture.context.set(*record,opts::Key(u"texParser"),opts::Value(outer->instanceIdentity()));
        const auto innerValue=fixture.context.get(*record,opts::Key(u"parseOptions"));
        const auto* innerIdentity=std::get_if<opts::ObjectRef>(&innerValue.data);
        expect(innerIdentity!=nullptr);if(!innerIdentity)return;
        const auto inner=fixture.runtime.parseOptionsIdentities.resolve(*innerIdentity);
        struct Case { std::u16string_view source; double consumed; };
        // Source rows are in textmacros-method-branch-corpus and
        // textparser-font-size-and-text-argument-macros. Each case exercises
        // the registered TextMacros/BaseMethods FunctionRef via the actual
        // TextParser, ParseOptions, Stack, and MmlFactory owners.
        constexpr Case cases[]{
            {u"\\%",2},{u"\\dagger",7},{u"\\'{e}",5},{u"\\frak{x}",8},
            {u"\\quad",5},{u"\\hspace{1em}",12},{u"\\rule{1em}{2pt}",15},
            {u"\\ref{missing}",13},{u"\\underline{x}",13},{u"\\llap{x}",8},
            {u"\\phantom{x}",11},{u"\\smash{x}",9},{u"\\mmlToken{mi}{x}",16},
            {u"\\emph emphatic",14},{u"\\textbf{bold}",10},
            {u"\\textit{italic}",12},{u"\\texttt{mono}",10},
        };
        for(const auto& item:cases){
            inner->clear();
            std::shared_ptr<tex::TextParser> parser;
            mml::NodeValue result;
            try{
                parser=tex::TextParser::make(fixture.runtime,std::u16string(item.source),PropertyList{},*inner);
                result=parser->mml();
            }catch(const opts::Exception& error){
                throw opts::Exception(error.name,
                    u"TextParser.mml input="+std::u16string(item.source)+u"; "+error.message);
            }catch(const tex::TexError& error){
                throw opts::Exception("NativeTest",
                    u"TextParser.mml input="+std::u16string(item.source)+u"; "+error.id+u": "+error.message);
            }
            expect(result.isNode());
            expect(eq(parser->i,item.consumed));
            const auto* active=std::get_if<opts::ObjectRef>(&inner->parsers.data);
            expect(active&&eq(fixture.context.arrayLength(*active),std::uint32_t{0}));
        }
    };

    "textmacros_config_uses_the_real_shared_parser_graph"_test=[] {
        TextMacrosRuntime fixture;
        const auto main=fixture.mainOptions();
        fixture.runSourcePreprocessors();
        const auto data=fixture.packageData()->get(u"textmacros");
        expect(data.has_value());
        if(!data)return;
        const auto* record=std::get_if<opts::ObjectRef>(&data->data);
        expect(record!=nullptr);
        if(!record)return;

        auto& context=fixture.context;
        const opts::Value innerValue=context.get(*record,opts::Key(u"parseOptions"));
        const auto* innerIdentity=std::get_if<opts::ObjectRef>(&innerValue.data);
        expect(innerIdentity!=nullptr);
        if(!innerIdentity)return;
        const auto inner=fixture.runtime.parseOptionsIdentities.resolve(*innerIdentity);
        const auto* outerOptions=std::get_if<opts::ObjectRef>(&main->options.data);
        const auto* innerOptions=std::get_if<opts::ObjectRef>(&inner->options.data);
        expect(outerOptions&&innerOptions&&*outerOptions==*innerOptions);
        const auto* outerData=std::get_if<std::shared_ptr<folia::math::SourceStringMap<opts::Value>>>(&main->packageData);
        const auto* innerData=std::get_if<std::shared_ptr<folia::math::SourceStringMap<opts::Value>>>(&inner->packageData);
        expect(outerData&&innerData&&outerData->get()==innerData->get());
        const auto* factoryIdentity=std::get_if<opts::ObjectRef>(&inner->nodeFactory.data);
        expect(factoryIdentity!=nullptr);
        expect(factoryIdentity&&fixture.runtime.nodeFactoryIdentities.resolve(*factoryIdentity)==
               &inner->nodeFactoryOwner());
        const auto function=context.get(*outerOptions,opts::Key(u"internalMath"));
        const auto* functionObject=std::get_if<opts::ObjectRef>(&function.data);
        expect(functionObject&&context.kind(*functionObject)==opts::ObjectKind::function);
    };

    "textmacros_preprocessor_runs_with_real_filter_and_factory_owners"_test=[] {
        TextMacrosRuntime fixture;
        const auto main=fixture.mainOptions();
        auto filter=fixture.filterData(main);
        std::array<std::any,1> arguments{std::any(filter)};
        bool continued=false;
        try{continued=fixture.jax->preFilters->execute(arguments);}
        catch(const opts::Exception& error){
            throw opts::Exception(error.name,
                u"TextMacros.preFilters.execute input=actual ParseOptions; "+error.message);
        }
        expect(continued);
        const auto data=fixture.packageData()->get(u"textmacros");
        expect(data.has_value());
        if(!data)return;
        const auto* record=std::get_if<opts::ObjectRef>(&data->data);
        if(!record)return;
        const auto innerValue=fixture.context.get(*record,opts::Key(u"parseOptions"));
        const auto* identity=std::get_if<opts::ObjectRef>(&innerValue.data);
        expect(identity!=nullptr);
        if(!identity)return;
        const auto inner=fixture.runtime.parseOptionsIdentities.resolve(*identity);
        const auto factory=inner->nodeFactoryOwnerRef();
        expect(factory!=nullptr);
        if(factory){
            const std::array<std::any,1> textArgument{std::any(Property(property_host::debugText(u"actual factory")))};
            const auto node=factory->create("text",textArgument);
            expect(node.isNode());
            expect(tex::NodeUtil::instance().isType(node,"text"));
        }
    };

    "textparser_parses_text_to_real_mml_using_the_loaded_package_graph"_test=[] {
        TextMacrosRuntime fixture;
        const auto main=fixture.mainOptions();
        fixture.runSourcePreprocessors();
        const auto data=fixture.packageData()->get(u"textmacros");
        expect(data.has_value());
        if(!data)return;
        const auto* record=std::get_if<opts::ObjectRef>(&data->data);
        expect(record!=nullptr);
        if(!record)return;

        // TextParser.texParser resolves this exact ordinary package-record
        // field back to the real outer TexParser instance.
        const auto outer=tex::TexParser::make(u"",PropertyList{},*main);
        fixture.context.set(*record,opts::Key(u"texParser"),opts::Value(outer->instanceIdentity()));
        const auto innerValue=fixture.context.get(*record,opts::Key(u"parseOptions"));
        const auto* innerIdentity=std::get_if<opts::ObjectRef>(&innerValue.data);
        expect(innerIdentity!=nullptr);
        if(!innerIdentity)return;
        const auto inner=fixture.runtime.parseOptionsIdentities.resolve(*innerIdentity);
        struct Observation { std::string kind; Property mathvariant; Property mathsize; double index; };
        const auto observe=[&](std::u16string source){
            inner->clear();
            std::shared_ptr<tex::TextParser> parser;
            mml::NodeValue result;
            try{
                parser=tex::TextParser::make(fixture.runtime,source,PropertyList{},*inner);
                result=parser->mml();
            }catch(const opts::Exception& error){
                throw opts::Exception(error.name,
                    u"TextParser.mml input="+source+u"; "+error.message);
            }catch(const tex::TexError& error){
                throw opts::Exception("NativeTest",
                    u"TextParser.mml input="+source+u"; "+error.id+u": "+error.message);
            }
            const auto attributes=result.requireNode("attributes")->attributes();
            expect(parser->text().empty());
            expect(parser->envStackEmpty());
            const auto* parserList=std::get_if<opts::ObjectRef>(&inner->parsers.data);
            expect(parserList&&eq(fixture.context.arrayLength(*parserList),std::uint32_t{0}));
            if(const auto children=result.requireNode("childNodes")->childNodes;children&&children->size()==1)
                expect(eq(children->get(0).requireNode("kind")->kind(),std::string("text")));
            else expect(false);
            return Observation{result.requireNode("kind")->kind(),attributes->get("mathvariant"),
                attributes->get("mathsize"),parser->i};
        };

        // These expected MML rows are the corresponding real CJS observations
        // in textmacros-package-original-source.json.
        const auto bold=observe(u"\\bf bold");
        expect(eq(bold.kind,std::string("mtext")));
        expect(bold.mathvariant==Property("bold"));
        expect(eq(bold.index,8.0));

        const auto italic=observe(u"\\it italic");
        expect(eq(italic.kind,std::string("mtext")));
        expect(italic.mathvariant==Property("-tex-mathit"));
        expect(eq(italic.index,10.0));

        const auto tiny=observe(u"\\tiny small");
        expect(eq(tiny.kind,std::string("mtext")));
        expect(tiny.mathsize==Property(0.6));
        expect(eq(tiny.index,11.0));

        inner->clear();
        auto scripted=tex::TextParser::make(fixture.runtime,u"abc",PropertyList{},*inner,Property(2.0));
        const auto scriptNode=scripted->mml();
        expect(eq(scriptNode.requireNode("kind")->kind(),std::string("mstyle")));
        const auto scriptAttributes=scriptNode.requireNode("attributes")->attributes();
        expect(scriptAttributes->get("displaystyle")==Property(false));
        expect(scriptAttributes->get("scriptlevel")==Property(2.0));
    };
};
}
