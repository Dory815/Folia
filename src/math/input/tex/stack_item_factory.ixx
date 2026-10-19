// MathJax 4.1.3 input/tex/StackItemFactory.ts. Apache-2.0.
export module folia.math.input.tex.stack_item_factory;
export import folia.math.input.tex.stack_item;
export import folia.math.core.tree.factory;
import std;
export namespace folia::math::tex {
class StackItemFactory:public tree::AbstractFactory<BaseItem>,public StackItemFactoryService {
    std::vector<std::shared_ptr<BaseItem>> products_;
    std::uint64_t retirementGeneration_=0;
public:
    struct ProductMark { const StackItemFactory* owner=nullptr;std::size_t size=0;std::uint64_t generation=0; };
    // Distinct Source static field: inherited generic defaultNodes stays empty.
    static tree::FactoryClassMap<BaseItem> DefaultStackItems;
    explicit StackItemFactory(std::optional<tree::FactoryClassMap<BaseItem>> nodes=std::nullopt,
                             const tree::FactoryClassMap<BaseItem>* classDefaults=nullptr);
    tree::FactoryServices<BaseItem>& itemFactory()override;
    BaseItem* adopt(std::unique_ptr<BaseItem>)override;
    StackItemRef reference(BaseItem*)const override;
    ProductMark productMark()const noexcept{return {this,products_.size(),retirementGeneration_};}
    std::size_t productCountForDiagnostics()const noexcept{return products_.size();}
    void retireProductsFrom(ProductMark);
};
}
