// MathJax 4.1.3 input/tex/mathtools/MathtoolsTags.ts.
export module folia.math.input.tex.mathtools.tags;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
// Source's TagFormat subclass is dynamic over the selected Tags constructor.
// This owner installs that one child class and operates on its live Context
// fields; it does not substitute a C++ tags implementation for the parent.
struct MathtoolsTags {
    static Property moduleExports(ParserRuntime&);
    static void registerTagFormat(ParserRuntime&, ParserConfiguration&, TeX&,
                                  const std::shared_ptr<std::uint64_t>& tagId);
    static TagsClassRef initializeModule(ParserRuntime&, options::ObjectRef parentConstructor,
                                         options::Value forms, std::uint64_t tagId,
                                         std::u16string* className=nullptr);
    static bool hasFormat(ParserRuntime&, Tags&, std::u16string_view);
    static options::Value format(ParserRuntime&, Tags&, std::u16string_view);
    static void setFormat(ParserRuntime&, Tags&, std::u16string_view, options::Value);
    static options::Value current(ParserRuntime&, Tags&);
    static void setCurrent(ParserRuntime&, Tags&, options::Value);
};
}
