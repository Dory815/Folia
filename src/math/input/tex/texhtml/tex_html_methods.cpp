/*************************************************************
 * Native port of MathJax 4.1.3 input/tex/texhtml/TexHtmlConfiguration.ts.
 * The only HTML parser is the active document's Lite DOM adaptor.
 *************************************************************/
module folia.math.input.tex.texhtml.tex_html_methods;
import folia.math.input.tex.tex_parser_context;
import folia.math.input.tex.tex_error;
import folia.math.input.tex.texhtml.tex_html_services;
import folia.math.core.mml_tree.mml_nodes.html_node;
import folia.math.util.utf16;
import folia.math.util.property_host;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
TexParser& parser(ParserContext& source){
    auto* value=dynamic_cast<TexParser*>(&source);
    if(!value)property_host::typeError("TexHtml requires a TexParser");
    return *value;
}
std::u16string macroName(const ParseAtom& atom){
    if(const auto* value=std::get_if<Property>(&atom))return property_host::string(*value);
    if(const auto* value=std::get_if<TokenRef>(&atom);value&&*value)
        return property_host::string((*value)->character());
    property_host::typeError("TexHtml macro name is not a string");
}
ParseOptions& parseOptions(TexParser& parser){
    const auto* value=std::get_if<ParseOptions*>(&parser.configuration);
    if(!value||!*value)property_host::typeError("Cannot read properties of null or undefined (reading 'packageData')");
    return **value;
}
bool jsSpace(char16_t c){
    return c==u' '||c==u'\t'||c==u'\n'||c==u'\v'||c==u'\f'||c==u'\r'||
        c==0x00a0||c==0x1680||(c>=0x2000&&c<=0x200a)||c==0x2028||c==0x2029||
        c==0x202f||c==0x205f||c==0x3000||c==0xfeff;
}
std::u16string trimHtml(std::u16string_view text){
    std::size_t first=0,last=text.size();
    while(first<last&&jsSpace(text[first]))++first;
    while(last>first&&jsSpace(text[last-1]))--last;
    return std::u16string(text.substr(first,last-first));
}
struct StartTag {std::u16string text;std::u16string number;std::size_t length=0;};
std::optional<StartTag> startTag(std::u16string_view input){
    constexpr std::u16string_view prefix=u"tex-html";
    if(!input.starts_with(prefix))return {};
    std::size_t at=prefix.size();std::u16string number;
    if(at<input.size()&&input[at]==u' '){
        constexpr std::u16string_view attribute=u" n=\"";
        if(input.substr(at,attribute.size())!=attribute)return {};
        at+=attribute.size();const auto begin=at;
        while(at<input.size()&&input[at]>=u'0'&&input[at]<=u'9')++at;
        if(at==begin||at>=input.size()||input[at]!=u'\"')return {};
        number=std::u16string(input.substr(begin,at-begin));++at;
    }
    if(at>=input.size()||input[at]!=u'>')return {};
    ++at;
    return StartTag{std::u16string(input.substr(0,at)),std::move(number),at};
}
TexHtmlServices services(ParseOptions& options,const TexHtmlAdaptorResolver& resolver){
    const auto* package=std::get_if<std::shared_ptr<SourceStringMap<Value>>>(&options.packageData);
    if(!package||!*package)property_host::typeError("Cannot read properties of undefined (reading 'get')");
    const Value value=(*package)->get(u"texhtml").value_or(Value(Undefined{}));
    Context& context=options.runtime().context();
    // Source reads packageData.texhtml.adaptor on every macro invocation.
    const Value adaptor=context.get(value,key(u"adaptor"));
    return resolveTexHtmlServices(context,adaptor,resolver);
}
DOMNodeRef requireDomNode(ContextPropertyBindings& properties,const Value& value){
    const auto projected=propertyNode(properties.from(value));
    if(const auto* node=std::get_if<DOMNodeRef>(&projected))return *node;
    property_host::typeError("HTML parser returned a non-node child");
}
ParseResult texHtml(TexParser& parser,const std::u16string&,const TexHtmlAdaptorResolver& resolver){
    auto& options=parseOptions(parser);
    auto& context=options.runtime().context();
    if(!options::truthy(context.get(parser.options(),key(u"allowTexHTML"))))
        return Property(false);
    const auto source=parser.string();
    const auto offset=static_cast<std::size_t>(std::max(0.0,parser.i));
    const auto tag=offset<=source.size()?startTag(std::u16string_view(source).substr(offset)):std::nullopt;
    if(!tag)return Property(false);

    std::u16string end;
    if(!tag->number.empty())end=u"<!"+tag->number+u">";
    end+=u"</tex-html>";
    const auto bodyStart=offset+tag->length;
    parser.i=static_cast<double>(bodyStart);
    const auto found=source.find(end,bodyStart);
    if(found==std::u16string::npos){
        const std::array<Property,2> arguments{Property(utf16::encode(end)),Property(utf16::encode(u"<"+tag->text))};
        throw TexError(u"TokenNotFoundForCommand",u"Could not find %1 for %2",arguments);
    }
    const auto htmlSource=trimHtml(std::u16string_view(source).substr(bodyStart,found-bodyStart));
    parser.i=static_cast<double>(found+11+(tag->number.empty()?0:3+tag->number.size()));
    auto service=services(options,resolver);
    const Value adaptor(service.identity);
    const std::array<Value,1> parseArgs{Value(htmlSource)};
    const Value parsed=context.callMember(service.parseMethod,adaptor,parseArgs,u"adaptor.parse");
    const std::array<Value,1> bodyArgs{parsed};
    const Value body=context.callMember(service.bodyMethod,adaptor,bodyArgs,u"adaptor.body");
    const std::array<Value,1> childArgs{body};
    const Value nodes=context.callMember(service.childNodesMethod,adaptor,childArgs,u"adaptor.childNodes");
    const Value firstLength=context.get(nodes,key(u"length"));
    if(const auto* number=std::get_if<double>(&firstLength.data);number&&*number==0)return Property(true);
    Value domIdentity;
    const Value secondLength=context.get(nodes,key(u"length"));
    if(const auto* number=std::get_if<double>(&secondLength.data);number&&*number==1)
        domIdentity=context.get(nodes,key(u"0"));
    else{
        const Value definition=context.object();
        const std::array<Value,3> nodeArgs{Value(u"div"),definition,nodes};
        const Value nodeMethod=context.get(adaptor,key(u"node"));
        domIdentity=context.callMember(nodeMethod,adaptor,nodeArgs,u"adaptor.node");
    }
    const DOMNodeRef dom=requireDomNode(options.runtime().properties,domIdentity);
    const auto xml=service.xml->fromDOM(dom);
    std::array<std::any,1> createHtml{std::any(Property("html"))};
    const auto htmlNode=parser.create(u"node",FunctionArgs(createHtml));
    auto* html=dynamic_cast<mml::HtmlNode*>(htmlNode.requireNode());
    if(!html)property_host::typeError("NodeFactory did not create an HtmlNode");
    html->setHTML(xml,service.xml.get());
    auto mtextChildren=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    mtextChildren->push_back(htmlNode);
    std::array<std::any,2> createMtext{std::any(Property("mtext")),std::any(mtextChildren)};
    parser.Push(parser.create(u"node",FunctionArgs(createMtext)));
    return Property(true);
}
ParseCallableRef method(TexHtmlAdaptorResolver resolver){
    return makeParseCallable([resolver=std::move(resolver)](const FunctionReceiver&,ParserContext& source,
                                const ParseAtom& name,ParseRest)->ParseResult{
        return texHtml(parser(source),macroName(name),resolver);
    });
}
}

Property TexHtmlMethods::initializeModule(ParserRuntime& runtime,TexHtmlAdaptorResolver resolver){
    if(!runtime.texHtmlMethods.missing())return runtime.texHtmlMethods;
    runtime.texHtmlMethods=Property(PropertyList{{
        "TexHTML",Property(method(std::move(resolver)))}});
    return runtime.texHtmlMethods;
}
}
