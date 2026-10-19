// Real-runtime candidate for the pinned AMS/Newcommand Source observations.
// Kept outside tests/ and the automatic test target until Root wires the gate.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.ams.configuration;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.token;
import folia.math.core.math_item;
import folia.math.core.input_jax_host;
import folia.math.core.find_math_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_node;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.util.option_functions;
import folia.math.util.property_host;

namespace {
namespace tex=folia::math::tex;
namespace opts=folia::math::options;
using namespace folia::math;

std::string quote(std::string_view s){
  std::string out="\"";constexpr char h[]="0123456789abcdef";
  for(unsigned char c:s)switch(c){case '\\':out+="\\\\";break;case '"':out+="\\\"";break;case '\b':out+="\\b";break;case '\f':out+="\\f";break;case '\n':out+="\\n";break;case '\r':out+="\\r";break;case '\t':out+="\\t";break;default:if(c<32){out+="\\u00";out.push_back(h[c>>4]);out.push_back(h[c&15]);}else out.push_back(static_cast<char>(c));}
  return out+'"';
}
std::string number(double x){if(std::isnan(x))return "{\"$\":\"NaN\"}";if(std::isinf(x))return x<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}";if(x==0&&std::signbit(x))return "{\"$\":\"-0\"}";std::ostringstream s;s<<std::setprecision(17)<<x;return s.str();}
std::string utf8(std::u16string_view s){return property_host::debugText(s);}
std::string functionJson(std::string_view name,double length){return "{\"$\":\"function\",\"name\":"+quote(name)+",\"length\":"+number(length)+"}";}

struct Serializer{
  opts::Context& context;OptionFunctionBindings& functions;std::unordered_set<const void*> seen;
  void resetSeen(){seen.clear();}
  bool markSeen(const void* identity){return seen.insert(identity).second;}
  std::string property(const Property& p){
    if(p.missing())return "{\"$\":\"undefined\"}";
    if(std::holds_alternative<PropertyNull>(p.value))return "null";
    if(const auto*x=std::get_if<bool>(&p.value))return *x?"true":"false";
    if(const auto*x=std::get_if<double>(&p.value))return number(*x);
    if(const auto*x=std::get_if<std::string>(&p.value))return quote(*x);
    if(const auto*x=std::get_if<FunctionRef>(&p.value)){auto id=functions.find(*x);if(!id)throw std::runtime_error("MML contains unbound FunctionRef");const auto n=context.get(*id,opts::Key(u"name")),l=context.get(*id,opts::Key(u"length"));const auto*ns=std::get_if<std::u16string>(&n.data);const auto*ln=std::get_if<double>(&l.data);if(!ns||!ln)throw std::runtime_error("MML FunctionRef metadata missing");return functionJson(utf8(*ns),*ln);}
    if(std::holds_alternative<std::shared_ptr<PropertyStorage>>(p.value)){
      const auto& identity=std::get<std::shared_ptr<PropertyStorage>>(p.value);
      if(identity&&!markSeen(identity.get()))return "{\"$\":\"cycle\"}";
      if(auto a=PropertyArray::array(p)){std::string out="{\"$\":\"array\",\"items\":[";for(std::size_t i=0;i<a->size();++i){if(i)out+=',';out+=a->has(i)?property(a->get(i)):"{\"$\":\"hole\"}";}return out+"]}";}
      auto r=PropertyList::record(p);if(!r)throw std::runtime_error("unsupported Property storage");std::string out="{";bool first=true;for(const auto&k:r->keys()){if(!first)out+=',';first=false;out+=quote(k)+":"+property(r->getOwn(k));}return out+"}";
    }
    throw std::runtime_error("unsupported Property variant in strict AMS dump");
  }
  std::string value(const opts::Value& v){
    if(std::holds_alternative<opts::Undefined>(v.data))return "{\"$\":\"undefined\"}";
    if(std::holds_alternative<opts::Null>(v.data))return "null";
    if(const auto*x=std::get_if<bool>(&v.data))return *x?"true":"false";
    if(const auto*x=std::get_if<double>(&v.data))return number(*x);
    if(const auto*x=std::get_if<std::u16string>(&v.data))return quote(utf8(*x));
    if(const auto*x=std::get_if<opts::ObjectRef>(&v.data)){
      if(context.kind(*x)==opts::ObjectKind::array){const auto len=context.get(opts::Value(*x),opts::Key(u"length"));const auto*n=std::get_if<double>(&len.data);if(!n||*n<0)throw std::runtime_error("invalid Context array length");std::string out="{\"$\":\"array\",\"items\":[";for(std::size_t i=0;i<static_cast<std::size_t>(*n);++i){if(i)out+=',';const auto raw=std::to_string(i);const std::u16string key(raw.begin(),raw.end());if(!context.hasOwn(opts::Value(*x),opts::Key(key))){out+="{\"$\":\"hole\"}";continue;}const auto item=context.get(opts::Value(*x),opts::Key(key));out+=value(item);}return out+"]}";}
      std::string out="{";bool first=true;for(const auto&key:context.ownKeys(opts::Value(*x))){if(!first)out+=',';first=false;const auto raw=context.keyString(key);out+=quote(utf8(raw))+":"+value(context.get(opts::Value(*x),key));}return out+"}";
    }
    throw std::runtime_error("unsupported Context Value in strict AMS dump");
  }
  std::string node(mml::NodeValue n){if(!n.isNode())return "null";auto*p=n.requireNode();auto attrs=p->attributes();std::string out="{\"kind\":"+quote(p->kind());
    if(auto* text=dynamic_cast<mml::TextNode*>(p))out+=",\"text\":"+quote(utf8(text->getText()));
    else if(auto* token=dynamic_cast<mml::AbstractMmlTokenNode*>(p))out+=",\"text\":"+quote(utf8(token->getText()));
    auto record=[&](const PropertyList& r){std::string s="{";bool first=true;for(const auto&k:r.keys()){if(!first)s+=',';first=false;s+=quote(k)+":"+property(r.getOwn(k));}return s+"}";};
    out+=",\"attrs\":"+record(attrs?attrs->getAllAttributes():PropertyList{})+",\"properties\":"+record(p->getAllProperties())+",\"children\":[";const auto c=p->childNodes;if(c)for(std::size_t i=0;i<c->size();++i){if(i)out+=',';out+=node(c->get(i));}return out+"]}";
  }
};

std::string tokenLookup(Serializer&s,const tex::TokenLookup& lookup){
  if(const auto*p=std::get_if<Property>(&lookup))return s.property(*p);
  if(const auto*p=std::get_if<tex::MacroRef>(&lookup)){if(!*p)throw std::runtime_error("null MacroRef in token map");if(!s.markSeen(p->get()))return "{\"$\":\"cycle\"}";const auto method=(*p)->func();std::string fn;
    if(const auto*f=std::get_if<FunctionRef>(&method)){auto id=s.functions.find(*f);if(!id)throw std::runtime_error("unbound macro function");const auto n=s.context.get(*id,opts::Key(u"name")),l=s.context.get(*id,opts::Key(u"length"));const auto*ns=std::get_if<std::u16string>(&n.data);const auto*ln=std::get_if<double>(&l.data);if(!ns||!ln)throw std::runtime_error("macro function metadata missing");fn=functionJson(utf8(*ns),*ln);}else fn=s.property(std::get<Property>(method));
    return "{\"_token\":"+s.property((*p)->token())+",\"_func\":"+fn+",\"_args\":"+s.property((*p)->args())+"}";
  }
  if(const auto*p=std::get_if<tex::TokenRef>(&lookup)){if(!*p)throw std::runtime_error("null TokenRef in token map");return "{\"_token\":"+s.property((*p)->token())+",\"_char\":"+s.property((*p)->character())+",\"_attributes\":"+s.property((*p)->attributes())+"}";}
  throw std::runtime_error("unsupported token-map lookup variant");
}
std::string mapEntries(Serializer&s,const tex::TokenMapRef& map,std::string_view label){s.resetSeen();auto parsed=std::dynamic_pointer_cast<tex::AbstractParseMap>(map);if(!parsed)throw std::runtime_error("dynamic registry map is not AbstractParseMap");const auto& entries=parsed->mapStorage();std::string out="{\"$\":\"map\",\"entries\":[";bool first=true;if(entries)for(const auto&[key,lookup]:*entries){if(!first)out+=',';first=false;try{out+='['+s.property(key)+','+tokenLookup(s,lookup)+']';}catch(const std::exception&e){throw std::runtime_error("dynamic map "+std::string(label)+" token "+s.property(key)+": "+e.what());}}return out+"]}";}
std::string mapState(Serializer&s,tex::TeX& jax,tex::ParserRuntime& runtime){auto po=jax.parseOptions();auto*p=std::get_if<tex::ParseOptionsRef>(&po);if(!p||!*p)throw std::runtime_error("missing ParseOptions");auto handlers=std::get_if<std::shared_ptr<tex::SubHandlers>>(&(*p)->handlers);if(!handlers||!*handlers)throw std::runtime_error("missing handlers");const std::array<std::string_view,3>names{"new-Delimiter","new-Command","new-Environment"};std::string out="{";for(std::size_t i=0;i<names.size();++i){if(i)out+=',';const auto name=names[i];out+=quote(name)+":";auto registered=runtime.mapHandler.getMap(Property(std::string(name)));auto selected=(*handlers)->retrieve(Property(std::string(name)));if(!registered)out+="null";else out+="{\"sameRegistryMap\":"+std::string(registered==selected?"true":"false")+",\"entries\":"+mapEntries(s,registered,name)+"}";}return out+"}";}

struct CandidateMathItem:core::AbstractMathItem{
  CandidateMathItem(Property math,core::MathInputJaxReference input,
                    Property display=Missing{},
                    core::MathLocation start=Property(Missing{}),
                    core::MathLocation end=Property(Missing{}))
      :AbstractMathItem(std::move(math),std::move(input),std::move(display),
                        std::move(start),std::move(end)){}
};
struct RuntimeGraph{
  opts::Context context;OptionFunctionBindings functions{context};regexp_host::RegExpBindings regex{context};Unicode16Normalization unicode;core::InputJaxModule input{context};core::FindMathModule findMath{context};tex::FindTeXModule findTeX{context,findMath,regex};tex::ParserRuntime runtime{context,functions,regex,unicode,input,findTeX};mml::MmlFactory factory;
  RuntimeGraph(){runtime.initializeParserRuntime();tex::BasePackage::initializeModule(runtime);tex::NewcommandConfiguration::initializeModule(runtime);tex::AmsConfiguration::initializeModule(runtime);}
  tex::TeXRef jax(std::span<const std::u16string_view> names){auto options=context.object();auto packages=context.array(static_cast<std::uint32_t>(names.size()));std::size_t i=0;for(auto name:names){auto raw=std::to_string(i++);context.set(packages,opts::Key(std::u16string(raw.begin(),raw.end())),opts::Value(std::u16string(name)));}context.set(options,opts::Key(u"packages"),opts::Value(packages));auto result=tex::TeX::make(runtime,opts::Value(options));result->setMmlFactory(&factory);return result;}
};
struct Case{std::string_view name;std::u16string_view input;std::vector<std::u16string_view> packages;};
const Case cases[]={
 {"plain-x",u"x",{u"base",u"newcommand",u"ams"}},
 {"newcommand-simple",u"\\newcommand{\\foo}{x}\\foo",{u"base",u"newcommand",u"ams"}},
 {"newcommand-redefinition",u"\\newcommand{\\foo}{x}\\renewcommand{\\foo}{y}\\foo",{u"base",u"newcommand",u"ams"}},
 {"newcommand-optional-default",u"\\newcommand{\\foo}[2][d]{#1+#2}\\foo{x}",{u"base",u"newcommand",u"ams"}},
 {"newcommand-optional-explicit",u"\\newcommand{\\foo}[2][d]{#1+#2}\\foo[z]{x}",{u"base",u"newcommand",u"ams"}},
 {"newcommand-invalid-count",u"\\newcommand{\\foo}[a]{x}",{u"base",u"newcommand",u"ams"}},
 {"def-parameter-template",u"\\def\\foo#1,#2;{#2+#1}\\foo a,b;",{u"base",u"newcommand",u"ams"}},
 {"def-optional-brace",u"\\def\\foo#1#2{#1+#2}\\foo{x}{y}",{u"base",u"newcommand",u"ams"}},
 {"let-control-sequence",u"\\let\\foo=\\alpha\\foo",{u"base",u"newcommand",u"ams"}},
 {"new-environment",u"\\newenvironment{xx}{a}{b}\\begin{xx}x\\end{xx}",{u"base",u"newcommand",u"ams"}},
 {"new-environment-optional",u"\\newenvironment{xx}[2][d]{#1}{#2}\\begin{xx}{x}z\\end{xx}",{u"base",u"newcommand",u"ams"}},
 {"ams-frac",u"\\dfrac{1}{2}",{u"base",u"newcommand",u"ams"}},
 {"ams-binomial",u"\\binom{n}{k}",{u"base",u"newcommand",u"ams"}},
 {"ams-matrix",u"\\begin{matrix}a&b\\\\c&d\\end{matrix}",{u"base",u"newcommand",u"ams"}},
 {"ams-cases",u"\\begin{cases}x&x>0\\\\-x&x<0\\end{cases}",{u"base",u"newcommand",u"ams"}},
 {"ams-align",u"\\begin{align}a&=b\\\\c&=d\\end{align}",{u"base",u"newcommand",u"ams"}},
 {"ams-align-star",u"\\begin{align*}a&=b\\\\c&=d\\end{align*}",{u"base",u"newcommand",u"ams"}},
 {"ams-multline",u"\\begin{multline}a+b\\\\c+d\\end{multline}",{u"base",u"newcommand",u"ams"}},
 {"ams-flalign",u"\\begin{flalign}a&=b\\end{flalign}",{u"base",u"newcommand",u"ams"}},
 {"ams-operatorname",u"\\operatorname{rank}(A)",{u"base",u"newcommand",u"ams"}},
 {"ams-tag-reference",u"\\begin{equation}\\tag{A}x\\end{equation}",{u"base",u"newcommand",u"ams"}},
 {"ams-sideset",u"\\sideset{_a^b}{_c^d}\\sum",{u"base",u"newcommand",u"ams"}},
 {"ams-arrow",u"\\xrightarrow[a]{b}",{u"base",u"newcommand",u"ams"}},
 {"protected-recursive-limit",u"\\newcommand{\\foo}{\\foo}\\foo",{u"base",u"newcommand",u"ams"}},
};
}

