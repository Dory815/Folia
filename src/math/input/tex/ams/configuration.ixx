// MathJax 4.1.3 input/tex/ams/AmsConfiguration.ts; Apache-2.0.
export module folia.math.input.tex.ams.configuration;
export import folia.math.input.tex.ams.items;
export import folia.math.input.tex.ams.mappings;
export namespace folia::math::tex {
class AmsTags : public AbstractTags {
  public:
    using AbstractTags::AbstractTags;
};
struct AmsConfiguration {
    static ConfigurationRef initializeModule(ConfigurationHandler &, MapHandler &);
};
} // namespace folia::math::tex
