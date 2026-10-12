module folia.math.core.math_item;
import folia.math.core.math_item_host;
import folia.math.util.property_host;
import std;
namespace folia::math::core {
AbstractMathItem::AbstractMathItem(Property mathValue,MathInputJaxReference jax,Property mode,
                                   MathLocation begin,MathLocation finish){
    math=std::move(mathValue);
    inputJax=std::move(jax);
    display=mode.missing()?Property(true):std::move(mode);
    const auto defaultLocation=[](){
        return MathLocation(PropertyList{{"i",0},{"n",0},{"delim",""}});
    };
    if(auto* value=std::get_if<Property>(&begin);value&&value->missing())start=defaultLocation();
    else start=std::move(begin);
    if(auto* value=std::get_if<Property>(&finish);value&&value->missing())end=defaultLocation();
    else end=std::move(finish);
    root=PropertyNull{};
    typesetRoot=PropertyNull{};
    metrics=PropertyList{};
    inputData=PropertyList{};
    outputData=PropertyList{};
    state_=math_item_host::constant("UNPROCESSED");
}
Property protoItem(Property open,Property math,Property close,Property n,
                   Property begin,Property finish,Property mode){
    if(mode.missing())mode=PropertyNull{};
    const Property start=PropertyList{{"n",std::move(begin)}};
    const Property end=PropertyList{{"n",std::move(finish)}};
    return PropertyList{{"open",std::move(open)},{"math",std::move(math)},
        {"close",std::move(close)},{"n",std::move(n)},{"start",start},
        {"end",end},{"display",std::move(mode)}};
}
bool AbstractMathItem::isEscaped()const{return std::holds_alternative<PropertyNull>(display.value);}
void AbstractMathItem::setMetrics(Property em,Property ex,Property cwidth,Property scale){
    metrics=PropertyList{{"em",std::move(em)},{"ex",std::move(ex)},
                         {"containerWidth",std::move(cwidth)},{"scale",std::move(scale)}};
}
Property AbstractMathItem::state(Property target,Property restore){
    if(restore.missing())restore=false;
    if(!property_host::nullish(target)){
        const Property insertedInput=math_item_host::constant("INSERTED");
        if(math_item_host::less(target,insertedInput)){
            const Property currentState=state_;
            const Property insertedCurrent=math_item_host::constant("INSERTED");
            if(math_item_host::greaterEqual(currentState,insertedCurrent))
                this->removeFromDocument(restore);
        }

        const Property typesetInput=math_item_host::constant("TYPESET");
        if(math_item_host::less(target,typesetInput)){
            const Property currentState=state_;
            const Property typesetCurrent=math_item_host::constant("TYPESET");
            if(math_item_host::greaterEqual(currentState,typesetCurrent))outputData=PropertyList{};
        }

        const Property compiledInput=math_item_host::constant("COMPILED");
        if(math_item_host::less(target,compiledInput)){
            const Property currentState=state_;
            const Property compiledCurrent=math_item_host::constant("COMPILED");
            if(math_item_host::greaterEqual(currentState,compiledCurrent))inputData=PropertyList{};
        }
        state_=target;
    }
    return state_;
}
void AbstractMathItem::reset(Property restore){
    if(restore.missing())restore=false;
    this->state(math_item_host::constant("UNPROCESSED"),std::move(restore));
}
void newState(std::string_view name,Property state){
    if(math_item_host::hasState(name))math_item_host::duplicateState(name);
    STATE.set(name,std::move(state));
}
}
