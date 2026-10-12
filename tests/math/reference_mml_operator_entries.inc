// Root Native-only proof that effective adjustment entries are implemented.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.tex_atom;
namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::mml;
suite operatorEntries=[] {
    "math_port_operator_real_entries"_test=[] {
        MmlFactory factory;
        auto check=[](auto call) {
            try { call(); expect(true); }
            catch(const Error& e) { expect(e.name!="Unimplemented")<<e.name; }
        };
        auto* mo=dynamic_cast<MmlMo*>(factory.create("mo"));
        auto* atom=factory.create("TeXAtom");
        check([&]{mo->adjustTeXclass(PropertyNull{});});
        check([&]{atom->setTeXclass(PropertyNull{});});
    };
};
}
