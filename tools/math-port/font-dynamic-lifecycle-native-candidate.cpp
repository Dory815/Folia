// Full native dynamic-font lifecycle trace candidate. Root owns the standalone
// build target; this source intentionally does not use the table-dump host.
import std;
import folia.math.fonts.newcm_font_data;
import folia.math.util.promise;
import folia.math.util.option_types;
import folia.math.util.property_host;
import folia.math.util.option_functions;
import folia.math.util.context_property_bindings;
import folia.math.util.map_bindings;
import folia.math.output.common.font_data_bindings;
import folia.math.output.common.font_data_options;

namespace {
using namespace folia::math;
using namespace folia::math::output::common;
using namespace folia::math::fonts;
namespace math_options=folia::math::options;
using folia::math::async::Promise;
using folia::math::async::PromiseRef;
using folia::math::async::QueueRef;
options::ObjectRef registerNewcmSourceClass(options::Context& context,
    OptionFunctionBindings& functions,ContextPropertyBindings& properties,
    FontDataContextBindings& identities,options::MapContextBindings& maps,
    const std::shared_ptr<FontLoadHost>& host,
    const std::shared_ptr<NewcmFontDataClass>& owner){
    (void)functions;
    const auto constructor=identities.registerClass(u"MathJaxNewcmFont",owner,*host,
        std::static_pointer_cast<void>(host),
        [](FontDataClass& actual,FontLoadHost& loadHost,options::Context& cx,options::Value supplied){
            return std::make_shared<NewcmFontData>(static_cast<NewcmFontDataClass&>(actual),
                loadHost,cx,std::move(supplied));
        });
    owner->bindSourceIdentity(properties,constructor,maps);
    owner->dynamicExtensions=options::Value(maps.create());
    context.bindData(constructor,options::Key(u"dynamicExtensions"),
        std::make_shared<options::NativeDataField>(std::static_pointer_cast<void>(owner),
            [owner]{return owner->dynamicExtensions;},
            [owner](options::Value value){owner->dynamicExtensions=std::move(value);}));
    bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
        u"defaultVariants",owner->defaultVariants);
    bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
        u"defaultSizeVariants",owner->defaultSizeVariants);
    bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),
        u"defaultStretchVariants",owner->defaultStretchVariants);
    owner->initializeSourceData();
    return constructor;
}

