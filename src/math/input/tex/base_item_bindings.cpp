// Root-owned allocation phases and captured constructor/static identities.
module folia.math.input.tex.stack_item;
import folia.math.core.mml_tree.mml_node_host;
import std;
namespace folia::math::tex {
std::shared_ptr<CheckTuple> BaseItem::fail=std::make_shared<CheckTuple>(CheckTuple{PropertyNull{},false});
std::shared_ptr<CheckTuple> BaseItem::success=std::make_shared<CheckTuple>(CheckTuple{PropertyNull{},true});
Property BaseItem::errors=PropertyList{
    {"end",PropertyArray{"MissingBeginExtraEnd","Missing \\begin{%1} or extra \\end{%1}"}},
    {"close",PropertyArray{"ExtraCloseMissingOpen","Extra close brace or missing open brace"}},
    {"right",PropertyArray{"MissingLeftExtraRight","Missing \\left or extra \\right"}},
    {"middle",PropertyArray{"ExtraMiddle","Extra \\middle"}}
};
void BaseItem::initializeBaseItem(StackFactoryReference factory,mml::Children nodes,
                                 tree::FactoryClassRef<BaseItem> constructor){
    constructor_=std::move(constructor);
    initializeMmlStack(std::move(nodes));
    factory_=factory;
    global=PropertyList{};
    properties_=StackPropertyList{};
    if(isOpen())env_=PropertyList{};
}
std::shared_ptr<StackItemClassInfo> BaseItem::classInfo()const{
    if(!constructor_)mml::host::typeError("Native stack constructor is not bound");
    auto* info=std::any_cast<std::shared_ptr<StackItemClassInfo>>(&constructor_->staticData);
    if(!info||!*info)mml::host::typeError("Native stack constructor metadata is not bound");
    return *info;
}
Property BaseItem::baseErrors(){return errors;}
}
