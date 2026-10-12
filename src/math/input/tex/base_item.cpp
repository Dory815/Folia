module folia.math.input.tex.stack_item;
import folia.math.core.mml_tree.mml_node_host;
import std;
namespace folia::math::tex {
std::string BaseItem::kind()const{throw tree::FactoryError("Unimplemented",u"BaseItem.kind");}
Property BaseItem::env()const{throw tree::FactoryError("Unimplemented",u"BaseItem.env");}
void BaseItem::setEnv(Property){throw tree::FactoryError("Unimplemented",u"BaseItem.setEnv");}
bool BaseItem::copyEnv()const{throw tree::FactoryError("Unimplemented",u"BaseItem.copyEnv");}
StackProperty BaseItem::getProperty(std::string_view)const{throw tree::FactoryError("Unimplemented",u"BaseItem.getProperty");}
BaseItem* BaseItem::setProperty(std::string_view,StackProperty){throw tree::FactoryError("Unimplemented",u"BaseItem.setProperty");}
BaseItem* BaseItem::setProperties(StackProperty){throw tree::FactoryError("Unimplemented",u"BaseItem.setProperties");}
bool BaseItem::isOpen()const{throw tree::FactoryError("Unimplemented",u"BaseItem.isOpen");}
bool BaseItem::isClose()const{throw tree::FactoryError("Unimplemented",u"BaseItem.isClose");}
bool BaseItem::isFinal()const{throw tree::FactoryError("Unimplemented",u"BaseItem.isFinal");}
bool BaseItem::isKind(std::string_view)const{throw tree::FactoryError("Unimplemented",u"BaseItem.isKind");}
CheckType BaseItem::checkItem(BaseItem*){throw tree::FactoryError("Unimplemented",u"BaseItem.checkItem");}
void BaseItem::clearEnv(){throw tree::FactoryError("Unimplemented",u"BaseItem.clearEnv");}
StackProperty BaseItem::getName()const{throw tree::FactoryError("Unimplemented",u"BaseItem.getName");}
std::string BaseItem::toString()const{throw tree::FactoryError("Unimplemented",u"BaseItem.toString");}
Property BaseItem::getErrors(std::string_view)const{throw tree::FactoryError("Unimplemented",u"BaseItem.getErrors");}
void BaseItem::addLatexItem(mml::NodeValue,Property){throw tree::FactoryError("Unimplemented",u"BaseItem.addLatexItem");}
}
