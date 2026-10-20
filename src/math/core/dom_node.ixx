export module folia.math.core.dom_node;
import std;

export namespace folia::math {
class DOMNode;
class DOMDocument;
class DOMWindow;

using DOMNodeRef = std::shared_ptr<DOMNode>;
using DOMDocumentRef = std::shared_ptr<DOMDocument>;
using DOMWindowRef = std::shared_ptr<DOMWindow>;

class DOMNode {
    std::weak_ptr<DOMNode> parent_;
public:
    virtual ~DOMNode() = default;
    virtual std::u16string_view kind() const noexcept = 0;

    DOMNodeRef parent() const noexcept { return parent_.lock(); }
    void setParent(DOMNodeRef parent) noexcept { parent_ = std::move(parent); }
};

class DOMDocument {
public:
    virtual ~DOMDocument() = default;
};

class DOMWindow {
public:
    virtual ~DOMWindow() = default;
};
}
