export module folia.math.output.common:wrapper_factory;
export import :types;
export import :wrapper;
export import :notation;
export import folia.math.core.tree.wrapper_factory;
import std;

export namespace folia::math::output::common {
class CommonWrapperFactory
    : public tree::AbstractWrapperFactory<CommonWrapper, mml::MmlNode, WrapperBuildArguments> {
    using Base = tree::AbstractWrapperFactory<CommonWrapper, mml::MmlNode, WrapperBuildArguments>;
    CommonOutputJax &jax_; // borrowed; Jax owns this factory
    std::vector<std::unique_ptr<CommonWrapper>> wrappers_;
    std::vector<std::pair<mml::NodeKind, CommonWrapperClassData>>
        classData_; // values, not creators
  protected:
    WrapperBuildArguments creationArguments(mml::MmlNode &, CommonWrapper *) override;

  public:
    explicit CommonWrapperFactory(CommonOutputJax &, CommonWrapperCatalog = defaultNodes());
    virtual ~CommonWrapperFactory() = default;
    // Own and initialize after full derived construction. CommonWrapper::wrap
    // retains Source indexing and explicit-parent attachment responsibilities.
    CommonWrapper &adopt(std::unique_ptr<CommonWrapper>) override;
    CommonOutputJax &jax() const noexcept;
    static CommonWrapperCatalog defaultNodes();
    // The concrete backend-neutral Common profile used by native rendering
    // and tests. This is deliberately separate from Source's abstract empty
    // CommonWrapperFactory.defaultNodes() contract.
    static CommonWrapperCatalog standardNodes(std::span<NotationSpec const> notationProfile = {});
    std::vector<mml::NodeKind> getKinds() const;
    std::optional<tree::FactoryClass<CommonWrapper, WrapperBuildArguments>>
        getWrapperClass(mml::NodeKind) const;
    void setWrapperClass(CommonWrapperRegistration);
    CommonWrapperClassData const &classData(mml::NodeKind) const;
    std::size_t wrapperCount() const noexcept;
};
} // namespace folia::math::output::common
