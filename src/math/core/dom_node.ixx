export module folia.math.core.dom_node;
import std;
import folia.math.util.instance_count;

export namespace folia::math {
class DOMNode;
class DOMDocument;
class DOMWindow;

using DOMNodeRef = std::shared_ptr<DOMNode>;
using DOMDocumentRef = std::shared_ptr<DOMDocument>;
using DOMWindowRef = std::shared_ptr<DOMWindow>;

class DOMNode {
    [[no_unique_address]] util::InstanceCount<DOMNode> instance_count_;
    std::weak_ptr<DOMNode> parent_;
public:
    virtual ~DOMNode() = default;
    virtual std::u16string_view kind() const noexcept = 0;

    DOMNodeRef parent() const noexcept { return parent_.lock(); }
    void setParent(DOMNodeRef parent) noexcept { parent_ = std::move(parent); }
};

class DOMDocument {
    [[no_unique_address]] util::InstanceCount<DOMDocument> instance_count_;

  public:
    virtual ~DOMDocument() = default;
};

class DOMWindow {
    [[no_unique_address]] util::InstanceCount<DOMWindow> instance_count_;

  public:
    virtual ~DOMWindow() = default;
};
}
