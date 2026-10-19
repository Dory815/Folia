#include "../../tests/support/folia_test.hpp"
import folia.math.core.mml_tree.xml_dom_adaptor;
import folia.math.core.mml_tree.mml_factory;
import folia.math.adaptors.lite.lite_module;

namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::adaptors::lite;
namespace opts=folia::math::options;

suite xmlDomOwnership=[] {
    "xml_dom_projection_preserves_node_identity_and_cross_adaptor_mutations"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        LiteModule module(context,functions,properties,regexp);
        auto left=module.constructAdaptor();
        auto right=module.constructAdaptor();
        auto first=mml::XmlDomAdaptor::create(left);
        auto second=mml::XmlDomAdaptor::create(right);
        const auto text=left->text(u"x&漢");
        const auto node=left->node(u"span",{},PropertyArray{nodeProperty(text)});
        const auto xml=first->fromDOM(node);
        expect(first->fromDOM(node)==xml);
        expect(first->resolve(xml)==node);
        expect(second->resolve(xml)==node);
        expect(properties.to(first->nodeProperty(xml))==opts::Value(module.objectFor(node)));
        right->setAttribute(node,u"data-live",Property("a&b"));
        expect(first->getAttribute(xml,"data-live")==Property("a&b"));
        const bool sameText=first->textContent(xml)==std::u16string(u"x&漢");
        expect(sameText);
        const auto clone=second->clone(xml);
        expect(second->resolve(clone)!=node);
        expect(second->resolve(clone).store==node.store);
        expect(second->outerHTML(clone)==first->outerHTML(xml));
        right->setAttribute(node,u"data-live",Property("changed"));
        expect(second->getAttribute(clone,"data-live")==Property("a&b"));
        expect(first->getAttribute(xml,"data-live")==Property("changed"));
    };
    "mml_xml_handle_retains_the_real_adaptor_for_serialization_and_copy"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        LiteModule module(context,functions,properties,regexp);
        mml::MmlFactory factory;
        auto* xml=dynamic_cast<mml::XMLNode*>(factory.create("XML",tree::FactoryArgs{}));
        expect(xml!=nullptr);
        if(!xml)return;
        std::weak_ptr<mml::XmlDomAdaptor> bridgeLease;
        std::string expected;
        {
            auto adaptor=module.constructAdaptor();
            auto bridge=mml::XmlDomAdaptor::create(adaptor);
            bridgeLease=bridge;
            const auto text=adaptor->text(u"x&y");
            const auto node=adaptor->node(u"span",{},PropertyArray{nodeProperty(text)});
            xml->setXML(bridge->fromDOM(node),bridge.get());
            expected=bridge->serializeXML(xml->getXML());
        }
        expect(!bridgeLease.expired());
        expect(xml->getSerializedXML()==expected);
        auto bridge=bridgeLease.lock();
        auto* copy=dynamic_cast<mml::XMLNode*>(xml->copy());
        expect(copy!=nullptr);
        if(copy){
            expect(copy->getXML()!=xml->getXML());
            expect(bridge->resolve(copy->getXML())!=bridge->resolve(xml->getXML()));
            expect(bridge->serializeXML(copy->getXML())==expected);
        }
        bridge.reset();
        xml->setXML({});
        if(copy)copy->setXML({});
        expect(bridgeLease.expired());
    };
};
}
