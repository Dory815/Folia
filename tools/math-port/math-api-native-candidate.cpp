// Exercises the public math module without accessing its parser or Context.
import std;
import folia.math;

namespace {
std::string quote(std::string_view text) {
    std::string out="\"";constexpr char hex[]="0123456789abcdef";
    for(const unsigned char c:text) {
        if(c=='"')out+="\\\"";else if(c=='\\')out+="\\\\";
        else if(c<32){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}
        else out+=static_cast<char>(c);
    }
    return out+'"';
}
std::string decode(std::string_view value) {
    constexpr std::string_view digits="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;unsigned bits=0,buffer=0;
    for(const char c:value) {
        if(c=='=')break;
        const auto digit=digits.find(c);
        if(digit==std::string_view::npos)throw std::invalid_argument("Invalid case input");
        buffer=(buffer<<6)|static_cast<unsigned>(digit);bits+=6;
        if(bits>=8){bits-=8;out.push_back(static_cast<char>((buffer>>bits)&255));}
    }
    return out;
}
std::size_t count(const folia::math::output::scene::FrozenNodeRef& node) {
    if(!node)return 0;
    std::size_t result=1;
    for(const auto& child:node->children)result+=count(child);
    return result;
}
}
int main() {
    using namespace folia::math;
    std::vector<RenderResult> results;
    std::vector<std::string> records;
    try {
        {
            Renderer renderer;
            for(std::string row;std::getline(std::cin,row);) {
                std::istringstream input(row);
                std::string name,tex;int display=0;double em=0,width=0;
                if(!(input>>name>>tex>>display>>em>>width))throw std::invalid_argument("Incomplete case input");
                std::string svg="null",error="null";
                try {
                    auto result=renderer.render(decode(tex),RenderOptions{display!=0,em,width});
                    svg=quote(result.svg);results.push_back(std::move(result));
                } catch(const RenderError& caught) {
                    error="{\"name\":"+quote(caught.name)+",\"message\":"+quote(caught.what())+"}";
                }
                records.push_back("{\"name\":"+quote(decode(name))+",\"svg\":"+svg+",\"error\":"+error+"}");
            }
        }
        // Access every completed immutable tree after all session owners end.
        std::size_t sceneNodes=0;
        for(const auto& result:results)for(const auto& root:result.scene.roots())sceneNodes+=count(root);
        std::cout<<"{\"schema\":1,\"records\":[";
        for(std::size_t i=0;i<records.size();++i){if(i)std::cout<<',';std::cout<<records[i];}
        std::cout<<"],\"detachedScenes\":"<<results.size()<<",\"detachedSceneNodes\":"<<sceneNodes<<"}\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';return 1;
    }
}
