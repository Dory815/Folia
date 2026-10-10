// Root-owned eager family defaults; Apache-2.0 MathJax source mapping.
export module folia.math.core.mml_tree.leaf_defaults;
export import folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.leaf_host;
import std;
namespace folia::math::mml {namespace {struct State {
PropertyList mrow;
PropertyList math;
PropertyList mi;
PropertyList mn;
PropertyList mtext;
PropertyList ms;
PropertyList mspace;
PropertyList mstyle;
PropertyList merror;
PropertyList mphantom;
State(){
mrow.assign(AbstractMmlNode::defaults());
math.assign(AbstractMmlLayoutNode::defaults());
math.set("mathvariant","normal");
math.set("mathsize","normal");
math.set("mathcolor","");
math.set("mathbackground","transparent");
math.set("dir","ltr");
math.set("scriptlevel",0);
math.set("displaystyle",false);
math.set("display","inline");
math.set("maxwidth","");
math.set("overflow","linebreak");
math.set("altimg","");
math.set("altimg-width","");
math.set("altimg-height","");
math.set("altimg-valign","");
math.set("alttext","");
math.set("cdgroup","");
math.set("scriptsizemultiplier",1/std::sqrt(2.0));
math.set("scriptminsize",".4em");
math.set("infixlinebreakstyle","before");
math.set("lineleading","100%");
math.set("linebreakmultchar",host::debugText(u"\u2062"));
math.set("indentshift","auto");
math.set("indentalign","auto");
math.set("indenttarget","");
math.set("indentalignfirst","indentalign");
math.set("indentshiftfirst","indentshift");
math.set("indentalignlast","indentalign");
math.set("indentshiftlast","indentshift");
mi.assign(AbstractMmlTokenNode::defaults());
mn.assign(AbstractMmlTokenNode::defaults());
mtext.assign(AbstractMmlTokenNode::defaults());
ms.assign(AbstractMmlTokenNode::defaults());
ms.set("lquote","\"");
ms.set("rquote","\"");
mspace.assign(AbstractMmlTokenNode::defaults());
mspace.set("width","0em");
mspace.set("height","0ex");
mspace.set("depth","0ex");
mspace.set("linebreak","auto");
mspace.set("indentshift","auto");
mspace.set("indentalign","auto");
mspace.set("indenttarget","");
mspace.set("indentalignfirst","indentalign");
mspace.set("indentshiftfirst","indentshift");
mspace.set("indentalignlast","indentalign");
mspace.set("indentshiftlast","indentshift");
mstyle.assign(AbstractMmlLayoutNode::defaults());
mstyle.set("scriptlevel",std::string(INHERIT));
mstyle.set("displaystyle",std::string(INHERIT));
mstyle.set("scriptsizemultiplier",1/std::sqrt(2.0));
mstyle.set("scriptminsize",".4em");
mstyle.set("mathbackground",std::string(INHERIT));
mstyle.set("mathcolor",std::string(INHERIT));
mstyle.set("dir",std::string(INHERIT));
mstyle.set("infixlinebreakstyle","before");
merror.assign(AbstractMmlNode::defaults());
mphantom.assign(AbstractMmlLayoutNode::defaults());
}
};State& state(){static State s;return s;} } }
export namespace folia::math::mml {
PropertyList& mrowDefaults(){return state().mrow;}
PropertyList& mathDefaults(){return state().math;}
PropertyList& miDefaults(){return state().mi;}
PropertyList& mnDefaults(){return state().mn;}
PropertyList& mtextDefaults(){return state().mtext;}
PropertyList& msDefaults(){return state().ms;}
PropertyList& mspaceDefaults(){return state().mspace;}
PropertyList& mstyleDefaults(){return state().mstyle;}
PropertyList& merrorDefaults(){return state().merror;}
PropertyList& mphantomDefaults(){return state().mphantom;}
}
