// MathJax 4.1.3 TexError.ts semantic methods; Apache-2.0.
module folia.math.input.tex.tex_error;
import folia.math.core.tree.factory_types;
import folia.math.input.tex.tex_error_pattern;
import folia.math.util.property_host;
import folia.math.util.js_numeric_input;
import std;
namespace folia::math::tex {
TexError::TexError(std::u16string,std::u16string_view,std::span<const Property>) {
    throw tree::FactoryError("Unimplemented",u"Root-frozen TexError constructor");
}
std::u16string TexError::processString(std::u16string_view,std::span<const Property>) {
    throw tree::FactoryError("Unimplemented",u"Root-frozen TexError processing");
}
}
