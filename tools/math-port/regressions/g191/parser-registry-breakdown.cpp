import folia.math;
import std;

int main(){
    using namespace folia::math;
    constexpr std::array<std::string_view,24> names={
        "parseSymbols","subHandlers","configurationIdentities","parserConfigurationIdentities",
        "parseOptionsIdentities","nodeFactoryIdentities","parserIdentities","tagIdentities",
        "tagClassIdentities","texIdentities","keyValueIdentities","labelIdentities",
        "tagInfoIdentities","nodeArrayIdentities","globalDefinitionScopeIdentities","itemClassIdentities",
        "stackItemFactoryIdentities","stackItemIdentities","stackPropertyIdentities","mmlNodeIdentities",
        "stackIdentities","mmlFactoryIdentities","columnStateIdentities","columnParserIdentities"};
    auto print=[&](const Renderer& renderer,std::string_view phase){
        const auto values=RendererDiagnosticsProbe::parserIdentitySnapshot(renderer);
        std::cout<<phase;
        for(std::size_t i=0;i<values.size();++i)std::cout<<' '<<names[i]<<'='<<values[i];
        std::cout<<'\n';
    };
    Renderer renderer;
    print(renderer,"constructed");
    (void)renderer.render("x");print(renderer,"warm");
    (void)renderer.render("x");print(renderer,"after-x-1");
    (void)renderer.render("x");print(renderer,"after-x-2");
    for(std::size_t i=0;i<64;++i)(void)renderer.render("x");
    print(renderer,"after-x-66");
}
