// Native candidate for the real Begingroup/Newcommand runtime graph.
#include "../../../tests/support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.begingroup.configuration;
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

struct BegingroupRuntime {
    opts::Context context;
    OptionFunctionBindings functions{context};
    regexp_host::RegExpBindings regex{context};
    Unicode16Normalization unicode;
    core::InputJaxModule inputJax{context};
    core::FindMathModule findMath{context};
    tex::FindTeXModule findTeX{context,findMath,regex};
    tex::ParserRuntime runtime{context,functions,regex,unicode,inputJax,findTeX};
    mml::MmlFactory mml;
    tex::TeXRef jax;

    BegingroupRuntime(){
        runtime.initializeParserRuntime();
        tex::BasePackage::initializeModule(runtime);
        tex::NewcommandConfiguration::initializeModule(runtime);
        tex::BegingroupConfiguration::initializeModule(runtime);
        const auto options=context.object();
        context.set(options,opts::Key(u"packages"),strings(context,{u"base",u"begingroup",u"newcommand"}));
        jax=tex::TeX::make(runtime,opts::Value(options));
        jax->setMmlFactory(&mml);
    }

    tex::ParseOptionsRef parseOptions(){
        const auto value=jax->parseOptions();
        const auto* result=std::get_if<tex::ParseOptionsRef>(&value);
        if(!result||!*result)throw opts::Exception("TypeError",u"TeX ParseOptions are missing");
        return *result;
    }
    mml::NodeValue parse(std::u16string input){
        const auto options=parseOptions();
        options->clear();
        return tex::TexParser::make(std::move(input),PropertyList{},*options)->mml();
    }
};