void quote(std::ostream& o,std::string_view s){o.put('"');for(unsigned char c:s){switch(c){case '"':o<<"\\\"";break;case '\\':o<<"\\\\";break;case '\n':o<<"\\n";break;case '\r':o<<"\\r";break;case '\t':o<<"\\t";break;default:o.put(static_cast<char>(c));}}o.put('"');}
std::string narrow(std::u16string_view s){std::string out;for(char16_t c:s)out.push_back(c<128?static_cast<char>(c):'?');return out;}
std::string sourceText(const options::Value& value){
    if(const auto* text=std::get_if<std::u16string>(&value.data))return property_host::debugText(*text);
    throw std::runtime_error("expected a Source string-valued DynamicFile field");
}
void numberJson(std::ostream& out,double value){
    if(std::isnan(value)){out<<"{\"$\":\"NaN\"}";return;}
    if(std::isinf(value)){out<<(value<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}");return;}
    if(value==0&&std::signbit(value)){out<<"{\"$\":\"-0\"}";return;}
    out<<std::setprecision(std::numeric_limits<double>::max_digits10)<<value;
}
void propertyJson(std::ostream&,const Property&);
void recordJson(std::ostream& out,const PropertyList& value){
    auto keys=value.keys();std::sort(keys.begin(),keys.end());out.put('{');
    for(std::size_t i=0;i<keys.size();++i){if(i)out.put(',');quote(out,keys[i]);out.put(':');propertyJson(out,value.getOwn(keys[i]));}out.put('}');
}
void propertyJson(std::ostream& out,const Property& value){
    std::visit([&](const auto& item){using T=std::decay_t<decltype(item)>;
        if constexpr(std::is_same_v<T,Missing>)out<<"{\"$\":\"undefined\"}";
        else if constexpr(std::is_same_v<T,PropertyNull>)out<<"null";
        else if constexpr(std::is_same_v<T,bool>)out<<(item?"true":"false");
        else if constexpr(std::is_same_v<T,double>)numberJson(out,item);
        else if constexpr(std::is_same_v<T,std::string>)quote(out,item);
        else if constexpr(std::is_same_v<T,std::shared_ptr<PropertyStorage>>){
            if(!item)throw std::runtime_error("null glyph option record");
            if(item->array){out.put('[');for(std::size_t i=0;i<item->arrayValues.size();++i){if(i)out.put(',');
                if(i>=item->arrayPresent.size()||!item->arrayPresent[i])out<<"{\"$\":\"hole\"}";else propertyJson(out,item->arrayValues[i]);}out.put(']');}
            else {Property record;record.value=item;recordJson(out,PropertyList::record(record).value());}
        }else throw std::runtime_error("unsupported glyph option value");
    },value.value);
}
void glyphValue(std::ostream& out,const Property& glyph){
    propertyJson(out,glyph);
}
Property makeGlyph(double height,double depth,double width,std::string path){
    PropertyArray value;value.resize(4);value.set(0,Property(height));value.set(1,Property(depth));value.set(2,Property(width));
    value.set(3,Property(PropertyList{{"p",Property(std::move(path))}}));return Property(value);
}
struct ErrorView{std::string name,message;};
ErrorView errorView(const std::exception& e){if(auto* x=dynamic_cast<const options::Exception*>(&e))return {x->name,narrow(x->message)};return {"Error",e.what()};}
std::optional<ErrorView> errorAny(const std::any& value){
    if(auto* e=std::any_cast<options::Exception>(&value))return ErrorView{e->name,narrow(e->message)};
    if(auto* s=std::any_cast<std::string>(&value))return ErrorView{"Error",*s};
    if(auto* p=std::any_cast<std::exception_ptr>(&value)){try{std::rethrow_exception(*p);}catch(const std::exception& e){return errorView(e);}catch(int n){return ErrorView{"number",std::to_string(n)};}catch(...){return ErrorView{"Error","unknown exception"};}}
    if(auto* s=std::any_cast<std::u16string>(&value))return ErrorView{"Error",narrow(*s)};
    return {};
}
void errorJson(std::ostream& o,const ErrorView& e){o<<"{\"name\":";quote(o,e.name);o<<",\"message\":";quote(o,e.message);o<<'}';}
std::optional<ErrorView> caught(const std::function<void()>& f){try{f();return {};}catch(const std::exception& e){return errorView(e);}catch(int n){return ErrorView{"number",std::to_string(n)};}catch(...){return ErrorView{"Error","unknown exception"};}}
void nullableError(std::ostream& o,const std::optional<ErrorView>& e){if(e)errorJson(o,*e);else o<<"null";}
void traceJson(std::ostream& o,const std::vector<std::string>& trace){o.put('[');for(std::size_t i=0;i<trace.size();++i){if(i)o.put(',');quote(o,trace[i]);}o.put(']');}
std::string promiseState(PromiseRef p){switch(p->state()){case async::State::pending:return "pending";case async::State::fulfilled:return "fulfilled";case async::State::rejected:return "rejected";}return "?";}
void outcomeJson(std::ostream& o,PromiseRef p){o<<"{\"state\":";quote(o,promiseState(p));if(p->state()==async::State::rejected){o<<",\"error\":";if(auto e=errorAny(p->value()))errorJson(o,*e);else o<<"null";}o.put('}');}
void drain(QueueRef q){std::size_t guard=0;while(q->runOne()){if(++guard>10000)throw std::runtime_error("promise queue did not quiesce");}}
DynamicFileRef definition(NewcmFontDataClass& cls,std::string name){auto d=std::make_shared<DynamicFile>();d->file=options::Value(property_host::text("__full_lifecycle_"+name+"__"));cls.dynamicFiles.insert_or_assign(sourceText(d->file),d);return d;}
struct Scenario{std::string name;std::vector<std::string> trace;bool initialSetup=false;std::size_t loads=0;bool failed=false,setup=false,promiseRetained=false;std::optional<ErrorView> firstError,secondError;std::string asyncStored="",asyncFirst="",asyncSecond="",glyph="";};
void emit(std::ostream& o,const Scenario& s,bool async){o<<"{\"name\":";quote(o,s.name);o<<",\"trace\":";traceJson(o,s.trace);o<<",\"initialSetup\":";quote(o,s.initialSetup?"function":"undefined");o<<",\"loads\":"<<s.loads<<",\"failed\":"<<(s.failed?"true":"false")<<",\"setup\":";quote(o,s.setup?"function":"undefined");o<<",\"promiseRetained\":"<<(s.promiseRetained?"true":"false");if(async){o<<",\"storedResult\":"<<s.asyncStored<<",\"firstResult\":"<<s.asyncFirst<<",\"secondResult\":"<<s.asyncSecond;}else{o<<",\"firstError\":";nullableError(o,s.firstError);o<<",\"secondError\":";nullableError(o,s.secondError);}if(!s.glyph.empty())o<<",\"glyph\":"<<s.glyph;o<<'}';}

Scenario syncScenario(std::string name,int providerMode){
    options::Context context;OptionFunctionBindings functions(context);ContextPropertyBindings properties(context,functions);FontDataContextBindings identities(context);options::MapContextBindings maps(context);auto cls=std::make_shared<NewcmFontDataClass>(context);auto q=std::make_shared<async::JobQueue>();auto hostOwner=std::make_shared<FontLoadHost>();auto& host=*hostOwner;host.queue=q;host.asyncIsSynchronous=[] {return true;};(void)registerNewcmSourceClass(context,functions,properties,identities,maps,hostOwner,cls);auto d=definition(*cls,name);std::vector<std::string> trace;std::size_t loads=0,warnings=0;
    if(providerMode!=0)host.syncLoad=[&,d,name,providerMode](FontData&,const DynamicFileRef&,std::string path){trace.push_back("load:"+path);++loads;if(providerMode==1)throw options::Exception("Error",u"provider boom");if(providerMode==2){d->setup=[&trace](FontData&){throw options::Exception("Error",u"setup boom");};}else if(providerMode==3){auto glyph=makeGlyph(1,0,.5,"M0 0");CharMapMap chars;chars["normal"].insert_or_assign(0xE000,glyph);cls->dynamicSetup("",sourceText(d->file),std::move(chars));auto setup=d->setup;d->setup=[&trace,setup](FontData& f){trace.push_back("setup");setup(f);};}};
    host.warn=[&](const std::any& a){++warnings;if(auto* s=std::any_cast<std::string>(&a)){trace.push_back("warn:"+*s);return;}auto e=errorAny(a);trace.push_back("warn:"+(e?e->name+":"+e->message:"unknown"));};
    NewcmFontData font(*cls,host,context);Scenario s;s.name=std::move(name);s.initialSetup=static_cast<bool>(d->setup);s.firstError=caught([&]{font.loadDynamicFileSync(d);});auto promise=d->promise;s.secondError=caught([&]{font.loadDynamicFileSync(d);});if(providerMode==3){auto found=font.getChar("normal",0xE000);if(!found)s.glyph="{\"$\":\"undefined\"}";else{std::ostringstream glyph;glyphValue(glyph,*found);s.glyph=glyph.str();}}s.trace=std::move(trace);s.loads=loads;s.failed=d->failed;s.setup=static_cast<bool>(d->setup);s.promiseRetained=promise==d->promise;(void)warnings;return s;
}
Scenario asyncScenario(std::string name,int providerMode){
    options::Context context;OptionFunctionBindings functions(context);ContextPropertyBindings properties(context,functions);FontDataContextBindings identities(context);options::MapContextBindings maps(context);auto cls=std::make_shared<NewcmFontDataClass>(context);auto q=std::make_shared<async::JobQueue>();auto hostOwner=std::make_shared<FontLoadHost>();auto& host=*hostOwner;host.queue=q;(void)registerNewcmSourceClass(context,functions,properties,identities,maps,hostOwner,cls);auto d=definition(*cls,name);std::vector<std::string> trace;std::size_t loads=0;
    if(providerMode!=0)host.asyncLoad=[&,d,name,providerMode,q](std::shared_ptr<FontData>,DynamicFileRef,std::string path)->PromiseRef{trace.push_back("load:"+path);++loads;if(providerMode==3)throw options::Exception("Error",u"sync throw from asyncLoad");if(providerMode==2){auto p=Promise::create(q);p->reject(options::Exception("Error",u"async provider boom"));return p;}auto p=Promise::create(q);if(providerMode==4){d->setup=[&trace](FontData&){trace.push_back("setup");throw options::Exception("Error",u"async setup boom");};}else if(providerMode==1){auto glyph=makeGlyph(1,0,.5,"M1 1");CharMapMap chars;chars["normal"].insert_or_assign(0xE001,glyph);cls->dynamicSetup("",sourceText(d->file),std::move(chars));auto setup=d->setup;d->setup=[&trace,setup](FontData& f){trace.push_back("setup");setup(f);};}p->resolve();return p;};
    host.warn=[&](const std::any& a){if(const auto* s=std::any_cast<std::string>(&a)){trace.push_back("warn:"+*s);return;}auto e=errorAny(a);trace.push_back("warn:"+(e?e->name+":"+e->message:"unknown"));};
    auto font=std::make_shared<NewcmFontData>(*cls,host,context);Scenario s;s.name=std::move(name);s.initialSetup=static_cast<bool>(d->setup);auto first=font->loadDynamicFile(d);auto stored=d->promise;drain(q);auto second=font->loadDynamicFile(d);drain(q);std::ostringstream storedOut,one,two;outcomeJson(storedOut,stored);outcomeJson(one,first);outcomeJson(two,second);s.asyncStored=storedOut.str();s.asyncFirst=one.str();s.asyncSecond=two.str();s.trace=std::move(trace);s.loads=loads;s.failed=d->failed;s.setup=static_cast<bool>(d->setup);s.promiseRetained=stored==d->promise;return s;
}
}
int main(){
    std::string phase="construct scenarios";
    try {
    std::vector<Scenario> cases;
    phase="sync success";
    cases.push_back(syncScenario("sync_success",3));
    phase="sync missing provider";
    cases.push_back(syncScenario("sync_missing",0));
    phase="sync provider throw";
    cases.push_back(syncScenario("sync_provider_throw",1));
    phase="sync setup throw";
    cases.push_back(syncScenario("sync_setup_throw",2));
    phase="async success";
    cases.push_back(asyncScenario("async_success",1));
    phase="async missing provider";
    cases.push_back(asyncScenario("async_missing",0));
    phase="async provider reject";
    cases.push_back(asyncScenario("async_provider_reject",2));
    phase="async provider synchronous throw";
    cases.push_back(asyncScenario("async_provider_sync_throw",3));
    phase="async setup throw";
    cases.push_back(asyncScenario("async_setup_throw",4));
    phase="serialize output";
    std::cout<<"{\"scenarios\":[";for(std::size_t i=0;i<cases.size();++i){if(i)std::cout.put(',');emit(std::cout,cases[i],i>=4);}std::cout<<"]}\n";
    return 0;
    } catch(const options::Exception& e) {
        std::cerr<<"font lifecycle phase="<<phase<<" exception="<<e.name<<":"<<property_host::debugText(e.message)<<"\n";
    } catch(const std::exception& e) {
        std::cerr<<"font lifecycle phase="<<phase<<" exception=Error:"<<e.what()<<"\n";
    } catch(...) {
        std::cerr<<"font lifecycle phase="<<phase<<" exception=unknown\n";
    }
    return 2;
}
