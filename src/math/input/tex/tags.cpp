// MathJax 4.1.3 Tags.ts; Apache-2.0.
module folia.math.input.tex.parser;
import :tex_parser;
import folia.math.util.utf16;
import std;
namespace folia::math::tex {
AbstractTags::AbstractTags(ParseOptions &owner) noexcept : configuration_(owner) {}
AbstractTags::~AbstractTags() = default;
ParseOptions &AbstractTags::configuration() const noexcept {
    return configuration_;
}
std::string_view AbstractTags::env() const noexcept {
    return configuration_.equation.current_tag.environment;
}
std::u16string_view AbstractTags::label() const noexcept {
    return configuration_.equation.current_tag.label_id;
}
void AbstractTags::setLabel(std::u16string label) {
    configuration_.equation.current_tag.label_id = std::move(label);
}
namespace {
TagFormatValue defaultTag(std::u16string_view value) {
    return std::vector<std::u16string>{u"(", std::u16string(value), u")"};
}
std::u16string defaultUrl(std::u16string_view id, std::u16string_view base) {
    for (std::size_t i = 0; i < id.size(); ++i) {
        if (id[i] >= 0xD800 && id[i] <= 0xDBFF) {
            if (i + 1 >= id.size() || id[i + 1] < 0xDC00 || id[i + 1] > 0xDFFF)
                throw std::invalid_argument("URI contains an isolated surrogate");
            ++i;
        } else if (id[i] >= 0xDC00 && id[i] <= 0xDFFF)
            throw std::invalid_argument("URI contains an isolated surrogate");
    }
    std::u16string result(base);
    result += u'#';
    constexpr char16_t digits[] = u"0123456789ABCDEF";
    for (unsigned char byte : utf16::encode(id)) {
        if ((byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z') ||
            (byte >= '0' && byte <= '9') ||
            std::string_view("-_.!~*'()").find(static_cast<char>(byte)) != std::string_view::npos)
            result += static_cast<char16_t>(byte);
        else {
            result += u'%';
            result += digits[byte >> 4];
            result += digits[byte & 15];
        }
    }
    return result;
}
std::u16string defaultId(std::u16string_view value) {
    std::u16string text(value);
    regexp_host::RegExp whitespace(u"\\s", u"g");
    while (auto match = whitespace.exec(text))
        text[match->index] = u'_';
    return u"mjx-eqn:" + text;
}
std::u16string defaultNumber(std::int64_t number) {
    auto text = std::to_string(number);
    return {text.begin(), text.end()};
}
} // namespace
void AbstractTags::setFormatters(TexOptionValues const &options) {
    if (!options.tagformat)
        throw std::invalid_argument("Tag format options are absent");
    formatterOptions_ = &options;
    tagFormFirst_ = false;
}
void AbstractTags::setTagForm(std::unique_ptr<TagForm> form) {
    if (!form)
        throw std::invalid_argument("Tag form is absent");
    tagForm_ = std::move(form);
    tagFormFirst_ = true;
}
TagForm *AbstractTags::tagForm() noexcept {
    return tagForm_.get();
}
TagFormatOptions const &AbstractTags::formatters() const {
    if (!formatterOptions_->tagformat)
        throw std::logic_error("Tag format options were removed from the input jax");
    return *formatterOptions_->tagformat;
}
TagFormatValue AbstractTags::formatTag(std::u16string_view value) {
    if (tagForm_ && (tagFormFirst_ || !formatterOptions_)) {
        if (auto result = tagForm_->formatTag(value))
            return std::move(*result);
    }
    if (!formatterOptions_)
        return formatTagDefault(value);
    auto callback = formatters().tag.value_or(nullptr);
    if (!callback)
        throw std::logic_error("Tag formatter is absent");
    return callback(value);
}
TagFormatValue AbstractTags::formatRef(std::u16string_view value) {
    if (!formatterOptions_)
        return formatRefDefault(value);
    auto callback = formatters().ref.value_or(nullptr);
    return callback ? callback(value) : formatTag(value);
}
std::u16string AbstractTags::formatUrl(std::u16string_view id, std::u16string_view base) {
    if (!formatterOptions_)
        return formatUrlDefault(id, base);
    auto callback = formatters().url.value_or(nullptr);
    if (!callback)
        throw std::logic_error("Tag URL formatter is absent");
    return callback(id, base);
}
std::u16string AbstractTags::formatId(std::u16string_view value) {
    if (!formatterOptions_)
        return formatIdDefault(value);
    auto callback = formatters().id.value_or(nullptr);
    if (!callback)
        throw std::logic_error("Tag ID formatter is absent");
    return callback(value);
}
std::u16string AbstractTags::formatNumber(std::int64_t number,
                                          std::optional<std::int64_t> subnumber) {
    if (!formatterOptions_)
        return formatNumberDefault(number, subnumber);
    auto callback = formatters().number.value_or(nullptr);
    if (!callback)
        throw std::logic_error("Tag number formatter is absent");
    return callback(number);
}
TagFormatValue AbstractTags::formatTagDefault(std::u16string_view value) {
    return defaultTag(value);
}
TagFormatValue AbstractTags::formatRefDefault(std::u16string_view value) {
    return formatTag(value);
}
std::u16string AbstractTags::formatUrlDefault(std::u16string_view id, std::u16string_view base) {
    return defaultUrl(id, base);
}
std::u16string AbstractTags::formatIdDefault(std::u16string_view value) {
    return defaultId(value);
}
std::u16string AbstractTags::formatNumberDefault(std::int64_t number, std::optional<std::int64_t>) {
    return defaultNumber(number);
}
void AbstractTags::start(std::string_view environment, bool taggable, bool defaultTags) {
    auto &equation = configuration_.equation;
    equation.stack.push_back(equation.current_tag);
    auto label = equation.current_tag.label_id;
    equation.current_tag = TagInfoState{
        .environment = std::string(environment), .taggable = taggable, .default_tags = defaultTags};
    setLabel(std::move(label));
}
void AbstractTags::end() {
    auto &equation = configuration_.equation;
    if (equation.stack.empty())
        throw std::out_of_range("No enclosing tag environment");
    equation.history.push_back(equation.current_tag);
    auto label = equation.current_tag.label_id;
    equation.current_tag = std::move(equation.stack.back());
    equation.stack.pop_back();
    if (!label.empty() && this->label().empty())
        setLabel(std::move(label));
}
void AbstractTags::tag(std::u16string_view value, bool noFormat) {
    auto &tag = configuration_.equation.current_tag;
    tag.tag = std::u16string(value);
    tag.tag_format = noFormat ? TagFormatValue(std::u16string(value)) : formatTag(value);
    tag.no_tag = false;
}
void AbstractTags::notag() {
    tag(u"", true);
    configuration_.equation.current_tag.no_tag = true;
}
bool AbstractTags::noTag() const noexcept {
    return configuration_.equation.current_tag.no_tag;
}
void AbstractTags::autoTag() {
    auto &equation = configuration_.equation;
    if (!equation.current_tag.tag) {
        if (equation.counter.equation == std::numeric_limits<std::int64_t>::max())
            throw std::overflow_error("Equation counter exhausted");
        tag(formatNumber(++equation.counter.equation), false);
    }
}
void AbstractTags::clearTag() {
    auto &tag = configuration_.equation.current_tag;
    tag.tag.reset();
    tag.tag_format = std::u16string{};
    tag.no_tag = false;
    tag.tag_id.clear();
}
void AbstractTags::resetTag() {
    auto &equation = configuration_.equation;
    equation.history.clear();
    equation.redo = false;
    equation.ref_update = false;
    clearTag();
}
void AbstractTags::reset(std::int64_t offset) {
    resetTag();
    configuration_.packageData.resetTags(offset);
    configuration_.equation.counter.equation = offset;
    setLabel({});
}
void AbstractTags::startEquation(core::MathItem const &math) {
    auto previousUpdate = configuration_.equation.ref_update;
    configuration_.equation = configuration_.packageData.beginEquation(
        math.inputData.recompile.has_value(),
        math.inputData.recompile ? std::optional(math.inputData.recompile->counter) : std::nullopt);
    if (!math.inputData.recompile)
        configuration_.equation.ref_update = previousUpdate;
}
void AbstractTags::finishEquation(core::MathItem &math) {
    auto result = configuration_.packageData.finishEquation(configuration_.equation);
    if (result.recompile_counter)
        math.inputData.recompile = core::MathRecompile{math.state(), *result.recompile_counter};
}
void AbstractTags::makeId() {
    auto &tag = configuration_.equation.current_tag;
    auto value = configuration_.options.useLabelIds.value_or(true) && !label().empty()
                     ? std::u16string(label())
                     : tag.tag.value_or(u"null");
    tag.tag_id = formatId(value);
}
mml::MmlNode &AbstractTags::makeTag() {
    makeId();
    auto &current = configuration_.equation.current_tag;
    if (!label().empty()) {
        configuration_.equation.labels.insert_or_assign(
            std::u16string(label()), LabelRecord{current.tag.value_or(u""), current.tag_id});
        setLabel({});
    }
    std::vector<std::u16string> parts;
    if (auto list = std::get_if<std::vector<std::u16string>>(&current.tag_format))
        parts = *list;
    else {
        auto const &text = std::get<std::u16string>(current.tag_format);
        regexp_host::RegExp delimiters(u"^(\\(|\\[|\\{)(.*)(\\}|\\]|\\))$");
        if (auto match = delimiters.exec(text)) {
            for (std::size_t i = 1; i <= 3; ++i)
                parts.push_back(match->at(i).value_or(u""));
        } else
            parts.push_back(text);
    }
    std::u16string source;
    for (auto const &part : parts)
        if (!part.empty())
            source += u"\\text{" + part + u"}";
    auto mml = TexParser(std::move(source), ParserEnvironmentSeed{}, configuration_).mml();
    if (!mml)
        throw std::logic_error("Tag parser returned no MML");
    mml::AttributeSet attributes;
    attributes.set(u"id", current.tag_id);
    attributes.set(u"rowalign", configuration_.options.tagAlign.value_or(u"baseline"));
    return configuration_.nodeFactory.makeNode(mml::NodeKind::mtd, {&mml, 1},
                                               std::move(attributes));
}
mml::MmlNode *AbstractTags::getTag(bool force) {
    if (force) {
        autoTag();
        return &makeTag();
    }
    auto &current = configuration_.equation.current_tag;
    if (current.taggable && !current.no_tag) {
        if (current.default_tags)
            autoTag();
        if (current.tag && !current.tag->empty())
            return &makeTag();
    }
    return nullptr;
}
mml::MmlNode &AbstractTags::enTag(mml::MmlNode &node, mml::MmlNode &tag) {
    auto &nf = configuration_.nodeFactory;
    // Use stable local pointer arrays: all products remain MmlDocument-owned.
    std::array cellChildren{&node};
    auto cell = &nf.makeNode(mml::NodeKind::mtd, cellChildren);
    std::array rowChildren{&tag, cell};
    auto row = &nf.makeNode(mml::NodeKind::mlabeledtr, rowChildren);
    mml::AttributeSet attrs;
    attrs.set(u"side", configuration_.options.tagSide.value_or(u"right"));
    attrs.set(u"minlabelspacing", configuration_.options.tagIndent.value_or(u"0.8em"));
    attrs.set(u"displaystyle", true);
    return nf.makeNode(mml::NodeKind::mtable, {&row, 1}, std::move(attrs));
}
mml::MmlNode &AbstractTags::finalize(mml::MmlNode &node, TexEnvironment const &env) {
    auto &current = configuration_.equation.current_tag;
    if (!env.display.value_or(false) || !current.environment.empty() || !current.tag)
        return node;
    return enTag(node, makeTag());
}
void NoTags::autoTag() {}
mml::MmlNode *NoTags::getTag(bool) {
    auto const &tag = configuration().equation.current_tag.tag;
    return !tag || tag->empty() ? nullptr : AbstractTags::getTag();
}
mml::MmlNode &AllTags::finalize(mml::MmlNode &node, TexEnvironment const &env) {
    if (!env.display.value_or(false) ||
        std::ranges::any_of(configuration().equation.history,
                            [](auto const &tag) { return tag.taggable; }))
        return node;
    return enTag(node, *getTag(true));
}
TexOptionValues TagsFactory::defaults() {
    TexOptionValues values;
    values.tags = u"none";
    values.tagSide = u"right";
    values.tagIndent = u"0.8em";
    values.useLabelIds = true;
    values.ignoreDuplicateLabels = false;
    values.tagAlign = u"baseline";
    return values;
}
TagFormatOptions TagsFactory::formatters() {
    return TagFormatOptions{.number = &defaultNumber,
                            .tag = &defaultTag,
                            .ref = nullptr,
                            .id = &defaultId,
                            .url = &defaultUrl};
}
void TagsFactory::add(ParserConfiguration &configuration, TagsCreatorEntry entry) {
    if (!entry.create)
        throw std::invalid_argument("Tags creator cannot be null");
    auto found = std::ranges::find(configuration.tags, entry.name, &TagsCreatorEntry::name);
    if (found == configuration.tags.end())
        configuration.tags.push_back(std::move(entry));
    else
        *found = std::move(entry);
}
void TagsFactory::addTags(ParserConfiguration &configuration,
                          std::span<TagsCreatorEntry const> entries) {
    for (auto const &entry : entries)
        add(configuration, entry);
}
void TagsFactory::setDefault(ParserConfiguration &configuration, std::u16string_view name) {
    configuration.defaultTags = name;
}
std::unique_ptr<Tags> TagsFactory::getDefault(ParseOptions &owner, std::u16string_view name,
                                              std::span<TagsCreatorEntry const> creators) {
    return create(owner, name, name, creators);
}
std::unique_ptr<Tags> TagsFactory::create(ParseOptions &owner, std::u16string_view requested,
                                          std::u16string_view fallback,
                                          std::span<TagsCreatorEntry const> creators) {
    auto make = creator(requested, fallback, creators);
    auto tags = make(owner);
    if (!tags)
        throw std::runtime_error("Tags creator returned no product");
    return tags;
}
TagsCreator TagsFactory::creator(std::u16string_view requested, std::u16string_view fallback,
                                 std::span<TagsCreatorEntry const> creators) {
    auto resolve = [&](std::u16string_view name) -> TagsCreator {
        for (auto const &creator : creators)
            if (creator.name == name)
                return creator.create;
        if (name == u"none")
            return [](ParseOptions &config) -> std::unique_ptr<Tags> {
                return std::make_unique<NoTags>(config);
            };
        if (name == u"all")
            return [](ParseOptions &config) -> std::unique_ptr<Tags> {
                return std::make_unique<AllTags>(config);
            };
        return {};
    };
    auto creator = resolve(requested);
    if (!creator)
        creator = resolve(fallback);
    if (!creator)
        throw std::runtime_error("Unknown tags class");
    return creator;
}
} // namespace folia::math::tex
