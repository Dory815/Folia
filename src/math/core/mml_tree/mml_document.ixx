export module folia.math.core.mml_tree.mml_document;
export import :node;
import std;

export namespace folia::math::mml {
class MmlDocument final {
    std::vector<std::unique_ptr<MmlNode>> arena_;
    MmlNode* root_ = nullptr;

    void requireOwned(const MmlNode&) const;
    void requireAcyclic(const MmlNode& parent, const MmlNode& child) const;
    void detach(MmlNode&) noexcept;

public:
    MmlDocument() = default;
    ~MmlDocument() = default;
    MmlDocument(MmlDocument const&) = delete;
    MmlDocument& operator=(MmlDocument const&) = delete;
    MmlDocument(MmlDocument&&) = delete;
    MmlDocument& operator=(MmlDocument&&) = delete;

    MmlNode& make(NodeKind);
    MmlNode* root() const noexcept { return root_; }
    void setRoot(MmlNode&);
    void reserveChildren(MmlNode& parent, std::size_t count);
    void append(MmlNode& parent, MmlNode& child);
    void insert(MmlNode& parent, std::size_t index, MmlNode& child);
    void replace(MmlNode& parent, MmlNode& oldChild, MmlNode& replacement);
    void remove(MmlNode& node);
    MmlNode& cloneSubtree(const MmlNode& node);
    std::size_t size() const noexcept { return arena_.size(); }
};
}
