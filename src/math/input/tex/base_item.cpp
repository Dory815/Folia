module folia.math.input.tex.stack_item;
import folia.math.input.tex.base_item_host;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
std::string BaseItem::kind()const{return "base";}
Property BaseItem::env()const{return env_;}
void BaseItem::setEnv(Property value){env_=std::move(value);}
bool BaseItem::copyEnv()const{return true;}
StackProperty BaseItem::getProperty(std::string_view key)const{
    return base_item_host::properties(properties_).get(key);
}
BaseItem* BaseItem::setProperty(std::string_view key,StackProperty value){
    base_item_host::properties(properties_).set(key,std::move(value));
    return this;
}
BaseItem* BaseItem::setProperties(StackProperty values){
    auto target=base_item_host::properties(properties_);
    stack_property_host::assign(target,values);
    return this;
}
bool BaseItem::isOpen()const{return false;}
bool BaseItem::isClose()const{return false;}
bool BaseItem::isFinal()const{return false;}
bool BaseItem::isKind(std::string_view expected)const{
    const std::string current=this->kind();
    return current==expected;
}
CheckType BaseItem::checkItem(BaseItem* rawItem){
    BaseItem& item=base_item_host::item(rawItem,"isKind");
    const bool isOver=item.isKind("over");
    if(isOver&&this->isOpen()){
        const mml::NodeValue number=this->toMml(false);
        item.setProperty("num",StackProperty(number));
        this->Clear();
    }

    const bool isCell=item.isKind("cell");
    if(isCell&&this->isOpen()){
        const StackProperty linebreak=item.getProperty("linebreak");
        if(stack_property_host::truthy(linebreak))return fail;
        const StackProperty name=item.getName();
        base_item_host::texError(Property("Misplaced"),Property("Misplaced %1"),name);
    }

    const bool closing=item.isClose();
    if(closing){
        const std::string closingKind=item.kind();
        const Property initialErrors=this->getErrors(closingKind);
        if(property_host::truthy(initialErrors)){
            const std::string errorKind=item.kind();
            const Property errors=this->getErrors(errorKind);
            const Property id=base_item_host::errorField(errors,0);
            const Property message=base_item_host::errorField(errors,1);
            const StackProperty name=item.getName();
            base_item_host::texError(id,message,name);
        }
    }

    const bool final=item.isFinal();
    if(!final)return success;
    const mml::NodeValue first=item.First();
    this->Push(std::span<const mml::NodeValue>(&first,1));
    return fail;
}
void BaseItem::clearEnv(){
    const Property environment=this->env();
    const auto keys=property_host::ownKeys(environment);
    for(const auto& key:keys){
        const Property current=this->env();
        base_item_host::deleteProperty(current,key);
    }
}
StackProperty BaseItem::getName()const{return this->getProperty("name");}
std::string BaseItem::toString()const{
    const std::string currentKind=this->kind();
    const mml::Children currentNodes=this->nodes();
    const std::string contents=base_item_host::join(currentNodes,"; ");
    return currentKind+"["+contents+"]";
}
Property BaseItem::getErrors(std::string_view key)const{
    const auto currentClass=this->classInfo();
    const Property currentErrors=currentClass->errorsBinding();
    const Property value=base_item_host::get(currentErrors,key);
    if(property_host::truthy(value))return value;
    const Property base=BaseItem::baseErrors();
    return base_item_host::get(base,key);
}
void BaseItem::addLatexItem(mml::NodeValue node,Property prefix){
    if(prefix.missing())prefix="";
    const std::u16string source=base_item_host::slice(startStr,startI,stopI);
    if(source.empty())return;
    const std::u16string tex=property_host::truthy(prefix)?property_host::string(prefix)+source:source;

    const auto firstAttributes=base_item_host::attributes(node);
    const Property firstAttr=base_item_host::get(TexConstant,"Attr");
    const Property firstName=base_item_host::get(firstAttr,"LATEXITEM");
    firstAttributes->set(property_host::debugText(property_host::string(firstName)),
                         Property(property_host::debugText(tex)));

    if(tex!=u"}"){
        const auto secondAttributes=base_item_host::attributes(node);
        const Property secondAttr=base_item_host::get(TexConstant,"Attr");
        const Property secondName=base_item_host::get(secondAttr,"LATEX");
        secondAttributes->set(property_host::debugText(property_host::string(secondName)),
                              Property(property_host::debugText(tex)));
    }
}
}
