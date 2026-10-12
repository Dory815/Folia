module folia.math.core.math_item;
import folia.math.core.math_item_host;
import folia.math.util.property_host;
import std;
namespace folia::math::core {
void AbstractMathItem::render(MathDocumentReference document){
    auto actions=math_item_host::actions(document);
    math_item_host::require<MathRenderActionsService>(actions,"renderMath")
        .renderMath(*this,document,Missing{});
}
void AbstractMathItem::rerender(MathDocumentReference document,Property start){
    if(start.missing())start=math_item_host::constant("RERENDER");
    const Property current=this->state();
    if(math_item_host::greaterEqual(current,start))
        this->state(Property(property_host::number(start)-1),Missing{});
    auto actions=math_item_host::actions(document);
    math_item_host::require<MathRenderActionsService>(actions,"renderMath")
        .renderMath(*this,document,start);
}
void AbstractMathItem::convert(MathDocumentReference document,Property end){
    if(end.missing())end=math_item_host::constant("LAST");
    auto actions=math_item_host::actions(document);
    math_item_host::require<MathRenderActionsService>(actions,"renderConvert")
        .renderConvert(*this,document,end);
}
void AbstractMathItem::compile(MathDocumentReference document){
    const Property current=this->state();
    const Property threshold=math_item_host::constant("COMPILED");
    if(math_item_host::less(current,threshold)){
        auto& input=math_item_host::require<MathInputJaxService>(inputJax,"compile");
        root=input.compile(*this,document);
        const Property completed=math_item_host::constant("COMPILED");
        this->state(completed,Missing{});
    }
}
void AbstractMathItem::typeset(MathDocumentReference document){
    const Property current=this->state();
    const Property threshold=math_item_host::constant("TYPESET");
    if(math_item_host::less(current,threshold)){
        const MathOutputJaxReference outputReceiver=math_item_host::output(document);
        const bool escaped=this->isEscaped();
        auto& output=math_item_host::require<MathOutputJaxService>(
            outputReceiver,escaped?"escaped":"typeset");
        typesetRoot=escaped?output.escaped(*this,document):output.typeset(*this,document);
        const Property completed=math_item_host::constant("TYPESET");
        this->state(completed,Missing{});
    }
}
void AbstractMathItem::updateDocument(MathDocumentReference){}
void AbstractMathItem::removeFromDocument(Property){this->clear();}
void AbstractMathItem::clear(){}
}
