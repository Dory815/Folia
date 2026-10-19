// Native port of MathJax 4.1.3 input/tex/bbox/BboxConfiguration.ts.
module folia.math.input.tex.bbox.configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.token_map;
import folia.math.util.js_number;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
TexParser& parser(ParserContext& value){auto* p=dynamic_cast<TexParser*>(&value);if(!p)throw Exception("TypeError",u"BBox requires a TexParser");return *p;}
std::u16string text(const Property& value){return property_host::string(value);}
std::u16string trim(std::u16string value){
    const auto ws=[](char16_t c){return c==u' '||c==u'\t'||c==u'\n'||c==u'\r'||c==u'\v'||c==u'\f'||c==0x00A0||c==0x1680||(c>=0x2000&&c<=0x200A)||c==0x2028||c==0x2029||c==0x202F||c==0x205F||c==0x3000||c==0xFEFF;};
    std::size_t a=0,b=value.size();while(a<b&&ws(value[a]))++a;while(b>a&&ws(value[b-1]))--b;return value.substr(a,b-a);
}
bool asciiPattern(const std::u16string& value,std::string_view expression){
    const std::string utf8=property_host::debugText(value);
    return std::regex_match(utf8,std::regex(std::string(expression),std::regex::ECMAScript|std::regex::icase));
}
std::optional<std::vector<std::string>> regexSearch(const std::u16string& value,std::string_view expression){
    const std::string utf8=property_host::debugText(value);std::smatch match;
    if(!std::regex_match(utf8,match,std::regex(std::string(expression),std::regex::ECMAScript)))return {};
    std::vector<std::string> result;result.reserve(match.size());for(const auto& part:match)result.push_back(part.str());return result;
}
bool stylePrefix(const std::u16string& part){
    const auto colon=part.find(u':');if(colon==std::u16string::npos||colon==0)return false;
    return std::all_of(part.begin(),part.begin()+static_cast<std::ptrdiff_t>(colon),[](char16_t c){return(c>=u'a'&&c<=u'z')||(c>=u'A'&&c<=u'Z')||c==u'-';});
}
FunctionRef bboxMethod(){
    return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parser(context);const std::u16string name=text(std::get<Property>(atom));
        const std::u16string options=text(p.GetBrackets(name,Property("")));
        mml::NodeValue math=p.ParseArg(name);
        std::optional<PropertyList> padding;
        std::optional<std::u16string> background,style;
        std::size_t start=0;
        while(true){
            const auto comma=options.find(u',',start);
            const std::u16string part=trim(options.substr(start,comma==std::u16string::npos?options.size()-start:comma-start));
            const auto dimension=regexSearch(part,R"(^((?:\.\d+|\d+(?:\.\d*)?))(pt|em|ex|mu|px|in|cm|mm)$)");
            if(dimension){
                if(padding){const std::array<Property,2> rest{Property("Padding"),Property(property_host::debugText(name))};throw TexError(u"MultipleBBoxProperty",u"%1 specified twice in %2",rest);}
                const std::string numeric=(*dimension)[1],unit=(*dimension)[2];
                const double amount=std::strtod(numeric.c_str(),nullptr);const auto amountText=property_host::text(js_number::toString(amount*2));
                const std::u16string pad=property_host::text(numeric+unit);
                padding=PropertyList{{"height",property_host::debugText(u"+"+pad)},
                    {"depth",property_host::debugText(u"+"+pad)},{"lspace",property_host::debugText(pad)},
                    {"width",property_host::debugText(u"+"+amountText+property_host::text(unit))}};
            }else if(asciiPattern(part,R"(^([a-z0-9]+|#[0-9a-f]{6}|#[0-9a-f]{3})$)")){
                if(background){const std::array<Property,2> rest{Property("Background"),Property(property_host::debugText(name))};throw TexError(u"MultipleBBoxProperty",u"%1 specified twice in %2",rest);}
                background=part;
            }else if(stylePrefix(part)){
                if(style){const std::array<Property,2> rest{Property("Style"),Property(property_host::debugText(name))};throw TexError(u"MultipleBBoxProperty",u"%1 specified twice in %2",rest);}
                style=part;
            }else if(!part.empty()){
                const std::array<Property,1> rest{Property(property_host::debugText(part))};throw TexError(u"InvalidBBoxProperty",u"\"%1\" doesn't look like a color, a padding dimension, or a style",rest);
            }
            if(comma==std::u16string::npos)break;start=comma+1;
        }
        if(padding){
            auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(math);
            const std::array<std::any,3> nodeArgs{std::any(Property("mpadded")),std::any(children),std::any(Property(*padding))};math=p.create(u"node",nodeArgs);
        }
        if(background||style){PropertyList attrs;if(background)attrs.setOwn("mathbackground",property_host::debugText(*background));if(style)attrs.setOwn("style",property_host::debugText(*style));
            auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(math);
            const std::array<std::any,3> nodeArgs{std::any(Property("mstyle")),std::any(children),std::any(Property(attrs))};math=p.create(u"node",nodeArgs);
        }
        p.Push(math);return Property(Missing{});
    });
}
Value object(Context& cx,std::initializer_list<std::pair<std::u16string_view,Value>> fields){const auto out=cx.object();for(const auto&[name,value]:fields)cx.set(out,key(name),value);return Value(out);}
Value array(Context& cx,std::initializer_list<Value> fields){const auto out=cx.array(fields.size());std::size_t i=0;for(const auto& value:fields){const auto digits=std::to_string(i++);cx.set(out,key(std::u16string(digits.begin(),digits.end())),value);}return Value(out);}
}
ConfigurationRef BboxConfiguration::initializeModule(ParserRuntime& runtime){
    const auto loaded=runtime.configurationHandler.get(u"bbox");
    if(const auto* old=std::get_if<ConfigurationRef>(&loaded);old&&*old)return *old;
    const FunctionRef method=bboxMethod();
    const auto bound=BaseMethods::bind(runtime,method,u"BBox");
    const auto* identity=std::get_if<ObjectRef>(&bound.data);
    if(!identity)throw Exception("TypeError",u"Bbox method has no Context identity");
    runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(2.0),false,false,true));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("bbox"),Property(PropertyList{{"bbox",Property(method)}}));
    Context& cx=runtime.context();const Value handler=object(cx,{{u"macro",array(cx,{Value(u"bbox")})}});
    return Configuration::create(runtime,u"bbox",object(cx,{{u"handler",handler}}));
}
}
