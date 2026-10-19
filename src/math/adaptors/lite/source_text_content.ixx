// Context-backed Source method body for LiteAdaptor.textContent.
export module folia.math.adaptors.lite.source_text_content;
export import folia.math.util.option_types;
import std;

export namespace folia::math::adaptors::lite {
options::Value sourceTextContent(options::Context& context,
                                options::Value receiver,
                                options::Value node);
}
