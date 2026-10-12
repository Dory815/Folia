// Root generated exact delimiter string errors from pinned original.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mfenced;
import folia.math.core.mml_tree.mml_node_host;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;struct Probe:MmlMfenced{using MmlMfenced::MmlMfenced;void add(){addFakeNodes();}};struct Case{std::string_view field,tag,expected;};std::string run(const Case&i){MmlFactory f;f.setNodeClass("string-errors",classBinding<Probe>());auto*n=dynamic_cast<Probe*>(f.create("string-errors"));n->attributes()->set(i.field,i.tag=="U"?Property(Missing{}):i.tag=="N"?Property(PropertyNull{}):Property(false));try{n->add();return "ok";}catch(const Error&e){return e.name+"|"+host::debugText(e.message);}}suite stringErrors=[] {"math_port_mfenced_string_errors_source"_test=[] {const Case cases[]={
{"open","U","TypeError|Cannot read properties of undefined (reading 'replace')"},
{"open","N","TypeError|Cannot read properties of null (reading 'replace')"},
{"open","F","TypeError|open.replace is not a function"},
{"close","U","TypeError|Cannot read properties of undefined (reading 'replace')"},
{"close","N","TypeError|Cannot read properties of null (reading 'replace')"},
{"close","F","TypeError|close.replace is not a function"},
{"separators","U","TypeError|Cannot read properties of undefined (reading 'replace')"},
{"separators","N","TypeError|Cannot read properties of null (reading 'replace')"},
{"separators","F","TypeError|separators.replace is not a function"},
};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.field<<i.tag;};};}
