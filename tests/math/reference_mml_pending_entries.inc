// Root Native-only witnesses: missing implementation must remain visible.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mmultiscripts;
namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::mml;
struct Multi : MmlMmultiscripts {
    using MmlMmultiscripts::MmlMmultiscripts;
    void verifyHook() { verifyChildren(PropertyList{}); }
};
suite pendingEntries=[] {
    "math_port_pending_real_entries"_test=[] {
        MmlFactory factory;
        factory.setNodeClass("mmultiscripts",classBinding<Multi>());
        auto check=[](auto call) {
            try { call(); expect(true); }
            catch(const Error& e) { expect(e.name!="Unimplemented")<<e.name; }
        };
        auto* table=factory.create("mtable");
        auto* multi=dynamic_cast<Multi*>(factory.create("mmultiscripts"));
        check([&]{table->setTeXclass(PropertyNull{});});
        check([&]{multi->verifyHook();});
    };
};
}
