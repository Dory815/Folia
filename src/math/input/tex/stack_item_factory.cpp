// MathJax 4.1.3 StackItemFactory.ts; Apache-2.0.
module folia.math.input.tex.parser;
import :stack_item_factory;
import std;
namespace folia::math::tex {
StackItemFactory::StackItemFactory(ParseOptions &owner, StackItemCatalog classes,
                                   std::optional<StackKind> fallback)
    : AbstractFactory(std::move(classes), fallback), configuration_(owner) {}
ParseOptions &StackItemFactory::configuration() const noexcept {
    return configuration_;
}
FrameHandle StackItemFactory::create(StackKind kind, StackItemInit const &init) {
    return AbstractFactory::create(kind, StackBuildArguments{*this, init});
}
FrameHandle StackItemFactory::adopt(std::unique_ptr<BaseItem> item) {
    return FrameHandle(std::move(item));
}
} // namespace folia::math::tex
