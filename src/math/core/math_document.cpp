module folia.math.core.math_document;
import folia.math.core.math_item_host;
import folia.math.util.options;
import folia.math.util.property_host;
import std;

namespace folia::math::core {
using namespace options;
namespace {
Key key(std::u16string_view text){return Key(std::u16string(text));}
Key indexKey(std::size_t index){
    const auto ascii=std::to_string(index);
    return key(std::u16string(ascii.begin(),ascii.end()));
}
Value array(Context& context,std::span<const Value> values){
    auto result=context.array(static_cast<std::uint32_t>(values.size()));
    for(std::size_t i=0;i<values.size();++i)context.set(result,indexKey(i),values[i]);
    return Value(result);
}
Property property(ContextPropertyBindings& properties,const Value& value){
    return properties.from(value);
}
}

MathDocument::MathDocument(Context& context,ContextPropertyBindings& properties,
    util::RuntimeModules& modules,std::vector<InputJaxBinding> inputJax,
    std::shared_ptr<OutputJax> output,ObjectRef outputIdentity,
    std::shared_ptr<AbstractDOMAdaptor> adaptor,ObjectRef adaptorIdentity,
    std::shared_ptr<mml::MmlFactory> mmlFactory,DOMNodeRef document,
    std::function<Value(const mml::NodeValue&)> mmlProject,
    std::function<mml::MmlNode&(ObjectRef)> mmlResolve,
    std::shared_ptr<void> serviceOwner)
    :context_(&context),properties_(&properties),modules_(&modules),
     serviceOwner_(std::move(serviceOwner)),inputJax_(std::move(inputJax)),
     output_(std::move(output)),outputIdentity_(outputIdentity),
     adaptor_(std::move(adaptor)),adaptorIdentity_(adaptorIdentity),
     mmlFactory_(std::move(mmlFactory)),mmlProject_(std::move(mmlProject)),mmlResolve_(std::move(mmlResolve)),
     documentNode_(std::move(document)){
    if(&properties.context()!=&context||&modules.context()!=&context)
        throw Exception("TypeError",u"MathDocument services belong to another Context");
    if(!output_||outputIdentity_==ObjectRef{}||!adaptor_||adaptorIdentity_==ObjectRef{}||
       !mmlFactory_||!mmlProject_||!mmlResolve_||!documentNode_||!serviceOwner_)
        throw Exception("TypeError",u"MathDocument requires its actual input, output, DOM and MML owners");
}

void MathDocument::initializeCore(Value supplied,ObjectRef mathItemConstructor,Value classOptions){
    if(mathItemConstructor==ObjectRef{})throw Exception("TypeError",u"MathDocument requires its actual MathItem constructor");
    itemConstructor_=mathItemConstructor;
    for(auto& binding:inputJax_){
        if(!binding.owner||binding.identity==ObjectRef{})
            throw Exception("TypeError",u"MathDocument InputJax entries need their actual owner and identity");
        binding.owner->setAdaptor(InputAdaptorReference{adaptorIdentity_});
        binding.owner->setMmlFactory(InputMmlFactoryReference{mmlFactory_});
    }
    auto linked=modules_->getOrCreate<util::LinkedListModule>(
        u"@mathjax/src/ts/util/LinkedList",[this]{
            return std::make_shared<util::LinkedListModule>(*context_);
        });
    math_=std::make_unique<AbstractMathList>(*linked,this);
    renderActionsIdentity_=context_->object();
    renderActions_=std::make_unique<RenderActions>(*context_,renderActionsIdentity_);

    const auto tuple=[&](std::initializer_list<Value> values){
        const std::vector<Value> slots(values);
        return array(*context_,slots);
    };
    const ObjectRef actions=context_->object();
    context_->set(actions,key(u"find"),tuple({Value(10),Value(u"findMath"),Value(u""),Value(false)}));
    context_->set(actions,key(u"compile"),tuple({Value(20)}));
    context_->set(actions,key(u"metrics"),tuple({Value(110),Value(u"getMetrics"),Value(u""),Value(false)}));
    context_->set(actions,key(u"typeset"),tuple({Value(150)}));
    context_->set(actions,key(u"update"),tuple({Value(200),Value(u"updateDocument"),Value(false)}));

    const ObjectRef defaults=context_->object();
    if(std::holds_alternative<ObjectRef>(classOptions.data)){
        const auto source=std::get<ObjectRef>(classOptions.data);
        for(const auto& name:context_->enumerableStringKeys(source))
            context_->set(defaults,key(name),context_->get(Value(source),key(name)));
    }
    std::vector<Value> inputIds;
    inputIds.reserve(inputJax_.size());
    for(const auto& binding:inputJax_)inputIds.emplace_back(binding.identity);
    context_->set(defaults,key(u"InputJax"),array(*context_,inputIds));
    context_->set(defaults,key(u"OutputJax"),Value(outputIdentity_));
    context_->set(defaults,key(u"MathItem"),Value(itemConstructor_));
    context_->set(defaults,key(u"renderActions"),Value(actions));
    const std::array<Value,1> definitions{std::move(supplied)};
    options_=userOptions(*context_,defaultOptions(*context_,Value(defaults),{}),definitions);
    renderActions_->create(context_->get(options_,key(u"renderActions")));

    context_->set(identity_,key(u"options"),options_);
    context_->set(identity_,key(u"document"),Value(properties_->to(nodeProperty(documentNode_))));
    context_->set(identity_,key(u"inputJax"),context_->get(options_,key(u"InputJax")));
    context_->set(identity_,key(u"outputJax"),Value(outputIdentity_));
    context_->set(identity_,key(u"adaptor"),Value(adaptorIdentity_));
    context_->set(identity_,key(u"renderActions"),Value(renderActionsIdentity_));
    context_->set(identity_,key(u"_actionPromises"),Value(context_->array(0)));
    installCoreDocumentMethods();
}

void MathDocument::installCoreDocumentMethods(){
    auto install=[&](std::u16string_view name,NativeCall call){
        const auto function=context_->function(std::u16string(name),std::move(call));
        context_->set(identity_,key(name),Value(function));
    };
    install(u"getMetrics",[](Context& cx,Value receiver,std::span<const Value>)->Value{
        const auto* id=std::get_if<ObjectRef>(&receiver.data);
        if(!id)throw Exception("TypeError",u"MathDocument receiver is not an object");
        const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&cx.payload(*id));
        auto document=payload?std::dynamic_pointer_cast<MathDocument>(*payload):nullptr;
        if(!document)throw Exception("TypeError",u"MathDocument receiver has no native owner");
        document->output_->getMetrics(document.get());
        return Value(Undefined{});
    });
    install(u"updateDocument",[](Context& cx,Value receiver,std::span<const Value>)->Value{
        const auto* id=std::get_if<ObjectRef>(&receiver.data);
        if(!id)throw Exception("TypeError",u"MathDocument receiver is not an object");
        const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&cx.payload(*id));
        auto document=payload?std::dynamic_pointer_cast<MathDocument>(*payload):nullptr;
        if(!document)throw Exception("TypeError",u"MathDocument receiver has no native owner");
        document->math_->forEach([&](MathItem& item){item.updateDocument(document.get());});
        return Value(Undefined{});
    });
    install(u"clearPromises",[](Context& cx,Value receiver,std::span<const Value>)->Value{
        const auto* id=std::get_if<ObjectRef>(&receiver.data);
        if(!id)throw Exception("TypeError",u"MathDocument receiver is not an object");
        (void)cx.get(receiver,key(u"_actionPromises"));
        cx.set(*id,key(u"_actionPromises"),Value(cx.array(0)));
        return Value(Undefined{});
    });
}

