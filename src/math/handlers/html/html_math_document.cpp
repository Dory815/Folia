module folia.math.handlers.html.html_math_document;
import folia.math.util.options;
import folia.math.handlers.html.html_dom_strings;
import folia.math.input.tex.parser;
import std;

namespace folia::math::handlers::html {
using namespace options;
namespace {
Key key(std::u16string_view s){return Key(std::u16string(s));}
Value arg(std::span<const Value> a,std::size_t i){return i<a.size()?a[i]:Value(Undefined{});}
}

HTMLMathDocument::HTMLMathDocument(Context& context,ContextPropertyBindings& properties,
    util::RuntimeModules& modules,regexp_host::RegExpBindings& regexp,
    std::vector<core::InputJaxBinding> inputJax,
    std::shared_ptr<core::OutputJax> output,ObjectRef outputIdentity,
    std::shared_ptr<AbstractDOMAdaptor> adaptor,ObjectRef adaptorIdentity,
    std::shared_ptr<mml::MmlFactory> mmlFactory,DOMNodeRef document,
    std::function<Value(const mml::NodeValue&)> mmlProject,
    std::function<mml::MmlNode&(ObjectRef)> mmlResolve,std::shared_ptr<void> serviceOwner)
    :core::MathDocument(context,properties,modules,std::move(inputJax),std::move(output),
       outputIdentity,std::move(adaptor),adaptorIdentity,std::move(mmlFactory),
       std::move(document),std::move(mmlProject),std::move(mmlResolve),std::move(serviceOwner)),
     regexp_(&regexp){}

std::shared_ptr<HTMLMathDocument> HTMLMathDocument::create(Context& context,
    ContextPropertyBindings& properties,util::RuntimeModules& modules,
    regexp_host::RegExpBindings& regexp,
    std::vector<core::InputJaxBinding> inputJax,std::shared_ptr<core::OutputJax> output,
    ObjectRef outputIdentity,std::shared_ptr<AbstractDOMAdaptor> adaptor,ObjectRef adaptorIdentity,
    std::shared_ptr<mml::MmlFactory> mmlFactory,DOMNodeRef document,
    std::function<Value(const mml::NodeValue&)> mmlProject,
    std::function<mml::MmlNode&(ObjectRef)> mmlResolve,Value supplied,
    std::shared_ptr<void> serviceOwner){
    auto result=std::shared_ptr<HTMLMathDocument>(new HTMLMathDocument(context,properties,modules,regexp,
        std::move(inputJax),std::move(output),outputIdentity,std::move(adaptor),adaptorIdentity,
        std::move(mmlFactory),std::move(document),std::move(mmlProject),std::move(mmlResolve),std::move(serviceOwner)));
    result->identity_=context.object();
    context.payload(result->identity_)=std::static_pointer_cast<NativeObject>(result);
    result->initialize(std::move(supplied),{},Undefined{});
    return result;
}

void HTMLMathDocument::initialize(Value supplied,ObjectRef itemConstructor,Value classOptions){
    // The HTML handler owns the real MathItem constructor; generic default
    // options and conversion remain in core::MathDocument.
    const std::weak_ptr<core::MathDocument> weak=core::MathDocument::weak_from_this();
    if(itemConstructor==ObjectRef{}){
        itemConstructor=context_->nativeConstructor(u"HTMLMathItem",
            [weak](Context&,std::span<const Value> args)->ObjectRef{
                auto base=weak.lock();
                auto self=base?std::dynamic_pointer_cast<HTMLMathDocument>(base):nullptr;
                if(!self)throw Exception("TypeError",u"HTMLMathDocument was released");
                return self->constructItem(arg(args,0),arg(args,1),arg(args,2),arg(args,3),arg(args,4));
            });
    }
    itemConstructor_=itemConstructor;
    if(!context_->hasOwn(itemConstructor_,key(u"prototype")))
        throw Exception("TypeError",u"HTMLMathItem constructor has no prototype");
    itemPrototype_=std::get<ObjectRef>(context_->get(Value(itemConstructor_),key(u"prototype")).data);
    installItemSurface();
    auto stringsModule=HTMLDomStringsModule::initializeModule(*modules_,*properties_,*regexp_);
    const Value domStringOptions=context_->get(Value(stringsModule->identity()),key(u"OPTIONS"));
    const std::array<Value,1> selectors{domStringOptions};
    auto separated=separateOptions(*context_,std::move(supplied),selectors);
    if(separated.size()!=2)throw Exception("TypeError",u"HTMLMathDocument options separation failed");
    Value htmlOptions=std::move(separated[0]);
    Value domOptions=std::move(separated[1]);
    const bool hasSuppliedDomStrings=context_->hasOwn(htmlOptions,key(u"DomStrings"));
    const Value suppliedDomStrings=hasSuppliedDomStrings?
        context_->get(htmlOptions,key(u"DomStrings")):Value(Undefined{});
    if(!std::holds_alternative<ObjectRef>(classOptions.data))classOptions=Value(context_->object());
    const Value classDefaults(classOptions);
    if(!context_->hasOwn(classDefaults,key(u"DomStrings")))
        context_->set(classDefaults,key(u"DomStrings"),Value(Null{}));
    initializeCore(std::move(htmlOptions),itemConstructor_,classDefaults);
    // DomStrings is an owner-valued option. Preserve the supplied Source
    // object through option merging so the document assigns its adaptor to
    // that same identity below.
    if(hasSuppliedDomStrings&&truthy(suppliedDomStrings))
        context_->set(options_,key(u"DomStrings"),suppliedDomStrings);

    Value domStrings=context_->get(options_,key(u"DomStrings"));
    if(!truthy(domStrings)){
        auto nativeStrings=stringsModule->create(std::move(domOptions));
        domStrings=Value(nativeStrings->identity());
    }
    context_->set(identity_,key(u"domStrings"),domStrings);
    // Source always assigns the actual adaptor identity. Only the canonical
    // HTMLDomStrings owner retains its native provider for the field's lease.
    const auto* domIdentity=std::get_if<ObjectRef>(&domStrings.data);
    if(domIdentity){
        const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(
            &context_->payload(*domIdentity));
        auto nativeStrings=payload?std::dynamic_pointer_cast<HTMLDomStrings>(*payload):nullptr;
        if(nativeStrings)nativeStrings->setAdaptor(Value(adaptorIdentity_),adaptor_);
        else context_->set(*domIdentity,key(u"adaptor"),Value(adaptorIdentity_));
    }else if(!std::holds_alternative<Null>(domStrings.data)&&
             !std::holds_alternative<Undefined>(domStrings.data)){
        // Match Source assignment semantics for primitive custom values: the
        // attempted property write is observable through Context.set.
        context_->set(domStrings,key(u"adaptor"),Value(adaptorIdentity_));
    }
}

ObjectRef HTMLMathDocument::constructItem(Value mathValue,Value inputValue,
    Value displayValue,Value startValue,Value endValue){
    core::MathInputJaxReference input=PropertyNull{};
    if(const auto* id=std::get_if<ObjectRef>(&inputValue.data))
        for(const auto& candidate:inputJax_)if(candidate.identity==*id){input=candidate.owner.get();break;}
    const auto id=context_->object();
    if(privateRequestOwnerScopeActive_){
        try{privateRequestItems_.push_back(id);}
        catch(...){context_->retireObjectRecord(id);throw;}
    }
    auto item=std::make_shared<HTMLMathItem>(*context_,id,*this,
        properties_->from(mathValue),std::move(input),properties_->from(displayValue),
        properties_->from(startValue),properties_->from(endValue));
    context_->setPrototype(id,itemPrototype_);
    context_->payload(id)=std::static_pointer_cast<NativeObject>(item);
    auto* propertyBindings=properties_;
    auto graphLease=serviceOwner_;
    // `_state` is the actual Source field backed by AbstractMathItem::state_.
    // Keep direct property writes separate from state(), whose Source method
    // performs document/output cleanup when advancing or rewinding phases.
    auto stateCell=std::make_shared<NativeDataField>(item,
        [item,propertyBindings,graphLease]{return propertyBindings->to(item->sourceStateField());},
        [item,propertyBindings,graphLease](Value value){item->setSourceStateField(propertyBindings->from(std::move(value)));},
        [item,graphLease]{item->setSourceStateField(Missing{});});
    context_->bindData(id,key(u"_state"),std::move(stateCell));
    const auto propField=[&](std::u16string_view name,Property& field){
        auto* cellValue=&field;
        auto cell=std::make_shared<NativeDataField>(item,
            [item,propertyBindings,graphLease,cellValue]{return propertyBindings->to(*cellValue);},
            [item,propertyBindings,graphLease,cellValue](Value value){*cellValue=propertyBindings->from(std::move(value));},
            [item,graphLease,cellValue]{*cellValue=Missing{};});
        context_->bindData(id,key(name),std::move(cell));
    };
    propField(u"math",item->math); propField(u"display",item->display);
    propField(u"metrics",item->metrics); propField(u"inputData",item->inputData);
    propField(u"outputData",item->outputData);
    const auto locationField=[&](std::u16string_view name,core::MathLocation& location){
        auto* locationValue=&location;
        auto cell=std::make_shared<NativeDataField>(item,
            [item,propertyBindings,graphLease,locationValue]{
                if(const auto* propertyValue=std::get_if<Property>(locationValue))return propertyBindings->to(*propertyValue);
                throw Exception("Unimplemented",u"HTMLMathItem native location identity projection is unavailable");
            },
            [item,propertyBindings,graphLease,locationValue](Value value){*locationValue=propertyBindings->from(std::move(value));},
            [item,graphLease,locationValue]{*locationValue=Property(Missing{});});
        context_->bindData(id,key(name),std::move(cell));
    };
    locationField(u"start",item->start); locationField(u"end",item->end);
    const auto inputOwners=inputJax_;
    auto inputCell=std::make_shared<NativeDataField>(item,
        [item,inputOwners,graphLease]{
            if(const auto* input=std::get_if<core::MathInputJaxService*>(&item->inputJax))
                for(const auto& binding:inputOwners)if(binding.owner.get()==*input)return Value(binding.identity);
            return Value(Null{});
        },
        [item,inputOwners,graphLease](Value value){
            const auto* identity=std::get_if<ObjectRef>(&value.data);
            if(!identity){item->inputJax=PropertyNull{};return;}
            for(const auto& binding:inputOwners)if(binding.identity==*identity){item->inputJax=binding.owner.get();return;}
            item->inputJax=PropertyNull{};
        });
    context_->bindData(id,key(u"inputJax"),std::move(inputCell));
    auto projectMml=mmlProject_;
    auto resolveMml=mmlResolve_;
    auto rootCell=std::make_shared<NativeDataField>(item,
        [item,projectMml,graphLease]{return projectMml(item->root);},
        [item,resolveMml,graphLease](Value value){
            if(std::holds_alternative<Undefined>(value.data)){item->root=Missing{};return;}
            if(std::holds_alternative<Null>(value.data)){item->root=PropertyNull{};return;}
            if(const auto* identity=std::get_if<ObjectRef>(&value.data)){item->root=&resolveMml(*identity);return;}
            throw Exception("TypeError",u"HTMLMathItem.root must be an MML node or nullish value");
        });
    context_->bindData(id,key(u"root"),std::move(rootCell));
    auto outputCell=std::make_shared<NativeDataField>(item,
        [item,propertyBindings,graphLease]{return core::sourceOutputValue(item->typesetRoot,*propertyBindings);},
        [item,propertyBindings,graphLease](Value value){
            const auto projected=propertyNode(propertyBindings->from(std::move(value)));
            if(const auto* node=std::get_if<DOMNodeRef>(&projected)){item->typesetRoot=*node;return;}
            if(std::holds_alternative<Missing>(projected)){item->typesetRoot=Missing{};return;}
            if(std::holds_alternative<PropertyNull>(projected)){item->typesetRoot=PropertyNull{};return;}
            if(const auto* raw=std::get_if<Property>(&projected);raw&&raw->missing()){
                item->typesetRoot=Missing{};return;
            }
            throw Exception("TypeError",u"HTMLMathItem.typesetRoot must be a canonical adaptor node");
        });
    context_->bindData(id,key(u"typesetRoot"),std::move(outputCell));
    return id;
}

void HTMLMathDocument::beginRequestOwnerScopeImpl(){
    if(!privateRequestItems_.empty()||!filterHandles_.empty()||!requestTexOwners_.empty())
        throw Exception("TypeError",u"HTML document request owner cleanup is pending");
    beginPrivateRequestOwnerScope();
    privateRequestItemRetireIndex_=0;
    const auto open=[this](std::shared_ptr<core::JaxFilterRecordOwnerScope> owner){
        if(!owner)return;
        if(std::ranges::any_of(filterHandles_,[&](const FilterHandle& existing){
            return existing.owner.get()==owner.get();}))return;
        auto token=owner->begin(sourceIdentity());
        filterHandles_.emplace_back(std::move(owner),std::move(token));
    };
    try{
        filterHandles_.reserve(inputJax_.size()+1);
        requestTexOwners_.reserve(inputJax_.size());
        for(const auto& input:inputJax_)
            if(auto* jax=dynamic_cast<core::AbstractInputJax*>(input.owner.get()))
                open(jax->filterRecordOwner());
        if(auto* jax=dynamic_cast<core::AbstractOutputJax*>(output_.get()))
            open(jax->filterRecordOwner());
        for(const auto& input:inputJax_){
            if(auto* tex=dynamic_cast<tex::TeX*>(input.owner.get())){
                tex->beginRequestOwnerScope();
                requestTexOwners_.push_back(input.owner.get());
            }
        }
    }catch(...){
        // Only the tokens and TeX scopes opened by this document are saved.
        try{retireRequestItems();}catch(...){}
        throw;
    }
}
void HTMLMathDocument::retireRequestScopes(){
    while(!filterHandles_.empty()){
        auto& handle=filterHandles_.back();
        handle.owner->close(handle.token);
        filterHandles_.pop_back();
    }
    while(!requestTexOwners_.empty()){
        auto* tex=dynamic_cast<tex::TeX*>(requestTexOwners_.back());
        tex->releaseRequestReferences();
        requestTexOwners_.pop_back();
    }
}
void HTMLMathDocument::retireRequestItems(){
    if(!privateRequestOwnerScopeActive_)return;
    retireRequestScopes();
    while(privateRequestItemRetireIndex_<privateRequestItems_.size()){
        const auto identity=privateRequestItems_[privateRequestItemRetireIndex_];
        context_->retireObjectRecord(identity);
        ++privateRequestItemRetireIndex_;
    }
    privateRequestItems_.clear();
    privateRequestItemRetireIndex_=0;
    retirePrivateRequestOwnerScope();
}

HTMLMathItem& HTMLMathDocument::requireItem(Context& context,Value receiver){
    const auto* id=std::get_if<ObjectRef>(&receiver.data);
    if(!id)throw Exception("TypeError",u"HTMLMathItem receiver is not an object");
    const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&context.payload(*id));
    auto item=payload?std::dynamic_pointer_cast<HTMLMathItem>(*payload):nullptr;
    if(!item)throw Exception("TypeError",u"HTMLMathItem receiver has no canonical owner");
    return *item;
}

