// MathJax 4.1.3 input/tex/setoptions/SetOptionsConfiguration.ts; Apache-2.0.
module folia.math.input.tex.setoptions.set_options_configuration;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.parse_util;
import folia.math.util.js_numeric_input;
import std;
namespace folia::math::tex {
namespace {
// This finite visitor addresses the input-jax's existing sparse values directly.
// A matched but absent optional is distinct from a present false/empty value.
template <class T, class F> bool field(T &record, std::u16string_view key, F &&use) {
#define KEY(spelling, member)                                                                      \
    if (key == spelling) {                                                                         \
        use(record.member);                                                                        \
        return true;                                                                               \
    }
#define FIELD(member) KEY(u"" #member, member)
    if constexpr (std::is_same_v<T, TexOptionValues>) {
        FIELD(preFilters);
        FIELD(postFilters);
        KEY(u"FindTeX", findTeX);
        FIELD(packages);
        FIELD(maxBuffer);
        FIELD(maxTemplateSubtitutions);
        FIELD(maxMacros);
        FIELD(mathStyle);
        FIELD(formatError);
        FIELD(tags);
        FIELD(tagSide);
        FIELD(tagIndent);
        FIELD(tagAlign);
        FIELD(useLabelIds);
        FIELD(ignoreDuplicateLabels);
        FIELD(textmacros);
        FIELD(setoptions);
        FIELD(noundefined);
        FIELD(bboldx);
        FIELD(units);
        FIELD(allowTexHTML);
        FIELD(tagformat);
        FIELD(physics);
        FIELD(protectedMacros);
        FIELD(mathtools);
        FIELD(dsfont);
        FIELD(active);
        FIELD(macros);
        FIELD(environments);
        FIELD(color);
        FIELD(begingroup);
        FIELD(bbm);
        FIELD(amscd);
        FIELD(multlineWidth);
        FIELD(ams);
        FIELD(digits);
        FIELD(numberPattern);
        FIELD(initialDigit);
        FIELD(identifierPattern);
        FIELD(initialLetter);
        FIELD(baseURL);
        FIELD(internalMath);
    } else if constexpr (std::is_same_v<T, TextMacrosOptions>) {
        FIELD(packages);
    } else if constexpr (std::is_same_v<T, SetOptionsOptions>) {
        FIELD(filterPackage);
        FIELD(filterOption);
        FIELD(filterValue);
        FIELD(allowPackageDefault);
        FIELD(allowOptionsDefault);
        FIELD(allowOptions);
    } else if constexpr (std::is_same_v<T, NoUndefinedOptions>) {
        FIELD(color);
        FIELD(background);
        FIELD(size);
    } else if constexpr (std::is_same_v<T, BboldxOptions>) {
        FIELD(bfbb);
        FIELD(light);
    } else if constexpr (std::is_same_v<T, UnitsOptions>) {
        FIELD(loose);
        FIELD(ugly);
    } else if constexpr (std::is_same_v<T, TagFormatOptions>) {
        FIELD(number);
        FIELD(tag);
        FIELD(ref);
        FIELD(id);
        FIELD(url);
    } else if constexpr (std::is_same_v<T, PhysicsOptions>) {
        FIELD(italicdiff);
        FIELD(arrowdel);
    } else if constexpr (std::is_same_v<T, MathtoolsOptions>) {
        KEY(u"multlined-gap", multlinedGap);
        KEY(u"multlined-pos", multlinedPos);
        KEY(u"multlined-width", multlinedWidth);
        KEY(u"firstline-afterskip", firstlineAfterskip);
        KEY(u"lastline-preskip", lastlinePreskip);
        KEY(u"smallmatrix-align", smallmatrixAlign);
        KEY(u"shortvdotsadjustabove", shortvdotsAdjustAbove);
        KEY(u"shortvdotsadjustbelow", shortvdotsAdjustBelow);
        FIELD(centercolon);
        KEY(u"centercolon-offset", centercolonOffset);
        KEY(u"thincolon-dx", thincolonDx);
        KEY(u"thincolon-dw", thincolonDw);
        KEY(u"use-unicode", useUnicode);
        FIELD(legacycolonsymbols);
        KEY(u"prescript-sub-format", prescriptSubFormat);
        KEY(u"prescript-sup-format", prescriptSupFormat);
        KEY(u"prescript-arg-format", prescriptArgFormat);
        KEY(u"allow-mathtoolsset", allowMathtoolsset);
        FIELD(pairedDelimiters);
        FIELD(tagforms);
    } else if constexpr (std::is_same_v<T, DsfontOptions>) {
        FIELD(sans);
    } else if constexpr (std::is_same_v<T, ColorOptions>) {
        FIELD(padding);
        FIELD(borderWidth);
    } else if constexpr (std::is_same_v<T, BegingroupOptions>) {
        FIELD(allowGlobal);
    } else if constexpr (std::is_same_v<T, BbmOptions>) {
        FIELD(bold);
    } else if constexpr (std::is_same_v<T, AmscdOptions>) {
        FIELD(colspace);
        FIELD(rowspace);
        FIELD(harrowsize);
        FIELD(varrowsize);
        FIELD(hideHorizontalLabels);
    } else if constexpr (std::is_same_v<T, AmsOptions>) {
        FIELD(operatornamePattern);
        FIELD(multlineWidth);
        FIELD(multlineIndent);
    }
#undef FIELD
#undef KEY
    return false;
}
template <class F>
bool option(TexOptionValues &values, std::u16string_view package, std::u16string_view key,
            F &&use) {
    if (package == u"tex")
        return field(values, key, use);
#define PACKAGE(member)                                                                            \
    if (package == u"" #member)                                                                    \
        return values.member && field(*values.member, key, use);
    PACKAGE(textmacros);
    PACKAGE(setoptions);
    PACKAGE(noundefined);
    PACKAGE(bboldx);
    PACKAGE(units);
    PACKAGE(tagformat);
    PACKAGE(physics);
    PACKAGE(mathtools);
    PACKAGE(dsfont);
    PACKAGE(color);
    PACKAGE(begingroup);
    PACKAGE(bbm);
    PACKAGE(amscd);
    PACKAGE(ams);
#undef PACKAGE
    return false;
}
SetOptionsOptions &settings(TexParser &parser) {
    if (!parser.options().setoptions)
        throw std::logic_error("Setoptions is not configured");
    return *parser.options().setoptions;
}
[[noreturn]] void invalidValue(std::u16string_view key) {
    throw TexError("InvalidValue", u"Invalid value for option %1", std::array{std::u16string(key)});
}
bool truthy(OptionInputValue const &value) {
    if (auto b = std::get_if<bool>(&value))
        return *b;
    if (auto s = std::get_if<std::u16string>(&value))
        return !s->empty();
    return true;
}
std::optional<std::uint32_t> integerKey(std::u16string_view key) {
    if (key.empty() || (key.size() > 1 && key.front() == u'0'))
        return {};
    std::uint64_t index{};
    for (auto c : key) {
        if (c < u'0' || c > u'9')
            return {};
        index = index * 10 + c - u'0';
        if (index >= 4294967295ULL)
            return {};
    }
    return static_cast<std::uint32_t>(index);
}
template <class T>
void assign(std::optional<T> &target, OptionInputValue const &value, std::u16string_view package,
            std::u16string_view key) {
    if (auto regexp = std::get_if<regexp_host::RegExpRef>(&value); regexp && !*regexp)
        invalidValue(key);
    if constexpr (std::is_same_v<T, bool>)
        target = truthy(value);
    else if constexpr (std::is_same_v<T, std::u16string>) {
        if (auto s = std::get_if<std::u16string>(&value))
            target = *s;
        else if (package == u"noundefined" && std::holds_alternative<bool>(value) &&
                 !std::get<bool>(value))
            target = u"";
        else
            invalidValue(key);
    } else if constexpr (std::is_same_v<T, double>) {
        if (auto b = std::get_if<bool>(&value))
            target = *b ? 1.0 : 0.0;
        else if (auto s = std::get_if<std::u16string>(&value))
            target = js_input::number(*s);
        else
            target = std::numeric_limits<double>::quiet_NaN();
    } else if constexpr (std::is_same_v<T, regexp_host::RegExpRef>) {
        if (auto r = std::get_if<regexp_host::RegExpRef>(&value); r && *r)
            target = *r;
        else
            invalidValue(key);
    } else if constexpr (std::is_same_v<T, std::variant<std::u16string, regexp_host::RegExpRef>>) {
        if (auto s = std::get_if<std::u16string>(&value))
            target = *s;
        else if (auto r = std::get_if<regexp_host::RegExpRef>(&value); r && *r)
            target = *r;
        else
            invalidValue(key);
    } else if constexpr (std::is_same_v<T, TagTextFormatter>) {
        const auto boolean = std::get_if<bool>(&value);
        const auto text = std::get_if<std::u16string>(&value);
        if (package == u"tagformat" && key == u"ref" &&
            ((boolean && !*boolean) || (text && text->empty())))
            target = TagTextFormatter{};
        else
            invalidValue(key);
    } else
        invalidValue(key); // Containers and compiled callbacks retain their declared types.
}
} // namespace
bool SetOptionsUtil::filterPackage(TexParser &parser, std::u16string_view package) {
    if (package != u"tex" && !parser.configuration.registry().get(package))
        throw TexError("NotAPackage", u"Not a defined package: %1",
                       std::array{std::u16string(package)});
    auto &config = settings(parser);
    SetOptionsOptions::Permission const *permission{};
    if (config.allowOptions) {
        auto found = config.allowOptions->find(std::u16string(package));
        if (found != config.allowOptions->end())
            permission = &found->second;
    }
    if ((!permission && !config.allowPackageDefault.value_or(false)) ||
        (permission && std::holds_alternative<bool>(*permission) && !std::get<bool>(*permission)))
        throw TexError("PackageNotSettable", u"Options can't be set for package \"%1\"",
                       std::array{std::u16string(package)});
    return true;
}
bool SetOptionsUtil::filterOption(TexParser &parser, std::u16string_view package,
                                  std::u16string_view key) {
    auto &config = settings(parser);
    std::optional<bool> allow;
    if (config.allowOptions) {
        auto found = config.allowOptions->find(std::u16string(package));
        if (found != config.allowOptions->end())
            if (auto fields =
                    std::get_if<std::unordered_map<std::u16string, bool>>(&found->second)) {
                auto entry = fields->find(std::u16string(key));
                if (entry != fields->end())
                    allow = entry->second;
            }
    }
    if (allow == false || (!allow && !config.allowOptionsDefault.value_or(false))) {
        if (package == u"tex")
            throw TexError("TeXOptionNotSettable", u"Option \"%1\" is not allowed to be set",
                           std::array{std::u16string(key)});
        throw TexError("OptionNotSettable",
                       u"Option \"%1\" is not allowed to be set for package %2",
                       std::array{std::u16string(key), std::u16string(package)});
    }
    bool present{};
    option(parser.options(), package, key, [&](auto &value) { present = value.has_value(); });
    if (!present) {
        if (package == u"tex")
            throw TexError("InvalidTexOption", u"Invalid TeX option \"%1\"",
                           std::array{std::u16string(key)});
        throw TexError("InvalidOptionKey", u"Invalid option \"%1\" for package \"%2\"",
                       std::array{std::u16string(key), std::u16string(package)});
    }
    return true;
}
OptionInputValue SetOptionsUtil::filterValue(TexParser &, std::u16string_view, std::u16string_view,
                                             OptionInputValue const &value) {
    auto source = std::get_if<std::u16string>(&value);
    if (!source)
        return value;
    regexp_host::RegExp matcher(u"^/(.*)/([dgimsuvy]*)$");
    auto match = matcher.exec(*source);
    return match ? OptionInputValue{std::make_shared<regexp_host::RegExp>(*match->at(1),
                                                                          *match->at(2))}
                 : value;
}
DispatchOutcome SetOptions(TexParser &parser, MacroInvocation const &invocation) {
    auto package = parser.GetBrackets(invocation.dispatch_token).value_or(u"");
    if (package.empty())
        package = u"tex";
    auto options = parse_util::keyvalOptions(parser.GetArgument(invocation.dispatch_token).value());
    auto &config = settings(parser);
    if (!config.filterPackage || !*config.filterPackage)
        throw std::logic_error("Setoptions package filter is not configured");
    if (!(*config.filterPackage)(parser, package))
        return NoParseResult{};
    std::vector<mml::AttributeSet::Entry const *> entries;
    for (auto const &entry : options.ownEntries())
        entries.push_back(&entry);
    // Source Object.keys visits canonical integer keys first, then insertion order.
    std::stable_sort(entries.begin(), entries.end(), [](auto a, auto b) {
        auto left = integerKey(a->first), right = integerKey(b->first);
        return left && (!right || *left < *right);
    });
    for (auto entry : entries) {
        auto const &[key, raw] = *entry;
        if (!config.filterOption || !*config.filterOption)
            throw std::logic_error("Setoptions option filter is not configured");
        if (!(*config.filterOption)(parser, package, key))
            continue;
        if (!config.filterValue || !*config.filterValue)
            throw std::logic_error("Setoptions value filter is not configured");
        OptionInputValue input;
        if (auto b = std::get_if<bool>(&raw))
            input = *b;
        else
            input = std::get<std::u16string>(raw);
        auto value = (*config.filterValue)(parser, package, key, input);
        bool found = option(parser.options(), package, key,
                            [&](auto &target) { assign(target, value, package, key); });
        if (!found)
            invalidValue(key);
    }
    parser.Push(parser.itemFactory().create(StackKind::null_item));
    return NoParseResult{};
}
void setoptionsConfig(ParserConfiguration &config, TeX &jax) {
    auto map = std::make_shared<CommandMap>(CommandMapInit{
        MacroMapInit{{u"setoptions", std::nullopt}, {Macro(u"setOptions", &SetOptions)}}});
    config.maps().registerMap(map);
    auto macros = jax.parseOptions().handlers.get(HandlerKind::macro);
    const std::array<std::u16string, 1> names{u"setoptions"};
    macros->add(names, nullptr, 4);
    auto lookup = macros->lookup(u"require");
    if (auto require = std::get_if<Macro const *>(&lookup); require && *require) {
        map->add(Macro(u"Require", (*require)->function));
        map->add(
            Macro(u"require", &BaseMethods::Macro,
                  {std::u16string(u"\\Require{#2}\\setOptions[#2]{#1}"), 2.0, std::u16string{}}));
    }
}
ConfigurationRef SetOptionsConfiguration::initializeModule(ConfigurationHandler &registry) {
    ConfigurationDefinition definition;
    definition.priority = 3;
    definition.config = ProcessorSpec<ConfigMethod>{&setoptionsConfig};
    SetOptionsOptions options;
    options.filterPackage = &SetOptionsUtil::filterPackage;
    options.filterOption = &SetOptionsUtil::filterOption;
    options.filterValue = &SetOptionsUtil::filterValue;
    options.allowPackageDefault = true;
    options.allowOptionsDefault = true;
    options.allowOptions = std::unordered_map<std::u16string, SetOptionsOptions::Permission>{
        {u"tex", std::unordered_map<std::u16string, bool>{{u"FindTeX", false},
                                                          {u"formatError", false},
                                                          {u"package", false},
                                                          {u"baseURL", false},
                                                          {u"tags", false},
                                                          {u"maxBuffer", false},
                                                          {u"maxMaxros", false},
                                                          {u"macros", false},
                                                          {u"environments", false}}},
        {u"setoptions", false},
        {u"autoload", false},
        {u"require", false},
        {u"configmacros", false},
        {u"tagformat", false}};
    definition.options.setoptions = std::move(options);
    return Configuration::create(registry, u"setoptions", std::move(definition));
}
} // namespace folia::math::tex
