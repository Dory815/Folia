// MathJax 4.1.3 NodeFactory.ts semantic methods; Apache-2.0.
module folia.math.input.tex.node_factory;
import folia.math.input.tex.node_factory_host;
import folia.math.input.tex.node_util;
import std;
namespace folia::math::tex {
mml::NodeValue NodeFactory::createNode(NodeFactory&,Property,mml::Children,Property,mml::NodeValue){throw mml::Error("Unimplemented",u"Root-frozen NodeFactory operation");}
mml::NodeValue NodeFactory::createToken(NodeFactory&,Property,Property,Property){throw mml::Error("Unimplemented",u"Root-frozen NodeFactory operation");}
mml::NodeValue NodeFactory::createText(NodeFactory&,Property){throw mml::Error("Unimplemented",u"Root-frozen NodeFactory operation");}
mml::NodeValue NodeFactory::createError(NodeFactory&,Property){throw mml::Error("Unimplemented",u"Root-frozen NodeFactory operation");}
void NodeFactory::setMmlFactory(MmlFactoryReference){throw mml::Error("Unimplemented",u"Root-frozen NodeFactory operation");}
void NodeFactory::set(std::string_view,Property){throw mml::Error("Unimplemented",u"Root-frozen NodeFactory operation");}
void NodeFactory::setCreators(Property){throw mml::Error("Unimplemented",u"Root-frozen NodeFactory operation");}
mml::NodeValue NodeFactory::create(std::string_view,FunctionArgs){throw mml::Error("Unimplemented",u"Root-frozen NodeFactory operation");}
Property NodeFactory::get(std::string_view)const{throw mml::Error("Unimplemented",u"Root-frozen NodeFactory operation");}
}
