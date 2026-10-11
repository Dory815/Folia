// Native factory-family contract: reject incompatible storage before downcast.
// This is not a JavaScript Source-equivalence assertion.
#include "support/folia_test.hpp"
import folia.math.core.tree.node;
import folia.math.core.tree.node_factory;
namespace tr=folia::math::tree;
using namespace boost::ut;
namespace {
struct Product:tr::Node<Product>{using Node::Node;};
struct Ordinary:tr::AbstractNode<Product,Product>{using AbstractNode::AbstractNode;};
struct OtherBase:Product{using Product::Product;};
struct OtherFamily:tr::AbstractNode<Product,OtherBase>{using AbstractNode::AbstractNode;};
template<class T>tr::FactoryClassRef<Product> nodeClass(){
 auto c=std::make_shared<tr::FactoryClass<Product>>();
 c->construct=[](tr::FactoryServices<Product>& f,tr::FactoryArgs args){auto n=std::make_unique<T>(f);auto* p=n.get();f.adopt(std::move(n));p->initializeNode(args);return p;};
 c->isInstance=[](const Product* p){return dynamic_cast<const T*>(p)!=nullptr;};return c;
}
}
suite math_native_tree_binding=[] {
 "math_native_tree_binding_foreign_factory_product_is_checked"_test=[] {
  tr::AbstractNodeFactory<Product> f(tr::FactoryClassMap<Product>{{"unknown",nodeClass<Ordinary>()}});
  auto* original=f.create("unknown");auto originalChildren=original->childNodes;
  f.setNodeClass("unknown",nodeClass<OtherFamily>());bool caught=false;
  try{(void)original->copy();}catch(const tr::FactoryError& e){caught=true;expect(e.name=="TypeError");}
  expect(caught);expect(original->childNodes==originalChildren);
 };
};
