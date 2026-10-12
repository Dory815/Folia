module folia.math.input.tex.stack_item;
import folia.math.input.tex.stack_item_host;
import std;
namespace folia::math::tex {
void MmlStack::Push(std::span<const mml::NodeValue>){throw tree::FactoryError("Unimplemented",u"MmlStack.Push");}
mml::NodeValue MmlStack::Pop(){throw tree::FactoryError("Unimplemented",u"MmlStack.Pop");}
mml::NodeValue MmlStack::First()const{throw tree::FactoryError("Unimplemented",u"MmlStack.First");}
void MmlStack::setFirst(mml::NodeValue){throw tree::FactoryError("Unimplemented",u"MmlStack.setFirst");}
mml::NodeValue MmlStack::Last()const{throw tree::FactoryError("Unimplemented",u"MmlStack.Last");}
void MmlStack::setLast(mml::NodeValue){throw tree::FactoryError("Unimplemented",u"MmlStack.setLast");}
mml::Children MmlStack::Peek(Property)const{throw tree::FactoryError("Unimplemented",u"MmlStack.Peek");}
double MmlStack::Size()const{throw tree::FactoryError("Unimplemented",u"MmlStack.Size");}
void MmlStack::Clear(){throw tree::FactoryError("Unimplemented",u"MmlStack.Clear");}
mml::NodeValue MmlStack::toMml(Property,Property){throw tree::FactoryError("Unimplemented",u"MmlStack.toMml");}
mml::NodeValue MmlStack::create(std::string_view,FunctionArgs){throw tree::FactoryError("Unimplemented",u"MmlStack.create");}
}
