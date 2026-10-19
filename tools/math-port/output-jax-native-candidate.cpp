// Native observer for tools/math-port/output-jax-original-oracle.cjs.
import folia.math.core.output_jax;
import folia.math.util.property_host;
import std;
using namespace folia::math;
namespace opts=folia::math::options;
namespace core=folia::math::core;
namespace {
std::string quote(std::string_view text){std::string out="\"";for(unsigned char c:text){switch(c){case '"':out+="\\\"";break;case '\\':out+="\\\\";break;case '\n':out+="\\n";break;case '\r':out+="\\r";break;case '\t':out+="\\t";break;default:if(c<0x20){char b[7];std::snprintf(b,sizeof(b),"\\u%04x",c);out+=b;}else out+=static_cast<char>(c);}}return out+"\"";}
std::string number(double value){if(std::isnan(value))return "{\"$\":\"NaN\"}";if(std::isinf(value))return value<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}";if(value==0&&std::signbit(value))return "{\"$\":\"-0\"}";char buffer[64];auto result=std::to_chars(buffer,buffer+sizeof(buffer),value,std::chars_format::general);return std::string(buffer,result.ptr);}
std::string value(opts::Context& context,opts::Value v,std::size_t depth=0){
    if(depth>30)throw std::runtime_error("observer object depth exceeded");
    if(std::holds_alternative<opts::Undefined>(v.data))return "{\"$\":\"undefined\"}";
    if(std::holds_alternative<opts::Null>(v.data))return "null";
    if(const auto* b=std::get_if<bool>(&v.data))return *b?"true":"false";
    if(const auto* n=std::get_if<double>(&v.data))return number(*n);
    if(const auto* s=std::get_if<std::u16string>(&v.data))return quote(property_host::debugText(*s));
    if(const auto* object=std::get_if<opts::ObjectRef>(&v.data)){
        const auto kind=context.kind(*object);
        if(kind==opts::ObjectKind::array){
            const auto length=context.arrayLength(*object);std::string out="[";
            for(std::uint32_t i=0;i<length;i++){if(i)out+=",";const auto digits=std::to_string(i);const auto key=opts::Key(std::u16string(digits.begin(),digits.end()));if(context.hasOwn(v,key))out+=value(context,context.get(v,key),depth+1);else out+="{\"$\":\"hole\"}";}
            return out+"]";
        }
        if(kind==opts::ObjectKind::function)return quote("[Function]");
        std::string out="{";bool first=true;
        for(const auto& key:context.enumerableStringKeys(v)){
            if(!first)out+=",";first=false;out+=quote(property_host::debugText(key))+":";
            out+=value(context,context.get(v,opts::Key(key)),depth+1);
        }
        return out+"}";
    }
    return "null";
}
std::string property(Property p,std::size_t depth=0){
    if(depth>30)throw std::runtime_error("Property observer depth exceeded");
    if(p.missing())return "{\"$\":\"undefined\"}";
    if(std::holds_alternative<PropertyNull>(p.value))return "null";
    if(const auto* b=std::get_if<bool>(&p.value))return *b?"true":"false";
    if(const auto* n=std::get_if<double>(&p.value))return number(*n);
    if(const auto* s=std::get_if<std::string>(&p.value))return quote(*s);
    if(auto a=PropertyArray::array(p)){
        std::string out="[";for(std::size_t i=0;i<a->size();i++){if(i)out+=",";out+=a->has(i)?property(a->get(i),depth+1):"{\"$\":\"hole\"}";}return out+"]";
    }
    if(auto o=PropertyList::record(p)){
        std::string out="{";bool first=true;for(const auto& key:o->keys()){if(!first)out+=",";first=false;out+=quote(key)+":"+property(o->getOwn(key),depth+1);}return out+"}";
    }
    throw std::runtime_error("Unsupported property in OutputJax observer");
}
std::string errorRecord(std::string name,std::string message){return "{\"error\":{\"name\":"+quote(name)+",\"message\":"+quote(message)+"}}";}
opts::ObjectRef object(opts::Context& c,std::initializer_list<std::pair<std::u16string,opts::Value>> fields){auto o=c.object();for(const auto&[k,v]:fields)c.set(o,opts::Key(k),v);return o;}
opts::ObjectRef array(opts::Context& c,std::initializer_list<opts::Value> fields){auto a=c.array();std::size_t i=0;for(auto v:fields){auto s=std::to_string(i++);c.set(a,opts::Key(std::u16string(s.begin(),s.end())),v);}return a;}
struct Doc final:core::MathDocumentService{core::MathRenderActionsReference renderActionsReference()const override{return Missing{};}core::MathOutputJaxReference outputJaxReference()const override{return Missing{};}};
struct Item final:core::AbstractMathItem{explicit Item(std::string name):AbstractMathItem(std::move(name),Missing{}){}};
struct Probe final:core::AbstractOutputJax{
    std::vector<std::any> resetArguments;
    Probe(OptionFunctionBindings& b,opts::ObjectRef id,FunctionListContextBindings& filters):AbstractOutputJax(b,id,filters){}
    core::MathOutput typeset(core::MathItem&,core::MathDocumentReference)override{return PropertyNull{};}
    core::MathOutput escaped(core::MathItem&,core::MathDocumentReference)override{return PropertyNull{};}
    void reset(FunctionArgs args={})override{resetArguments.assign(args.begin(),args.end());}
    using AbstractOutputJax::executeFilters;
};
struct Fixture{
    opts::Context context;OptionFunctionBindings bindings{context};core::OutputJaxModule module{context,bindings};
    opts::ObjectRef klass=context.constructor(u"Probe");std::shared_ptr<Probe> jax;
    Fixture(){context.setPrototype(klass,module.abstractClass());auto instancePrototype=std::get<opts::ObjectRef>(context.get(opts::Value(klass),opts::Key(u"prototype")).data);auto abstractPrototype=std::get<opts::ObjectRef>(context.get(opts::Value(module.abstractClass()),opts::Key(u"prototype")).data);context.setPrototype(instancePrototype,abstractPrototype);auto d=context.object();context.set(d,opts::Key(u"preFilters"),context.array());context.set(d,opts::Key(u"postFilters"),context.array());context.set(klass,opts::Key(u"NAME"),u"generic");context.set(klass,opts::Key(u"OPTIONS"),d);}
    opts::ObjectRef object(std::initializer_list<std::pair<std::u16string,opts::Value>> fields){auto result=context.object();for(const auto&[key,value]:fields)context.set(result,opts::Key(key),value);return result;}
    opts::ObjectRef array(std::initializer_list<opts::Value> values){auto result=context.array();std::size_t index=0;for(const auto& value:values){const auto digits=std::to_string(index++);context.set(result,opts::Key(std::u16string(digits.begin(),digits.end())),value);}return result;}
    std::shared_ptr<Probe> make(opts::Value supplied=opts::Undefined{}){return jax=core::makeOutputJax<Probe>(bindings,module.filterBindings(),klass,std::move(supplied));}
    opts::Value function(FunctionRef fn,std::u16string label=u"filter"){
        OptionCallBinding adapter;
        adapter.receiver=[](opts::Context& cx,opts::Value receiver)->FunctionReceiver{
            if(const auto* object=std::get_if<opts::ObjectRef>(&receiver.data)){
                const auto& payload=cx.payload(*object);
                if(const auto* native=std::any_cast<NativeObjectRef>(&payload))return *native;
            }
            return receiver;
        };
        adapter.arguments=[](opts::Context& cx,std::span<const opts::Value> args){
            std::vector<std::any> out;out.reserve(args.size());
            for(const auto& value:args){
                if(const auto* object=std::get_if<opts::ObjectRef>(&value.data)){
                    const auto& payload=cx.payload(*object);
                    if(payload.has_value()){out.push_back(payload);continue;}
                }
                out.emplace_back(value);
            }
            return out;
        };
        adapter.result=[this](opts::Context&,const std::any& result)->opts::Value{
            if(!result.has_value())return opts::Undefined{};
            if(const auto* value=std::any_cast<opts::Value>(&result))return *value;
            if(const auto* value=std::any_cast<Property>(&result))return module.filterBindings().toValue(*value);
            if(const auto* value=std::any_cast<bool>(&result))return *value;
            if(const auto* value=std::any_cast<double>(&result))return *value;
            if(const auto* value=std::any_cast<std::u16string>(&result))return *value;
            if(const auto* value=std::any_cast<std::string>(&result))return property_host::text(*value);
            throw opts::Exception("Unimplemented",u"OutputJax observer callback result has no Context binding");
        };
        return bindings.bind(std::move(fn),std::move(adapter),std::move(label));
    }
    opts::ObjectRef list(std::u16string_view name){return std::get<opts::ObjectRef>(context.get(opts::Value(jax->instanceIdentity()),opts::Key(std::u16string(name))).data);}
    void add(std::u16string_view name,opts::Value fn,std::optional<double> priority=std::nullopt){
        const auto target=list(name);const auto method=context.get(opts::Value(target),opts::Key(u"add"));
        if(priority){const std::array<opts::Value,2> args{std::move(fn),opts::Value(*priority)};(void)context.callMember(method,opts::Value(target),args,u"FunctionList.add");}
        else{const std::array<opts::Value,1> args{std::move(fn)};(void)context.callMember(method,opts::Value(target),args,u"FunctionList.add");}
    }
    opts::Value execute(opts::Value filters,opts::Value math,opts::Value document,opts::Value data){
        const auto self=opts::Value(jax->instanceIdentity());const auto method=context.get(self,opts::Key(u"executeFilters"));
        const std::array<opts::Value,4> args{std::move(filters),std::move(math),std::move(document),std::move(data)};
        return context.callMember(method,self,args,u"OutputJax.executeFilters");
    }
};
std::string errorName(const opts::Exception& e){return e.name;}
std::string errorMessage(const opts::Exception& e){return property_host::debugText(e.message);}
std::string typeOf(opts::Context& context,opts::Value v){
    if(std::holds_alternative<opts::Undefined>(v.data))return "undefined";
    if(std::holds_alternative<opts::Null>(v.data))return "object";
    if(std::holds_alternative<bool>(v.data))return "boolean";
    if(std::holds_alternative<double>(v.data))return "number";
    if(std::holds_alternative<std::u16string>(v.data))return "string";
    if(const auto* object=std::get_if<opts::ObjectRef>(&v.data))return context.kind(*object)==opts::ObjectKind::function?"function":"object";
    return "symbol";
}
}
int main(){
    std::vector<std::pair<std::string,std::string>> records;
    auto observe=[&](std::string name,auto&& action){try{records.emplace_back(std::move(name),std::string("{\"value\":")+action()+"}");}catch(const opts::Exception& e){records.emplace_back(std::move(name),errorRecord(errorName(e),errorMessage(e)));}catch(const std::exception& e){records.emplace_back(std::move(name),errorRecord("Error",e.what()));}};
    observe("base-defaults",[&]{Fixture e;auto j=e.make();const auto self=opts::Value(j->instanceIdentity());const auto name=e.context.get(self,opts::Key(u"name"));const auto options=e.context.get(self,opts::Key(u"options"));const auto adaptor=e.context.get(self,opts::Key(u"adaptor"));const auto pre=e.context.get(self,opts::Key(u"preFilters"));const auto post=e.context.get(self,opts::Key(u"postFilters"));const auto preItems=e.context.get(pre,opts::Key(u"items"));const auto postItems=e.context.get(post,opts::Key(u"items"));return std::string("{\"name\":")+value(e.context,name)+",\"options\":"+value(e.context,options)+",\"adaptor\":"+value(e.context,adaptor)+",\"pre\":"+value(e.context,preItems)+",\"post\":"+value(e.context,postItems)+"}";});
    observe("derived-options-and-live-name",[&]{Fixture e;auto defaults=std::get<opts::ObjectRef>(e.context.get(opts::Value(e.klass),opts::Key(u"OPTIONS")).data);auto nested=object(e.context,{{u"x",1},{u"y",2}});e.context.set(defaults,opts::Key(u"scale"),1);e.context.set(defaults,opts::Key(u"nested"),nested);auto derived=e.context.constructor(u"Derived");e.context.setPrototype(derived,e.module.abstractClass());auto derivedPrototype=std::get<opts::ObjectRef>(e.context.get(opts::Value(derived),opts::Key(u"prototype")).data);auto abstractPrototype=std::get<opts::ObjectRef>(e.context.get(opts::Value(e.module.abstractClass()),opts::Key(u"prototype")).data);e.context.setPrototype(derivedPrototype,abstractPrototype);e.context.set(derived,opts::Key(u"NAME"),u"scene");e.context.set(derived,opts::Key(u"OPTIONS"),defaults);auto userNested=object(e.context,{{u"x",3}});auto supplied=object(e.context,{{u"scale",2},{u"nested",userNested}});auto j=core::makeOutputJax<Probe>(e.bindings,e.module.filterBindings(),derived,supplied);const auto before=e.context.get(opts::Value(j->instanceIdentity()),opts::Key(u"name"));const auto actualOptions=e.context.get(opts::Value(j->instanceIdentity()),opts::Key(u"options"));const bool optionsDistinct=std::get<opts::ObjectRef>(actualOptions.data)!=defaults;const auto actualNested=std::get<opts::ObjectRef>(e.context.get(actualOptions,opts::Key(u"nested")).data);const bool nestedDistinct=actualNested!=nested;const auto outOptions=value(e.context,actualOptions),outDefaults=value(e.context,opts::Value(defaults));e.context.set(derived,opts::Key(u"NAME"),u"renamed");const auto after=e.context.get(opts::Value(j->instanceIdentity()),opts::Key(u"name"));return std::string("{\"before\":")+value(e.context,before)+",\"after\":"+value(e.context,after)+",\"options\":"+outOptions+",\"optionsDistinct\":"+(optionsDistinct?"true":"false")+",\"nestedDistinct\":"+(nestedDistinct?"true":"false")+",\"defaults\":"+outDefaults+"}";});
    observe("set-adaptor-preserves-identity",[&]{Fixture e;auto j=e.make();const auto self=opts::Value(j->instanceIdentity());const auto setAdaptor=e.context.get(self,opts::Key(u"setAdaptor"));const auto host=object(e.context,{{u"tag",u"host"}});const std::array<opts::Value,1> hostArg{opts::Value(host)};const auto first=e.context.call(std::get<opts::ObjectRef>(setAdaptor.data),self,hostArg);const auto stored=e.context.get(self,opts::Key(u"adaptor"));const bool same=stored==opts::Value(host);e.context.set(host,opts::Key(u"tag"),u"mutated");const auto observed=e.context.get(stored,opts::Key(u"tag"));const std::array<opts::Value,1> nullArg{opts::Value(opts::Null{})};const auto second=e.context.call(std::get<opts::ObjectRef>(setAdaptor.data),self,nullArg);const auto current=e.context.get(self,opts::Key(u"adaptor"));return std::string("{\"results\":[")+value(e.context,first)+","+value(e.context,second)+"],\"same\":"+(same?"true":"false")+",\"observed\":"+value(e.context,observed)+",\"current\":"+value(e.context,current)+"}";});
    observe("base-lifecycle-and-page-return-values",[&]{Fixture e;auto j=e.make();const auto proto=std::get<opts::ObjectRef>(e.context.get(opts::Value(e.module.abstractClass()),opts::Key(u"prototype")).data);auto call=[&](std::u16string_view key,std::span<const opts::Value> args={}){const auto method=e.context.get(opts::Value(proto),opts::Key(std::u16string(key)));return e.context.call(std::get<opts::ObjectRef>(method.data),opts::Value(e.context.object()),args);};const std::array<opts::Value,2> resetArgs{opts::Value(11),opts::Value(u"unused")};const auto initialize=call(u"initialize");const auto reset=call(u"reset",resetArgs);const std::array<opts::Value,1> documentArg{opts::Value(e.context.object())};const auto metrics=call(u"getMetrics",documentArg);const auto sheet=call(u"styleSheet",documentArg),page=call(u"pageElements",documentArg);const auto type=typeOf(e.context,e.context.get(opts::Value(proto),opts::Key(u"typeset")));const auto escape=typeOf(e.context,e.context.get(opts::Value(proto),opts::Key(u"escaped")));return std::string("{\"initialize\":")+value(e.context,initialize)+",\"reset\":"+value(e.context,reset)+",\"metrics\":"+value(e.context,metrics)+",\"sheet\":"+value(e.context,sheet)+",\"page\":"+value(e.context,page)+",\"hasTypeset\":"+quote(type)+",\"hasEscaped\":"+quote(escape)+"}";});
    observe("filter-same-record-and-live-data",[&]{Fixture e;auto j=e.make();auto math=e.object({{u"m",1}}),document=e.object({{u"id",7}}),data=e.object({{u"n",1}});std::vector<bool> identity;core::OutputJaxFilterDataRef first;auto f=std::make_shared<Function>(),g=std::make_shared<Function>();f->call=[&](const FunctionReceiver&,FunctionArgs a)->std::any{first=std::any_cast<core::OutputJaxFilterDataRef>(a[0]);identity.push_back(first->field(u"math")==opts::Value(math));identity.push_back(first->field(u"document")==opts::Value(document));identity.push_back(first->dataValue()==opts::Value(data));e.context.set(data,opts::Key(u"n"),2);return {};};g->call=[&](const FunctionReceiver&,FunctionArgs a)->std::any{auto current=std::any_cast<core::OutputJaxFilterDataRef>(a[0]);identity.push_back(current==first);current->setField(u"data",opts::Value(e.object({{u"n",3}})));return {};};e.add(u"preFilters",e.function(f,u"first"));e.add(u"preFilters",e.function(g,u"second"));const auto result=e.execute(opts::Value(e.list(u"preFilters")),opts::Value(math),opts::Value(document),opts::Value(data));std::string flags="[";for(std::size_t i=0;i<identity.size();++i){if(i)flags+=",";flags+=identity[i]?"true":"false";}return std::string("{\"identity\":")+flags+"],\"original\":"+value(e.context,opts::Value(data))+",\"result\":"+value(e.context,result)+",\"returnedLive\":"+(result==first->dataValue()?"true":"false")+"}";});
    observe("filter-order-and-false-stop",[&]{Fixture e;auto j=e.make();std::vector<std::string> trace;auto fn=[&](std::string label,bool stop=false){auto f=std::make_shared<Function>();f->call=[&,label,stop](const FunctionReceiver&,FunctionArgs a)->std::any{trace.push_back(label);auto data=std::any_cast<opts::Value>(std::any_cast<core::OutputJaxFilterDataRef>(a[0])->dataValue());const std::array<opts::Value,1> valueArg{opts::Value(property_host::text(label))};e.context.arrayPush(std::get<opts::ObjectRef>(data.data),valueArg);if(stop)return false;return {};};return f;};e.add(u"preFilters",e.function(fn("late"),u"late"),10);e.add(u"preFilters",e.function(fn("early"),u"early"),-5);e.add(u"preFilters",e.function(fn("stop",true),u"stop"),0);e.add(u"preFilters",e.function(fn("equal"),u"equal"),0);const auto input=e.array({});const auto result=e.execute(opts::Value(e.list(u"preFilters")),opts::Value(e.object({})),opts::Null{},opts::Value(input));std::string t="[";for(std::size_t i=0;i<trace.size();i++){if(i)t+=",";t+=quote(trace[i]);}return std::string("{\"trace\":")+t+"],\"data\":"+value(e.context,result)+"}";});
    observe("configured-filter-definition-and-priority",[&]{std::vector<std::string> trace;Fixture e;auto fn=[&](std::string label,int n){auto f=std::make_shared<Function>();f->call=[&,label,n](const FunctionReceiver&,FunctionArgs a)->std::any{trace.push_back(label);auto record=std::any_cast<core::OutputJaxFilterDataRef>(a[0]);auto data=std::any_cast<opts::Value>(record->dataValue());const std::array<opts::Value,1> valueArg{opts::Value(n)};e.context.arrayPush(std::get<opts::ObjectRef>(data.data),valueArg);return {};};return e.function(f,std::u16string(label.begin(),label.end()));};auto second=fn("second",2),first=fn("first",1),post=fn("post",3);auto pre=array(e.context,{opts::Value(array(e.context,{second,4})),opts::Value(array(e.context,{first,-3}))});auto after=array(e.context,{post});auto supplied=object(e.context,{{u"preFilters",opts::Value(pre)},{u"postFilters",opts::Value(after)}});e.jax=core::makeOutputJax<Probe>(e.bindings,e.module.filterBindings(),e.klass,supplied);const auto result1=e.execute(opts::Value(e.list(u"preFilters")),opts::Value(e.object({})),opts::Null{},opts::Value(e.array({})));const auto result2=e.execute(opts::Value(e.list(u"postFilters")),opts::Value(e.object({})),opts::Null{},result1);std::string t="[";for(std::size_t i=0;i<trace.size();i++){if(i)t+=",";t+=quote(trace[i]);}return std::string("{\"trace\":")+t+"],\"result\":"+value(e.context,result2)+"}";});
    observe("filter-undefined-replacement",[&]{Fixture e;auto j=e.make();auto f=std::make_shared<Function>();f->call=[](const FunctionReceiver&,FunctionArgs a)->std::any{std::any_cast<core::OutputJaxFilterDataRef>(a[0])->setField(u"data",opts::Value(opts::Undefined{}));return {};};e.add(u"postFilters",e.function(f));return value(e.context,e.execute(opts::Value(e.list(u"postFilters")),opts::Value(e.object({})),opts::Undefined{},opts::Value(e.object({{u"n",1}}))));});
    observe("filter-thrown-error-propagates",[&]{Fixture e;auto j=e.make();auto f=std::make_shared<Function>();f->call=[](const FunctionReceiver&,FunctionArgs)->std::any{throw opts::Exception("TypeError",u"original-filter-probe");};e.add(u"preFilters",e.function(f));(void)e.execute(opts::Value(e.list(u"preFilters")),opts::Value(e.object({})),opts::Null{},opts::Value(e.object({})));return "null";});
    observe("unknown-option-source-warning",[&]{Fixture e;std::vector<std::u16string>warnings;auto old=opts::OPTIONS.warningSink;opts::OPTIONS.warningSink=[&](std::u16string m){warnings.push_back(std::move(m));};auto j=e.make(object(e.context,{{u"unknown",4}}));opts::OPTIONS.warningSink=std::move(old);std::string w="[";for(std::size_t i=0;i<warnings.size();i++){if(i)w+=",";w+=value(e.context,opts::Value(warnings[i]));}return std::string("{\"options\":")+value(e.context,j->options)+",\"warnings\":"+w+"]}";});
    observe("null-options",[&]{Fixture e;auto j=e.make(opts::Null{});return value(e.context,j->options);});
    observe("null-filter-definition",[&]{Fixture e;auto j=e.make(object(e.context,{{u"preFilters",opts::Null{}}}));const auto items=e.context.get(opts::Value(e.list(u"preFilters")),opts::Key(u"items"));return value(e.context,items);});
    observe("instances-do-not-share-filter-lists",[&]{Fixture e;auto a=e.make(),b=core::makeOutputJax<Probe>(e.bindings,e.module.filterBindings(),e.klass);auto f=std::make_shared<Function>();f->call=[](const FunctionReceiver&,FunctionArgs)->std::any{return {};};e.add(u"preFilters",e.function(f));const auto apre=opts::Value(e.list(u"preFilters")),bpre=e.context.get(opts::Value(b->instanceIdentity()),opts::Key(u"preFilters"));const auto aitems=e.context.get(apre,opts::Key(u"items")),bitems=e.context.get(bpre,opts::Key(u"items"));return std::string("{\"preDistinct\":")+(apre!=bpre?"true":"false")+",\"postDistinct\":"+(e.context.get(opts::Value(a->instanceIdentity()),opts::Key(u"postFilters"))!=e.context.get(opts::Value(b->instanceIdentity()),opts::Key(u"postFilters"))?"true":"false")+",\"firstCount\":"+std::to_string(e.context.arrayLength(std::get<opts::ObjectRef>(aitems.data)))+",\"secondCount\":"+std::to_string(e.context.arrayLength(std::get<opts::ObjectRef>(bitems.data)))+"}";});
    observe("prototype-method-inventory",[&]{Fixture e;const auto proto=std::get<opts::ObjectRef>(e.context.get(opts::Value(e.module.abstractClass()),opts::Key(u"prototype")).data);std::string out="[";bool first=true;for(const auto& key:e.context.ownKeys(opts::Value(proto))){const auto* name=std::get_if<std::u16string>(&key.data);if(!name)continue;const auto d=e.context.descriptor(opts::Value(proto),key);if(!d)continue;if(!first)out+=",";first=false;const bool isFunction=std::holds_alternative<opts::ObjectRef>(d->value.data)&&e.context.kind(std::get<opts::ObjectRef>(d->value.data))==opts::ObjectKind::function;const bool hasGetter=d->getter.has_value();const auto writable=d->accessor?"{\"$\":\"undefined\"}":d->writable?"true":"false";out+="{\"name\":"+quote(property_host::debugText(*name))+",\"enumerable\":"+(d->enumerable?"true":"false")+",\"configurable\":"+(d->configurable?"true":"false")+",\"writable\":"+writable+",\"getter\":"+quote(hasGetter?"function":"undefined")+",\"value\":"+quote(isFunction?"function":"undefined")+"}";}return out+"]";});
    std::cout<<"{\"version\":\"4.1.3\",\"sourceFile\":\"ts/core/OutputJax.ts\",\"sourceHash\":\"e4932ba7a5d8f782022fa0ab3c8a8bc05a3072704cd8802213fdadb2722789a3\",\"cjsHash\":\"9f751917529fdf076b65746a9c910526599944193a9bffcc78c765f7140f8047\",\"records\":[";
    for(std::size_t i=0;i<records.size();i++){if(i)std::cout<<",";std::cout<<"{\"name\":"<<quote(records[i].first)<<",\"actual\":"<<records[i].second<<"}";}
    std::cout<<"]}\n";
}



