// Native lifetime/binding owner for one loaded Source module environment.
// This owner provides no TeX, configuration, tag or layout algorithms.
export module folia.math.input.tex.parser:runtime;
export import :configuration;
export import :tags;
export import folia.math.core.input_jax_host;
export import folia.math.input.tex.find_tex_host;
import std;
export namespace folia::math::tex {
struct ColumnState;
class ParserRuntime {
    options::Context& context_;
    OptionFunctionBindings& functions_;
    regexp_host::RegExpBindings& regexp_;
    UnicodeNormalization& unicode_;
    core::InputJaxModule& inputJaxModule_;
    FindTeXModule& findTeXModule_;
public:
    ConfigurationHandler configurationHandler;
    TagsFactory tagsFactory;
    SourceStringMap<options::Value> variantMap; // actual ParseOptions class Map
    options::Value KeyValueTypes=options::Undefined{};
    options::Value texClass=options::Undefined{};
    options::Value configurationClass=options::Undefined{},parserConfigurationClass=options::Undefined{};
    options::Value parseOptionsClass=options::Undefined{},texParserClass=options::Undefined{};
    options::Value labelClass=options::Undefined{},tagInfoClass=options::Undefined{};
    options::Value columnParserClass=options::Undefined{},keyValueClass=options::Undefined{};
    NativeIdentityBindings<ConfigurationRef> configurationIdentities;
    NativeIdentityBindings<ParserConfigurationRef> parserConfigurationIdentities;
    NativeIdentityBindings<ParseOptionsRef> parseOptionsIdentities;
    NativeIdentityBindings<TexParserRef> parserIdentities;
    NativeIdentityBindings<TagsRef> tagIdentities;
    NativeIdentityBindings<TagsClassRef> tagClassIdentities;
    NativeIdentityBindings<TeXRef> texIdentities;
    NativeIdentityBindings<KeyValueType> keyValueIdentities;
    NativeIdentityBindings<LabelRef> labelIdentities;
    NativeIdentityBindings<TagInfoRef> tagInfoIdentities;
    NativeIdentityBindings<mml::Children> nodeArrayIdentities;
    NativeIdentityBindings<StackItemClassRef> itemClassIdentities;
    NativeIdentityBindings<std::shared_ptr<ColumnState>> columnStateIdentities;
    ParserRuntime(options::Context&,OptionFunctionBindings&,regexp_host::RegExpBindings&,UnicodeNormalization&,
        core::InputJaxModule&,FindTeXModule&);
    ParserRuntime(const ParserRuntime&)=delete;
    ParserRuntime&operator=(const ParserRuntime&)=delete;
    options::Context& context()const;
    OptionFunctionBindings& functions()const;
    regexp_host::RegExpBindings& regexp()const;
    UnicodeNormalization& unicode()const;
    core::InputJaxModule& inputJaxModule()const;
    FindTeXModule& findTeXModule()const;
};
}
