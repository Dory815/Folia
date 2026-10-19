// Focused native checks for the pinned OutputJax source contract.
#include "../../tests/support/folia_test.hpp"
import folia.math.core.output_jax;
import folia.math.util.property_host;
namespace {
using namespace boost::ut;
using namespace folia::math;
namespace opts=folia::math::options;
struct Doc final:core::MathDocumentService {
    core::MathRenderActionsReference renderActionsReference()const override{return Missing{};}
    core::MathOutputJaxReference outputJaxReference()const override{return Missing{};}
};
struct Probe final:core::AbstractOutputJax {
    std::vector<std::any> resetArguments;
    Probe(OptionFunctionBindings& bindings,opts::ObjectRef identity,FunctionListContextBindings& filters):AbstractOutputJax(bindings,identity,filters){}
    core::MathOutput typeset(core::MathItem&,core::MathDocumentReference)override{return PropertyNull{};}
    core::MathOutput escaped(core::MathItem&,core::MathDocumentReference)override{return PropertyNull{};}
    void reset(FunctionArgs args={})override{resetArguments.assign(args.begin(),args.end());}
    using AbstractOutputJax::executeFilters;
};
struct Fixture {
    opts::Context context;
    OptionFunctionBindings bindings{context};
    core::OutputJaxModule module{context,bindings};
    opts::ObjectRef klass=context.constructor(u"Probe");
    std::shared_ptr<Probe> jax;
    Fixture(){
        context.setPrototype(klass,module.abstractClass());
        const auto instancePrototype=std::get<opts::ObjectRef>(context.get(opts::Value(klass),opts::Key(u"prototype")).data);
        const auto abstractPrototype=std::get<opts::ObjectRef>(context.get(opts::Value(module.abstractClass()),opts::Key(u"prototype")).data);
        context.setPrototype(instancePrototype,abstractPrototype);
        auto defaults=context.object();
        context.set(defaults,opts::Key(u"preFilters"),context.array());
        context.set(defaults,opts::Key(u"postFilters"),context.array());
        context.set(klass,opts::Key(u"NAME"),u"generic");
        context.set(klass,opts::Key(u"OPTIONS"),defaults);
    }
    opts::ObjectRef object(std::initializer_list<std::pair<std::u16string,opts::Value>> fields){
        auto result=context.object();
        for(const auto&[key,value]:fields)context.set(result,opts::Key(key),value);
        return result;
    }
    opts::ObjectRef array(std::initializer_list<opts::Value> values){
        auto result=context.array();std::size_t i=0;
        for(auto value:values){std::string digits=std::to_string(i++);context.set(result,opts::Key(std::u16string(digits.begin(),digits.end())),value);}
        return result;
    }
    std::shared_ptr<Probe> make(opts::Value supplied=opts::Undefined{}){
        return jax=core::makeOutputJax<Probe>(bindings,module.filterBindings(),klass,std::move(supplied));
    }
    opts::Value value(const Property& property){return module.filterBindings().toValue(property);}
    Property property(opts::Value value){return module.filterBindings().fromValue(std::move(value));}
    void setData(const core::OutputJaxFilterDataRef& record,Property value){record->setField(u"data",this->value(value));}
};
struct TestIndexedView final:opts::NativeIndexedView {
    std::vector<std::optional<opts::Descriptor>> indices;
    bool lengthWritable=true,lengthEnumerable=false,lengthConfigurable=false;
    std::uint32_t length()const override{return static_cast<std::uint32_t>(indices.size());}
    opts::Descriptor lengthDescriptor()const override{return opts::Descriptor::data(opts::Value(static_cast<double>(length())),lengthWritable,lengthEnumerable,lengthConfigurable);}
    std::optional<opts::Descriptor> indexDescriptor(std::uint32_t index)const override{
        return index<indices.size()?indices[index]:std::optional<opts::Descriptor>{};
    }
    std::vector<std::uint32_t> ownIndexKeys()const override{
        std::vector<std::uint32_t> result;
        for(std::uint32_t i=0;i<indices.size();++i)if(indices[i])result.push_back(i);
        return result;
    }
    void defineIndex(std::uint32_t index,opts::Descriptor descriptor)override{
        if(index>=indices.size())indices.resize(static_cast<std::size_t>(index)+1);
        indices[index]=std::move(descriptor);
    }
    void defineLength(opts::Descriptor descriptor)override{
        const auto n=std::get<double>(descriptor.value.data);
        indices.resize(static_cast<std::size_t>(n));
    }
    void deleteIndex(std::uint32_t index)override{if(index<indices.size())indices[index].reset();}
};
std::string yes(bool value){return value?"true":"false";}
std::string stringValue(opts::Value value){
    if(auto* text=std::get_if<std::u16string>(&value.data))return property_host::debugText(*text);
    if(std::holds_alternative<opts::Undefined>(value.data))return "undefined";
    if(std::holds_alternative<opts::Null>(value.data))return "null";
    return "other";
}
}

