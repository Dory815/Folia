#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
namespace {using namespace boost::ut;using namespace folia::math;
suite factory_product_mark=[] {
"math_port_factory_product_mark_is_bound_to_current_factory_generation"_test=[] {
    mml::MmlFactory factory,other;
    const auto mark=factory.productMark();
    (void)factory.create("mi");(void)factory.create("mn");
    std::size_t visited=0;factory.visitProductsFrom(mark,[&](const mml::MmlNode&){++visited;});
    expect(visited==2u);expect(factory.productCountForDiagnostics()==2u);
    expect(throws<tree::FactoryError>([&]{other.visitProductsFrom(mark,[](const mml::MmlNode&){});}));
    factory.retireProductsFrom(mark);
    expect(factory.productCountForDiagnostics()==0u);
    expect(throws<tree::FactoryError>([&]{factory.retireProductsFrom(mark);}));
};
};}
