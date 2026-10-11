// Generated only from original CJS protected-method observations.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
namespace {using namespace boost::ut;using namespace folia::math;using namespace folia::math::mml;
struct Probe:AbstractMmlNode{using AbstractMmlNode::AbstractMmlNode;std::string kind()const override{return "probe";}void update(NodeValue p){updateTeXclass(p);}void previous(NodeValue p){getPrevClass(p);}};
suite base_class_boundary=[] {
"math_port_class_state_boundary_setTeXclass"_test=[] {MmlFactory f;f.setNodeClass("probe",classBinding<Probe>());auto*n=dynamic_cast<Probe*>(f.create("probe"));auto*p=dynamic_cast<Probe*>(f.create("probe"));n->setTexClass(0);n->setPrevClass(11);n->setPrevLevel(7);p->setTexClass(2);p->setPrevClass(5);p->setPrevLevel(3);p->attributes()->set("scriptlevel",3);try{
expect((n->setTeXclass(NodeValue(PropertyNull{}))==NodeValue(n))==true);
expect(n->texClass()==Property(0));
expect(n->prevClass()==Property(11));
expect(n->prevLevel()==Property(7));
expect(p->texClass()==Property(2));
expect(p->prevClass()==Property(5));
expect(p->prevLevel()==Property(3));
}catch(const Error&e){std::cerr<<e.name<<" base-class boundary\n";expect(false)<<e.name;}};
"math_port_class_state_boundary_updateTeXclass"_test=[] {MmlFactory f;f.setNodeClass("probe",classBinding<Probe>());auto*n=dynamic_cast<Probe*>(f.create("probe"));auto*p=dynamic_cast<Probe*>(f.create("probe"));n->setTexClass(0);n->setPrevClass(11);n->setPrevLevel(7);p->setTexClass(2);p->setPrevClass(5);p->setPrevLevel(3);p->attributes()->set("scriptlevel",3);try{
n->update(NodeValue(p));
expect(n->texClass()==Property(2));
expect(n->prevClass()==Property(5));
expect(n->prevLevel()==Property(3));
expect(p->texClass()==Property(2));
expect(p->prevClass()==Property(PropertyNull{}));
expect(p->prevLevel()==Property(PropertyNull{}));
}catch(const Error&e){std::cerr<<e.name<<" base-class boundary\n";expect(false)<<e.name;}};
"math_port_class_state_boundary_getPrevClass"_test=[] {MmlFactory f;f.setNodeClass("probe",classBinding<Probe>());auto*n=dynamic_cast<Probe*>(f.create("probe"));auto*p=dynamic_cast<Probe*>(f.create("probe"));n->setTexClass(0);n->setPrevClass(11);n->setPrevLevel(7);p->setTexClass(2);p->setPrevClass(5);p->setPrevLevel(3);p->attributes()->set("scriptlevel",3);try{
n->previous(NodeValue(p));
expect(n->texClass()==Property(0));
expect(n->prevClass()==Property(2));
expect(n->prevLevel()==Property(3));
expect(p->texClass()==Property(2));
expect(p->prevClass()==Property(5));
expect(p->prevLevel()==Property(3));
}catch(const Error&e){std::cerr<<e.name<<" base-class boundary\n";expect(false)<<e.name;}};
};}