void HTMLMathDocument::installItemSurface(){
    const std::weak_ptr<core::MathDocument> weak=core::MathDocument::weak_from_this();
    auto compile=context_->function(u"compile",[weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
        auto documentLease=weak.lock();
        if(!documentLease)throw Exception("TypeError",u"HTMLMathDocument was released");
        (void)documentLease;
        auto& item=requireItem(cx,std::move(receiver));
        const Value documentValue=arg(args,0);
        const auto* docId=std::get_if<ObjectRef>(&documentValue.data);
        if(!docId)throw Exception("TypeError",u"MathItem.compile requires a document");
        const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&cx.payload(*docId));
        auto document=payload?std::dynamic_pointer_cast<core::MathDocument>(*payload):nullptr;
        if(!document)throw Exception("TypeError",u"MathItem.compile document is invalid");
        item.compile(document.get());return Value(Undefined{});
    });
    context_->set(itemPrototype_,key(u"compile"),Value(compile));
    auto typeset=context_->function(u"typeset",[weak](Context& cx,Value receiver,std::span<const Value> args)->Value{
        auto documentLease=weak.lock();
        if(!documentLease)throw Exception("TypeError",u"HTMLMathDocument was released");
        (void)documentLease;
        auto& item=requireItem(cx,std::move(receiver));
        const Value documentValue=arg(args,0);
        const auto* docId=std::get_if<ObjectRef>(&documentValue.data);
        if(!docId)throw Exception("TypeError",u"MathItem.typeset requires a document");
        const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&cx.payload(*docId));
        auto document=payload?std::dynamic_pointer_cast<core::MathDocument>(*payload):nullptr;
        if(!document)throw Exception("TypeError",u"MathItem.typeset document is invalid");
        item.typeset(document.get());return Value(Undefined{});
    });
    context_->set(itemPrototype_,key(u"typeset"),Value(typeset));
}

core::MathItem& HTMLMathDocument::resolveMathItem(ObjectRef identity){
    const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&context_->payload(identity));
    auto item=payload?std::dynamic_pointer_cast<HTMLMathItem>(*payload):nullptr;
    if(!item)throw Exception("TypeError",u"MathItem identity has no HTML native owner");
    return *item;
}
Value HTMLMathDocument::sourceResult(core::MathItem& item){
    if(const auto* node=std::get_if<DOMNodeRef>(&item.typesetRoot))
        return properties_->to(nodeProperty(*node));
    return mmlProject_(item.root);
}
bool HTMLMathDocument::processStrings(core::MathInputJaxReference reference)const{
    const auto* service=std::get_if<core::MathInputJaxService*>(&reference);
    auto* input=service&&*service?dynamic_cast<core::InputJax*>(*service):nullptr;
    if(!input)throw Exception("TypeError",u"MathItem.inputJax.processStrings is not callable");
    return input->processStrings();
}
}
