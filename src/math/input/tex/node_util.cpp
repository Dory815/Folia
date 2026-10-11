module folia.math.input.tex.node_util;
import std;
namespace folia::math::tex {
mml::NodeValue NodeUtil::getChildAt(mml::NodeValue,std::size_t) const {
    throw mml::Error("Unimplemented",u"Root-frozen NodeUtil child service");
}
void NodeUtil::setChild(mml::NodeValue,std::size_t,mml::NodeValue) {
    throw mml::Error("Unimplemented",u"Root-frozen NodeUtil child service");
}
void NodeUtil::copyChildren(mml::NodeValue,mml::NodeValue) {
    throw mml::Error("Unimplemented",u"Root-frozen NodeUtil child service");
}
}
