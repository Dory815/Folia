// MathJax 4.1.3 TexError.ts declarations; Apache-2.0.
// Native throwing/catching preserves this dedicated plain Source class identity.
export module folia.math.input.tex.tex_error;
export import folia.math.core.tree.property;
import std;
export namespace folia::math::tex {
class TexError {
    static std::u16string processString(std::u16string_view,std::span<const Property>);
public:
    std::u16string id;
    std::u16string message;
    TexError(std::u16string id,std::u16string_view message,std::span<const Property> rest={});
};
}
