// Root-frozen identity boundary; implementation delegated under P22a contract.
module folia.math.util.function_types;
import std;
namespace folia::math {
namespace functions {
FunctionRef reference(const FunctionValue&){throw tree::FactoryError("TypeError",u"P22a not implemented");}
FunctionValue fromProperty(const Property&){throw tree::FactoryError("TypeError",u"P22a not implemented");}
Property toProperty(const FunctionValue&){throw tree::FactoryError("TypeError",u"P22a not implemented");}
}
bool operator==(const FunctionValue&,const FunctionValue&){throw tree::FactoryError("TypeError",u"P22a not implemented");}
}
