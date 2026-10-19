// MathJax 4.1.3 input/tex/color/ColorUtil.ts.
export module folia.math.input.tex.color.util;
export import folia.math.input.tex.tex_error;
export import folia.math.input.tex.parser;
export import folia.math.util.map_bindings;
import std;
export namespace folia::math::tex {
struct ColorUtilModule { options::ObjectRef constructor; options::ObjectRef processors; options::ObjectRef colors; std::shared_ptr<options::MapContextBindings> maps; };
struct ColorUtil { static ColorUtilModule initializeModule(ParserRuntime&,std::shared_ptr<options::MapContextBindings>,options::ObjectRef colors); };
class ColorModel {
public:
    std::u16string normalizeColor(std::u16string_view model,std::u16string_view definition)const;
};
}
