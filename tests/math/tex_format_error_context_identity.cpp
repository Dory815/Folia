#include "support/folia_test.hpp"
import folia.math.output.common;
import folia.math.output.svg.render_session;
import folia.math.input.tex.parser;
import folia.math.core.input_jax;
import folia.math.core.math_item;
import folia.math.core.math_render_actions;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_nodes.mtext;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o=folia::math::options;
namespace t=folia::math::tex;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;
common::FontLoadHost fontHost(){common::FontLoadHost host;host.asyncIsSynchronous=[] {return true;};host.warn=[](const std::any&){};return host;}
std::optional<o::ObjectRef> objectValue(const o::Value& value){if(const auto* object=std::get_if<o::ObjectRef>(&value.data))return *object;return {};}
void expectFormattedError(t::ParserRuntime& runtime,mml::NodeValue value,std::string_view text,
                          mml::MmlFactory* expectedFactory=nullptr){
  const auto label=[&](std::string_view field,std::string actual){
    return std::string("formatError ")+std::string(field)+" expected ["+std::string(text)+"] actual ["+actual+"]";
  };
  expect(value.isNode()) << label("result kind",value.isNode()?"node":"non-node");
  if(!value.isNode())return;
  mml::MmlNode* error=value.requireNode();
  expect(error->kind()=="merror") << label("root kind",error->kind());
  const auto errorText=([&]{const auto actual=error->attributes()->get("data-mjx-error");if(const auto* value=std::get_if<std::string>(&actual.value))return *value;return std::string("<non-string>");})();
  expect(error->attributes()->get("data-mjx-error")==Property(std::string(text))) << label("data-mjx-error",errorText);
  const auto children=error->childNodes;
  expect(children&&children->size()==1) << label("merror child count",children?std::to_string(children->size()):"null");
  if(!children||children->size()!=1)return;
  auto* row=children->get(0).requireNode();
  expect(row->kind()=="inferredMrow") << label("child kind",row->kind());
  expect(row->isInferred()) << label("child isInferred",row->isInferred()?"true":"false");
  const auto rowChildren=row->childNodes;
  expect(rowChildren&&rowChildren->size()==1) << label("inferred row child count",rowChildren?std::to_string(rowChildren->size()):"null");
  if(!rowChildren||rowChildren->size()!=1)return;
  auto* mtext=rowChildren->get(0).requireNode();
  expect(mtext->kind()=="mtext") << label("inferred row child kind",mtext->kind());
  const auto textChildren=mtext->childNodes;
  expect(textChildren&&textChildren->size()==1) << label("mtext child count",textChildren?std::to_string(textChildren->size()):"null");
  if(!textChildren||textChildren->size()!=1)return;
  auto* leaf=dynamic_cast<mml::TextNode*>(textChildren->get(0).requireNode());
  expect(leaf!=nullptr) << label("mtext leaf kind",textChildren->get(0).requireNode()->kind());
  if(leaf){
    const auto actual=property_host::debugText(leaf->getText());
    expect(static_cast<bool>(leaf->getText()==std::u16string(text.begin(),text.end()))) << label("text leaf",actual);
  }
  const auto identity=runtime.mmlNodeIdentities.find(error);
  expect(identity.has_value()) << label("canonical identity",identity?"present":"missing");
  if(expectedFactory&&identity){
    const auto expectedFactoryIdentity=runtime.mmlFactoryIdentities.find(expectedFactory);
    expect(expectedFactoryIdentity.has_value()) << label("expected factory identity",expectedFactoryIdentity?"present":"missing");
    if(expectedFactoryIdentity){
      const auto actualFactory=runtime.context().get(o::Value(*identity),o::Key(u"factory"));
      expect(actualFactory==o::Value(*expectedFactoryIdentity)) << label("Source factory identity",std::holds_alternative<o::Undefined>(actualFactory.data)?"missing":(actualFactory==o::Value(*expectedFactoryIdentity)?"matches expected factory identity":"different factory identity"));
    }
  }
}

