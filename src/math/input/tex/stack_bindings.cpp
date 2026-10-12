module folia.math.input.tex.stack;
import std;
namespace folia::math::tex {
Stack::Stack(StackFactoryReference factory,Property environment,Property inner){
    initializeStack(std::move(factory),std::move(environment),std::move(inner));
}
}
