// MathJax 4.1.3 input/tex/mhchem/MhchemConfiguration.ts.
export module folia.math.input.tex.mhchem.configuration;
export import folia.math.input.tex.parser;
export import folia.math.util.runtime_modules;
import std;
export namespace folia::math::tex {
struct MhchemReplacement {
    std::u16string pattern;
    std::variant<std::u16string, FunctionRef> replacement;
};
// One actual loaded MhchemConfiguration.ts module environment. The maps retain
// its original callable references; exported records remain independent owners.
class MhchemModule {
    ParserRuntime& runtime_;
    Property methods_, utils_;
    TokenMapRef macros_, characters_;
    ConfigurationRef configuration_;
    std::array<MhchemReplacement, 6> rules_;
    explicit MhchemModule(ParserRuntime&);
    void initialize(const std::shared_ptr<MhchemModule>&);
public:
    static std::shared_ptr<MhchemModule> initializeModule(ParserRuntime&, util::RuntimeModules&);
    options::Context& context() const;
    Property methods() const { return methods_; }
    Property utils() const { return utils_; }
    Property method(std::u16string_view name) const;
    ParseMethod relmo() const;
    TokenMapRef macros() const { return macros_; }
    TokenMapRef characters() const { return characters_; }
    ConfigurationRef configuration() const { return configuration_; }
    std::span<const MhchemReplacement> replacementRules() const { return rules_; }
    std::u16string replacement(std::u16string tex, std::size_t index) const;
    std::u16string replacements(std::u16string tex) const;
};
struct MhchemConfiguration {
    static ConfigurationRef initializeModule(ParserRuntime&, util::RuntimeModules&);
};
struct MhchemMethods {
    static Property get(ParserRuntime&, util::RuntimeModules&, std::u16string_view name);
};
struct MhchemUtils {
    static ParseMethod relmo(ParserRuntime&, util::RuntimeModules&);
};
}