o::ObjectRef makeError(o::Context& context,std::u16string id,std::u16string message){
  const auto error=context.object();context.set(o::Value(error),o::Key(u"id"),o::Value(std::move(id)));
  context.set(o::Value(error),o::Key(u"message"),o::Value(std::move(message)));return error;
}
struct FormatErrorMathItem final:core::AbstractMathItem,core::MathItemSourceIdentity{
  o::Context& context;o::ObjectRef identity;
  FormatErrorMathItem(o::Context& cx,o::ObjectRef id,std::u16string input,core::MathInputJaxReference jax)
    :core::AbstractMathItem(Property(property_host::debugText(input)),std::move(jax),Property(true)),context(cx),identity(id){}
  o::Context& sourceContext()const override{return context;}
  o::ObjectRef sourceIdentity()const override{return identity;}
};
std::shared_ptr<FormatErrorMathItem> mathItem(o::Context& context,const t::TeXRef& jax,std::u16string input){
  const auto identity=context.object();auto item=std::make_shared<FormatErrorMathItem>(context,identity,std::move(input),
    core::MathInputJaxReference{static_cast<core::MathInputJaxService*>(jax.get())});
  context.payload(identity)=std::static_pointer_cast<NativeObject>(item);return item;
}
o::Value installFormatter(t::ParserRuntime& runtime,const t::TeXRef& jax,
                         std::function<std::any(const FunctionReceiver&,FunctionArgs)> body){
  auto function=std::make_shared<Function>();function->call=std::move(body);
  OptionCallBinding binding;
  binding.receiver=[](o::Context&,o::Value receiver)->FunctionReceiver{return FunctionReceiver(std::move(receiver));};
  binding.arguments=[](o::Context&,std::span<const o::Value>){return std::vector<std::any>{};};
  binding.result=[&runtime](o::Context&,const std::any& result)->o::Value{
    const auto* node=std::any_cast<mml::NodeValue>(&result);
    if(!node||!node->isNode())throw o::Exception("TypeError",u"formatter result is not an MML node");
    const auto identity=runtime.mmlNodeIdentities.find(node->requireNode());
    if(!identity)throw o::Exception("TypeError",u"formatter result has no canonical MML identity");
    return o::Value(*identity);
  };
  const auto callback=runtime.functions().bind(std::move(function),std::move(binding),u"customFormatError");
  runtime.context().set(jax->options,o::Key(u"formatError"),callback);return callback;
}
suite tex_format_error_context_identity=[] {
  "typed_context_and_default_formatters_return_canonical_error_nodes_and_read_message_before_id"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();
    auto first=t::TeX::make(runtime),second=t::TeX::make(runtime);
    auto firstFactory=std::make_shared<mml::MmlFactory>(),secondFactory=std::make_shared<mml::MmlFactory>();
    first->setMmlFactory(firstFactory);second->setMmlFactory(secondFactory);
    const auto typed=first->formatError(t::TexError(u"Typed",u"typed line\nignored"));
    expectFormattedError(runtime,typed,"typed line",firstFactory.get());
    const auto methodValue=context.get(o::Value(first->instanceIdentity()),o::Key(u"formatError"));
    const auto method=objectValue(methodValue);expect(method.has_value());if(!method)return;
    std::vector<std::string> reads;
    const auto error=context.object();
    const auto messageGetter=context.function(u"get message",[&](o::Context&,o::Value receiver,std::span<const o::Value>){
      reads.push_back(receiver==o::Value(error)?"message":"message/wrong-this");return o::Value(u"context line\nignored");});
    const auto idGetter=context.function(u"get id",[&](o::Context&,o::Value receiver,std::span<const o::Value>){
      reads.push_back(receiver==o::Value(error)?"id":"id/wrong-this");return o::Value(u"Context");});
    context.define(error,o::Key(u"message"),o::Descriptor::access(messageGetter,{},true,true));
    context.define(error,o::Key(u"id"),o::Descriptor::access(idGetter,{},true,true));
    const std::array<o::Value,1> directArgs{o::Value(error)};
    const auto direct=context.call(*method,o::Value(first->instanceIdentity()),directArgs);
    const auto directIdentity=objectValue(direct);expect(directIdentity.has_value());if(!directIdentity)return;
    auto* directNode=runtime.mmlNodeIdentities.resolve(*directIdentity);
    expectFormattedError(runtime,mml::NodeValue(directNode),"context line",firstFactory.get());
    expect(reads==std::vector<std::string>{"message","id"});
    reads.clear();
    const auto defaultsValue=context.get(o::Value(runtime.texClass),o::Key(u"OPTIONS"));
    const auto defaults=objectValue(defaultsValue);expect(defaults.has_value());if(!defaults)return;
    const auto callback=objectValue(context.get(*defaults,o::Key(u"formatError")));
    expect(callback.has_value());if(!callback)return;
    const std::array<o::Value,2> callbackArgs{o::Value(second->instanceIdentity()),o::Value(error)};
    const auto callbackResult=context.call(*callback,second->options,callbackArgs);
    const auto callbackIdentity=objectValue(callbackResult);expect(callbackIdentity.has_value());if(!callbackIdentity)return;
    expectFormattedError(runtime,mml::NodeValue(runtime.mmlNodeIdentities.resolve(*callbackIdentity)),"context line",secondFactory.get());
    expect(reads==std::vector<std::string>{"message","id"});
  };
  "borrowed_formatError_method_uses_the_call_time_tex_factory"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();
    auto owner=t::TeX::make(runtime),receiver=t::TeX::make(runtime);
    auto ownerFactory=std::make_shared<mml::MmlFactory>(),receiverFactory=std::make_shared<mml::MmlFactory>();
    owner->setMmlFactory(ownerFactory);receiver->setMmlFactory(receiverFactory);
    const auto ownerFactoryIdentity=runtime.mmlFactoryIdentities.find(ownerFactory.get());
    const auto receiverFactoryIdentity=runtime.mmlFactoryIdentities.find(receiverFactory.get());
    expect(ownerFactoryIdentity.has_value()&&receiverFactoryIdentity.has_value());
    if(ownerFactoryIdentity&&receiverFactoryIdentity)expect(*ownerFactoryIdentity!=*receiverFactoryIdentity);
    const auto method=objectValue(context.get(o::Value(owner->instanceIdentity()),o::Key(u"formatError")));
    expect(method.has_value());if(!method)return;
    const auto error=makeError(context,u"Borrowed",u"receiver line\nignored");
    const std::array<o::Value,1> args{o::Value(error)};
    const auto returned=objectValue(context.call(*method,o::Value(receiver->instanceIdentity()),args));
    expect(returned.has_value());if(!returned)return;
    expectFormattedError(runtime,mml::NodeValue(runtime.mmlNodeIdentities.resolve(*returned)),"receiver line",receiverFactory.get());
  };
  "native_compile_parser_error_calls_custom_formatter_with_source_receiver_and_arguments"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();
    auto jax=t::TeX::make(runtime);auto factory=std::make_shared<mml::MmlFactory>();jax->setMmlFactory(factory);
    std::vector<std::string> trace;bool receiverMatches=false;bool jaxMatches=false;std::u16string id,message;
    (void)installFormatter(runtime,jax,[&](const FunctionReceiver& receiver,FunctionArgs args)->std::any{
      trace.push_back("callback");const auto* sourceReceiver=std::get_if<o::Value>(&receiver);
      receiverMatches=sourceReceiver&&*sourceReceiver==jax->options;
      expect(args.size()==2);if(args.size()!=2)return std::any(mml::NodeValue(Missing{}));
      const auto* calledJax=std::any_cast<t::TeXRef>(&args[0]);const auto* error=std::any_cast<t::TexError>(&args[1]);
      jaxMatches=calledJax&&*calledJax==jax;expect(error!=nullptr);
      if(error){id=error->id;message=error->message;}
      return std::any(jax->formatError(t::TexError(u"Custom",u"custom result\nignored")));
    });
    auto item=mathItem(context,jax,u"\\notACommand");
    const auto result=jax->compile(*item,core::MathDocumentReference{PropertyNull{}});
    expect(result.isNode());expect(trace==std::vector<std::string>{"callback"});expect(receiverMatches&&jaxMatches);
    expect(static_cast<bool>(id==u"UndefinedControlSequence"));
    expect(static_cast<bool>(message==u"Undefined control sequence \\notACommand"));
    const auto parseOptionsValue=jax->parseOptions();
    const auto* parseOptions=std::get_if<std::shared_ptr<t::ParseOptions>>(&parseOptionsValue);
    expect(parseOptions&&*parseOptions&&(*parseOptions)->error);
    bool found=false;const auto visit=[&](auto&& self,mml::NodeValue node)->void{
      if(!node.isNode())return;auto* current=node.requireNode();
      if(current->kind()=="merror"&&current->attributes()->get("data-mjx-error")==Property("custom result")){
        found=true;
        const auto nodeIdentity=runtime.mmlNodeIdentities.find(current);expect(nodeIdentity.has_value());
        const auto factoryIdentity=runtime.mmlFactoryIdentities.find(factory.get());expect(factoryIdentity.has_value());
        if(nodeIdentity&&factoryIdentity)expect(context.get(o::Value(*nodeIdentity),o::Key(u"factory"))==o::Value(*factoryIdentity));
      }
      const auto children=current->childNodes;for(std::size_t i=0;children&&i<children->size();++i)self(self,children->get(i));
    };visit(visit,result);expect(found);
  };
  "native_compile_propagates_custom_formatter_exception_after_parser_error"_test=[] {
    svg::NativeRenderSession session(fontHost());auto& runtime=session.services().parser();auto& context=session.context();
    auto jax=t::TeX::make(runtime);jax->setMmlFactory(std::make_shared<mml::MmlFactory>());int calls=0;
    (void)installFormatter(runtime,jax,[&](const FunctionReceiver& receiver,FunctionArgs args)->std::any{
      ++calls;const auto* options=std::get_if<o::Value>(&receiver);expect(options&&*options==jax->options);expect(args.size()==2);
      throw o::Exception("RangeError",u"formatter failure");
    });
    auto item=mathItem(context,jax,u"\\notACommand");bool propagated=false;
    try{(void)jax->compile(*item,core::MathDocumentReference{PropertyNull{}});}
    catch(const o::Exception& error){propagated=error.name=="RangeError"&&error.message==u"formatter failure";}
    expect(calls==1);expect(propagated);
  };
};
} // namespace
