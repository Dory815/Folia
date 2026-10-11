// MathJax 4.1.3 mo.ts adjustTeXclass; Apache-2.0.
// Source SHA256 2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605
// Root mechanical extraction: one operation shared by Mo and TeXAtom.
module folia.math.core.mml_tree.operator_class;
import folia.math.core.mml_tree.mo_host;
import std;
namespace folia::math::mml {
MmlNode* adjustOperatorTeXclass(MmlNode& receiver, MmlNode* prev) {
    tree::TreeCallGuard<MmlNode> guard;
    const Property texClassValue = receiver.texClass();
    Property prevClassValue = receiver.prevClass();
    if (texClassValue == Property(TEXCLASS.NONE)) return prev;

    if (prev) {
        const Property autoOp = prev->getProperty("autoOP");
        if (host::truthy(autoOp) &&
            (texClassValue == Property(TEXCLASS.BIN) ||
             texClassValue == Property(TEXCLASS.REL))) {
            prev->setTexClass(TEXCLASS.ORD);
            prevClassValue = TEXCLASS.ORD;
        }
        const Property previousTexClass = prev->texClass();
        prevClassValue = host::truthy(previousTexClass)
            ? previousTexClass : Property(TEXCLASS.ORD);
        receiver.setPrevClass(prevClassValue);
        const auto attributes = host::moAttributes(receiver);
        receiver.setPrevLevel(attributes->getInherited("scriptlevel"));
    } else {
        prevClassValue = TEXCLASS.NONE;
        receiver.setPrevClass(prevClassValue);
    }

    if (texClassValue == Property(TEXCLASS.BIN) &&
        (prevClassValue == Property(TEXCLASS.NONE) ||
         prevClassValue == Property(TEXCLASS.BIN) ||
         prevClassValue == Property(TEXCLASS.OP) ||
         prevClassValue == Property(TEXCLASS.REL) ||
         prevClassValue == Property(TEXCLASS.OPEN) ||
         prevClassValue == Property(TEXCLASS.PUNCT))) {
        receiver.setTexClass(TEXCLASS.ORD);
    } else if (prevClassValue == Property(TEXCLASS.BIN) &&
               (texClassValue == Property(TEXCLASS.REL) ||
                texClassValue == Property(TEXCLASS.CLOSE) ||
                texClassValue == Property(TEXCLASS.PUNCT))) {
        if (!prev) host::typeError("Cannot set texClass on null previous node");
        receiver.setPrevClass(TEXCLASS.ORD);
        prev->setTexClass(TEXCLASS.ORD);
    } else if (texClassValue == Property(TEXCLASS.BIN)) {
        MmlNode* child = nullptr;
        MmlNode* parent = receiver.parent;
        while (parent && parent->parent && parent->isEmbellished() &&
               ([&]() {
                   if (!parent->childNodes) host::typeError("Cannot read length of null childNodes");
                   return parent->childNodes->size() == 1 ||
                       (!parent->isKind("mrow") && parent->core() == child);
               }())) {
            child = parent;
            parent = parent->parent;
        }
        child = child ? child : &receiver;
        if (!parent) host::typeError("Cannot read childNodes of null parent");
        if (!parent->childNodes) host::typeError("Cannot read length of null childNodes");
        const auto& children = *parent->childNodes;
        if (!children.empty() && children.back() == child) receiver.setTexClass(TEXCLASS.ORD);
    }
    return &receiver;
}
}
