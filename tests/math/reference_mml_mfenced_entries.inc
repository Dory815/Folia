// Root Native entry proof, distinct from original Source observations.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mfenced;
namespace {
using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
struct FencedEntry:MmlMfenced {
    using MmlMfenced::MmlMfenced;
    void inheritChildren(){setChildInheritedAttributes(PropertyList{},false,0,false);}
    void add(){addFakeNodes();}
    void fake(){fakeNode(u"x");}
};
suite fencedEntries=[] {
    "math_port_mfenced_native_entries"_test=[] {
        MmlFactory factory;factory.setNodeClass("fenced-entry",classBinding<FencedEntry>());
        auto* node=dynamic_cast<FencedEntry*>(factory.create("fenced-entry"));
        auto check=[](auto call){try{call();expect(true);}catch(const Error& e){expect(e.name!="Unimplemented")<<e.name;}};
        check([&]{node->setTeXclass(PropertyNull{});});check([&]{node->inheritChildren();});
        check([&]{node->add();});check([&]{node->fake();});
    };
};
}
