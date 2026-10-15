// Public API candidate for a strict SVG comparison against the pinned CJS witness.
import std;
import folia.math;

namespace {
std::string quote(std::string_view text) {
    std::string out="\"";constexpr char hex[]="0123456789abcdef";
    for(const unsigned char c:text) {
        if(c=='\"')out+="\\\"";else if(c=='\\')out+="\\\\";
        else if(c<32){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}
        else out+=static_cast<char>(c);
    }
    return out+'\"';
}
bool mmlError(std::string_view svg) {
    return svg.find("data-mjx-error=")!=std::string_view::npos ||
           svg.find("data-mml-node=\"merror\"")!=std::string_view::npos;
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
    constexpr std::array<std::pair<std::string_view,std::string_view>,3> cases{{
        {"unary-proof",R"(\begin{prooftree}\AxiomC{A}\UnaryInfC{B}\end{prooftree})"},
        {"ordinary-after-proof","x+1"},
        {"binary-proof-after-formula",R"(\begin{prooftree}\AxiomC{P}\AxiomC{Q}\BinaryInfC{C}\end{prooftree})"}
    }};
    std::vector<std::string> records;
    try {
        Renderer renderer;
        for(const auto& [name,tex]:cases) {
            std::string svg="null",error="null";
            std::size_t sceneNodes=0;
            bool hasError=false;
            try {
                auto result=renderer.render(tex,RenderOptions{false,16,1280});
                svg=quote(result.svg);
                hasError=mmlError(result.svg);
                for(const auto& root:result.scene.roots())sceneNodes+=count(root);
            } catch(const RenderError& caught) {
                error="{\"name\":"+quote(caught.name)+",\"message\":"+quote(caught.what())+"}";
            }
            records.push_back("{\"name\":"+quote(name)+",\"tex\":"+quote(tex)+
                ",\"svg\":"+svg+",\"mmlError\":"+(hasError?"true":"false")+
                ",\"error\":"+error+",\"sceneNodes\":"+std::to_string(sceneNodes)+"}");
        }
        std::cout<<"{\"schema\":1,\"profile\":\"mathjax-full\",\"records\":[";
        for(std::size_t i=0;i<records.size();++i){if(i)std::cout<<',';std::cout<<records[i];}
        std::cout<<"]}\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';return 1;
    }
}
