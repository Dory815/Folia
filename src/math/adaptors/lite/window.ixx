export module folia.math.adaptors.lite.window;
export import folia.math.adaptors.lite.document;
import std;
export namespace folia::math::adaptors::lite {
class LiteWindow {
    LiteNodeRef ref_;
public:
    explicit LiteWindow(std::shared_ptr<LiteNodeStore> store = {});
    explicit LiteWindow(LiteNodeRef ref);
    LiteNodeRef ref() const { return ref_; }
    LiteNodeValue document() const;
};
}
