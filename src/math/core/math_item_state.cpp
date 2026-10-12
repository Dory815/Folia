module folia.math.core.math_item;
import folia.math.core.math_item_host;
import folia.math.util.property_host;
import std;
namespace folia::math::core {
AbstractMathItem::AbstractMathItem(Property,MathInputJaxReference,Property,MathLocation,MathLocation){property_host::typeError("Unimplemented MathItem constructor");}
Property protoItem(Property,Property,Property,Property,Property,Property,Property){property_host::typeError("Unimplemented protoItem");}
bool AbstractMathItem::isEscaped()const{property_host::typeError("Unimplemented isEscaped");}
void AbstractMathItem::setMetrics(Property,Property,Property,Property){property_host::typeError("Unimplemented setMetrics");}
Property AbstractMathItem::state(Property,Property){property_host::typeError("Unimplemented state");}
void AbstractMathItem::reset(Property){property_host::typeError("Unimplemented reset");}
void newState(std::string_view,Property){property_host::typeError("Unimplemented newState");}
}
