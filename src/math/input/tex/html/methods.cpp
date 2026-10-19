// Native port of MathJax 4.1.3 input/tex/html/HtmlMethods.ts.
module folia.math.input.tex.html.methods;
import folia.math.input.tex.node_util;
import folia.math.input.tex.parser;
import folia.math.input.tex.tex_error;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
TexParser& parser(ParserContext& source){auto* p=dynamic_cast<TexParser*>(&source);if(!p)property_host::typeError("HTML method requires a TexParser");return *p;}
std::u16string nameOf(const ParseAtom& atom){if(const auto* p=std::get_if<Property>(&atom))return property_host::string(*p);if(const auto* t=std::get_if<TokenRef>(&atom);t&&*t)return property_host::string((*t)->character());property_host::typeError("HTML method name is not a string");}
ParseOptions& parseOptions(TexParser& p){const auto* value=std::get_if<ParseOptions*>(&p.configuration);if(!value||!*value)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");return **value;}
mml::NodeValue argument(TexParser& p,const std::u16string& name){
    auto value=p.ParseArg(name);auto& util=NodeUtil::instance();if(!util.isInferred(value))return value;
    const std::array<std::any,1> createArgs{std::any(Property("mrow"))};const auto row=p.create(u"node",FunctionArgs(createArgs));
    util.copyChildren(value,row);util.copyAttributes(value,row);return row;
}
bool legalName(std::u16string_view name){
    for(std::size_t i=0;i<name.size();++i){const auto c=name[i];
        if(c<=0x1f||(c>=0x7f&&c<=0x9f)||c==u' '||c==u'"'||c==u'\''||c==u'>'||c==u'/'||c==u'=')return false;
        if(c>=0xd800&&c<=0xdbff){if(i+1<name.size()&&name[i+1]>=0xdc00&&name[i+1]<=0xdfff){const auto cp=0x10000+((c-0xd800)<<10)+(name[++i]-0xdc00);if((cp&0xffff)>=0xfffe)return false;}}
        else if(c>=0xfdd0&&c<=0xfdef)return false;
        else if(c==0xfffe||c==0xffff)return false;
    }return true;
}
FunctionRef make(std::function<ParseResult(TexParser&,const std::u16string&)> body){return makeParseCallable([body=std::move(body)](const FunctionReceiver&,ParserContext& source,const ParseAtom& atom,ParseRest)->ParseResult{auto& p=parser(source);return body(p,nameOf(atom));});}
}
Property HtmlMethods::initializeModule(ParserRuntime& runtime){
    if(!runtime.htmlMethods.missing())return runtime.htmlMethods;
    auto data=make([&runtime](TexParser& p,const std::u16string& name)->ParseResult{
        const auto dataset=property_host::string(p.GetArgument(name));const auto node=argument(p,name);
        const Property entries=ParseUtil::keyvalOptions(runtime,dataset);
        if(const auto* list=std::get_if<std::shared_ptr<PropertyStorage>>(&entries.value);list&&*list){const auto values=PropertyList::record(entries);if(!values)property_host::typeError("keyval options did not produce a record");for(const auto& key:values->keys()){
            std::u16string attribute=u"data-";attribute+=property_host::text(key);
            if(!legalName(attribute)){const std::array<Property,1> rest{Property(property_host::debugText(attribute))};throw TexError(u"InvalidHTMLAttr",u"Invalid HTML attribute: %1",rest);}
            NodeUtil::instance().setAttribute(node,property_host::debugText(attribute),values->getOwn(key));
        }}
        p.Push(node);return Property(Missing{});
    });
    auto href=make([](TexParser& p,const std::u16string& name)->ParseResult{const auto url=property_host::string(p.GetArgument(name));const auto node=argument(p,name);NodeUtil::instance().setAttribute(node,"href",property_host::debugText(url));p.Push(node);return Property(Missing{});});
    auto className=make([](TexParser& p,const std::u16string& name)->ParseResult{auto value=property_host::string(p.GetArgument(name));const auto node=argument(p,name);const auto prior=NodeUtil::instance().getAttribute(node,"class");if(property_host::truthy(prior))value=property_host::string(prior)+u" "+value;NodeUtil::instance().setAttribute(node,"class",property_host::debugText(value));p.Push(node);return Property(Missing{});});
    auto style=make([](TexParser& p,const std::u16string& name)->ParseResult{auto value=property_host::string(p.GetArgument(name));const auto node=argument(p,name);const auto prior=NodeUtil::instance().getAttribute(node,"style");if(property_host::truthy(prior)){auto old=property_host::string(prior);const auto index=value.empty()?std::u16string::npos:value.size()-1;if(index>=old.size()||old[index]!=u';')old.push_back(u';');value=old+u" "+value;}NodeUtil::instance().setAttribute(node,"style",property_host::debugText(value));p.Push(node);return Property(Missing{});});
    auto id=make([](TexParser& p,const std::u16string& name)->ParseResult{const auto value=property_host::string(p.GetArgument(name));const auto node=argument(p,name);NodeUtil::instance().setAttribute(node,"id",property_host::debugText(value));p.Push(node);return Property(Missing{});});
    runtime.htmlMethods=Property(PropertyList{{"data",Property(data)},{"href",Property(href)},
        {"class",Property(className)},{"style",Property(style)},{"cssId",Property(id)}});
    return runtime.htmlMethods;
}
}
