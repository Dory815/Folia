import folia.math;
import std;

namespace {
using namespace folia::math;
void dump(const Renderer& renderer,std::string_view phase){
    std::cout<<phase;
    for(const auto& [id,kind,name]:RendererDiagnosticsProbe::liteNodeIdentityNamedSnapshot(renderer))
        std::cout<<' '<<id<<':'<<kind<<':'<<name;
    std::cout<<'\n';
}
}
int main(){
    try{
        Renderer renderer;
        (void)renderer.render("x");dump(renderer,"warm");
        (void)renderer.render(R"(\frac{)");dump(renderer,"after-malformed");
        (void)renderer.render(R"(\frac{)");dump(renderer,"after-second-malformed");
        (void)renderer.render("x");dump(renderer,"after-plain-x");
        return 0;
    }catch(const RenderError& error){
        std::cerr<<"diagnostic RenderError "<<error.name<<": "<<error.what()<<'\n';return 1;
    }catch(const std::exception& error){
        std::cerr<<"diagnostic exception: "<<error.what()<<'\n';return 1;
    }catch(...){
        std::cerr<<"diagnostic exception: non-standard\n";return 1;
    }
}
