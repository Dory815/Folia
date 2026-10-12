#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_nodes.mi;
import folia.math.core.mml_tree.mml_node;

namespace {
using namespace boost::ut;
namespace m=folia::math::mml;
namespace t=folia::math::tree;
struct FirstMi:m::MmlMi{using m::MmlMi::MmlMi;};
struct SecondMi:m::MmlMi{using m::MmlMi::MmlMi;};
suite mml_constructor_identity=[] {
    "mml_constructor_identity_survives_registry_replacement_and_deletion"_test=[] {
        const auto original=m::classBinding<FirstMi>();
        const auto replacement=m::classBinding<SecondMi>();
        m::MmlFactory factory;
        factory.setNodeClass("mi",original);
        auto* first=factory.create("mi");
        expect(first->nativeConstructorClass()==original);
        factory.setNodeClass("mi",replacement);
        auto* second=factory.create("mi");
        expect(first->nativeConstructorClass()==original);
        expect(second->nativeConstructorClass()==replacement);
        expect(first->nativeConstructorClass()!=second->nativeConstructorClass());
        factory.deleteNodeClass("mi");
        expect(first->nativeConstructorClass()==original);
        expect(second->nativeConstructorClass()==replacement);
    };
    "mml_constructor_identity_retains_the_actual_descriptor_lifetime"_test=[] {
        auto descriptor=m::classBinding<m::MmlMi>();
        const std::weak_ptr<t::FactoryClass<m::MmlNode>> weak=descriptor;
        m::MmlFactory factory;
        factory.setNodeClass("mi",descriptor);
        auto* node=factory.create("mi");
        descriptor.reset();
        factory.deleteNodeClass("mi");
        expect(!weak.expired());
        expect(node->nativeConstructorClass()==weak.lock());
    };
    "mml_constructor_identity_is_bound_before_virtual_initialization"_test=[] {
        struct ObservedMi final:m::MmlMi {
            using m::MmlMi::MmlMi;
            bool observed=false;
            void initializeClassFields()override{observed=static_cast<bool>(nativeConstructorClass());}
        };
        const auto descriptor=m::classBinding<ObservedMi>();
        m::MmlFactory factory;
        factory.setNodeClass("mi",descriptor);
        auto* node=dynamic_cast<ObservedMi*>(factory.create("mi"));
        expect(node&&node->observed);
        expect(node&&node->nativeConstructorClass()==descriptor);
    };
    "mml_constructor_identity_cannot_be_rebound_to_a_peer_class"_test=[] {
        const auto descriptor=m::classBinding<m::MmlMi>();
        const auto peer=m::classBinding<m::MmlMi>();
        m::MmlFactory factory;
        factory.setNodeClass("mi",descriptor);
        auto* node=factory.create("mi");
        node->bindNativeConstructorClass(descriptor);
        bool rejected=false;
        try{node->bindNativeConstructorClass(peer);}catch(const m::Error& error){rejected=error.name=="TypeError";}
        expect(rejected&&node->nativeConstructorClass()==descriptor);
    };
    "mml_constructor_identity_includes_the_actual_empty_text_branch"_test=[] {
        const auto descriptor=m::classBinding<m::TextNode>();
        m::MmlFactory factory;
        factory.setNodeClass("text",descriptor);
        auto* node=factory.create("text");
        expect(node->nativeConstructorClass()==descriptor);
    };
    "mml_constructor_identity_saved_creator_retains_original_class"_test=[] {
        auto descriptor=m::classBinding<FirstMi>();
        const std::weak_ptr<t::FactoryClass<m::MmlNode>> weak=descriptor;
        m::MmlFactory factory;
        factory.setNodeClass("mi",descriptor);
        const auto creators=factory.MML();
        const auto entry=std::ranges::find_if(creators->entries,[](const auto& pair){return pair.first=="mi";});
        const auto saved=entry->second;
        factory.setNodeClass("mi",m::classBinding<SecondMi>());
        descriptor.reset();
        expect(!weak.expired());
        auto* node=saved->call({});
        expect(node->nativeConstructorClass()==weak.lock());
        expect(dynamic_cast<FirstMi*>(node)!=nullptr);
        factory.deleteNodeClass("mi");
        bool rejected=false;
        try{(void)saved->call({});}catch(const m::Error& error){rejected=error.name=="TypeError";}
        expect(rejected);
    };
};
}
