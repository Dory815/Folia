#include "../../tests/support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace opts=folia::math::options;
namespace tex=folia::math::tex;
namespace mml=folia::math::mml;

suite mmlNodeContextMethods=[] {
    "mml_node_context_methods_use_canonical_nodes_live_prototypes_parent_and_runtime_lease"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        regexp_host::RegExpBindings regexp(context);
        Unicode16Normalization unicode;
        core::InputJaxModule inputJax(context);
        core::FindMathModule findMath(context);
        tex::FindTeXModule findTeX(context,findMath,regexp);
        auto runtime=std::make_unique<tex::ParserRuntime>(
            context,functions,regexp,unicode,inputJax,findTeX);
        auto factory=std::make_shared<mml::MmlFactory>();
        (void)runtime->bindMmlFactory(factory);

        auto* parent=factory->create("mrow",tree::FactoryArgs{});
        auto* token=factory->create("mtext",tree::FactoryArgs{});
        auto* text=factory->create("text",tree::FactoryArgs{});
        auto* textNode=dynamic_cast<mml::TextNode*>(text);
        expect(textNode!=nullptr);
        if(!textNode)return;
        textNode->setText(u"tip");
        token->appendChild(mml::NodeValue(text));

        const auto parentIdentity=runtime->mmlNodeIdentities.find(parent);
        const auto tokenIdentity=runtime->mmlNodeIdentities.find(token);
        const auto textIdentity=runtime->mmlNodeIdentities.find(text);
        expect(parentIdentity.has_value());
        expect(tokenIdentity.has_value());
        expect(textIdentity.has_value());
        if(!parentIdentity||!tokenIdentity||!textIdentity)return;
        const auto parentObject=opts::Value(*parentIdentity);
        const auto tokenObject=opts::Value(*tokenIdentity);
        const auto textObject=opts::Value(*textIdentity);
        expect(runtime->mmlNodeIdentities.find(token)==tokenIdentity);
        expect(context.hasOwn(*tokenIdentity,opts::Key(u"parent")));
        expect(!context.hasOwn(*tokenIdentity,opts::Key(u"isKind")));
        expect(!context.hasOwn(*tokenIdentity,opts::Key(u"getText")));
        const auto parentPrototype=context.prototype(*parentIdentity);
        const auto tokenPrototype=context.prototype(*tokenIdentity);
        expect(parentPrototype.has_value());
        expect(tokenPrototype.has_value());
        if(!parentPrototype||!tokenPrototype)return;
        expect(*parentPrototype!=*tokenPrototype);
        expect(context.prototype(*tokenPrototype)==parentPrototype);
        expect(context.get(tokenObject,opts::Key(u"parent"))==opts::Value(opts::Null{}));
        expect(context.get(parentObject,opts::Key(u"getText"))==opts::Value(opts::Undefined{}));
        expect(context.get(textObject,opts::Key(u"parent"))==tokenObject);

        const auto kindMethod=context.get(tokenObject,opts::Key(u"isKind"));
        const auto getTextMethod=context.get(tokenObject,opts::Key(u"getText"));
        expect(std::holds_alternative<opts::ObjectRef>(kindMethod.data));
        expect(std::holds_alternative<opts::ObjectRef>(getTextMethod.data));
        const auto kindFunction=std::get<opts::ObjectRef>(kindMethod.data);
        const auto getTextFunction=std::get<opts::ObjectRef>(getTextMethod.data);
        expect(context.get(kindMethod,opts::Key(u"name"))==opts::Value(u""));
        expect(context.get(kindMethod,opts::Key(u"length"))==opts::Value(1));
        expect(context.get(getTextMethod,opts::Key(u"name"))==opts::Value(u""));
        expect(context.get(getTextMethod,opts::Key(u"length"))==opts::Value(0));
        const auto kindPrototypeDescriptor=context.descriptor(*parentPrototype,opts::Key(u"isKind"));
        const auto textPrototypeDescriptor=context.descriptor(*tokenPrototype,opts::Key(u"getText"));
        expect(kindPrototypeDescriptor.has_value());
        expect(textPrototypeDescriptor.has_value());
        if(kindPrototypeDescriptor){
            expect(kindPrototypeDescriptor->value==kindMethod);
            expect(kindPrototypeDescriptor->writable);
            expect(kindPrototypeDescriptor->enumerable);
            expect(kindPrototypeDescriptor->configurable);
        }
        if(textPrototypeDescriptor){
            expect(textPrototypeDescriptor->value==getTextMethod);
            expect(textPrototypeDescriptor->writable);
            expect(textPrototypeDescriptor->enumerable);
            expect(textPrototypeDescriptor->configurable);
        }
        const auto functionNameDescriptor=context.descriptor(kindMethod,opts::Key(u"name"));
        const auto functionLengthDescriptor=context.descriptor(kindMethod,opts::Key(u"length"));
        const auto textNameDescriptor=context.descriptor(getTextMethod,opts::Key(u"name"));
        const auto textLengthDescriptor=context.descriptor(getTextMethod,opts::Key(u"length"));
        expect(functionNameDescriptor.has_value());
        expect(functionLengthDescriptor.has_value());
        expect(textNameDescriptor.has_value());
        expect(textLengthDescriptor.has_value());
        if(functionNameDescriptor){
            expect(!functionNameDescriptor->writable);
            expect(!functionNameDescriptor->enumerable);
            expect(functionNameDescriptor->configurable);
        }
        if(functionLengthDescriptor){
            expect(!functionLengthDescriptor->writable);
            expect(!functionLengthDescriptor->enumerable);
            expect(functionLengthDescriptor->configurable);
        }
        if(textNameDescriptor){
            expect(!textNameDescriptor->writable);
            expect(!textNameDescriptor->enumerable);
            expect(textNameDescriptor->configurable);
        }
        if(textLengthDescriptor){
            expect(!textLengthDescriptor->writable);
            expect(!textLengthDescriptor->enumerable);
            expect(textLengthDescriptor->configurable);
        }
        const std::array<opts::Value,1> kindArg{opts::Value(u"mtext")};
        expect(context.call(kindFunction,tokenObject,kindArg)==opts::Value(true));
        const std::array<opts::Value,1> wrongKindArg{opts::Value(u"mrow")};
        expect(context.call(kindFunction,tokenObject,wrongKindArg)==opts::Value(false));
        expect(context.call(getTextFunction,tokenObject)==opts::Value(u"tip"));

        bool sawCanonicalReceiver=false;
        const auto override=context.function(u"isKind",
            [&](opts::Context&,opts::Value receiver,std::span<const opts::Value>)->opts::Value{
                sawCanonicalReceiver=receiver==tokenObject;
                return opts::Value(true);
            });
        context.set(*tokenIdentity,opts::Key(u"isKind"),opts::Value(override));
        const std::array<opts::Value,1> overrideArg{opts::Value(u"not-a-kind")};
        const auto overridden=context.get(tokenObject,opts::Key(u"isKind"));
        expect(context.call(std::get<opts::ObjectRef>(overridden.data),tokenObject,overrideArg)==opts::Value(true));
        expect(sawCanonicalReceiver);
        context.erase(*tokenIdentity,opts::Key(u"isKind"));
        const auto restored=context.get(tokenObject,opts::Key(u"isKind"));
        expect(context.call(std::get<opts::ObjectRef>(restored.data),tokenObject,kindArg)==opts::Value(true));

        context.set(*tokenIdentity,opts::Key(u"parent"),parentObject);
        expect(context.get(tokenObject,opts::Key(u"parent"))==parentObject);
        expect(parent->childNodes->empty());
        context.set(*tokenIdentity,opts::Key(u"parent"),opts::Value(opts::Null{}));
        expect(context.get(tokenObject,opts::Key(u"parent"))==opts::Value(opts::Null{}));
        context.set(*tokenIdentity,opts::Key(u"parent"),opts::Value(opts::Undefined{}));
        expect(context.get(tokenObject,opts::Key(u"parent"))==opts::Value(opts::Undefined{}));

        runtime.reset();
        bool expired=false;
        try{(void)context.call(kindFunction,tokenObject,kindArg);}
        catch(const opts::Exception& error){expired=error.name=="TypeError";}
        expect(expired);
        bool parentExpired=false;
        try{(void)context.get(tokenObject,opts::Key(u"parent"));}
        catch(const opts::Exception& error){parentExpired=error.name=="TypeError";}
        expect(parentExpired);
    };
};
}
