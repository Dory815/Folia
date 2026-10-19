/*************************************************************
 * SVG session assembly for the actual handlers/html/HTMLMathDocument.
 * Conversion and MathItem behavior live in their Source-matched owners.
 *************************************************************/
export module folia.math.output.svg.native_document;
export import folia.math.handlers.html.html_math_document;
export import folia.math.output.svg.render_session;
import std;

export namespace folia::math::output::svg {
std::shared_ptr<handlers::html::HTMLMathDocument> createNativeSvgDocument(
    NativeRenderSession&,options::Value documentOptions=options::Undefined{},
    options::Value inputJaxOptions=options::Undefined{});
}
