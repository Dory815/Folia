// Full SVG observation through the public Renderer API. Input rows come from
// the locked JSON corpus via run-profile75-native.cjs; formulas are not copied
// into this translation unit.
import std;
import folia.math;

namespace {
std::string quote(std::string_view text) {
    std::string out="\"";constexpr char hex[]="0123456789abcdef";
    for(std::size_t i=0;i<text.size();) {
        const auto c=static_cast<unsigned char>(text[i]);
        if(c=='\"'){out+="\\\"";++i;}
        else if(c=='\\'){out+="\\\\";++i;}
        else if(c=='\n'){out+="\\n";++i;}
        else if(c=='\r'){out+="\\r";++i;}
        else if(c=='\t'){out+="\\t";++i;}
        else if(c<0x20){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];++i;}
        else{out+=static_cast<char>(c);++i;}
    }
    return out+'"';
}
std::string decode64(std::string_view text) {
    constexpr std::string_view digits="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;unsigned bits=0,buffer=0;
    for(const char c:text) {
        if(c=='=')break;
        const auto digit=digits.find(c);
        if(digit==std::string_view::npos)throw std::invalid_argument("Invalid base64 case input");
        buffer=(buffer<<6)|static_cast<unsigned>(digit);bits+=6;
        if(bits>=8){bits-=8;out.push_back(static_cast<char>((buffer>>bits)&255));}
    }
    return out;
}
std::vector<std::string_view> split(std::string_view row) {
    std::vector<std::string_view> result;std::size_t start=0;
    while(true){const auto end=row.find('\t',start);result.push_back(row.substr(start,
        end==std::string_view::npos?row.size()-start:end-start));
        if(end==std::string_view::npos)break;start=end+1;}
    return result;
}
std::size_t sceneCount(const folia::math::output::scene::FrozenNodeRef& node) {
    if(!node)return 0;
    std::size_t result=1;
    for(const auto& child:node->children)result+=sceneCount(child);
    return result;
}
std::string utf8(std::u16string_view text) {
    std::string out;
    for(std::size_t i=0;i<text.size();++i) {
        std::uint32_t c=text[i];
        if(c>=0xD800&&c<=0xDBFF&&i+1<text.size()&&text[i+1]>=0xDC00&&text[i+1]<=0xDFFF)
            c=0x10000+((c-0xD800)<<10)+(text[++i]-0xDC00);
        if(c<=0x7F)out.push_back(static_cast<char>(c));
        else if(c<=0x7FF){out.push_back(static_cast<char>(0xC0|(c>>6)));out.push_back(static_cast<char>(0x80|(c&0x3F)));}
        else if(c<=0xFFFF){out.push_back(static_cast<char>(0xE0|(c>>12)));out.push_back(static_cast<char>(0x80|((c>>6)&0x3F)));out.push_back(static_cast<char>(0x80|(c&0x3F)));}
        else{out.push_back(static_cast<char>(0xF0|(c>>18)));out.push_back(static_cast<char>(0x80|((c>>12)&0x3F)));out.push_back(static_cast<char>(0x80|((c>>6)&0x3F)));out.push_back(static_cast<char>(0x80|(c&0x3F)));}
    }
    return out;
}
void appendEvents(const folia::math::output::scene::FrozenNodeRef& node,
                  std::vector<std::string>& path,std::size_t index,std::vector<std::string>& output) {
    if(!node)return;
    std::string mml;
    for(const auto& [key,value]:node->attributes)if(key=="data-mml-node")mml=value;
    const auto segment="{\"index\":"+std::to_string(index)+",\"kind\":"+quote(utf8(node->kind))+
        ",\"dataMmlNode\":"+(mml.empty()?"null":quote(mml))+"}";
    path.push_back(segment);
    if(!node->eventTypes.empty()) {
        std::string segments="[";
        for(std::size_t i=0;i<path.size();++i){if(i)segments+=',';segments+=path[i];}
        segments+=']';
        std::string events="[";
        for(std::size_t i=0;i<node->eventTypes.size();++i){if(i)events+=',';events+=quote(utf8(node->eventTypes[i]));}
        events+=']';
        output.push_back("{\"path\":"+segments+",\"kind\":"+quote(utf8(node->kind))+
            ",\"dataMmlNode\":"+(mml.empty()?"null":quote(mml))+",\"eventTypes\":"+events+"}");
    }
    for(std::size_t i=0;i<node->children.size();++i)appendEvents(node->children[i],path,i,output);
    path.pop_back();
}
std::string sceneEvents(const folia::math::output::scene::Scene& scene) {
    const auto& roots=scene.roots();
    // produceScene() adds each output root directly to Scene. The selected
    // Maction inputs must therefore have exactly one top-level SVG root; do
    // not search and silently strip arbitrary scene prefixes.
    if(roots.size()!=1||!roots.front()||roots.front()->kind!=u"svg")
        throw std::runtime_error("Expected exactly one top-level SVG scene root for Maction path comparison");
    std::vector<std::string> found;std::vector<std::string> path;
    appendEvents(roots.front(),path,0,found);
    std::string out="[";for(std::size_t i=0;i<found.size();++i){if(i)out+=',';out+=found[i];}return out+"]";
}
bool mmlError(std::string_view svg) {
    return svg.find("data-mjx-error=")!=std::string_view::npos||
           svg.find("data-mml-node=\"merror\"")!=std::string_view::npos;
}
struct Row {std::string group,name,tex;std::size_t index=0;bool display=false;double em=16,width=1280;};
Row parseCase(std::string_view line) {
    const auto fields=split(line);if(fields.size()!=6)throw std::runtime_error("Expected six case fields");
    Row row;row.index=static_cast<std::size_t>(std::stoull(std::string(fields[0])));
    row.name=decode64(fields[1]);row.tex=decode64(fields[2]);row.display=fields[3]=="1";
    row.em=std::stod(std::string(fields[4]));row.width=std::stod(std::string(fields[5]));
    return row;
}
std::string errorJson(std::string_view name,std::string_view message) {
    return "{\"name\":"+quote(name)+",\"message\":"+quote(message)+"}";
}
std::string renderRecord(const Row& row,folia::math::Renderer& renderer) {
    std::string svg="null",error="null",events="[]";bool hasMmlError=false;std::size_t nodes=0;
    try {
        const auto result=renderer.render(row.tex,folia::math::RenderOptions{row.display,row.em,row.width});
        svg=quote(result.svg);hasMmlError=mmlError(result.svg);
        if(row.name=="action-toggle"||row.name=="action-tooltip")
            events=sceneEvents(result.scene);
        for(const auto& root:result.scene.roots())nodes+=sceneCount(root);
    } catch(const folia::math::RenderError& caught) {
        error=errorJson(caught.name,caught.what());
    } catch(const std::exception& caught) {
        error=errorJson("Error",caught.what());
    } catch(...) {
        error=errorJson("Error","Unknown native exception");
    }
    return "{\"group\":"+quote(row.group)+",\"index\":"+std::to_string(row.index)+
        ",\"name\":"+quote(row.name)+",\"tex\":"+quote(row.tex)+
        ",\"display\":"+(row.display?"true":"false")+",\"em\":"+std::to_string(row.em)+
        ",\"ex\":"+std::to_string(row.em*.5)+
        ",\"width\":"+std::to_string(row.width)+",\"svg\":"+svg+
        ",\"mmlError\":"+(hasMmlError?"true":"false")+",\"error\":"+error+
        ",\"sceneNodes\":"+std::to_string(nodes)+",\"eventTargets\":"+events+"}";
}
}

int main() {
    using namespace folia::math;
    std::vector<std::string> records;
    std::unique_ptr<Renderer> continuous;
    std::string activeGroup;
    try {
        for(std::string line;std::getline(std::cin,line);) {
            if(line.empty())continue;
            if(line.starts_with("@group\t")) {
                activeGroup=decode64(std::string_view(line).substr(7));
                if(activeGroup=="continuous")continuous=std::make_unique<Renderer>();
                else if(activeGroup=="fresh-session-per-case")continuous.reset();
                else throw std::runtime_error("Unknown render group: "+activeGroup);
                continue;
            }
            if(activeGroup.empty())throw std::runtime_error("Missing @group header");
            Row row=parseCase(line);row.group=activeGroup;
            if(activeGroup=="continuous")records.push_back(renderRecord(row,*continuous));
            else {Renderer fresh;records.push_back(renderRecord(row,fresh));}
        }
        std::cout<<"{\"schema\":1,\"renderer\":\"folia.math::Renderer\",\"groups\":[\"continuous\",\"fresh-session-per-case\"],\"records\":[";
        for(std::size_t i=0;i<records.size();++i){if(i)std::cout<<',';std::cout<<records[i];}
        std::cout<<"]}\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';return 1;
    }
}
