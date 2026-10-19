/*************************************************************
 * Source port of MathJax 4.1.3 core/Tree/Visitor.ts.
 * The visitor is factory-kind driven; callers provide the native equivalents
 * of property lookup on the concrete visitor and childNodes access.
 *************************************************************/
export module folia.math.core.tree.visitor;
export import folia.math.core.tree.factory_types;
import std;

export namespace folia::math::tree {
template<class Product> class AbstractVisitor;

template<class Product>
using VisitorFunction = std::function<std::any(
    AbstractVisitor<Product>&, Product&, std::span<const std::any>)>;

template<class Product> struct VisitorMethod {
    bool truthy = false;
    VisitorFunction<Product> function;
    std::any value;
};

template<class Product> struct VisitorChildren {
    // The Source for-of captures the array object once, but reads its length
    // before every step and Gets the current slot.  These callbacks preserve
    // that live sequence profile during recursive visitor callbacks.
    std::function<std::size_t()> size;
    std::function<ChildValue<Product>(std::size_t)> get;
};

template<class Product>
class AbstractVisitor {
public:
    using MethodLookup = std::function<VisitorMethod<Product>(std::string_view)>;
    using ChildrenLookup = std::function<std::optional<VisitorChildren<Product>>(
        Product&)>;

private:
    std::unordered_map<std::string, VisitorMethod<Product>> nodeHandlers_;
    ChildrenLookup children_;

    static bool identifierChar(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || c == '_';
    }

public:
    static std::string methodName(std::string_view kind) {
        // Source performs charAt(0).toUpperCase() + substring(1), then applies
        // /[^a-z0-9_]/gi to the complete resulting name.  Factory kinds in the
        // current MML profile are ASCII; non-ASCII case expansion is tracked
        // separately rather than treating UTF-8 bytes as UTF-16 code units.
        std::string suffix(kind);
        if (!suffix.empty() && suffix[0] >= 'a' && suffix[0] <= 'z')
            suffix[0] = static_cast<char>(suffix[0] - 'a' + 'A');
        std::string result = "visit";
        for (char c : suffix) result.push_back(identifierChar(c) ? c : '_');
        result += "Node";
        return result;
    }

    AbstractVisitor(FactoryServices<Product>& factory, MethodLookup methodLookup,
                    ChildrenLookup childrenLookup)
        : children_(std::move(childrenLookup)) {
        // Source captures the method function into its Map during construction;
        // later changes to a visitor method do not rewrite this entry.
        for (const auto& kind : factory.getKinds()) {
            const auto name = methodName(kind);
            if (methodLookup) {
                auto method = methodLookup(name);
                if (method.truthy) nodeHandlers_.insert_or_assign(kind, std::move(method));
            }
        }
    }

    virtual ~AbstractVisitor() = default;

    virtual std::any visitTree(Product& tree, std::span<const std::any> args = {}) {
        return visitNode(tree, args);
    }

    virtual std::any visitNode(Product& node, std::span<const std::any> args = {}) {
        const auto kind = node.kind();
        const auto handler = nodeHandlers_.find(kind);
        if (handler != nodeHandlers_.end()) {
            if(!handler->second.truthy)return visitDefault(node,args);
            if (!handler->second.function)
                throw options::Exception("TypeError",u"handler.call is not a function");
            return handler->second.function(*this, node, args);
        }
        return visitDefault(node, args);
    }

    virtual std::any visitDefault(Product& node, std::span<const std::any> args = {}) {
        if (!children_) return {};
        const auto children = children_(node);
        if (!children) return {};
        if(!children->size||!children->get)
            throw options::Exception("TypeError",u"Visitor child iterator is invalid");
        for (std::size_t i=0;i<children->size();++i) {
            const auto child=children->get(i);
            (void)visitNode(*child.requireNode("kind"), args);
        }
        return {};
    }

    virtual void setNodeHandler(std::string kind, VisitorFunction<Product> handler) {
        VisitorMethod<Product> method;
        method.truthy=static_cast<bool>(handler);
        method.function=std::move(handler);
        nodeHandlers_.insert_or_assign(std::move(kind), std::move(method));
    }

    virtual void removeNodeHandler(std::string_view kind) {
        nodeHandlers_.erase(std::string(kind));
    }
};
}
