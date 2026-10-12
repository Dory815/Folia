module folia.math.core.math_item;
import folia.math.core.math_item_host;
import folia.math.util.property_host;
import std;
namespace folia::math::core {
void AbstractMathItem::render(MathDocumentReference){property_host::typeError("Unimplemented render");}
void AbstractMathItem::rerender(MathDocumentReference,Property){property_host::typeError("Unimplemented rerender");}
void AbstractMathItem::convert(MathDocumentReference,Property){property_host::typeError("Unimplemented convert");}
void AbstractMathItem::compile(MathDocumentReference){property_host::typeError("Unimplemented compile");}
void AbstractMathItem::typeset(MathDocumentReference){property_host::typeError("Unimplemented typeset");}
void AbstractMathItem::updateDocument(MathDocumentReference){property_host::typeError("Unimplemented updateDocument");}
void AbstractMathItem::removeFromDocument(Property){property_host::typeError("Unimplemented removeFromDocument");}
void AbstractMathItem::clear(){property_host::typeError("Unimplemented clear");}
}
