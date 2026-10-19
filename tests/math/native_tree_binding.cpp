// A registry replacement may return another implementation of the same Product.
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
 "math_native_tree_binding_copy_uses_current_product_implementation"_test=[] {
  tr::AbstractNodeFactory<Product> f(tr::FactoryClassMap<Product>{{"unknown",nodeClass<Ordinary>()}});
  f.setNodeClass("child",nodeClass<Ordinary>());
  auto* original=f.create("unknown");original->setProperty("tag",folia::math::Property("original"));
  auto* child=f.create("child");original->appendChild(tr::ChildValue<Product>(child));
  auto originalChildren=original->childNodes;
  f.setNodeClass("unknown",nodeClass<OtherFamily>());
  auto* copied=original->copy();
  expect(dynamic_cast<OtherFamily*>(copied)!=nullptr);
  expect(copied->getProperty("tag")==folia::math::Property("original"));
  expect(copied->childNodes!=originalChildren);expect(copied->childNodes->size()==1);
  expect((*copied->childNodes).get(0).nodeOrNull()!=child);
  expect(original->childNodes==originalChildren);expect(original->childNodes->size()==1);
  expect(original->getProperty("tag")==folia::math::Property("original"));
 };
};
