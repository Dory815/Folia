// Root Native termination/recovery checks; no Source stack-capacity claim.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mo;
namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::mml;
struct TextLoop:AbstractMmlNode {
    using AbstractMmlNode::AbstractMmlNode;
    std::string kind()const override{return "walk-text";}
    bool isKind(std::string_view k)const override{return k=="mrow";}
    bool isEmbellished()const override{return false;}
};
struct ParentLoop:AbstractMmlNode {
    using AbstractMmlNode::AbstractMmlNode;
    std::string kind()const override{return "walk-parent";}
    bool isEmbellished()const override{return true;}
    NodeValue Parent()const override{return const_cast<ParentLoop*>(this);}
};
suite walkBudget=[] {
    "math_port_operator_walk_budget_recovery"_test=[] {
        MmlFactory factory;
        factory.setNodeClass("walk-text",classBinding<TextLoop>());
        factory.setNodeClass("walk-parent",classBinding<ParentLoop>());
        auto* mo=dynamic_cast<MmlMo*>(factory.create("mo"));
        auto bounded=[](auto call){
            try{call();expect(false)<<"walk did not exhaust its Native budget";}
            catch(const Error& e){expect(e.name=="RangeError"&&e.message==u"Native tree call budget exceeded");}
        };
        mo->setParent(mo);
        bounded([&]{mo->coreParent();});
        mo->setParent(PropertyNull{});
        expect(mo->coreParent()==mo);

        auto* textLoop=factory.create("walk-text");
        textLoop->childNodes->push_back(textLoop);
        bounded([&]{mo->coreText(textLoop);});
        textLoop->childNodes->resize(0);
        expect(mo->coreText(textLoop).empty());

        auto* parentLoop=factory.create("walk-parent");
        parentLoop->setParent(parentLoop);
        parentLoop->childNodes->push_back(parentLoop);
        mo->setParent(parentLoop);
        bounded([&]{mo->getForms();});
        mo->setParent(PropertyNull{});
        expect(mo->getForms().get(0)==Property(std::string("infix")));

        auto* previous=factory.create("mi");
        previous->setTexClass(TEXCLASS.ORD);
        mo->setParent(parentLoop);
        mo->setTexClass(TEXCLASS.BIN);
        bounded([&]{mo->adjustTeXclass(previous);});
        mo->setParent(PropertyNull{});
        mo->setTexClass(TEXCLASS.ORD);
        expect(mo->adjustTeXclass(previous).nodeOrNull()==mo);
    };
};
}
