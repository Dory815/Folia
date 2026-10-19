// MathJax 4.1.3 core/MathItem.ts location-record access.
export module folia.math.core.math_location;
export import folia.math.core.tree.property;
export import folia.math.util.object_types;
import std;

export namespace folia::math::core {
// One canonical Source location value type, independent of MathItem so both
// the item and its document/list services can share it without a module cycle.
using MathLocation=std::variant<Property,NativeObjectRef>;

// Access to an opaque native location record is delegated to the owner of
// that exact record. Property-backed records are read and written directly;
// this service never projects them into a second dictionary.
class MathLocationService {
public:
    virtual ~MathLocationService()=default;
    virtual Property get(const NativeObjectRef&,std::string_view key)=0;
    virtual void set(const NativeObjectRef&,std::string_view key,Property value)=0;
};

Property readMathLocation(const MathLocation&,std::string_view key,
                          MathLocationService* service=nullptr);
void writeMathLocation(const MathLocation&,std::string_view key,Property value,
                       MathLocationService* service=nullptr);
}
