/*************************************************************
 * Materialize a completed Source-unit scene through the actual DOM adaptor.
 * This unit serializes; it does not calculate placement or wrapper geometry.
 *************************************************************/
export module folia.math.output.svg.serializer;
export import folia.math.output.scene;
export import folia.math.core.dom_adaptor;
import std;

export namespace folia::math::output::svg {
inline constexpr std::u16string_view SVG_NAMESPACE=u"http://www.w3.org/2000/svg";
inline constexpr std::u16string_view XLINK_NAMESPACE=u"http://www.w3.org/1999/xlink";
class Serializer {
    folia::math::AbstractDOMAdaptor& adaptor_;
    folia::math::DOMNodeRef materialize(const folia::math::output::scene::FrozenNodeRef&);
    folia::math::DOMNodeRef materializeHtml(const folia::math::output::scene::HtmlNodePrimitive&);
    PropertyList attributes(const folia::math::output::scene::FrozenNode&);
public:
    using MaterializationObserver=std::function<void()>;
    explicit Serializer(folia::math::AbstractDOMAdaptor& adaptor):adaptor_(adaptor){}
    static MaterializationObserver exchangeMaterializationObserver(MaterializationObserver);
    std::vector<folia::math::DOMNodeRef> materialize(const folia::math::output::scene::Scene&);
    std::u16string serialize(const folia::math::DOMNodeRef& root)const;
};
}
