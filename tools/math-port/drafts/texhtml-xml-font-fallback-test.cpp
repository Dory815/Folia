#include "../../../tests/support/folia_test.hpp"
import folia.math.core.math_item;
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.xml_dom_adaptor;
import folia.math.output.common;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.util.option_types;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace opts=folia::math::options;
namespace core=folia::math::core;
namespace mml=folia::math::mml;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

common::FontLoadHost fontHost(){
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=[](const std::any&){};
    return host;
}

struct FontFallbackFixture {
    svg::NativeRenderSession session{fontHost()};
    opts::Context& context=session.context();
    std::shared_ptr<common::CommonOutputJax> output;
    std::shared_ptr<handlers::html::HTMLMathDocument> document;
    std::shared_ptr<adaptors::lite::LiteAdaptor> adaptor;
    std::shared_ptr<mml::XmlDomAdaptor> xmlAdaptor;
    std::shared_ptr<core::MathItem> item;
    common::CommonXmlNode* xmlWrapper=nullptr;

    FontFallbackFixture(){
        const auto outputOptions=context.object();
        context.set(outputOptions,opts::Key(u"fontCache"),opts::Value(u"none"));
        output=session.construct(opts::Value(outputOptions));
        document=svg::createNativeSvgDocument(session);
        adaptor=session.adaptor();
        auto factory=session.mathFactory();
        expect(factory!=nullptr);
        if(!factory)return;

        auto* math=factory->create("math",std::span<const std::any>{});
        auto* xml=dynamic_cast<mml::XMLNode*>(factory->create("XML",std::span<const std::any>{}));
        expect(math!=nullptr&&xml!=nullptr);
        if(!math||!xml)return;
        xmlAdaptor=mml::XmlDomAdaptor::create(adaptor);
        const auto html=adaptor->node(u"section");
        xml->setXML(xmlAdaptor->fromDOM(html),xmlAdaptor.get());
        (void)math->appendChild(mml::NodeValue(xml));

        const opts::Value documentValue(document->sourceIdentity());
        const auto documentOptions=context.get(documentValue,opts::Key(u"options"));
        const auto itemConstructor=context.get(documentOptions,opts::Key(u"MathItem"));
        const auto inputJax=context.get(context.get(documentValue,opts::Key(u"inputJax")),opts::Key(u"0"));
        const std::array<opts::Value,3> itemArguments{opts::Value(u"<xml font fallback test>"),inputJax,opts::Value(true)};
        const auto itemIdentity=context.construct(std::get<opts::ObjectRef>(itemConstructor.data),itemArguments);
        const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&context.payload(itemIdentity));
        item=payload?std::dynamic_pointer_cast<core::MathItem>(*payload):nullptr;
        expect(item!=nullptr);
        if(!item)return;
        item->root=mml::NodeValue(math);
        item->setMetrics(Property(16),Property(8),Property(640),Property(1));
        const auto start=adaptor->node(u"div");
        item->start=Property(PropertyList{{"node",nodeProperty(start)},
            {"n",Property(0)},{"delim",PropertyNull{}}});

        const auto container=adaptor->node(u"div");
        auto* commonJax=static_cast<common::CommonOutputJax*>(output.get());
        (void)commonJax->beginLayoutJob(*item,core::MathDocumentReference{document.get()},container);
        expect(commonJax->factory!=nullptr);
        if(!commonJax->factory)return;
        (void)commonJax->factory->wrap(math);
        auto* base=commonJax->nodeMap->find(xml);
        xmlWrapper=dynamic_cast<common::CommonXmlNode*>(base);
        expect(xmlWrapper!=nullptr);
        if(xmlWrapper&&xmlWrapper->parent)xmlWrapper->parent->styles.reset();
    }

    PropertyList styles(){
        if(!xmlWrapper)return {};
        const auto value=PropertyList::record(xmlWrapper->getFontStyles());
        expect(value.has_value());
        return value.value_or(PropertyList{});
    }
};

suite texhtmlXmlFontFallback=[] {
    "xml_font_family_falls_back_for_missing_and_false_metrics_and_keeps_explicit_family"_test=[] {
        FontFallbackFixture fixture;
        if(!fixture.xmlWrapper||!fixture.item)return;
        auto metrics=PropertyList::record(fixture.item->metrics);
        expect(metrics.has_value());
        if(!metrics)return;

        metrics->erase("family");
        fixture.item->metrics=Property(*metrics);
        const auto missingStyles=fixture.styles();
        expect(property_host::string(missingStyles.get("font-family"))==u"Times");
        expect(!missingStyles.get("font-size").missing());
        expect(missingStyles.keys().size()==2);

        metrics->setOwn("family",Property(false));
        fixture.item->metrics=Property(*metrics);
        const auto falseStyles=fixture.styles();
        expect(property_host::string(falseStyles.get("font-family"))==u"Times");
        expect(falseStyles.get("font-size")==missingStyles.get("font-size"));
        expect(falseStyles.keys().size()==2);

        metrics->setOwn("family",Property("ExplicitMetrics"));
        fixture.item->metrics=Property(*metrics);
        const auto explicitStyles=fixture.styles();
        expect(property_host::string(explicitStyles.get("font-family"))==u"ExplicitMetrics");
        expect(explicitStyles.get("font-size")==missingStyles.get("font-size"));
        expect(explicitStyles.keys().size()==2);
        static_cast<common::CommonOutputJax*>(fixture.output.get())->finishLayoutJob();
    };
};
} // namespace
