// Root-frozen generic visitor view; no MML or rendering algorithms.
export module folia.math.core.tree.wrapper_types;
export import folia.math.core.tree.node_types;
import std;

export namespace folia::math::tree {
template<class Product,class Source> class Wrapper : public FactoryNode {
protected:
    WrapperServices<Product,Source>& factory;
public:
    Source* node;
    // nullopt = absent property; engaged nullptr = present null, which throws.
    std::optional<FactoryChildren<Product>> childNodes;
    Wrapper(WrapperServices<Product,Source>& service,Source* source) : factory(service),node(source) {}
    Wrapper(const Wrapper&)=delete;Wrapper& operator=(const Wrapper&)=delete;
    Wrapper(Wrapper&&)=delete;Wrapper& operator=(Wrapper&&)=delete;
    virtual Product* wrap(Source* source,FactoryArgs args={})=0;
    virtual std::any walkTree(const TreeVisitor<Product>& visit,const std::any& data={})=0;
};
}
