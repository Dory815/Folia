export module folia.math.core.mml_tree.attributes;
import std;
export namespace folia::math::mml {
struct AttributeNull { friend constexpr bool operator==(AttributeNull,AttributeNull)=default; };
using AttributeValue = std::variant<AttributeNull,bool,double,std::u16string>;
class AttributeSet {
public:
 using Entry=std::pair<std::u16string,AttributeValue>;
private:
 std::vector<Entry> own_, defaults_, inherited_;
 static const AttributeValue* find(const std::vector<Entry>&,std::u16string_view) noexcept;
 static void put(std::vector<Entry>&,std::u16string,AttributeValue);
public:
 void set(std::u16string name,AttributeValue value);
 bool erase(std::u16string_view name);
 const AttributeValue* own(std::u16string_view name)const noexcept;
 const AttributeValue* defaultValue(std::u16string_view name)const noexcept;
 const AttributeValue* inherited(std::u16string_view name)const noexcept;
 const AttributeValue* lookup(std::u16string_view name)const noexcept;
 void setDefault(std::u16string name,AttributeValue value);
 void setInherited(std::u16string name,AttributeValue value);
 const std::vector<Entry>& ownEntries()const noexcept{return own_;}
 const std::vector<Entry>& defaultEntries()const noexcept{return defaults_;}
 const std::vector<Entry>& inheritedEntries()const noexcept{return inherited_;}
};
}
