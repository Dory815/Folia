// Root-frozen portable declarations for complete MmlFactory.ts; Apache-2.0.
export module folia.math.core.mml_tree.mml_factory;
export import folia.math.core.mml_tree.mml;
export import folia.math.core.tree.node_factory;
import std;
export namespace folia::math::mml {
class MmlFactory:public tree::AbstractNodeFactory<MmlNode> {
    using Base=tree::AbstractNodeFactory<MmlNode>;
public:
    inline static std::shared_ptr<tree::FactoryClassMap<MmlNode>> defaultNodes=folia::math::mml::MML();
    explicit MmlFactory(std::optional<tree::FactoryClassMap<MmlNode>> nodes=std::nullopt,
                       const tree::FactoryClassMap<MmlNode>* classDefaults=nullptr);
    tree::FactoryCreatorView<MmlNode> MML()const;
};
}
