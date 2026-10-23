export module folia.math.core.mml_tree.mml_document;
export import :node;
export import :factory;
import folia.math.util.instance_count;
import std;

export namespace folia::math::mml {
// One exact-clone operation. Nodes remain exclusively document-owned; this map
// only preserves identity between normal children and family auxiliary links.
class MmlClone final {
    friend class MmlDocument;
    MmlDocument &document_;
    std::unordered_map<MmlNode const *, MmlNode *> nodes_;
    explicit MmlClone(MmlDocument &document) : document_(document) {}

  public:
    MmlClone(MmlClone const &) = delete;
    MmlClone &operator=(MmlClone const &) = delete;
    MmlClone(MmlClone &&) = delete;
    MmlClone &operator=(MmlClone &&) = delete;

    MmlDocument &document() const noexcept {
        return document_;
    }

    MmlNode &clone(MmlNode const &source);
};

class MmlDocument final : public tree::NodeOwner<MmlNode, NodeKind> {
    friend class MmlFactory;
    [[no_unique_address]] util::InstanceCount<MmlDocument> instance_count_;
    std::unique_ptr<MmlFactory> factory_;
    std::vector<std::unique_ptr<MmlNode>> arena_;
    MmlNode *root_ = nullptr;

    MmlNode &adopt(std::unique_ptr<MmlNode>);
    void requireOwned(const MmlNode &) const;
    void requireAcyclic(const MmlNode &parent, const MmlNode &child) const;
    void detach(MmlNode &) noexcept;

  public:
    MmlDocument();
    ~MmlDocument();
    MmlDocument(MmlDocument const &) = delete;
    MmlDocument &operator=(MmlDocument const &) = delete;
    MmlDocument(MmlDocument &&) = delete;
    MmlDocument &operator=(MmlDocument &&) = delete;

    MmlFactory &factory() noexcept;
    MmlFactory const &factory() const noexcept;
    MmlNode &make(NodeKind);

    MmlNode *root() const noexcept {
        return root_;
    }

    void setRoot(MmlNode &);
    void reserveChildren(MmlNode &parent, std::size_t count);
    void setChildren(MmlNode &, std::span<MmlNode *const>) override;
    // TeX scripts are assembled by physical child slot before postfilters
    // choose msub/msup or munder/mover. nullptr means an absent math slot.
    void setChildSlot(MmlNode &, std::size_t index, MmlNode *);
    void append(MmlNode &parent, MmlNode &child) override;
    void insert(MmlNode &parent, std::size_t index, MmlNode &child);
    void replace(MmlNode &parent, MmlNode &oldChild, MmlNode &replacement) override;
    void remove(MmlNode &node) override;
    // MathJax's mfenced generated operators have a semantic parent but are not
    // normal childNodes. Ownership remains in this arena, with one parent link.
    void attachAuxiliary(MmlNode &parent, MmlNode &child);
    MmlNode &cloneSubtree(MmlNode const &node);
    MmlNode &clone(MmlNode const &, bool keepIds) override;
    bool nodeIsKind(MmlNode const &, NodeKind) const override;
    std::string_view kindName(NodeKind) const override;

    std::size_t size() const noexcept {
        return arena_.size();
    }
};
} // namespace folia::math::mml
