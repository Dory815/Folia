// MathJax 4.1.3 mo.ts adjustTeXclass; Apache-2.0.
// Source SHA256 2082cd372a4c21fa46c67ab3972b51bcc950f711dd3369cbb2bd7ced62c2f605
// Root mechanical extraction: one operation shared by Mo and TeXAtom.
module folia.math.core.mml_tree.operator_class;
import folia.math.core.mml_tree.mo_host;
import std;
namespace folia::math::mml {
NodeValue adjustOperatorTeXclass(MmlNode& receiver, NodeValue prev) {
    tree::TreeCallGuard<MmlNode> guard;

    Property texClass = receiver.texClass();
    Property prevClass = receiver.prevClass();
    if (texClass == Property(TEXCLASS.NONE)) return prev;

    if (MmlNode* previous = prev.nodeOrNull()) {
        if (host::truthy(previous->getProperty("autoOP")) &&
            (texClass == Property(TEXCLASS.BIN) ||
             texClass == Property(TEXCLASS.REL))) {
            // Source assignment `prevClass = prev.texClass = ORD` invokes the
            // previous-node setter before storing its assignment result locally.
            previous->setTexClass(Property(TEXCLASS.ORD));
            prevClass = Property(TEXCLASS.ORD);
        }

        // `prevClass = this.prevClass = prev.texClass || ORD`: preserve the
        // virtual receiver write before retaining the local value.
        const Property previousTexClass = previous->texClass();
        prevClass = host::truthy(previousTexClass)
                        ? previousTexClass
                        : Property(TEXCLASS.ORD);
        receiver.setPrevClass(prevClass);

        auto attributes = receiver.attributes();
        if (!attributes) host::typeError("Cannot read properties of null (reading 'getInherited')");
        receiver.setPrevLevel(attributes->getInherited("scriptlevel"));
    } else {
        prevClass = Property(TEXCLASS.NONE);
        receiver.setPrevClass(prevClass);
    }

    if (texClass == Property(TEXCLASS.BIN) &&
        (prevClass == Property(TEXCLASS.NONE) ||
         prevClass == Property(TEXCLASS.BIN) ||
         prevClass == Property(TEXCLASS.OP) ||
         prevClass == Property(TEXCLASS.REL) ||
         prevClass == Property(TEXCLASS.OPEN) ||
         prevClass == Property(TEXCLASS.PUNCT))) {
        receiver.setTexClass(Property(TEXCLASS.ORD));
    } else if (prevClass == Property(TEXCLASS.BIN) &&
               (texClass == Property(TEXCLASS.REL) ||
                texClass == Property(TEXCLASS.CLOSE) ||
                texClass == Property(TEXCLASS.PUNCT))) {
        // Source assignment `prev.texClass = this.prevClass = ORD` is
        // right-associative: update this node before the previous node.
        receiver.setPrevClass(Property(TEXCLASS.ORD));
        prevClass = Property(TEXCLASS.ORD);
        prev.requireNode("texClass")->setTexClass(Property(TEXCLASS.ORD));
    } else if (texClass == Property(TEXCLASS.BIN)) {
        NodeValue child(PropertyNull{});
        NodeValue parent = receiver.getParent();
        tree::TreeWalkBudget<MmlNode> walkBudget;
        while (parent.isNode()) {
            MmlNode& parentNode = host::moRequired(parent, "parent");
            const NodeValue conditionParent = parentNode.getParent();
            if (!conditionParent.isNode()) break;
            if (!parentNode.isEmbellished()) break;

            const auto conditionChildren = parentNode.childArrayOwner();
            if (!conditionChildren) {
                host::typeError("Cannot read properties of null (reading 'length')");
            }
            const bool oneChild = conditionChildren->size() == 1;
            bool continueWalk = oneChild;
            if (!oneChild) {
                const bool isRow = parentNode.isKind("mrow");
                if (!isRow) continueWalk = parentNode.core() == child;
            }
            if (!continueWalk) break;
            walkBudget.step();

            child = parent;
            // The loop condition and assignment each read parent.parent.
            parent = parentNode.getParent();
        }
        if (!child.isNode()) child = NodeValue(&receiver);

        // Source captures the indexed array before evaluating the current
        // length expression, then indexes that captured handle.
        MmlNode& endpoint = host::moRequired(parent, "childNodes");
        const auto indexedChildren = endpoint.childArrayOwner();
        MmlNode& lengthReceiver = host::moRequired(parent, "childNodes");
        const auto lengthChildren = lengthReceiver.childArrayOwner();
        if (!lengthChildren) {
            host::typeError("Cannot read properties of null (reading 'length')");
        }
        const std::size_t length = lengthChildren->size();
        const NodeValue last = length ? indexedChildren->get(length - 1)
                                      : NodeValue(Missing{});
        if (last == child) receiver.setTexClass(Property(TEXCLASS.ORD));
    }
    return NodeValue(&receiver);
}
}
