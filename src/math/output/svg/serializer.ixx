/*************************************************************
 * Materialize owned HTML values in the Lite DOM and serialize as XML.
 * This narrow boundary does not calculate placement or retain a DOM.
 *************************************************************/
export module folia.math.output.svg.serializer;
export import folia.math.output.scene.html_types;
import std;

export namespace folia::math::output::svg {
[[nodiscard]] std::string
serializeHtml(folia::math::output::scene::HtmlNodePrimitive const &source);
}
