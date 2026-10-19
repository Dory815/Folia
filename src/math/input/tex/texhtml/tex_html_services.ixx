// Per-parse references to the active document's canonical Lite DOM services.
export module folia.math.input.tex.texhtml.tex_html_services;
export import folia.math.adaptors.lite_adaptor;
export import folia.math.core.mml_tree.xml_dom_adaptor;
import std;
export namespace folia::math::tex {
using TexHtmlAdaptorResolver=std::function<
    std::shared_ptr<folia::math::AbstractDOMAdaptor>(folia::math::options::ObjectRef)>;
struct TexHtmlServices {
    std::shared_ptr<folia::math::AbstractDOMAdaptor> dom;
    std::shared_ptr<folia::math::mml::XmlDomAdaptor> xml;
    folia::math::options::ObjectRef identity;
    folia::math::options::Value childNodesMethod;
    folia::math::options::Value bodyMethod;
    folia::math::options::Value parseMethod;
};
TexHtmlServices resolveTexHtmlServices(folia::math::options::Context&,
    folia::math::options::Value adaptor,const TexHtmlAdaptorResolver&);
}