Value MathDocument::convertCore(Value mathValue,Value supplied){
    const Value inputList=context_->get(Value(identity_),key(u"inputJax"));
    const Value firstInput=context_->get(inputList,indexKey(0));
    const ObjectRef defaults=context_->object();
    context_->set(defaults,key(u"format"),context_->get(firstInput,key(u"name")));
    context_->set(defaults,key(u"display"),Value(true));
    context_->set(defaults,key(u"end"),properties_->to(math_item_host::constant("LAST")));
    context_->set(defaults,key(u"em"),Value(16));
    context_->set(defaults,key(u"ex"),Value(8));
    context_->set(defaults,key(u"containerWidth"),Value(Null{}));
    context_->set(defaults,key(u"scale"),Value(1));
    context_->set(defaults,key(u"family"),Value(u""));
    const std::array<Value,1> definitions{std::move(supplied)};
    const Value settings=userOptions(*context_,Value(defaults),definitions);
    const auto getOption=[&](std::u16string_view name){return context_->get(settings,key(name));};
    const Value format=getOption(u"format");
    const Value display=getOption(u"display");
    const Value end=getOption(u"end");
    const Value ex=getOption(u"ex");
    const Value em=getOption(u"em");
    const Value width=getOption(u"containerWidth");
    const Value scale=getOption(u"scale");
    const Value family=getOption(u"family");
    const Value effectiveWidth=std::holds_alternative<Null>(width.data)
        ?Value(80*context_->toNumber(ex)):width;
    const Value reduce=context_->get(inputList,key(u"reduce"));
    const auto reducer=context_->function(u"MathDocument.convert.inputJax.reduce",
        [format](Context& cx,Value,std::span<const Value> args)->Value{
            const Value accumulator=args.empty()?Value(Undefined{}):args[0];
            const Value candidate=args.size()<2?Value(Undefined{}):args[1];
            return cx.get(candidate,Key(u"name"))==format?candidate:accumulator;
        });
    const std::array<Value,2> reduceArguments{Value(reducer),Value(Null{})};
    const Value selectedInput=context_->callMember(reduce,inputList,reduceArguments,
        u"MathDocument.convert inputJax.reduce");
    const Value selectedConstructor=context_->get(Value(identity_),key(u"options"));
    const Value actualItemConstructor=context_->get(selectedConstructor,key(u"MathItem"));
    const auto* actualItemCtor=std::get_if<ObjectRef>(&actualItemConstructor.data);
    if(!actualItemCtor)throw Exception("TypeError",u"MathDocument.options.MathItem is not a constructor");
    const std::array<Value,3> constructorArgs{mathValue,selectedInput,display};
    const ObjectRef itemIdentity=context_->construct(*actualItemCtor,constructorArgs);
    MathItem& item=resolveMathItem(itemIdentity);
    const DOMNodeValue body=adaptor_->body(documentNode_);
    const auto* bodyNode=std::get_if<DOMNodeRef>(&body);
    if(!bodyNode)throw Exception("TypeError",u"MathDocument adaptor.body did not return a node");
    writeMathLocation(item.start,"node",nodeProperty(*bodyNode),this);
    item.setMetrics(property(*properties_,em),property(*properties_,ex),
        property(*properties_,effectiveWidth),property(*properties_,scale));
    if(truthy(family)){
        const Value outputOptions=context_->get(Value(outputIdentity_),key(u"options"));
        if(truthy(context_->get(outputOptions,key(u"mtextInheritFont"))))
            if(auto outputData=PropertyList::record(item.outputData))outputData->set("mtextFamily",property(*properties_,family));
        if(truthy(context_->get(outputOptions,key(u"merrorInheritFont"))))
            if(auto outputData=PropertyList::record(item.outputData))outputData->set("merrorFamily",property(*properties_,family));
    }
    const Value clearPromises=context_->get(Value(identity_),key(u"clearPromises"));
    (void)context_->callMember(clearPromises,Value(identity_),{},u"MathDocument.clearPromises");
    item.convert(this,property(*properties_,end));
    return sourceResult(item);
}

Value MathDocument::convert(Value math,Value supplied){
    if(std::holds_alternative<Undefined>(supplied.data))supplied=Value(context_->object());
    return convertCore(std::move(math),std::move(supplied));
}
Context& MathDocument::sourceContext()const{return *context_;}
ObjectRef MathDocument::sourceIdentity()const{return identity_;}
MathRenderActionsReference MathDocument::renderActionsReference()const{return renderActions_.get();}
MathOutputJaxReference MathDocument::outputJaxReference()const{
    return static_cast<MathOutputJaxService*>(output_.get());
}
std::shared_ptr<AbstractDOMAdaptor> MathDocument::domAdaptorOwner()const{return adaptor_;}
DOMNodeValue MathDocument::documentNode()const{return documentNode_;}
MathListService& MathDocument::mathList()const{return *math_;}
MathLocationService& MathDocument::locationService()const{return const_cast<MathDocument&>(*this);}
Property MathDocument::get(const NativeObjectRef&,std::string_view){
    throw Exception("Unimplemented",u"Property-backed MathLocations are required by this document provider");
}
void MathDocument::set(const NativeObjectRef&,std::string_view,Property){
    throw Exception("Unimplemented",u"Property-backed MathLocations are required by this document provider");
}
}
