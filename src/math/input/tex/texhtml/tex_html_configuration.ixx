// MathJax 4.1.3 input/tex/texhtml/TexHtmlConfiguration.ts.
export module folia.math.input.tex.texhtml.tex_html_configuration;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.texhtml.tex_html_services;
import std;
import folia.math.util.runtime_modules;
export namespace folia::math::tex {
struct TexHtmlConfiguration {
    static ConfigurationRef initializeModule(ParserRuntime&,util::RuntimeModules&,
        TexHtmlAdaptorResolver);
};
}
