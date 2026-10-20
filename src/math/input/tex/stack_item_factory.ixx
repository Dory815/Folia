// MathJax 4.1.3 StackItemFactory.ts; Apache-2.0.
export module folia.math.input.tex.parser:stack_item_factory;
export import :stack_item;
import std;

export namespace folia::math::tex {
using StackItemClass = tree::FactoryClass<BaseItem, StackBuildArguments>;
using StackItemCatalog = tree::FactoryClassMap<BaseItem, StackKind, StackBuildArguments>;

class StackItemFactory
    : public tree::AbstractFactory<BaseItem, StackKind, StackBuildArguments, FrameHandle> {
    ParseOptions &configuration_;

  public:
    StackItemFactory(ParseOptions &, StackItemCatalog,
                     std::optional<StackKind> defaultKind = StackKind::dummy);
    ParseOptions &configuration() const noexcept;
    using AbstractFactory::create;
    FrameHandle create(StackKind, StackItemInit const &init = NoItemInit{});
    FrameHandle adopt(std::unique_ptr<BaseItem>) override;
};
} // namespace folia::math::tex
