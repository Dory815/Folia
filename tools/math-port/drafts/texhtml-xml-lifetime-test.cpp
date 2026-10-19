#include "../../../tests/support/folia_test.hpp"
import folia.math.adaptors.lite_adaptor;
import folia.math.core.dom_adaptor;
import folia.math.core.mml_tree.xml_dom_adaptor;

namespace {
using namespace boost::ut;
using namespace folia::math;
using folia::math::adaptors::lite::LiteAdaptor;
using folia::math::mml::XmlDomAdaptor;

suite texhtmlXmlLifetime=[] {
"retains_the_actual_dom_adaptor_owner"_test=[] {
    auto adaptor=std::make_shared<LiteAdaptor>();
    std::weak_ptr<AbstractDOMAdaptor> weakAdaptor=adaptor;
    const auto parsed=adaptor->parse(u"<b data-probe=\"x\">content</b>",u"text/html");
    auto bridge=XmlDomAdaptor::create(adaptor);
    std::weak_ptr<XmlDomAdaptor> weakBridge=bridge;
    auto html=bridge->fromDOM(parsed);

    adaptor.reset();
    bridge.reset();
    expect(!weakAdaptor.expired()) << "XML reference retains adaptor owner";
    expect(!weakBridge.expired()) << "XML reference retains bridge";
    auto retainedBridge=weakBridge.lock();
    expect(retainedBridge!=nullptr) << "bridge remains lockable";
    expect(retainedBridge->resolve(html)==parsed) << "bridge resolves the original DOM node";
    expect(retainedBridge->domAdaptor()->kind(parsed)==u"#document") << "retained adaptor remains usable";

    html.reset();
    retainedBridge.reset();
    expect(weakBridge.expired()) << "bridge releases with final XML reference";
    expect(weakAdaptor.expired()) << "adaptor releases with final XML reference";
};
};
}