suite native_output_jax=[] {
    "native_indexed_view_prototype_assignment_uses_inherited_descriptors"_test=[] {
        opts::Context context;
        auto view=std::make_shared<TestIndexedView>();
        opts::ObjectRef receiver{};
        const auto setter=context.function(u"setter",[&](opts::Context&,opts::Value self,std::span<const opts::Value> args){
            receiver=std::get<opts::ObjectRef>(self.data);
            expect(eq(args.size(),std::size_t{1}));
            return opts::Value(opts::Undefined{});
        });
        view->indices.push_back(opts::Descriptor::access({},setter,true,true));
        view->indices.push_back(opts::Descriptor::data(opts::Value(1),false,true,true));
        const auto prototype=context.nativeArray(view);
        const auto object=context.objectWithPrototype(prototype);
        const std::string zero="0",one="1";
        context.set(object,opts::Key(std::u16string(zero.begin(),zero.end())),opts::Value(9));
        expect(receiver==object);
        std::string error="none";
        try{context.set(object,opts::Key(std::u16string(one.begin(),one.end())),opts::Value(2));}
        catch(const opts::Exception& caught){error=caught.name;}
        expect(eq(error,std::string("TypeError")));
    };
    "native_array_view_requires_ordinary_length_descriptor_flags"_test=[] {
        opts::Context context;auto invalid=std::make_shared<TestIndexedView>();invalid->lengthEnumerable=true;
        std::string error="none";try{(void)context.nativeArray(invalid);}catch(const opts::Exception& caught){error=caught.name;}
        expect(eq(error,std::string("TypeError")));
        auto fixed=std::make_shared<TestIndexedView>();fixed->lengthWritable=false;
        expect(std::holds_alternative<opts::ObjectRef>(opts::Value(context.nativeArray(fixed)).data));
    };
    "mathjax_output_jax_base_defaults_and_live_class_name"_test=[] {
        Fixture e;auto jax=e.make();
        expect(eq(stringValue(e.context.get(opts::Value(jax->instanceIdentity()),opts::Key(u"name"))),std::string("generic")));
        expect(std::holds_alternative<PropertyNull>(jax->adaptor));
        expect(eq(jax->preFilters->storageRef()->size(),std::size_t{0}));
        expect(eq(jax->postFilters->storageRef()->size(),std::size_t{0}));
        e.context.set(e.klass,opts::Key(u"NAME"),u"changed");
        expect(eq(stringValue(e.context.get(opts::Value(jax->instanceIdentity()),opts::Key(u"name"))),std::string("changed")));
    };
    "mathjax_output_jax_options_use_derived_defaults_and_shallow_nested_merge"_test=[] {
        Fixture e;auto inherited=e.context.get(opts::Value(e.klass),opts::Key(u"OPTIONS"));
        auto defaults=std::get<opts::ObjectRef>(inherited.data);
        auto nested=e.object({{u"x",1},{u"y",2}});
        e.context.set(defaults,opts::Key(u"scale"),1);
        e.context.set(defaults,opts::Key(u"nested"),nested);
        auto derived=e.context.constructor(u"Derived");e.context.setPrototype(derived,e.module.abstractClass());
        const auto derivedPrototype=std::get<opts::ObjectRef>(e.context.get(opts::Value(derived),opts::Key(u"prototype")).data);
        const auto abstractPrototype=std::get<opts::ObjectRef>(e.context.get(opts::Value(e.module.abstractClass()),opts::Key(u"prototype")).data);
        e.context.setPrototype(derivedPrototype,abstractPrototype);
        e.context.set(derived,opts::Key(u"NAME"),u"scene");e.context.set(derived,opts::Key(u"OPTIONS"),defaults);
        auto userNested=e.object({{u"x",3}});auto supplied=e.object({{u"scale",2},{u"nested",userNested}});
        auto jax=core::makeOutputJax<Probe>(e.bindings,e.module.filterBindings(),derived,supplied);
        auto options=std::get<opts::ObjectRef>(jax->options.data);
        auto actualNested=std::get<opts::ObjectRef>(e.context.get(jax->options,opts::Key(u"nested")).data);
        expect(eq(e.context.toNumber(e.context.get(jax->options,opts::Key(u"scale"))),2.0));
        expect(eq(e.context.toNumber(e.context.get(opts::Value(actualNested),opts::Key(u"x"))),3.0));
        expect(eq(e.context.toNumber(e.context.get(opts::Value(actualNested),opts::Key(u"y"))),2.0));
        expect(actualNested!=nested&&actualNested!=userNested);
        expect(options!=defaults);
        expect(eq(stringValue(e.context.get(opts::Value(jax->instanceIdentity()),opts::Key(u"name"))),std::string("scene")));
    };
    "mathjax_output_jax_set_adaptor_preserves_identity_and_null"_test=[] {
        Fixture e;auto jax=e.make();const auto self=opts::Value(jax->instanceIdentity());
        const auto setAdaptor=e.context.get(self,opts::Key(u"setAdaptor"));
        auto adaptor=e.object({{u"tag",u"host"}});
        const std::array<opts::Value,1> objectArg{opts::Value(adaptor)};
        expect(std::holds_alternative<opts::Undefined>(e.context.call(std::get<opts::ObjectRef>(setAdaptor.data),self,objectArg).data));
        const auto retained=e.context.get(self,opts::Key(u"adaptor"));expect(retained==opts::Value(adaptor));
        e.context.set(adaptor,opts::Key(u"tag"),u"mutated");
        expect(e.context.get(retained,opts::Key(u"tag"))==opts::Value(u"mutated"));
        const std::array<opts::Value,1> nullArg{opts::Value(opts::Null{})};
        expect(std::holds_alternative<opts::Undefined>(e.context.call(std::get<opts::ObjectRef>(setAdaptor.data),self,nullArg).data));
        expect(std::holds_alternative<opts::Null>(e.context.get(self,opts::Key(u"adaptor")).data));
    };
    "mathjax_output_jax_base_lifecycle_and_null_page_outputs"_test=[] {
        Fixture e;auto jax=e.make();
        const auto prototype=std::get<opts::ObjectRef>(e.context.get(opts::Value(e.module.abstractClass()),opts::Key(u"prototype")).data);
        const auto arbitraryReceiver=opts::Value(e.context.object());
        auto call=[&](std::u16string_view name,std::span<const opts::Value> args={}){
            const auto function=e.context.get(opts::Value(prototype),opts::Key(std::u16string(name)));
            return e.context.call(std::get<opts::ObjectRef>(function.data),arbitraryReceiver,args);
        };
        expect(std::holds_alternative<opts::Undefined>(call(u"initialize").data));
        const std::array<opts::Value,2> resetArgs{opts::Value(1),opts::Value(u"x")};
        expect(std::holds_alternative<opts::Undefined>(call(u"reset",resetArgs).data));
        const std::array<opts::Value,1> arbitraryDocument{opts::Value(e.context.object())};
        expect(std::holds_alternative<opts::Undefined>(call(u"getMetrics",arbitraryDocument).data));
        expect(std::holds_alternative<opts::Null>(call(u"styleSheet",arbitraryDocument).data));
        expect(std::holds_alternative<opts::Null>(call(u"pageElements",arbitraryDocument).data));
        expect(!e.context.has(opts::Value(prototype),opts::Key(u"typeset")));
        expect(!e.context.has(opts::Value(prototype),opts::Key(u"escaped")));
        (void)jax;
    };
    "mathjax_output_jax_base_reset_is_lexical_and_derived_lookup_owns_its_override"_test=[] {
        Fixture e;auto jax=e.make();
        const auto abstractPrototype=std::get<opts::ObjectRef>(e.context.get(opts::Value(e.module.abstractClass()),opts::Key(u"prototype")).data);
        const auto baseReset=e.context.get(opts::Value(abstractPrototype),opts::Key(u"reset"));
        const std::array<opts::Value,2> args{7,u"tail"};
        const auto baseResult=e.context.call(std::get<opts::ObjectRef>(baseReset.data),opts::Value(jax->instanceIdentity()),args);
        expect(std::holds_alternative<opts::Undefined>(baseResult.data));
        expect(jax->resetArguments.empty());
        const auto derivedPrototype=std::get<opts::ObjectRef>(e.context.get(opts::Value(e.klass),opts::Key(u"prototype")).data);
        const auto override=e.context.function(u"reset",[jax](opts::Context&,opts::Value,std::span<const opts::Value> actual){
            std::vector<std::any> rest;for(const auto& value:actual)rest.emplace_back(value);jax->reset(std::move(rest));return opts::Value(opts::Undefined{});
        });
        e.context.define(derivedPrototype,opts::Key(u"reset"),opts::Descriptor::data(opts::Value(override)));
        const auto derivedReset=e.context.get(opts::Value(jax->instanceIdentity()),opts::Key(u"reset"));
        (void)e.context.call(std::get<opts::ObjectRef>(derivedReset.data),opts::Value(jax->instanceIdentity()),args);
        expect(eq(jax->resetArguments.size(),std::size_t{2}));
        expect(std::any_cast<opts::Value>(jax->resetArguments[0])==opts::Value(7));
        expect(std::any_cast<opts::Value>(jax->resetArguments[1])==opts::Value(u"tail"));
    };
    "mathjax_output_jax_filters_share_one_mutable_record_and_return_current_data"_test=[] {
        Fixture e;auto jax=e.make();Doc doc;struct Item:core::AbstractMathItem{Item():AbstractMathItem("x",Missing{}){}} item;
        auto original=PropertyList{{"n",1}};core::OutputJaxFilterDataRef first;std::vector<int> trace;
        auto a=std::make_shared<Function>();a->call=[&](const FunctionReceiver&,FunctionArgs args)->std::any{
            first=std::any_cast<core::OutputJaxFilterDataRef>(args[0]);trace.push_back(static_cast<bool>(first->sourceRecord));
            PropertyList::object(e.property(first->dataValue()))->set("n",2);e.setData(first,Property(7));return {};
        };
        auto b=std::make_shared<Function>();b->call=[&](const FunctionReceiver&,FunctionArgs args)->std::any{
            auto current=std::any_cast<core::OutputJaxFilterDataRef>(args[0]);trace.push_back(current==first);
            trace.push_back(current->sourceRecord==first->sourceRecord);return {};
        };
        auto filters=makeFunctionList();filters->add(a);filters->add(b);
        const auto result=jax->executeFilters(*filters,&item,&doc,Property(original));
        expect(eq(trace.size(),std::size_t{3}));expect(trace[0]==1&&trace[1]==1&&trace[2]==1);
        expect(e.property(std::any_cast<opts::Value>(result))==Property(7));expect(original.get("n")==Property(2));
    };
    "mathjax_output_jax_typed_adaptor_keeps_native_identity_without_context_surrogate"_test=[] {
        struct Adaptor final:NativeObject{};
        Fixture e;auto jax=e.make();auto adaptor=std::make_shared<Adaptor>();
        jax->setAdaptor(NativeObjectRef(adaptor));
        const auto* retained=std::get_if<NativeObjectRef>(&jax->adaptor);
        expect(retained&&*retained==std::static_pointer_cast<NativeObject>(adaptor));
        std::string error;
        try{(void)e.context.get(opts::Value(jax->instanceIdentity()),opts::Key(u"adaptor"));}
        catch(const opts::Exception& exception){error=exception.name+":"+property_host::debugText(exception.message);}
        expect(eq(error,std::string("Unimplemented:Native adaptor has no Context identity")));
    };
    "mathjax_output_jax_filter_false_stops_and_returns_replacement"_test=[] {
        Fixture e;auto jax=e.make();std::vector<std::string> trace;
        auto first=std::make_shared<Function>();first->call=[&](const FunctionReceiver&,FunctionArgs args)->std::any{
            trace.push_back("early");auto data=std::any_cast<core::OutputJaxFilterDataRef>(args[0]);e.setData(data,PropertyList{{"n",1}});return {};
        };
        auto stop=std::make_shared<Function>();stop->call=[&](const FunctionReceiver&,FunctionArgs args)->std::any{
            trace.push_back("stop");auto data=std::any_cast<core::OutputJaxFilterDataRef>(args[0]);e.setData(data,Property(23));return false;
        };
        auto never=std::make_shared<Function>();never->call=[&](const FunctionReceiver&,FunctionArgs)->std::any{trace.push_back("never");return {};};
        auto filters=makeFunctionList();filters->add(never,10);filters->add(stop,0);filters->add(first,-5);
        const auto result=jax->executeFilters(*filters,Missing{},PropertyNull{},{});
        expect(eq(trace.size(),std::size_t{2}));expect(trace[0]=="early"&&trace[1]=="stop");
        expect(e.property(std::any_cast<opts::Value>(result))==Property(23));
    };
    "mathjax_output_jax_configured_filter_arrays_use_priorities"_test=[] {
        Fixture e;std::vector<std::string> trace;
        auto fn=[&](std::string name){auto f=std::make_shared<Function>();f->call=[&,name](const FunctionReceiver&,FunctionArgs)->std::any{trace.push_back(name);return {};};return f;};
        auto second=fn("second"),first=fn("first"),post=fn("post");
        auto adapter=OptionCallBinding{
            [](opts::Context&,opts::Value){return FunctionReceiver{};},
            [](opts::Context&,std::span<const opts::Value>){return std::vector<std::any>{};},
            [](opts::Context&,const std::any&){return opts::Value(opts::Undefined{});}};
        const auto secondValue=e.bindings.bind(second,adapter,u"second");
        const auto firstValue=e.bindings.bind(first,adapter,u"first");
        const auto postValue=e.bindings.bind(post,adapter,u"post");
        auto tupleSecond=e.array({secondValue,4});auto tupleFirst=e.array({firstValue,-3});
        auto pre=e.array({opts::Value(tupleSecond),opts::Value(tupleFirst)});auto after=e.array({postValue});
        auto supplied=e.object({{u"preFilters",pre},{u"postFilters",after}});
        auto jax=core::makeOutputJax<Probe>(e.bindings,e.module.filterBindings(),e.klass,supplied);
        jax->preFilters->execute({});jax->postFilters->execute({});
        expect(eq(trace.size(),std::size_t{3}));expect(trace[0]=="first"&&trace[1]=="second"&&trace[2]=="post");
    };
    "mathjax_output_jax_undefined_filter_data_replacement_is_preserved"_test=[] {
        Fixture e;auto jax=e.make();auto fn=std::make_shared<Function>();
        fn->call=[](const FunctionReceiver&,FunctionArgs args)->std::any{
            auto data=std::any_cast<core::OutputJaxFilterDataRef>(args[0]);data->setField(u"data",opts::Value(opts::Undefined{}));return {};
        };
        auto filters=makeFunctionList();filters->add(fn);const auto result=jax->executeFilters(*filters,Missing{},PropertyNull{},Property(5));
        expect(std::holds_alternative<opts::Undefined>(std::any_cast<opts::Value>(result).data));
    };
    "mathjax_output_filter_record_uses_the_module_runtime_lease"_test=[] {
        opts::Context context;OptionFunctionBindings bindings(context);
        core::OutputJaxFilterDataRef retained;
        {
            core::OutputJaxModule module(context,bindings);
            auto klass=context.constructor(u"Probe");
            context.setPrototype(klass,module.abstractClass());
            const auto instancePrototype=std::get<opts::ObjectRef>(context.get(opts::Value(klass),opts::Key(u"prototype")).data);
            const auto abstractPrototype=std::get<opts::ObjectRef>(context.get(opts::Value(module.abstractClass()),opts::Key(u"prototype")).data);
            context.setPrototype(instancePrototype,abstractPrototype);
            auto defaults=context.object();context.set(defaults,opts::Key(u"preFilters"),context.array());context.set(defaults,opts::Key(u"postFilters"),context.array());
            context.set(klass,opts::Key(u"NAME"),u"generic");context.set(klass,opts::Key(u"OPTIONS"),defaults);
            auto jax=core::makeOutputJax<Probe>(bindings,module.filterBindings(),klass);
            auto callback=std::make_shared<Function>();
            callback->call=[&](const FunctionReceiver&,FunctionArgs args)->std::any{retained=std::any_cast<core::OutputJaxFilterDataRef>(args[0]);return {};};
            auto filters=makeFunctionList();filters->add(callback);
            (void)jax->executeFilters(*filters,Missing{},PropertyNull{},opts::Value(u"live"));
            expect(retained&&retained->sourceIdentity()==retained->sourceRecord->identity());
        }
        std::string error="none";
        try{(void)retained->dataValue();}catch(const opts::Exception& value){error=value.name;}
        expect(eq(error,std::string("TypeError")));
    };
    "mathjax_output_jax_filter_exception_propagates"_test=[] {
        Fixture e;auto jax=e.make();auto fn=std::make_shared<Function>();
        fn->call=[](const FunctionReceiver&,FunctionArgs)->std::any{throw opts::Exception("TypeError",u"original-filter-probe");};
        auto filters=makeFunctionList();filters->add(fn);std::string error="none";
        try{jax->executeFilters(*filters,Missing{},PropertyNull{},{});}catch(const opts::Exception& value){error=value.name+":"+property_host::debugText(value.message);}
        expect(eq(error,std::string("TypeError:original-filter-probe")));
    };
    "mathjax_output_jax_unknown_option_warning_and_omission"_test=[] {
        Fixture e;std::vector<std::u16string> warnings;auto old=opts::OPTIONS.warningSink;
        opts::OPTIONS.warningSink=[&](std::u16string warning){warnings.push_back(std::move(warning));};
        auto supplied=e.object({{u"unknown",4}});auto jax=core::makeOutputJax<Probe>(e.bindings,e.module.filterBindings(),e.klass,supplied);
        opts::OPTIONS.warningSink=std::move(old);
        expect(!e.context.has(jax->options,opts::Key(u"unknown")));
        expect(eq(warnings.size(),std::size_t{1}));
        expect(eq(property_host::debugText(warnings[0]),std::string("MathJax: Invalid option \"unknown\" (no default value).")));
    };
    "mathjax_output_jax_null_options_and_null_filter_definition"_test=[] {
        Fixture e;auto nullOptions=e.make(opts::Null{});
        expect(std::holds_alternative<opts::ObjectRef>(nullOptions->options.data));
        auto supplied=e.object({{u"preFilters",opts::Null{}}});auto nullFilters=core::makeOutputJax<Probe>(e.bindings,e.module.filterBindings(),e.klass,supplied);
        expect(eq(nullFilters->preFilters->storageRef()->size(),std::size_t{0}));
    };
    "mathjax_output_jax_instances_have_distinct_filter_lists"_test=[] {
        Fixture e;auto a=e.make();auto b=core::makeOutputJax<Probe>(e.bindings,e.module.filterBindings(),e.klass);
        auto fn=std::make_shared<Function>();fn->call=[](const FunctionReceiver&,FunctionArgs)->std::any{return {};};a->preFilters->add(fn);
        expect(a->preFilters!=b->preFilters&&a->postFilters!=b->postFilters);
        expect(eq(a->preFilters->storageRef()->size(),std::size_t{1}));expect(eq(b->preFilters->storageRef()->size(),std::size_t{0}));
    };
    "mathjax_output_jax_context_filter_items_view_tracks_one_storage_across_rebind"_test=[] {
        Fixture e;auto jax=e.make();const auto self=opts::Value(jax->instanceIdentity());
        const auto listValue=e.context.get(self,opts::Key(u"preFilters"));
        const auto listObject=std::get<opts::ObjectRef>(listValue.data);
        const auto oldItems=e.context.get(listValue,opts::Key(u"items"));
        const auto oldArray=std::get<opts::ObjectRef>(oldItems.data);
        expect(e.context.get(listValue,opts::Key(u"items"))==oldItems);
        expect(eq(e.context.arrayLength(oldArray),std::uint32_t{0}));
        const auto lengthDescriptor=e.context.descriptor(opts::Value(oldArray),opts::Key(u"length"));
        expect(lengthDescriptor&&lengthDescriptor->writable&&!lengthDescriptor->enumerable&&!lengthDescriptor->configurable);
        auto fn=std::make_shared<Function>();fn->call=[](const FunctionReceiver&,FunctionArgs)->std::any{return {};};
        OptionCallBinding adapter{
            [](opts::Context&,opts::Value receiver)->FunctionReceiver{return receiver;},
            [](opts::Context&,std::span<const opts::Value> args){std::vector<std::any> result;for(const auto& arg:args)result.emplace_back(arg);return result;},
            [](opts::Context&,const std::any&){return opts::Value(opts::Undefined{});}};
        const auto callable=e.bindings.bind(fn,std::move(adapter),u"context item");
        const auto add=e.context.get(listValue,opts::Key(u"add"));
        const std::array<opts::Value,1> addArgs{callable};
        (void)e.context.callMember(add,listValue,addArgs,u"FunctionList.add");
        expect(eq(e.context.arrayLength(oldArray),std::uint32_t{1}));
        const auto retainedRecord=e.context.get(opts::Value(oldArray),opts::Key(u"0"));
        expect(std::holds_alternative<opts::ObjectRef>(retainedRecord.data));
        expect(e.context.hasOwn(opts::Value(oldArray),opts::Key(u"0")));
        expect(!e.context.hasOwn(opts::Value(oldArray),opts::Key(u"1")));
        expect(e.context.get(retainedRecord,opts::Key(u"item"))==callable);
        expect(eq(e.context.toNumber(e.context.get(retainedRecord,opts::Key(u"priority"))),5.0));
        auto typedFunction=std::make_shared<Function>();typedFunction->call=[](const FunctionReceiver&,FunctionArgs)->std::any{return {};};
        OptionCallBinding typedAdapter{
            [](opts::Context&,opts::Value receiver)->FunctionReceiver{return receiver;},
            [](opts::Context&,std::span<const opts::Value> args){std::vector<std::any> result;for(const auto& arg:args)result.emplace_back(arg);return result;},
            [](opts::Context&,const std::any&){return opts::Value(opts::Undefined{});}};
        (void)e.bindings.bind(typedFunction,std::move(typedAdapter),u"typed");
        jax->preFilters->add(typedFunction);
        expect(eq(e.context.arrayLength(oldArray),std::uint32_t{2}));
        e.context.erase(oldArray,opts::Key(u"1"));
        expect(eq(e.context.arrayLength(oldArray),std::uint32_t{2}));
        expect(!e.context.hasOwn(opts::Value(oldArray),opts::Key(u"1")));
        expect(std::holds_alternative<opts::Undefined>(e.context.get(opts::Value(oldArray),opts::Key(u"1")).data));
        e.context.set(opts::Value(oldArray),opts::Key(u"length"),opts::Value(1));
        expect(eq(e.context.arrayLength(oldArray),std::uint32_t{1}));

        auto replacement=makeFunctionList();
        const auto replacementItems=e.module.filterBindings().view(replacement->storageRef());
        e.context.set(listObject,opts::Key(u"items"),opts::Value(replacementItems));
        const auto currentItems=e.context.get(listValue,opts::Key(u"items"));
        expect(std::get<opts::ObjectRef>(currentItems.data)==replacementItems);
        expect(eq(e.context.arrayLength(oldArray),std::uint32_t{1}));
        expect(eq(e.context.arrayLength(replacementItems),std::uint32_t{0}));
        expect(e.context.get(opts::Value(oldArray),opts::Key(u"0"))==retainedRecord);
    };
    "mathjax_output_jax_context_execute_uses_live_prioritized_items"_test=[] {
        Fixture e;auto jax=e.make();const auto self=opts::Value(jax->instanceIdentity());
        const auto list=e.context.get(self,opts::Key(u"preFilters"));
        const auto add=e.context.get(list,opts::Key(u"add"));
        std::vector<std::string> trace;
        auto second=std::make_shared<Function>();second->call=[&](const FunctionReceiver&,FunctionArgs)->std::any{trace.push_back("second");return {};};
        OptionCallBinding adapter{
            [](opts::Context&,opts::Value receiver)->FunctionReceiver{return receiver;},
            [](opts::Context&,std::span<const opts::Value> args){std::vector<std::any> result;for(const auto& arg:args)result.emplace_back(arg);return result;},
            [](opts::Context&,const std::any&){return opts::Value(opts::Undefined{});}};
        const auto secondCallable=e.bindings.bind(second,std::move(adapter),u"second");
        auto first=std::make_shared<Function>();first->call=[&](const FunctionReceiver&,FunctionArgs)->std::any{
            trace.push_back("first");const std::array<opts::Value,1> args{secondCallable};
            (void)e.context.callMember(add,list,args,u"FunctionList.add");return {};
        };
        OptionCallBinding firstAdapter{
            [](opts::Context&,opts::Value receiver)->FunctionReceiver{return receiver;},
            [](opts::Context&,std::span<const opts::Value> args){std::vector<std::any> result;for(const auto& arg:args)result.emplace_back(arg);return result;},
            [](opts::Context&,const std::any&){return opts::Value(opts::Undefined{});}};
        const std::array<opts::Value,1> firstArg{e.bindings.bind(first,std::move(firstAdapter),u"first")};
        (void)e.context.callMember(add,list,firstArg,u"FunctionList.add");
        const auto executeFilters=e.context.get(self,opts::Key(u"executeFilters"));
        auto math=e.object({{u"x",1}}),document=e.object({{u"id",2}}),data=e.object({{u"n",3}});
        const std::array<opts::Value,4> executeArgs{list,opts::Value(math),opts::Value(document),opts::Value(data)};
        const auto result=e.context.callMember(executeFilters,self,executeArgs,u"OutputJax.executeFilters");
        expect(eq(trace.size(),std::size_t{2}));expect(trace[0]=="first"&&trace[1]=="second");
        expect(result==opts::Value(data));
        expect(eq(e.context.arrayLength(std::get<opts::ObjectRef>(e.context.get(list,opts::Key(u"items")).data)),std::uint32_t{2}));
    };
    "mathjax_output_jax_prototype_descriptor_inventory"_test=[] {
        Fixture e;auto prototype=std::get<opts::ObjectRef>(e.context.get(opts::Value(e.module.abstractClass()),opts::Key(u"prototype")).data);
        const auto names=e.context.ownKeys(opts::Value(prototype));
        std::vector<std::u16string> actual;
        for(const auto& key:names)if(const auto* name=std::get_if<std::u16string>(&key.data))actual.push_back(*name);
        const std::vector<std::u16string> expected{u"constructor",u"name",u"setAdaptor",u"initialize",u"reset",u"getMetrics",u"styleSheet",u"pageElements",u"executeFilters"};
        expect(actual==expected);
        expect(!e.context.has(opts::Value(prototype),opts::Key(u"typeset")));
        expect(!e.context.has(opts::Value(prototype),opts::Key(u"escaped")));
        const auto name=e.context.descriptor(opts::Value(prototype),opts::Key(u"name"));
        expect(name&&name->accessor&&!name->enumerable&&name->configurable&&name->getter.has_value()&&!name->setter.has_value());
        for(const auto member:{u"setAdaptor",u"initialize",u"reset",u"getMetrics",u"styleSheet",u"pageElements",u"executeFilters"}){
            const auto descriptor=e.context.descriptor(opts::Value(prototype),opts::Key(member));
            expect(descriptor&&!descriptor->accessor&&descriptor->enumerable&&descriptor->writable&&descriptor->configurable);
        }
    };
    "mathjax_output_jax_cjs_constructor_metadata_is_function_compatible"_test=[] {
        Fixture e;const auto ctor=e.module.abstractClass();
        const auto length=e.context.descriptor(opts::Value(ctor),opts::Key(u"length"));
        const auto prototype=e.context.descriptor(opts::Value(ctor),opts::Key(u"prototype"));
        expect(length&&!length->writable&&!length->enumerable&&length->configurable);
        expect(length&&std::get<double>(length->value.data)==1.0);
        expect(prototype&&prototype->writable&&!prototype->enumerable&&!prototype->configurable);
    };
    "mathjax_function_list_source_methods_are_enumerable_prototype_assignments"_test=[] {
        Fixture e;const auto functionProto=e.module.filterBindings().prototype();
        const auto ctor=e.module.filterBindings().constructor();
        const auto length=e.context.descriptor(opts::Value(ctor),opts::Key(u"length"));
        const auto prototype=e.context.descriptor(opts::Value(ctor),opts::Key(u"prototype"));
        expect(length&&!length->writable&&!length->enumerable&&length->configurable);
        expect(length&&std::get<double>(length->value.data)==1.0);
        expect(prototype&&prototype->writable&&!prototype->enumerable&&!prototype->configurable);
        const auto baseProto=std::get<opts::ObjectRef>(e.context.get(opts::Value(functionProto),opts::Key(u"__proto__")).data);
        for(const auto member:{u"addList",u"execute",u"asyncExecute"}){
            const auto descriptor=e.context.descriptor(opts::Value(functionProto),opts::Key(member));
            expect(descriptor&&!descriptor->accessor&&descriptor->enumerable&&descriptor->writable&&descriptor->configurable);
        }
        for(const auto member:{u"add",u"remove"}){
            const auto descriptor=e.context.descriptor(opts::Value(baseProto),opts::Key(member));
            expect(descriptor&&!descriptor->accessor&&descriptor->enumerable&&descriptor->writable&&descriptor->configurable);
        }
    };
};



