// Candidate differential dump for the BaseMethods source corpus.
// This is intentionally separate from the shared test target; Root may add it
// to the comparison gate after reviewing its real parser/runtime path.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.ams.configuration;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.boldsymbol.configuration;
import folia.math.input.tex.configmacros.configuration;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_node;
import folia.math.input.tex.find_tex_host;
import folia.math.input.tex.tex_error;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
namespace tex=folia::math::tex;
namespace opts=folia::math::options;

std::string quote(std::string_view value){
    std::string out="\"";constexpr char hex[]="0123456789abcdef";
    for(const unsigned char c:value){
        switch(c){case '\\':out+="\\\\";break;case '"':out+="\\\"";break;
        case '\b':out+="\\b";break;case '\f':out+="\\f";break;case '\n':out+="\\n";break;
        case '\r':out+="\\r";break;case '\t':out+="\\t";break;
        default:if(c<0x20){out+="\\u00";out.push_back(hex[c>>4]);out.push_back(hex[c&15]);}else out.push_back(static_cast<char>(c));}
    }
    return out+'"';
}
std::string number(double value){
    if(std::isnan(value))return "{\"$\":\"NaN\"}";
    if(std::isinf(value))return value<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}";
    if(value==0&&std::signbit(value))return "{\"$\":\"-0\"}";
    std::ostringstream out;out<<std::setprecision(17)<<value;return out.str();
}
struct Serializer{
    opts::Context& context;OptionFunctionBindings& functions;
    std::string property(const Property& value){
        if(std::holds_alternative<Missing>(value.value))return "{\"$\":\"undefined\"}";
        if(std::holds_alternative<PropertyNull>(value.value))return "null";
        if(const auto* p=std::get_if<bool>(&value.value))return *p?"true":"false";
        if(const auto* p=std::get_if<double>(&value.value))return number(*p);
        if(const auto* p=std::get_if<std::string>(&value.value))return quote(*p);
        if(const auto* p=std::get_if<FunctionRef>(&value.value)){
            if(!*p)throw std::runtime_error("empty FunctionRef in MML");
            const auto identity=functions.find(*p);if(!identity)throw std::runtime_error("unbound FunctionRef in MML");
            const auto name=context.get(*identity,opts::Key(u"name"));
            const auto length=context.get(*identity,opts::Key(u"length"));
            const auto* n=std::get_if<std::u16string>(&name.data);const auto* l=std::get_if<double>(&length.data);
            if(!n||!l)throw std::runtime_error("FunctionRef metadata missing");
            return "{\"$\":\"function\",\"name\":"+quote(property_host::debugText(*n))+",\"length\":"+number(*l)+"}";
        }
        if(const auto* p=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);p&&*p){
            if((*p)->array){auto a=PropertyArray::array(value);std::string out="[";
                for(std::size_t i=0;i<a->size();++i){if(i)out+=',';out+=a->has(i)?property(a->get(i)):"{\"$\":\"hole\"}";}return out+"]";}
            auto record=PropertyList::record(value);if(!record)throw std::runtime_error("bad Property record");
            std::string out="{";bool first=true;for(const auto& key:record->keys()){if(!first)out+=',';first=false;out+=quote(key)+":"+property(record->getOwn(key));}return out+"}";
        }
        throw std::runtime_error("unsupported Property value in MML dump");
    }
    std::string node(mml::NodeValue value){
        if(!value.isNode())return "null";
        auto* n=value.requireNode();const auto attrs=n->attributes();const auto children=n->childNodes;
        std::string out="{\"kind\":"+quote(n->kind());
        if(const auto* text=dynamic_cast<mml::TextNode*>(n))out+=",\"text\":"+quote(property_host::debugText(text->getText()));
        else out+=",\"text\":null";
        const auto record=[&](const PropertyList& fields){std::string s="{";bool first=true;for(const auto& key:fields.keys()){if(!first)s+=',';first=false;s+=quote(key)+":"+property(fields.getOwn(key));}return s+"}";};
        out+=",\"attrs\":"+record(attrs?attrs->getAllAttributes():PropertyList{});
        out+=",\"properties\":"+record(n->getAllProperties())+",\"children\":[";
        if(children)for(std::size_t i=0;i<children->size();++i){if(i)out+=',';out+=node(children->get(i));}
        return out+"]}";
    }
};

