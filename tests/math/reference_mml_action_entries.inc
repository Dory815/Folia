// Root Native entry proof, separate from original Source observations.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.mml_nodes.mathchoice;
import folia.math.core.mml_tree.mml_nodes.menclose;
import folia.math.core.mml_tree.mml_nodes.mpadded;
namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::mml;
struct ActionEntry:MmlMaction {
    using MmlMaction::MmlMaction;
    void inheritChildren(){setChildInheritedAttributes(PropertyList{},false,0,false);}
};
suite actionEntries=[] {
    "math_port_action_choice_native_entries"_test=[] {
        MmlFactory factory;
        factory.setNodeClass("action-entry",classBinding<ActionEntry>());
        auto* action=dynamic_cast<ActionEntry*>(factory.create("action-entry"));
        auto check=[](auto call){
            try{call();expect(true);}
            catch(const Error& error){expect(error.name!="Unimplemented")<<error.name;}
        };
        check([&]{action->selected();});
        check([&]{action->core();});
        check([&]{action->coreMO();});
        check([&]{action->setTeXclass(PropertyNull{});});
        check([&]{action->nextToggleSelection();});
        check([&]{action->inheritChildren();});
        auto* choice=factory.create("MathChoice");
        check([&]{choice->setInheritedAttributes(PropertyList{},false,0,false);});
        auto* enclose=factory.create("menclose");
        check([&]{enclose->setTeXclass(PropertyNull{});});
        auto* padded=factory.create("mpadded");
        check([&]{padded->setTeXclass(PropertyNull{});});
    };
};
}
