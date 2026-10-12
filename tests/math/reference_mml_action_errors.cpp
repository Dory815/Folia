// Root generated exact actiontype errors from pinned original.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.action_host;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;struct Probe:MmlMaction{using MmlMaction::MmlMaction;void hook(){setChildInheritedAttributes(PropertyList{},false,0,false);}};Property value(std::string_view tag){if(tag=="U")return Missing{};if(tag=="N")return PropertyNull{};if(tag=="false")return false;if(tag=="zero")return 0;if(tag=="array")return PropertyArray{};if(tag=="object")return PropertyList{};return tag=="tooltip"?"TOOLTIP":"toggle";}struct Case{std::string_view input,expected;};std::string run(const Case&i){MmlFactory f;f.setNodeClass("action-errors",classBinding<Probe>());auto*n=dynamic_cast<Probe*>(f.create("action-errors"));n->attributes()->set("actiontype",value(i.input));try{n->hook();return "ok";}catch(const Error&e){return e.name+"|"+host::debugText(e.message);}}suite actionErrors=[] {"math_port_action_exact_errors_source"_test=[] {const Case cases[]={
{"U","TypeError|Cannot read properties of undefined (reading 'toLowerCase')"},
{"N","TypeError|Cannot read properties of null (reading 'toLowerCase')"},
{"false","TypeError|this.attributes.get(...).toLowerCase is not a function"},
{"zero","TypeError|this.attributes.get(...).toLowerCase is not a function"},
{"array","TypeError|this.attributes.get(...).toLowerCase is not a function"},
{"object","TypeError|this.attributes.get(...).toLowerCase is not a function"},
{"tooltip","ok"},
{"toggle","ok"},
};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.input;};};}