struct RuntimeGraph{
    opts::Context context;OptionFunctionBindings functions{context};regexp_host::RegExpBindings regex{context};
    Unicode16Normalization unicode;core::InputJaxModule input{context};core::FindMathModule findMath{context};
    tex::FindTeXModule findTeX{context,findMath,regex};tex::ParserRuntime runtime{context,functions,regex,unicode,input,findTeX};
    std::shared_ptr<mml::MmlFactory> factory=std::make_shared<mml::MmlFactory>();
    RuntimeGraph(){
        runtime.initializeParserRuntime();
        tex::BasePackage::initializeModule(runtime);
        tex::BoldsymbolConfiguration::initializeModule(runtime);
        tex::NewcommandConfiguration::initializeModule(runtime);
        tex::AmsConfiguration::initializeModule(runtime);
        tex::ConfigMacrosConfiguration::initializeModule(runtime);
    }
    tex::TeXRef makeJax(std::span<const std::u16string_view> packages){
        const auto options=context.object();const auto packageArray=context.array(static_cast<std::uint32_t>(packages.size()));
        std::uint32_t index=0;for(const auto package:packages){const auto key=std::to_string(index++);context.set(packageArray,opts::Key(std::u16string(key.begin(),key.end())),opts::Value(std::u16string(package)));}
        context.set(options,opts::Key(u"packages"),opts::Value(packageArray));context.set(options,opts::Key(u"tags"),opts::Value(u"base"));
        auto jax=tex::TeX::make(runtime,opts::Value(options));jax->setMmlFactory(factory);return jax;
    }
};
struct Case{std::u16string_view name,tex;std::vector<std::u16string_view> packages;};
const Case cases[]={
    {u"bar-tilde-vec",u"\\bar{x}+\\tilde{y}+\\vec{z}+\\overline{w}",{u"base"}},
    {u"font-variants",u"\\mathbb{R}+\\mathbf{x}+\\mathsf{A}+\\mathtt{b}",{u"base"}},
    {u"style-switches",u"\\displaystyle{x+\\scriptstyle y}+\\textstyle z",{u"base"}},
    {u"spacing-family",u"\\hspace{1em}x\\kern2mu y\\quad z\\enspace w",{u"base"}},
    {u"break-controls",u"a\\allowbreak b\\nobreak c\\\\d",{u"base"}},
    {u"fraction-variants",u"a\\atop b+\\binom{n}{k}+\\genfrac{[}{]}{0pt}{}{n}{k}",{u"base"}},
    {u"root-variants",u"\\sqrt[3]{x}+\\sqrt[\\uproot{2}\\leftroot{1}4]{x}",{u"base"}},
    {u"over-under-family",u"\\stackrel{!}{=}+\\overset{a}{b}+\\underset{c}{d}",{u"base"}},
    {u"operator-classes",u"\\mathbin{+}+\\mathrel{=}+\\mathop{f}",{u"base"}},
    {u"boxes",u"\\boxed{x}+\\fbox{y}+\\framebox{z}+\\makebox[2em][c]{q}",{u"base"}},
    {u"smash-phantom-lap",u"\\strut x+\\phantom{y}+\\smash[t]{z}+\\llap{w}",{u"base"}},
    {u"big-delimiter-sizes",u"\\bigl(a\\bigr)+\\Bigl[b\\Bigr]+\\biggl\\{c\\biggr\\}",{u"base"}},
    {u"not-and-dots",u"x\\not\\in y+\\dots+\\ldots+\\cdots",{u"base"}},
    {u"array-matrix",u"\\begin{matrix}a&b\\\\c&d\\end{matrix}+\\begin{array}{|r|c|}x&y\\\\1&2\\end{array}",{u"base"}},
    {u"array-row-controls",u"\\begin{array}{c}\\hline a\\\\\\hfill b\\end{array}",{u"base"}},
    {u"aligned-break-align",u"\\begin{aligned}a&=b\\breakAlign{tc}\\\\c&=d\\end{aligned}",{u"base"}},
    {u"new-column-type",u"\\newcolumntype{C}{c}\\begin{array}{C}x\\end{array}",{u"base"}},
    {u"mml-token",u"\\mmlToken{mi}{abc}+\\mmlToken{mo}[mathvariant=normal]{+}",{u"base"}},
    {u"horizontal-size",u"\\hsize=3em x+\\vbox{a\\\\b}",{u"base"}},
    {u"paragraph-box",u"\\parbox{2em}{ab cd}",{u"base"}},
    {u"macro-definition",u"\\newcommand{\\foo}[1]{#1+#1}\\foo{x}",{u"base"}},
    {u"labels-and-reference",u"\\label{eq:a}x+\\ref{eq:a}+\\eqref{eq:a}",{u"base"}},
    {u"equation-environment",u"\\begin{equation}x=1\\label{eq:x}\\end{equation}",{u"base"}},
    {u"cases-and-array-close",u"\\begin{cases}x&x>0\\\\-x&x<0\\end{cases}",{u"base"}},
    {u"mathchoice",u"\\mathchoice{D}{T}{S}{SS}",{u"base"}},
    {u"primitive-over-with-delimiters",u"{a\\overwithdelims()b}+{a\\above2pt b}",{u"base"}},
    {u"font-command-set",u"\\mathrm{d}x+\\mathcal{F}+\\mathfrak{g}+\\boldsymbol{\\alpha}",{u"base"}},
    {u"move-and-raise",u"\\moveleft2mu{x}+\\moveright3mu{y}+\\raise1ex{z}+\\lower1ex{q}",{u"base"}},
    {u"ref-with-missing-label",u"\\ref{missing}",{u"base"}},
    {u"malformed-array",u"\\begin{array}{cc}x",{u"base"}},
    {u"undefined-command",u"\\notARealCommand",{u"base"}},
    {u"ams-matrix",u"\\begin{matrix}a&b\\\\c&d\\end{matrix}",{u"base",u"ams"}},
    {u"ams-aligned-break-align",u"\\begin{aligned}a&=b\\\\c&=d\\end{aligned}",{u"base",u"ams"}},
    {u"ams-cases",u"\\begin{cases}x&x>0\\\\-x&x<0\\end{cases}",{u"base",u"ams"}},
    {u"ams-binom-and-boldsymbol",u"\\binom{n}{k}+\\boldsymbol{\\alpha}",{u"base",u"ams",u"boldsymbol"}},
    {u"ams-eqref",u"\\eqref{missing}",{u"base",u"ams"}},
    {u"configmacros-newcommand",u"\\newcommand{\\foo}[1]{#1+#1}\\foo{x}",{u"base",u"newcommand",u"configmacros"}},
};
}

