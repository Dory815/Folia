module folia.math.adaptors.lite.window;
import folia.math.util.property_host;
import std;
namespace folia::math::adaptors::lite {
LiteWindow::LiteWindow(std::shared_ptr<LiteNodeStore> store) {
    if(!store)store=LiteNodeStore::create();
    ref_ = store->create(NodeKind::Window, u"#window");
    auto doc = LiteDocument(ref_,store);
    record(ref_).document = nodeProperty(doc.ref());
}
LiteWindow::LiteWindow(LiteNodeRef ref) : ref_(std::move(ref)) {
    if (!ref_ || recordConst(ref_).nodeKind != NodeKind::Window) throw std::invalid_argument("not a LiteWindow");
}
LiteNodeValue LiteWindow::document() const { return propertyNode(recordConst(ref_).document); }
}
