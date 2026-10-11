// Root-generated actual Source nullish assignment errors; do not edit.
#include "support/folia_test.hpp"
import folia.math.core.tree.child_array;
namespace fm=folia::math;namespace tr=fm::tree;using namespace boost::ut;namespace {
struct Product{};using Value=tr::ChildValue<Product>;
struct Input{Value value;std::string_view name;std::u16string_view message;};
const Input inputs[]={{fm::Missing{},"TypeError",u"Cannot set properties of undefined (setting 'parent')"},{fm::PropertyNull{},"TypeError",u"Cannot set properties of null (setting 'parent')"}};
}
suite math_port_child_write=[] {
 "math_port_child_write_actual_source_receiver_errors"_test=[] {
  for(const auto& input:inputs){bool caught=false;try{input.value.requireNodeForWrite("parent");}catch(const fm::options::Exception& e){caught=true;expect(e.name==input.name);expect(e.message==input.message);}expect(caught);}
  Product node;expect(Value(&node).requireNodeForWrite("parent")==&node);
 };
};