int main(int argc,char** argv){
    try{
        RuntimeGraph graph;Serializer serializer{graph.context,graph.functions};std::vector<std::pair<std::string,std::string>> rows;
        for(const auto& item:cases){
            std::string result="{}";
            try{
                auto jax=graph.makeJax(item.packages);auto options=jax->parseOptions();auto* parseOptions=std::get_if<tex::ParseOptionsRef>(&options);
                if(!parseOptions||!*parseOptions)throw std::runtime_error("TeX did not create real ParseOptions");
                const auto parser=tex::TexParser::make(std::u16string(item.tex),PropertyList{},**parseOptions);
                result="{\"result\":{\"tree\":"+serializer.node(parser->mml())+",\"parser\":{\"i\":"+number(parser->i)+
                    ",\"string\":"+quote(property_host::debugText(parser->string()))+",\"macroCount\":"+number(parser->macroCount)+"}}}";
            }
            catch(const tex::TexError& error){result="{\"error\":{\"name\":\"TexError\",\"id\":"+quote(property_host::debugText(error.id))+
                ",\"message\":"+quote(property_host::debugText(error.message))+"}}";}
            catch(const mml::Error& error){result="{\"error\":{\"name\":"+quote(error.name)+",\"id\":null,\"message\":"+
                quote(property_host::debugText(error.message))+"}}";}
            catch(const std::exception& error){result="{\"error\":{\"name\":\"Error\",\"id\":null,\"message\":"+quote(error.what())+"}}";}
            std::string packageJson="[";std::size_t packageIndex=0;for(const auto package:item.packages){if(packageIndex++)packageJson+=',';packageJson+=quote(property_host::debugText(package));}packageJson+=']';
            rows.emplace_back(property_host::debugText(item.name),"{\"packages\":"+packageJson+","+result.substr(1));
        }
        std::filesystem::path output=argc>1?argv[1]:"build/math-port/base-methods-native.json";
        std::filesystem::create_directories(output.parent_path());std::ofstream out(output,std::ios::binary);
        if(!out)throw std::runtime_error("cannot open BaseMethods native candidate output");
        const auto methodNames=property_host::ownKeys(graph.runtime.baseMethods);
        out<<"{\"methodNames\":[";for(std::size_t i=0;i<methodNames.size();++i){if(i)out<<',';out<<quote(methodNames[i]);}
        out<<"],\"cases\":{";for(std::size_t i=0;i<rows.size();++i){if(i)out<<',';out<<quote(rows[i].first)<<':'<<rows[i].second;}out<<"}}\n";
        if(!out)throw std::runtime_error("failed writing BaseMethods candidate");return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