int main(int argc,char**argv){
 try{
  RuntimeGraph graph;Serializer serializer{graph.context,graph.functions};std::string output="{\"cases\":{";
  for(std::size_t i=0;i<std::size(cases);++i){
   const auto& c=cases[i];if(i)output+=',';output+=quote(c.name)+":";
   std::string result="null",error="null",maps="null",packageData="null";tex::TeXRef jax;
   try{
    jax=graph.jax(c.packages);
    CandidateMathItem math(Property(property_host::debugText(c.input)),
      core::MathInputJaxReference(static_cast<core::MathInputJaxService*>(jax.get())),Property(true));
    result=serializer.node(jax->compile(math,core::MathDocumentReference(Missing{})));
   }catch(const tex::TexError&e){
    error="{\"name\":\"TexError\",\"id\":"+quote(utf8(e.id))+",\"message\":"+quote(utf8(e.message))+"}";
   }catch(const opts::Exception&e){
    error="{\"name\":"+quote(e.name)+",\"id\":null,\"message\":"+quote(utf8(e.message))+"}";
   }catch(const std::exception&e){
    error="{\"name\":\"Error\",\"id\":null,\"message\":"+quote(e.what())+"}";
   }
   if(jax){
    try{maps=mapState(serializer,*jax,graph.runtime);}catch(const std::exception&e){throw std::runtime_error("case "+std::string(c.name)+": "+e.what());}
    auto po=jax->parseOptions();auto*p=std::get_if<tex::ParseOptionsRef>(&po);
    if(!p||!*p)throw std::runtime_error("missing ParseOptions after compile");
    const auto*data=std::get_if<std::shared_ptr<SourceStringMap<opts::Value>>>(&(*p)->packageData);
    if(!data||!*data)throw std::runtime_error("missing packageData map");
    packageData="{\"$\":\"map\",\"entries\":[";auto keys=(*data)->keys();bool first=true;
    while(auto entry=keys.next()){
     if(!first)packageData+=',';first=false;auto val=(*data)->get(*entry);
     if(!val)throw std::runtime_error("packageData key disappeared");
     packageData+="["+quote(utf8(*entry))+","+serializer.value(*val)+"]";
    }
    packageData+="]}";
   }
   output+="{\"result\":"+result+",\"error\":"+error+",\"dynamicMaps\":"+maps+",\"packageData\":"+packageData+"}";
  }
  output+="}}\n";const std::filesystem::path path=argc>1?argv[1]:"build/math-port/ams-newcommand-native.json";
  std::filesystem::create_directories(path.parent_path());std::ofstream file(path,std::ios::binary);
  if(!file)throw std::runtime_error("cannot open AMS candidate output");file<<output;
  if(!file)throw std::runtime_error("failed writing AMS candidate output");return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
