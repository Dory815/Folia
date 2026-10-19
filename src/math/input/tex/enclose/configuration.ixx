// MathJax 4.1.3 input/tex/enclose/EncloseConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.enclose.configuration;
export import folia.math.input.tex.parser;
export import folia.math.core.tree.property;
import std;
export namespace folia::math::tex {
struct EncloseConfiguration {
    static ConfigurationRef initializeModule(ParserRuntime&);
    // The exported ENCLOSE_OPTIONS object is shared by CancelMethods and
    // EncloseMethods in Source. Keeping one PropertyStorage preserves edits
    // and identity instead of recreating an allowlist per invocation.
    static const Property& options();
};
}
