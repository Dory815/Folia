// Strict full-MML candidate for the locked Boldsymbol/ConfigMacros CJS corpus.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.boldsymbol.configuration;
import folia.math.input.tex.configmacros.configuration;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.token;
import folia.math.input.tex.node_util;
import folia.math.core.math_item;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
namespace tex=folia::math::tex; namespace opts=folia::math::options;
std::string quote(std::string_view s){
 std::string o="\"";constexpr char h[]="0123456789abcdef";
 for(unsigned char c:s){switch(c){case '\\':o+="\\\\";break;case '"':o+="\\\"";break;case '\n':o+="\\n";break;case '\r':o+="\\r";break;case '\t':o+="\\t";break;default:if(c<32){o+="\\u00";o+=h[c>>4];o+=h[c&15];}else o+=char(c);}}
 return o+'"';
}
std::string number(double d){if(std::isnan(d))return "{\"$\":\"NaN\"}";if(std::isinf(d))return d<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}";if(d==0&&std::signbit(d))return "{\"$\":\"-0\"}";std::ostringstream o;o<<std::setprecision(17)<<d;return o.str();}
struct HandlerObserver{static tex::SubHandler::ConfigurationRef items(tex::SubHandler& handler){return handler.configurationOwner();}};
struct Dump{opts::Context& cx;OptionFunctionBindings& functions;
 std::string prop(const Property& p){
  if(std::holds_alternative<Missing>(p.value))return "{\"$\":\"undefined\"}";
  if(std::holds_alternative<PropertyNull>(p.value))return "null";
  if(auto v=std::get_if<bool>(&p.value))return *v?"true":"false";
  if(auto v=std::get_if<double>(&p.value))return number(*v);
  if(auto v=std::get_if<std::string>(&p.value))return quote(*v);
  if(auto v=std::get_if<std::shared_ptr<PropertyStorage>>(&p.value);v&&*v){
   if((*v)->array){auto a=PropertyArray::array(p);std::string o="{\"$\":\"array\",\"items\":[";for(size_t i=0;i<a->size();++i){if(i)o+=',';o+=a->has(i)?prop(a->get(i)):"{\"$\":\"hole\"}";}return o+"]}";}
   return object(PropertyList::record(p).value());
  }
  if(auto f=std::get_if<FunctionRef>(&p.value);f&&*f){auto id=functions.find(*f);if(!id)throw std::runtime_error("unbound FunctionRef in MML properties");auto n=cx.get(*id,opts::Key(u"name")),l=cx.get(*id,opts::Key(u"length"));auto ns=std::get_if<std::u16string>(&n.data);auto ln=std::get_if<double>(&l.data);if(!ns||!ln)throw std::runtime_error("function metadata missing");return "{\"$\":\"function\",\"name\":"+quote(property_host::debugText(*ns))+",\"length\":"+number(*ln)+"}";}
  throw std::runtime_error("unsupported MML Property value");
 }
 std::string object(const PropertyList& r){std::string o="{";bool first=true;for(auto& k:r.keys()){if(!first)o+=',';first=false;o+=quote(k)+":"+prop(r.getOwn(k));}return o+"}";}
 std::string lookup(const tex::TokenLookup& item){
  if(auto p=std::get_if<Property>(&item))return prop(*p);
  if(auto m=std::get_if<tex::MacroRef>(&item);m&&*m){std::string fn="null";const auto method=(*m)->func();if(auto f=std::get_if<FunctionRef>(&method)){auto id=functions.find(*f);if(!id)throw std::runtime_error("unbound token-map macro function");const auto n=cx.get(*id,opts::Key(u"name")),l=cx.get(*id,opts::Key(u"length"));auto ns=std::get_if<std::u16string>(&n.data);auto ln=std::get_if<double>(&l.data);if(!ns||!ln)throw std::runtime_error("macro function metadata missing");fn="{\"$\":\"function\",\"name\":"+quote(property_host::debugText(*ns))+",\"length\":"+number(*ln)+"}";}else fn=prop(std::get<Property>(method));return "{\"_token\":"+prop((*m)->token())+",\"_func\":"+fn+",\"_args\":"+prop((*m)->args())+"}";}
  if(auto t=std::get_if<tex::TokenRef>(&item);t&&*t)return "{\"_token\":"+prop((*t)->token())+",\"_character\":"+prop((*t)->character())+",\"_attributes\":"+prop((*t)->attributes())+"}";
  throw std::runtime_error("unsupported token-map entry in strict dump");
 }
 std::string mapEntries(const tex::TokenMapRef& map){auto parsed=std::dynamic_pointer_cast<tex::AbstractParseMap>(map);if(!parsed)throw std::runtime_error("selected Source map is not an AbstractParseMap");std::string out="{\"$\":\"map\",\"entries\":[";bool first=true;const auto& entries=parsed->mapStorage();if(entries)for(const auto& [key,item]:*entries){if(!first)out+=',';first=false;out+="["+prop(key)+","+lookup(item)+"]";}return out+"]}";}
 std::string mapState(tex::TeX& jax,tex::ParserRuntime& runtime){auto shared=jax.parseOptions();auto parse=std::get_if<tex::ParseOptionsRef>(&shared);if(!parse||!*parse)throw std::runtime_error("missing ParseOptions for map observer");auto handlers=std::get_if<std::shared_ptr<tex::SubHandlers>>(&(*parse)->handlers);if(!handlers||!*handlers)throw std::runtime_error("missing real SubHandlers");constexpr std::string_view names[]={"boldsymbol","configmacros-map","configmacros-active-map","configmacros-env-map"};std::string out="{\"maps\":{";for(size_t i=0;i<std::size(names);++i){if(i)out+=',';const auto name=names[i];const auto global=runtime.mapHandler.getMap(Property(std::string(name)));const auto selected=(*handlers)->retrieve(Property(std::string(name)));out+=quote(name)+":";if(!global){out+="{\"same\":false,\"entries\":{\"$\":\"undefined\"}}";continue;}std::string ctor=dynamic_cast<tex::CommandMap*>(global.get())?"CommandMap":dynamic_cast<tex::EnvironmentMap*>(global.get())?"EnvironmentMap":dynamic_cast<tex::MacroMap*>(global.get())?"MacroMap":dynamic_cast<tex::DelimiterMap*>(global.get())?"DelimiterMap":dynamic_cast<tex::CharacterMap*>(global.get())?"CharacterMap":"AbstractTokenMap";out+="{\"same\":"+std::string(global==selected?"true":"false")+",\"constructor\":"+quote(ctor)+",\"entries\":"+mapEntries(global)+"}";}out+="},\"handlers\":{";constexpr std::string_view kinds[]={"character","delimiter","macro","environment"};for(size_t i=0;i<std::size(kinds);++i){if(i)out+=',';const auto handler=(*handlers)->get(kinds[i]);out+=quote(kinds[i])+":";if(!handler){out+="[]";continue;}const auto list=HandlerObserver::items(*handler);out+='[';bool first=true;for(auto it=list->begin(),end=list->end();it!=end;++it){if(!first)out+=',';first=false;const auto record=it.record();if(!record.present())throw std::runtime_error("missing SubHandler configuration record");const auto item=record.readItem();const auto* refValue=std::get_if<tex::TokenMapRef>(&item);if(!refValue||!*refValue)throw std::runtime_error("SubHandler configuration item is not a native TokenMapRef");const auto& ref=*refValue;const auto priorityValue=record.readPriorityValue();const auto* priority=std::get_if<double>(&priorityValue.data);if(!priority)throw std::runtime_error("SubHandler configuration priority is not numeric");out+="{\"name\":"+prop(ref->name())+",\"priority\":"+number(*priority)+",\"registeredSame\":"+std::string(runtime.mapHandler.getMap(ref->name())==ref?"true":"false")+"}";}out+=']';}return out+"}}";}
 std::string value(const opts::Value& v){
  if(std::holds_alternative<opts::Undefined>(v.data))return "{\"$\":\"undefined\"}";
  if(std::holds_alternative<opts::Null>(v.data))return "null";
  if(auto x=std::get_if<bool>(&v.data))return *x?"true":"false";
  if(auto x=std::get_if<double>(&v.data))return number(*x);
  if(auto x=std::get_if<std::u16string>(&v.data))return quote(property_host::debugText(*x));
  if(auto x=std::get_if<opts::SymbolRef>(&v.data))return "{\"$\":\"symbol\",\"context\":"+std::to_string(x->context)+",\"index\":"+std::to_string(x->index)+"}";
  const auto* objectRef=std::get_if<opts::ObjectRef>(&v.data);if(!objectRef)throw std::runtime_error("unsupported Context value in packageData");
  if(cx.kind(*objectRef)==opts::ObjectKind::function){auto name=cx.get(v,opts::Key(u"name")),length=cx.get(v,opts::Key(u"length"));auto n=std::get_if<std::u16string>(&name.data);auto l=std::get_if<double>(&length.data);return "{\"$\":\"function\",\"name\":"+quote(n?property_host::debugText(*n):"")+",\"length\":"+number(l?*l:0)+"}";}
  if(cx.kind(*objectRef)==opts::ObjectKind::array){std::string out="{\"$\":\"array\",\"items\":[";const auto length=cx.arrayLength(*objectRef);for(std::uint32_t i=0;i<length;++i){if(i)out+=',';auto k=std::to_string(i);const opts::Key key(std::u16string(k.begin(),k.end()));out+=cx.has(v,key)?value(cx.get(v,key)):"{\"$\":\"hole\"}";}return out+"]}";}
  std::string out="{";bool first=true;for(const auto& key:cx.ownKeys(v)){if(!first)out+=',';first=false;if(const auto* s=std::get_if<std::u16string>(&key.data))out+=quote(property_host::debugText(*s))+":"+value(cx.get(v,key));}return out+"}";
 }
 std::string node(mml::NodeValue v){if(!v.isNode())return "null";auto*n=v.requireNode();auto a=n->attributes();auto ch=n->childNodes;std::string o="{\"kind\":"+quote(n->kind())+",\"text\":";if(auto*t=dynamic_cast<mml::TextNode*>(n))o+=quote(property_host::debugText(t->getText()));else if(auto*t=dynamic_cast<mml::AbstractMmlTokenNode*>(n))o+=quote(property_host::debugText(t->getText()));else o+="null";o+=",\"attrs\":"+object(a?a->getAllAttributes():PropertyList{})+",\"properties\":"+object(n->getAllProperties())+",\"children\":[";if(ch)for(size_t i=0;i<ch->size();++i){if(i)o+=',';o+=node(ch->get(i));}return o+"]}";}
};
struct Item final:core::AbstractMathItem{Item(Property p,core::MathInputJaxReference j):AbstractMathItem(std::move(p),std::move(j),Property(true)){} };
opts::Value arr(opts::Context& c,std::initializer_list<opts::Value> vs){auto a=c.array((uint32_t)vs.size());uint32_t i=0;for(auto&v:vs){auto x=std::to_string(i++);c.set(a,opts::Key(std::u16string(x.begin(),x.end())),v);}return opts::Value(a);}
opts::Value packages(opts::Context& c,std::initializer_list<std::u16string_view> vs){auto a=c.array((uint32_t)vs.size());uint32_t i=0;for(auto v:vs){auto x=std::to_string(i++);c.set(a,opts::Key(std::u16string(x.begin(),x.end())),opts::Value(std::u16string(v)));}return opts::Value(a);}
opts::Value caseOptions(opts::Context&c,std::u16string_view category,std::u16string_view name,opts::Value def){auto o=c.object(),m=c.object(),a=c.object(),e=c.object();auto target=category==u"macros"?m:category==u"active"?a:e;c.set(target,opts::Key(std::u16string(name)),std::move(def));c.set(o,opts::Key(u"macros"),opts::Value(m));c.set(o,opts::Key(u"active"),opts::Value(a));c.set(o,opts::Key(u"environments"),opts::Value(e));return opts::Value(o);}
struct Graph{opts::Context cx;OptionFunctionBindings fn{cx};regexp_host::RegExpBindings rx{cx};Unicode16Normalization uni;core::InputJaxModule input{cx};core::FindMathModule find{cx};tex::FindTeXModule ft{cx,find,rx};tex::ParserRuntime runtime{cx,fn,rx,uni,input,ft};mml::MmlFactory factory;Graph(){runtime.initializeParserRuntime();tex::BasePackage::initializeModule(runtime);tex::NewcommandConfiguration::initializeModule(runtime);tex::BoldsymbolConfiguration::initializeModule(runtime);tex::ConfigMacrosConfiguration::initializeModule(runtime);}tex::TeXRef jax(std::initializer_list<std::u16string_view> pkgs,opts::Value overrides){auto c=cx.object();cx.set(c,opts::Key(u"packages"),packages(cx,pkgs));if(auto o=std::get_if<opts::ObjectRef>(&overrides.data))for(auto&k:cx.ownKeys(*o))cx.set(c,k,cx.get(*o,k));auto j=tex::TeX::make(runtime,opts::Value(c));j->setMmlFactory(&factory);return j;}mml::NodeValue compile(tex::TeX& j,std::u16string_view input){Item item(Property(property_host::debugText(input)),core::MathInputJaxReference{static_cast<core::MathInputJaxService*>(&j)});return j.compile(item,core::MathDocumentReference{PropertyNull{}});}std::string packageData(tex::TeX& j,Dump& dump){auto shared=j.parseOptions();auto parse=std::get_if<tex::ParseOptionsRef>(&shared);if(!parse||!*parse)throw std::runtime_error("missing ParseOptions");auto map=std::get_if<std::shared_ptr<SourceStringMap<opts::Value>>>(&(*parse)->packageData);if(!map||!*map)throw std::runtime_error("missing packageData map");std::string out="{\"$\":\"map\",\"entries\":[";auto keys=(*map)->keys();bool first=true;while(auto key=keys.next()){if(!first)out+=',';first=false;auto value=(*map)->get(*key);if(!value)throw std::runtime_error("packageData key disappeared");out+="["+quote(property_host::debugText(*key))+","+dump.value(*value)+"]";}return out+"]}";}};
}
namespace { std::string currentStage="startup"; }
int runDump(){
 std::unique_ptr<Graph> graph;
 std::cerr<<"stage:graph-boot\n"<<std::flush;currentStage="graph-boot";
 try{graph=std::make_unique<Graph>();}
 catch(const opts::Exception&e){std::cerr<<"boot failure options::Exception "<<e.name<<" "<<property_host::debugText(e.message)<<"\n"<<std::flush;return 2;}
 catch(const tex::TexError&e){std::cerr<<"boot failure TexError "<<property_host::debugText(e.id)<<" "<<property_host::debugText(e.message)<<"\n"<<std::flush;return 2;}
 catch(const std::exception&e){std::cerr<<"boot failure std::exception "<<e.what()<<"\n"<<std::flush;return 2;}
 catch(...){std::cerr<<"boot failure unknown non-standard exception\n"<<std::flush;return 2;}
 Graph& g=*graph;Dump d{g.cx,g.fn};
 struct C{std::string_view name;std::u16string_view input;int kind;int option;};
 const C rows[]={
 {"bold:single-token",u"\\boldsymbol{x}",0,0},{"bold:greek-and-operator",u"\\boldsymbol{\\alpha+x}",0,0},{"bold:nested-scope",u"\\boldsymbol{x+\\boldsymbol{y}}+z",0,0},{"bold:text-exclusion",u"\\boldsymbol{\\text{abc}+x}",0,0},{"bold:variant-rewrites",u"\\boldsymbol{\\mathit{x}+\\mathfrak{y}+\\mathcal{z}}",0,0},{"bold:unsupported-variant-retained",u"\\boldsymbol{\\mathbb{R}}",0,0},
 {"configmacros:string-macro",u"\\foo",1,0},{"configmacros:array-macro",u"\\foo",1,1},{"configmacros:parameter-template",u"\\foo{a}{b}",1,2},{"configmacros:optional-default",u"\\foo{x}",1,3},{"configmacros:template-list",u"\\foo a,b",1,4},{"configmacros:active-character",u"~",2,0},{"configmacros:configured-environment",u"\\begin{xx}x\\end{xx}",3,0},{"configmacros:environment-option-array",u"\\begin{xx}{q}x\\end{xx}",3,1},{"configmacros:bad-macro-arity",u"\\foo",1,5},{"configmacros:standalone-package",u"\\foo",4,6}};
 std::cout.setf(std::ios::unitbuf);
 std::cout<<"{\"cases\":[";
 for(size_t i=0;i<std::size(rows);++i){if(i)std::cout<<',';const auto&r=rows[i];currentStage=std::string("case:")+std::string(r.name);std::cerr<<currentStage<<" start\n"<<std::flush;opts::Value opt(opts::Undefined{});if(r.kind==1){if(r.option==0||r.option==6)opt=caseOptions(g.cx,u"macros",u"foo",opts::Value(u"x+y"));else if(r.option==1)opt=caseOptions(g.cx,u"macros",u"foo",arr(g.cx,{opts::Value(u"x+y")}));else if(r.option==2)opt=caseOptions(g.cx,u"macros",u"foo",arr(g.cx,{opts::Value(2),opts::Value(u"#1+#2")}));else if(r.option==3)opt=caseOptions(g.cx,u"macros",u"foo",arr(g.cx,{opts::Value(2),opts::Value(u"#1+#2"),opts::Value(u"d")}));else if(r.option==4)opt=caseOptions(g.cx,u"macros",u"foo",arr(g.cx,{opts::Value(u"#1+#2"),opts::Value(2),arr(g.cx,{opts::Value(u""),opts::Value(u",")})}));else if(r.option==5)opt=caseOptions(g.cx,u"macros",u"foo",arr(g.cx,{opts::Value(4),opts::Value(u"#1")}));}else if(r.kind==2)opt=caseOptions(g.cx,u"active",u"~",opts::Value(u"x+y"));else if(r.kind==3&&r.option==0)opt=caseOptions(g.cx,u"environments",u"xx",arr(g.cx,{opts::Value(u"a+"),opts::Value(u"+b")}));else if(r.kind==3&&r.option==1)opt=caseOptions(g.cx,u"environments",u"xx",arr(g.cx,{opts::Value(u"#1+"),opts::Value(u"+end"),opts::Value(1),opts::Value(u"d")}));else if(r.kind==4)opt=caseOptions(g.cx,u"macros",u"foo",opts::Value(u"x+y"));
  const auto pkgs=r.kind==0?std::initializer_list<std::u16string_view>{u"base",u"boldsymbol"}:r.kind==1||r.kind==2||r.kind==3?std::initializer_list<std::u16string_view>{u"base",u"newcommand",u"configmacros"}:std::initializer_list<std::u16string_view>{u"base",u"configmacros"};
   tex::TeXRef jax;std::string result="null",error="null";try{jax=g.jax(pkgs,std::move(opt));result=d.node(g.compile(*jax,r.input));}catch(const tex::TexError&e){error="{\"name\":\"TexError\",\"id\":"+quote(property_host::debugText(e.id))+",\"message\":"+quote(property_host::debugText(e.message))+"}";}catch(const opts::Exception&e){error="{\"name\":"+quote(e.name)+",\"id\":null,\"message\":"+quote(property_host::debugText(e.message))+"}";}catch(const std::exception&e){std::cerr<<"unexpected native std::exception stage="<<currentStage<<" "<<e.what()<<"\\n"<<std::flush;throw;}std::string packageData="null",maps="null";if(jax){packageData=g.packageData(*jax,d);maps=d.mapState(*jax,g.runtime);}std::cout<<"{\"name\":"<<quote(r.name)<<",\"result\":"<<result<<",\"error\":"<<error<<",\"maps\":"<<maps<<",\"packageData\":"<<packageData<<"}";
 }
 std::cout<<"]}\n";
 return 0;
}
int main(){
 try{return runDump();}
 catch(const opts::Exception& e){std::cerr<<"failure stage="<<currentStage<<" options::Exception "<<e.name<<" "<<property_host::debugText(e.message)<<"\n"<<std::flush;return 2;}
 catch(const tex::TexError& e){std::cerr<<"failure stage="<<currentStage<<" TexError "<<property_host::debugText(e.id)<<" "<<property_host::debugText(e.message)<<"\n"<<std::flush;return 2;}
 catch(const std::exception& e){std::cerr<<"failure stage="<<currentStage<<" std::exception "<<e.what()<<"\n"<<std::flush;return 2;}
 catch(...){std::cerr<<"failure stage="<<currentStage<<" unknown non-standard exception\n"<<std::flush;return 2;}
}

