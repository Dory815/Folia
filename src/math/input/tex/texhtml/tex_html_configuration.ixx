// MathJax 4.1.3 input/tex/texhtml/TexHtmlConfiguration.ts.
export module folia.math.input.tex.texhtml.tex_html_configuration;
export import folia.math.input.tex.parser;
export import folia.math.handlers.html.html_dom_strings;
import std;

export namespace folia::math::tex {
struct TexHtmlConfiguration {
    static ConfigurationRef initializeModule(
        ConfigurationHandler &, MapHandler &,
        std::shared_ptr<handlers::html::HTMLDomStringsOptions>);
};

struct HtmlNodeMethods {
    static DispatchOutcome TexHTML(TexParser &, MacroInvocation const &);
};
} // namespace folia::math::tex