suite begingroup_native_candidate=[] {
    "actual_module_class_metadata_and_stack_identity"_test=[] {
        BegingroupRuntime fixture;
        auto& context=fixture.context;
        const auto constructor=fixture.runtime.begingroupStackClass;
        expect((context.get(constructor,opts::Key(u"name"))==opts::Value(u"BegingroupStack"))==true);
        expect((context.get(constructor,opts::Key(u"length"))==opts::Value(1))==true);
        const auto prototype=context.get(constructor,opts::Key(u"prototype"));
        for(const auto name:{u"getGlobal",u"checkGlobal",u"push",u"pop",u"finish",u"remove",u"reset",u"sandbox"})
            expect(context.has(prototype,opts::Key(name)));
        const auto staticHandlers=context.get(constructor,opts::Key(u"handlerConfig"));
        expect(context.has(staticHandlers,opts::Key(u"delimiter")));
        expect(context.has(staticHandlers,opts::Key(u"environment")));
        expect(context.has(staticHandlers,opts::Key(u"macro")));

        const auto options=fixture.parseOptions();
        const auto stack=tex::begingroupStack(*options);
        expect(stack!=nullptr);
        if(stack){
            expect(stack->index()==-100);
            expect(stack->top()==-100);
            expect(stack->base()==-100);
            expect(stack->global(u"new-Command")!=nullptr);
            expect(stack->global(u"new-Delimiter")!=nullptr);
            expect(stack->global(u"new-Environment")!=nullptr);
        }
        const auto package=std::get<std::shared_ptr<SourceStringMap<opts::Value>>>(options->packageData)->get(u"begingroup");
        expect(package.has_value());
        if(package){
            const auto identity=context.get(*package,opts::Key(u"stack"));
            const auto bound=fixture.runtime.globalDefinitionScopeIdentities.find(stack);
            expect(bound.has_value());
            if(bound)expect(std::get<opts::ObjectRef>(identity.data)==*bound);
        }
    };

    "real_parser_applies_local_shadow_and_global_definition_rules"_test=[] {
        BegingroupRuntime fixture;
        auto options=fixture.parseOptions();
        auto stack=tex::begingroupStack(*options);
        const auto parseStage=[&](std::u16string input,std::string_view stage){
            try{return fixture.parse(std::move(input));}
            catch(const opts::Exception& error){
                std::cerr<<stage<<" options::Exception "<<error.name<<":"<<property_host::debugText(error.message)<<"\n";
                expect(false)<<stage<<" options::Exception "<<error.name<<":"<<property_host::debugText(error.message);
                throw std::runtime_error(std::string(stage)+" options::Exception "+error.name+":"+property_host::debugText(error.message));
            }catch(const tex::TexError& error){
                std::cerr<<stage<<" TexError "<<property_host::debugText(error.id)<<":"<<property_host::debugText(error.message)<<"\n";
                expect(false)<<stage<<" TexError "<<property_host::debugText(error.id)<<":"<<property_host::debugText(error.message);
                throw std::runtime_error(std::string(stage)+" TexError "+property_host::debugText(error.id)+":"+property_host::debugText(error.message));
            }catch(const std::exception& error){std::cerr<<stage<<" std::exception "<<error.what()<<"\n";expect(false)<<stage<<" std::exception "<<error.what();throw std::runtime_error(std::string(stage)+" std::exception "+error.what());}
            catch(...){std::cerr<<stage<<" unknown native exception\n";expect(false)<<stage<<" unknown native exception";throw std::runtime_error(std::string(stage)+" unknown native exception");}
        };
        const auto inspectStage=[&](std::string_view stage,auto&& inspect)->decltype(auto){
            try{return inspect();}
            catch(const opts::Exception& error){
                std::cerr<<stage<<" options::Exception "<<error.name<<":"<<property_host::debugText(error.message)<<"\n";
                expect(false)<<stage<<" options::Exception "<<error.name<<":"<<property_host::debugText(error.message);
                throw std::runtime_error(std::string(stage)+" options::Exception "+error.name+":"+property_host::debugText(error.message));
            }catch(const tex::TexError& error){
                std::cerr<<stage<<" TexError "<<property_host::debugText(error.id)<<":"<<property_host::debugText(error.message)<<"\n";
                expect(false)<<stage<<" TexError "<<property_host::debugText(error.id)<<":"<<property_host::debugText(error.message);
                throw std::runtime_error(std::string(stage)+" TexError "+property_host::debugText(error.id)+":"+property_host::debugText(error.message));
            }catch(const std::exception& error){
                std::cerr<<stage<<" std::exception "<<error.what()<<"\n";
                expect(false)<<stage<<" std::exception "<<error.what();
                throw std::runtime_error(std::string(stage)+" std::exception "+error.what());
            }catch(...){
                std::cerr<<stage<<" unknown native exception\n";
                expect(false)<<stage<<" unknown native exception";
                throw std::runtime_error(std::string(stage)+" unknown native exception");
            }
        };
        // Match the pinned nested-local-shadow oracle: the three reads occur
        // in the inner, outer, then global scope, respectively.
        const auto local=parseStage(u"\\newcommand{\\bgLocal}{z}\\begingroup\\renewcommand{\\bgLocal}{x}\\begingroup\\renewcommand{\\bgLocal}{y}\\bgLocal\\endgroup\\bgLocal\\endgroup\\bgLocal","local-shadow");
        expect(local.isNode());
        bool wrapperGetTextTypeError=false;
        try{(void)tex::NodeUtil::instance().getText(local);}
        catch(const opts::Exception& error){
            wrapperGetTextTypeError=error.name=="TypeError"&&
                property_host::debugText(error.message)=="node.getText is not a function";
        }
        expect(wrapperGetTextTypeError)<<"pinned NodeUtil.getText delegates to a missing inferredMrow.getText";
        const auto localChildren=tex::NodeUtil::instance().getChildren(local);
        std::u16string localText;
        for(std::size_t index=0;index<localChildren->size();++index)
            localText+=tex::NodeUtil::instance().getText(localChildren->getProperty(std::to_string(index)));
        expect(localText==u"yxz")<<"local-shadow text="<<property_host::debugText(localText);
        expect(inspectStage("local-shadow stack index",[&]{return stack->index();})==-100);

        const auto global=parseStage(u"\\begingroup\\global\\def\\bgGlobal{x}\\endgroup\\bgGlobal","global-definition");
        expect(global.isNode());
        const auto globalText=inspectStage("global-definition NodeUtil::getText",[&]{return tex::NodeUtil::instance().getText(global);});
        expect(globalText==u"x")<<"global-definition text="<<property_host::debugText(globalText);
        expect(inspectStage("global-definition stack index",[&]{return stack->index();})==-100);
        const auto after=parseStage(u"\\bgGlobal","global-after-scope");
        expect(after.isNode());
        const auto afterText=inspectStage("global-after-scope NodeUtil::getText",[&]{return tex::NodeUtil::instance().getText(after);});
        expect(afterText==u"x")<<"global-after-scope text="<<property_host::debugText(afterText);
        expect(inspectStage("global-after-scope stack index",[&]{return stack->index();})==-100);
    };

    "reset_sandbox_and_repeated_formulas_keep_the_real_stack"_test=[] {
        BegingroupRuntime fixture;
        const auto options=fixture.parseOptions();
        const auto stack=tex::begingroupStack(*options);
        const auto parseStage=[&](std::u16string input,std::string_view stage){
            try{return fixture.parse(std::move(input));}
            catch(const opts::Exception& error){
                std::cerr<<stage<<" options::Exception "<<error.name<<":"<<property_host::debugText(error.message)<<"\n";
                expect(false)<<stage<<" options::Exception "<<error.name<<":"<<property_host::debugText(error.message);
                throw std::runtime_error(std::string(stage)+" options::Exception "+error.name+":"+property_host::debugText(error.message));
            }catch(const tex::TexError& error){
                std::cerr<<stage<<" TexError "<<property_host::debugText(error.id)<<":"<<property_host::debugText(error.message)<<"\n";
                expect(false)<<stage<<" TexError "<<property_host::debugText(error.id)<<":"<<property_host::debugText(error.message);
                throw std::runtime_error(std::string(stage)+" TexError "+property_host::debugText(error.id)+":"+property_host::debugText(error.message));
            }catch(const std::exception& error){std::cerr<<stage<<" std::exception "<<error.what()<<"\n";expect(false)<<stage<<" std::exception "<<error.what();throw std::runtime_error(std::string(stage)+" std::exception "+error.what());}
            catch(...){std::cerr<<stage<<" unknown native exception\n";expect(false)<<stage<<" unknown native exception";throw std::runtime_error(std::string(stage)+" unknown native exception");}
        };
        (void)parseStage(u"\\begingroup\\newcommand{\\bgShared}{x}","leave-group-open");
        expect(stack->index()==-101);
        const auto shared=parseStage(u"\\bgShared","repeated-formula-shared-definition");
        expect(tex::NodeUtil::instance().getText(shared)==u"x");
        (void)parseStage(u"\\endgroup","close-group");
        expect(stack->index()==-100);
        bool removed=false;
        try{(void)fixture.parse(u"\\bgShared");}
        catch(const tex::TexError& error){
            removed=error.id==u"UndefinedControlSequence"&&
                error.message==u"Undefined control sequence \\bgShared";
        }
        expect(removed)<<"post-group definition lookup must throw Source UndefinedControlSequence";
        expect(stack->index()==-100);

        (void)parseStage(u"\\begingroup\\begingroupReset","reset");
        expect(stack->index()==-100);
        (void)parseStage(u"\\begingroupSandbox","sandbox");
        expect(stack->base()==-101);
        expect(stack->index()==-101);
    };

    "group_underflow_uses_source_tex_error"_test=[] {
        BegingroupRuntime fixture;
        const auto stack=tex::begingroupStack(*fixture.parseOptions());
        bool threw=false;
        try{stack->pop();}catch(const tex::TexError& error){threw=error.id==u"MissingBegingroup";}
        expect(threw);
        expect(stack->index()==-100);
    };
};
}
