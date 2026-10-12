module folia.math.input.tex.stack;
import folia.math.input.tex.stack_host;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
void Stack::initializeStack(StackFactoryReference,Property,Property){property_host::typeError("Unimplemented Stack constructor");}
void Stack::setEnv(Property){property_host::typeError("Unimplemented Stack env setter");}
Property Stack::env()const{property_host::typeError("Unimplemented Stack env getter");}
void Stack::Push(std::span<const StackEntry>){property_host::typeError("Unimplemented Stack Push");}
StackEntry Stack::Pop(){property_host::typeError("Unimplemented Stack Pop");}
StackEntry Stack::Top(Property)const{property_host::typeError("Unimplemented Stack Top");}
mml::NodeValue Stack::Prev(Property){property_host::typeError("Unimplemented Stack Prev");}
double Stack::height()const{property_host::typeError("Unimplemented Stack height");}
std::string Stack::toString()const{property_host::typeError("Unimplemented Stack toString");}
}
