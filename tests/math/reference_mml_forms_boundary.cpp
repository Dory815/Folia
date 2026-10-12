// Root generated getForms boundary observations from pinned MathJax.
#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mrow;
namespace {using namespace boost::ut;using namespace folia::math::mml;std::size_t formLength;std::string formPosition,formEvents;MmlNode*formMo;void note(std::string_view s){if(!formEvents.empty())formEvents+=",";formEvents+=s;}struct Row:MmlMrow{using MmlMrow::MmlMrow;bool isEmbellished()const override{return false;}std::size_t nonSpaceLength()override{note("length");return formLength;}MmlNode*firstNonSpace()override{note("first");return formPosition=="first"?formMo:nullptr;}MmlNode*lastNonSpace()override{note("last");return formPosition=="last"?formMo:nullptr;}};struct Case{std::size_t length;std::string_view position,expected;};std::string run(const Case&i){formLength=i.length;formPosition=i.position;MmlFactory f;f.setNodeClass("forms-row",classBinding<Row>());auto*mo=dynamic_cast<MmlMo*>(f.create("mo"));formMo=mo;mo->setParent(f.create("forms-row"));formEvents.clear();const auto forms=mo->getForms();std::string value;for(std::size_t j=0;j<forms.size();++j){if(j)value+=",";value+=std::get<std::string>(forms.get(j).value);}return value+"|"+formEvents;}suite formsBoundary=[] {"math_port_forms_length_boundary_source"_test=[] {const Case cases[]={
{0,"first","prefix,infix,postfix|length,first"},
{0,"last","postfix,infix,prefix|length,first,last"},
{0,"neither","infix,prefix,postfix|length,first,last"},
{1,"first","infix,prefix,postfix|length"},
{1,"last","infix,prefix,postfix|length"},
{1,"neither","infix,prefix,postfix|length"},
{2,"first","prefix,infix,postfix|length,first"},
{2,"last","postfix,infix,prefix|length,first,last"},
{2,"neither","infix,prefix,postfix|length,first,last"},
};for(const auto&i:cases)expect(eq(run(i),i.expected))<<i.position<<i.length;};};}
