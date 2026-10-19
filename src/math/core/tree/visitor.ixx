/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native typed visitor based on MathJax 4.1.3 core/Tree/Visitor.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.core.tree.visitor;
export import folia.math.core.tree.factory_types;
import std;

export namespace folia::math::tree {
template <class Product, class Argument, class Result>
using VisitorFunction = std::function<Result(Product &, Argument const &)>;

template <class Product, class Kind, class Argument, class Result>
using VisitorHandlerTable = std::unordered_map<Kind, VisitorFunction<Product, Argument, Result>>;

template <class Product> struct VisitorChildren {
    std::function<std::size_t(Product &)> size;
    std::function<Product *(Product &, std::size_t)> get;
};

template <class Product, class Kind, class Argument = std::monostate, class Result = void>
class AbstractVisitor {
  public:
    using Handler = VisitorFunction<Product, Argument, Result>;
    using HandlerTable = VisitorHandlerTable<Product, Kind, Argument, Result>;

  private:
    std::unordered_map<Kind, std::shared_ptr<Handler const>> nodeHandlers_;
    VisitorChildren<Product> children_;

  public:
    template <class CreationArguments>
    explicit AbstractVisitor(FactoryServices<Product, Kind, CreationArguments> &factory,
                             HandlerTable handlers, VisitorChildren<Product> children)
        : children_(std::move(children)) {
        // Source captures handlers only for the factory's current kinds.
        // The copied functions remain stable if the caller later replaces its table.
        for (const auto &kind : factory.getKinds()) {
            const auto found = handlers.find(kind);
            if (found != handlers.end() && found->second)
                nodeHandlers_.insert_or_assign(kind,
                                               std::make_shared<Handler const>(found->second));
        }
    }

    AbstractVisitor(AbstractVisitor const &) = delete;
    AbstractVisitor &operator=(AbstractVisitor const &) = delete;
    AbstractVisitor(AbstractVisitor &&) = delete;
    AbstractVisitor &operator=(AbstractVisitor &&) = delete;
    virtual ~AbstractVisitor() = default;

    virtual Result visitTree(Product &tree, Argument const &argument = Argument{}) {
        return visitNode(tree, argument);
    }

    virtual Result visitNode(Product &node, Argument const &argument = Argument{}) {
        const auto found = nodeHandlers_.find(node.kind());
        // Keep the active target alive if it changes this registry entry while
        // executing. Copying only the shared owner preserves mutable functor state.
        std::shared_ptr<Handler const> handler;
        if (found != nodeHandlers_.end())
            handler = found->second;
        if (handler && *handler)
            return (*handler)(node, argument);
        return visitDefault(node, argument);
    }

    virtual Result visitDefault(Product &node, Argument const &argument = Argument{}) {
        const bool hasSize = static_cast<bool>(children_.size);
        const bool hasGet = static_cast<bool>(children_.get);
        if (hasSize != hasGet)
            throw std::invalid_argument("Tree visitor child accessors must be supplied together");
        if (hasSize) {
            // Query the size on every iteration and get the current slot only
            // after the callback, matching Source's live for-of child traversal.
            for (std::size_t index = 0; index < children_.size(node); ++index) {
                auto *child = children_.get(node, index);
                if (!child)
                    throw std::invalid_argument("Tree visitor child accessor returned null");
                (void)visitNode(*child, argument);
            }
        }
        if constexpr (std::is_void_v<Result>) {
            return;
        } else {
            return Result{};
        }
    }

    virtual void setNodeHandler(Kind kind, Handler handler) {
        if (handler)
            nodeHandlers_.insert_or_assign(std::move(kind),
                                           std::make_shared<Handler const>(std::move(handler)));
        else
            nodeHandlers_.erase(kind);
    }

    virtual void removeNodeHandler(Kind const &kind) {
        nodeHandlers_.erase(kind);
    }
};
} // namespace folia::math::tree
