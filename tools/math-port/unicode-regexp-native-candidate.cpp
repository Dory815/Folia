import folia.math.util.regexp_host;
import folia.math.util.option_types;
import std;

namespace {
using namespace folia::math::regexp_host;
using folia::math::options::Exception;
namespace rh=folia::math::regexp_host;
std::string q(std::u16string_view value){
    constexpr char hex[]="0123456789abcdef";
    std::string out="\"";
    for(char16_t c:value){
        if(c==u'"')out+="\\\"";
        else if(c==u'\\')out+="\\\\";
        else if(c==u'\n')out+="\\n";
        else if(c==u'\r')out+="\\r";
        else if(c==u'\t')out+="\\t";
        else if(c<0x20||c>=0x80){
            out+="\\u";for(int s=12;s>=0;s-=4)out+=hex[(c>>s)&15];
        }else out.push_back(static_cast<char>(c));
    }
    out+='"';return out;
}
std::string optional(const std::optional<std::u16string>& value){return value?q(*value):"{\"$\":\"undefined\"}";}
std::string matchValue(const std::optional<Match>& match){
    if(!match)return "null";
    std::string out="{\"captures\":[";
    for(std::size_t i=0;i<match->captures.size();++i){if(i)out+=',';out+=optional(match->captures[i]);}
    out+= "],\"index\":"+std::to_string(match->index)+",\"input\":"+q(match->input)+",\"groups\":";
    if(!match->groups)out+="{\"$\":\"undefined\"}";
    else {out+="{";bool first=true;for(const auto& [name,value]:*match->groups){if(!first)out+=',';first=false;out+=q(name)+":"+optional(value);}out+="}";}
    return out+"}";
}
std::string execute(std::u16string pattern,std::u16string flags,std::u16string input,double lastIndex=0,int count=1){
    RegExp expression(std::move(pattern),std::move(flags));expression.lastIndex=lastIndex;
    std::string out="{\"source\":"+q(expression.source())+",\"flags\":"+q(expression.flags())+",\"calls\":[";
    for(int i=0;i<count;++i){if(i)out+=',';const auto result=expression.exec(input);out+="{\"match\":"+matchValue(result)+",\"lastIndex\":"+std::to_string(expression.lastIndex)+"}";}
    return out+"]}";
}
std::string splitValue(std::u16string input,std::u16string pattern,std::u16string flags){
    const auto output=rh::split(input,std::move(pattern),std::move(flags));
    std::string out="[";for(std::size_t i=0;i<output.size();++i){if(i)out+=',';out+=optional(output[i]);}return out+"]";
}
std::string plain(const std::optional<std::u16string>& value){return optional(value);}
std::u16string cjkPattern(){
    std::u16string result=u"[";
    auto unit=[&](std::uint32_t cp){if(cp<=0xFFFF)result.push_back(static_cast<char16_t>(cp));else{cp-=0x10000;result.push_back(static_cast<char16_t>(0xD800+(cp>>10)));result.push_back(static_cast<char16_t>(0xDC00+(cp&0x3FF)));}};
    auto range=[&](std::uint32_t first,std::uint32_t last){unit(first);result.push_back(u'-');unit(last);};
    range(0x1100,0x115F);unit(0x2329);unit(0x232A);range(0x2E80,0x303E);range(0x3040,0x3247);range(0x3250,0x4DBF);range(0x4E00,0xA4C6);range(0xA960,0xA97C);range(0xAC00,0xD7A3);range(0xF900,0xFAFF);range(0xFE10,0xFE19);range(0xFE30,0xFE6B);range(0xFF01,0xFF60);range(0xFFE0,0xFFE6);range(0x1B000,0x1B001);range(0x1F200,0x1F251);range(0x20000,0x3FFFD);
    result.push_back(u']');return result;
}
std::u16string excludedNameClass(){std::u16string s=u"[^";s.push_back(0);s+=u"-";s.push_back(0x1F);s+=u" \"'>/=";s.push_back(0x7F);s+=u"-";s.push_back(0x9F);s.push_back(u']');return s;}
std::u16string exactTagPattern(){
    const std::u16string space=u"[ \\n]+",opt=u"[ \\n]*",nameClass=excludedNameClass();
    const auto tagName=std::u16string(u"[A-Za-z]")+nameClass+u"*";
    const auto attribute=nameClass+u"+(?:"+opt+u"="+opt+u"(?:'[^']*'|\"[^\"]*\"|"+space+u"))?";
    return u"(<(?:"+tagName+u"(?:"+space+attribute+u")*"+opt+u"/?|/"+tagName+u"|!--[^]*?--|![^]*?)(?:>|$))";
}
std::u16string exactAttributePattern(){const std::u16string space=u"[ \\n]+",opt=u"[ \\n]*";const auto nameClass=excludedNameClass();return nameClass+u"+(?:"+opt+u"="+opt+u"(?:'[^']*'|\"[^\"]*\"|"+space+u"))?";}
std::u16string exactAttributeSplitPattern(){const std::u16string space=u"[ \\n]+",opt=u"[ \\n]*";const auto nameClass=excludedNameClass();return u"("+nameClass+u"+)(?:"+opt+u"="+opt+u"(?:'([^']*)'|\"([^\"]*)\"|("+space+u")))?";}
void observe(std::string_view name,const std::function<std::string()>& body,std::vector<std::string>& records){
    try{records.push_back("{\"name\":\""+std::string(name)+"\",\"value\":"+body()+"}");}
    catch(const Exception& e){records.push_back("{\"name\":\""+std::string(name)+"\",\"error\":{\"name\":\""+e.name+"\",\"message\":"+q(e.message)+"}}");}
    catch(const std::exception& e){records.push_back("{\"name\":\""+std::string(name)+"\",\"error\":{\"name\":\"Error\",\"message\":\"native exception\"}}");}
}
std::string splitPattern(std::u16string text,std::u16string pattern,std::u16string flags){return splitValue(std::move(text),std::move(pattern),std::move(flags));}
std::string regexpSourceValue(const std::u16string& pattern,const std::u16string& flags){RegExp expression(pattern,flags);return q(expression.source());}
}
int main(){
    std::vector<std::string> rows;
    observe("dot-codepoint",[]{return execute(u".",u"gu",u"A\U0001F600B",0,4);},rows);
    observe("dot-legacy-units",[]{return execute(u".",u"g",u"A\U0001F600B",0,5);},rows);
    observe("sticky-inside-pair",[]{return execute(u".",u"uy",u"\U0001F600x",1,2);},rows);
    observe("global-inside-pair",[]{return execute(u".",u"gu",u"\U0001F600x",1,2);},rows);
    observe("nonglobal-lastindex",[]{return execute(u".",u"u",u"\U0001F600",1,2);},rows);
    observe("capture-offsets",[]{return execute(u"(\U0001F600)(.)",u"gu",u"x\U0001F600y",0,2);},rows);
    observe("named-capture",[]{return execute(u"(?<face>\U0001F600)(?<next>.)",u"u",u"\U0001F600x");},rows);
    observe("lone-surrogates",[]{return execute(u".",u"gu",std::u16string{char16_t(0xD800),u'A',char16_t(0xDC00)},0,4);},rows);
    observe("surrogate-class",[]{return execute(u"[\\uD800-\\uDFFF]",u"gu",
        std::u16string{char16_t(0xD83D),char16_t(0xDE00),char16_t(0xD800),char16_t(0xDC00),char16_t(0xD800)},0,2);},rows);
    observe("escaped-surrogate-pair",[]{return execute(u"\\uD83D\\uDE00",u"u",u"\U0001F600");},rows);
    observe("escaped-codepoint",[]{return execute(u"\\u{1F600}",u"u",u"x\U0001F600");},rows);
    observe("astral-range",[]{return execute(u"[\\u{1F600}-\\u{1F64F}]+",u"u",u"a\U0001F600\U0001F64Fx");},rows);
    observe("quantified-codepoint",[]{return execute(u".{2}",u"u",u"\U0001F600x");},rows);
    observe("negated-class",[]{return execute(u"[^x]+",u"u",u"\U0001F600x");},rows);
    observe("lookahead-capture-offset",[]{return execute(u"(?=(.))",u"gu",u"\U0001F600x",1);},rows);
    observe("empty-exec-keeps-index",[]{return execute(u"(?:)",u"gu",u"\U0001F600x",1,2);},rows);
    observe("split-empty-advances-codepoint",[]{return std::string("{\"unicode\":")+splitPattern(u"\U0001F600x",u"(?:)",u"u")+",\"legacy\":"+splitPattern(u"\U0001F600x",u"(?:)",u"")+"}";},rows);
    observe("split-captures",[]{return std::string("{\"unicode\":")+splitPattern(u"A\U0001F600B",u"(.)",u"u")+"}";},rows);
    observe("unicode-ignore-case",[]{return execute(u"[a-z]+",u"iu",u"\u212A\u017Fx");},rows);
    observe("unicode-word",[]{return execute(u"\\w+",u"iu",u"\u212A\u017Fx");},rows);
    observe("unicode-word-boundary",[]{return execute(u"\\b\\u212A\\b",u"iu",u" \u212A ");},rows);
    observe("unicode-backreference",[]{return execute(u"(.)\\1",u"iu",u"\u017FS");},rows);
    observe("dot-line-separators",[]{return execute(u".",u"gu",u"\r\n\u2028\u2029x",0,2);},rows);
    observe("lastindex-out-of-range",[]{return execute(u".",u"uy",u"\U0001F600",3);},rows);
    observe("lastindex-negative",[]{return execute(u".",u"gu",u"\U0001F600",-1);},rows);
    observe("lastindex-fraction",[]{return execute(u".",u"gu",u"\U0001F600x",1.9);},rows);
    const std::array<std::pair<std::string,std::u16string>,7> invalid{{
        {"identity-escape",u"\\q"},{"class-identity-escape",u"[\\-a]"},{"decimal-without-capture",u"\\1"},
        {"octal",u"\\01"},{"out-of-range-codepoint",u"\\u{110000}"},{"reversed-range",u"[z-a]"},{"incomplete-escape",u"\\u{"}}};
    for(const auto& [name,pattern]:invalid)observe("syntax-"+name,[&pattern]{return execute(pattern,u"u",u"q");},rows);
    const auto cjk=cjkPattern();
    observe("actual-cjk-class",[=]{return execute(cjk,u"gu",
        std::u16string{u'A',char16_t(0xD840),char16_t(0xDC00),char16_t(0xD8BF),char16_t(0xDFFD),char16_t(0xD8BF),char16_t(0xDFFE),char16_t(0xD800)},0,3);},rows);
    const auto tagPattern=exactTagPattern();
    observe("actual-parser-tag-split",[=]{return std::string("{\"source\":")+regexpSourceValue(tagPattern,u"u")+",\"flags\":\"u\",\"parts\":"+splitValue(u"A<mi data-\U0001F600=\"v\">\U0001F600</mi><!--x--><broken",tagPattern,u"u")+"}";},rows);
    const auto attrPattern=exactAttributePattern();
    observe("actual-parser-attr-match",[=]{return execute(std::u16string(attrPattern),u"u",u"data-\U0001F600=\"v\" id=two");},rows);
    const auto attrSplit=exactAttributeSplitPattern();
    observe("actual-parser-attrsplit",[=]{return std::string("{\"source\":")+regexpSourceValue(attrSplit,u"u")+",\"flags\":\"u\",\"parts\":"+splitValue(u"id=\"x\" data-\U0001F600=\"value\"",attrSplit,u"u")+"}";},rows);
    std::cout<<"{\"records\":[";for(std::size_t i=0;i<rows.size();++i){if(i)std::cout<<',';std::cout<<rows[i];}std::cout<<"]}\n";
}
