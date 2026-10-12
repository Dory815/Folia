module folia.math.input.tex.unit_util;
import folia.math.input.tex.unit_util_host;
import std;
namespace folia::math::tex {
UnitMap::UnitMap(UnitEntries){property_host::typeError("Unimplemented UnitMap constructor");}
void UnitMap::updateDimen(){property_host::typeError("Unimplemented UnitMap updateDimen");}
UnitMap* UnitMap::set(std::u16string,double){property_host::typeError("Unimplemented UnitMap set");}
Property UnitMap::get(Property)const{property_host::typeError("Unimplemented UnitMap get");}
bool UnitMap::erase(std::u16string_view){property_host::typeError("Unimplemented UnitMap delete");}
Dimension UnitUtil::matchDimen(Property,Property){property_host::typeError("Unimplemented UnitUtil matchDimen");}
double UnitUtil::dimen2em(Property){property_host::typeError("Unimplemented UnitUtil dimen2em");}
std::string UnitUtil::em(double){property_host::typeError("Unimplemented UnitUtil em");}
Property UnitUtil::trimSpaces(Property){property_host::typeError("Unimplemented UnitUtil trimSpaces");}
}
