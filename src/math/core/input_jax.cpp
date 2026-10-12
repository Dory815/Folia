// MathJax 4.1.3 core/InputJax.ts methods; Apache-2.0.
module folia.math.core.input_jax;
import std;
namespace folia::math::core {
options::Value AbstractInputJax::name(){throw tree::FactoryError("Unimplemented",u"InputJax.name");}
bool AbstractInputJax::processStrings()const{throw tree::FactoryError("Unimplemented",u"InputJax.processStrings");}
void AbstractInputJax::setAdaptor(InputAdaptorReference){throw tree::FactoryError("Unimplemented",u"InputJax.setAdaptor");}
void AbstractInputJax::setMmlFactory(InputMmlFactoryReference){throw tree::FactoryError("Unimplemented",u"InputJax.setMmlFactory");}
void AbstractInputJax::initialize(){throw tree::FactoryError("Unimplemented",u"InputJax.initialize");}
void AbstractInputJax::reset(FunctionArgs){throw tree::FactoryError("Unimplemented",u"InputJax.reset");}
ProtoMathItems AbstractInputJax::findMath(std::any,options::Value){throw tree::FactoryError("Unimplemented",u"InputJax.findMath");}
std::any AbstractInputJax::executeFilters(FunctionList&,MathItemReference,MathDocumentReference,std::any){throw tree::FactoryError("Unimplemented",u"InputJax.executeFilters");}
}
