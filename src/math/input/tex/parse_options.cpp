// MathJax 4.1.3 input/tex/ParseOptions.ts; Apache-2.0.
module folia.math.input.tex.parser;
import folia.math.util.unicode_normalization;
import :stack_item_factory;
import :column_parser;
import :tex_parser;
import std;

namespace folia::math::tex {
ParseOptions::ParseOptions(ParserConfiguration &configuration, TexOptionValues &effective,
                           TexSessionState &session, mml::MmlFactory &factory,
                           MathStyleResolver captured)
    : handlers(configuration.handlers), options(effective),
      itemFactory(std::make_unique<StackItemFactory>(*this, configuration.items)),
      nodeFactory(factory, this), mathStyle(captured ? captured : getVariant(u"TeX")),
      columnParser(std::make_unique<ColumnParser>(session.columnDefinitions())),
      packageData(session), configuration_(configuration) {
    nodeFactory.setConfiguration(*this);
    nodeFactory.setCreators(configuration.nodes);
    options.mergeDefaults(configuration.options);
}
ParseOptions::~ParseOptions() = default;
MapHandler &ParseOptions::maps() const noexcept {
    return configuration_.maps();
}
MacroCallbacks const &ParseOptions::callbacks() const noexcept {
    return configuration_.callbacks();
}
void ParseOptions::pushParser(TexParser &parser) {
    parsers.insert(parsers.begin(), &parser);
}
void ParseOptions::popParser() {
    if (!parsers.empty())
        parsers.erase(parsers.begin());
}
void ParseOptions::removeParser(TexParser &parser) noexcept {
    auto found = std::ranges::find(parsers, &parser);
    if (found != parsers.end())
        parsers.erase(found);
}
TexParser *ParseOptions::parser() const noexcept {
    return parsers.empty() ? nullptr : parsers.front();
}
TexEnvironment const &ParseOptions::environment() const {
    auto current = parser();
    if (!current)
        throw std::logic_error("No active TeX parser environment");
    return current->stack.env();
}
void ParseOptions::clear() {
    parsers.clear();
    root = nullptr;
    nodeLists.clear();
    error = false;
    if (!tags)
        throw std::logic_error("TeX tags have not been configured");
    tags->resetTag();
}
void ParseOptions::addNode(mml::NodeKind kind, mml::MmlNode &node) {
    auto name = mml::nodeKindName(kind);
    addNode(std::u16string(name.begin(), name.end()), node);
}
void ParseOptions::addNode(std::u16string_view property, mml::MmlNode &node) {
    nodeLists[std::u16string(property)].push_back(&node);
    auto kind = mml::nodeKindName(node.kind());
    if (property != std::u16string(kind.begin(), kind.end())) {
        if (!node.properties().tex.in_lists)
            node.properties().tex.in_lists.emplace();
        node.properties().tex.in_lists->emplace_back(property);
    }
}
bool ParseOptions::inTree(mml::MmlNode const &node) const noexcept {
    for (auto current = &node; current; current = current->parent())
        if (current == root)
            return true;
    return false;
}
std::span<mml::MmlNode *const> ParseOptions::getList(std::u16string_view property) {
    auto &list = nodeLists[std::u16string(property)];
    std::erase_if(list, [&](auto node) { return !node || !inTree(*node); });
    return list;
}
void ParseOptions::removeFromList(std::u16string_view property,
                                  std::span<mml::MmlNode *const> nodes) {
    auto found = nodeLists.find(std::u16string(property));
    if (found == nodeLists.end())
        return;
    std::vector<mml::MmlNode *> selected(nodes.begin(), nodes.end());
    for (auto node : selected) {
        auto entry = std::ranges::find(found->second, node);
        if (entry != found->second.end())
            found->second.erase(entry);
    }
}
namespace {
template <class T> void mergeValue(T &destination, T const &source) {
    if constexpr (requires { destination.mergeDefaults(source); }) {
        destination.mergeDefaults(source);
    } else if constexpr (std::is_same_v<T, SetOptionsOptions::Permission>) {
        using Permissions = std::unordered_map<std::u16string, bool>;
        auto *current = std::get_if<Permissions>(&destination);
        auto const *layer = std::get_if<Permissions>(&source);
        if (current && layer)
            mergeValue(*current, *layer);
        else
            destination = source;
    } else if constexpr (requires { typename T::mapped_type; }) {
        for (auto const &[name, value] : source) {
            auto [entry, inserted] = destination.try_emplace(name, value);
            if (!inserted)
                mergeValue(entry->second, value);
        }
    } else
        destination = source;
}

template <class T> void mergeField(std::optional<T> &destination, std::optional<T> const &source) {
    if (!source)
        return;
    if (destination)
        mergeValue(*destination, *source);
    else
        destination = source;
}

template <class Owner, class... Members>
void mergeFields(Owner &destination, Owner const &source, Members... members) {
    (mergeField(destination.*members, source.*members), ...);
}

std::u16string texVariant(std::u16string_view character, bool bold) {
    if (bold && !character.empty() &&
        ((character.front() >= u'\u0391' && character.front() <= u'\u03a9') ||
         character.front() == u'\u03f4'))
        return u"normal";
    return {};
}
std::u16string isoVariant(std::u16string_view, bool) {
    return u"italic";
}
std::u16string frenchVariant(std::u16string_view character, bool) {
    Unicode16Normalization normalization;
    auto decomposed = normalization.nfd(character);
    return !decomposed.empty() && decomposed.front() >= u'a' && decomposed.front() <= u'z'
               ? u"italic"
               : u"normal";
}
std::u16string uprightVariant(std::u16string_view, bool) {
    return u"normal";
}
} // namespace

ParseOptions::MathStyleResolver ParseOptions::getVariant(std::u16string_view name) noexcept {
    if (name == u"TeX")
        return texVariant;
    if (name == u"ISO")
        return isoVariant;
    if (name == u"French")
        return frenchVariant;
    if (name == u"upright")
        return uprightVariant;
    return nullptr;
}

void TextMacrosOptions::mergeDefaults(TextMacrosOptions const &layer) {
    mergeFields(*this, layer, &TextMacrosOptions::packages);
}
void PhysicsOptions::mergeDefaults(PhysicsOptions const &layer) {
    mergeFields(*this, layer, &PhysicsOptions::italicdiff, &PhysicsOptions::arrowdel);
}
void UnitsOptions::mergeDefaults(UnitsOptions const &layer) {
    mergeFields(*this, layer, &UnitsOptions::loose, &UnitsOptions::ugly);
}
void BboldxOptions::mergeDefaults(BboldxOptions const &layer) {
    mergeFields(*this, layer, &BboldxOptions::bfbb, &BboldxOptions::light);
}
void BbmOptions::mergeDefaults(BbmOptions const &layer) {
    mergeFields(*this, layer, &BbmOptions::bold);
}
void DsfontOptions::mergeDefaults(DsfontOptions const &layer) {
    mergeFields(*this, layer, &DsfontOptions::sans);
}
void NoUndefinedOptions::mergeDefaults(NoUndefinedOptions const &layer) {
    mergeFields(*this, layer, &NoUndefinedOptions::color, &NoUndefinedOptions::background,
                &NoUndefinedOptions::size);
}
void ColorOptions::mergeDefaults(ColorOptions const &layer) {
    mergeFields(*this, layer, &ColorOptions::padding, &ColorOptions::borderWidth);
}
void BegingroupOptions::mergeDefaults(BegingroupOptions const &layer) {
    mergeFields(*this, layer, &BegingroupOptions::allowGlobal);
}
void AmscdOptions::mergeDefaults(AmscdOptions const &layer) {
    mergeFields(*this, layer, &AmscdOptions::colspace, &AmscdOptions::rowspace,
                &AmscdOptions::harrowsize, &AmscdOptions::varrowsize,
                &AmscdOptions::hideHorizontalLabels);
}
void AmsOptions::mergeDefaults(AmsOptions const &layer) {
    mergeFields(*this, layer, &AmsOptions::operatornamePattern, &AmsOptions::multlineWidth,
                &AmsOptions::multlineIndent);
}
void TagFormatOptions::mergeDefaults(TagFormatOptions const &layer) {
    mergeFields(*this, layer, &TagFormatOptions::number, &TagFormatOptions::tag,
                &TagFormatOptions::ref, &TagFormatOptions::id, &TagFormatOptions::url);
}
void SetOptionsOptions::mergeDefaults(SetOptionsOptions const &layer) {
    mergeFields(*this, layer, &SetOptionsOptions::filterPackage, &SetOptionsOptions::filterOption,
                &SetOptionsOptions::filterValue, &SetOptionsOptions::allowPackageDefault,
                &SetOptionsOptions::allowOptionsDefault, &SetOptionsOptions::allowOptions);
}
void MathtoolsOptions::mergeDefaults(MathtoolsOptions const &layer) {
    mergeFields(*this, layer, &MathtoolsOptions::multlinedGap, &MathtoolsOptions::multlinedPos,
                &MathtoolsOptions::multlinedWidth, &MathtoolsOptions::firstlineAfterskip,
                &MathtoolsOptions::lastlinePreskip, &MathtoolsOptions::smallmatrixAlign,
                &MathtoolsOptions::shortvdotsAdjustAbove, &MathtoolsOptions::shortvdotsAdjustBelow,
                &MathtoolsOptions::centercolon, &MathtoolsOptions::centercolonOffset,
                &MathtoolsOptions::thincolonDx, &MathtoolsOptions::thincolonDw,
                &MathtoolsOptions::useUnicode, &MathtoolsOptions::legacycolonsymbols,
                &MathtoolsOptions::prescriptSubFormat, &MathtoolsOptions::prescriptSupFormat,
                &MathtoolsOptions::prescriptArgFormat, &MathtoolsOptions::allowMathtoolsset,
                &MathtoolsOptions::pairedDelimiters, &MathtoolsOptions::tagforms);
}
void TexOptionValues::mergeDefaults(TexOptionValues const &layer) {
    mergeFields(*this, layer, &TexOptionValues::preFilters, &TexOptionValues::postFilters,
                &TexOptionValues::findTeX, &TexOptionValues::packages, &TexOptionValues::maxBuffer,
                &TexOptionValues::maxTemplateSubtitutions, &TexOptionValues::maxMacros,
                &TexOptionValues::mathStyle, &TexOptionValues::formatError, &TexOptionValues::tags,
                &TexOptionValues::tagSide, &TexOptionValues::tagIndent, &TexOptionValues::tagAlign,
                &TexOptionValues::useLabelIds, &TexOptionValues::ignoreDuplicateLabels,
                &TexOptionValues::textmacros, &TexOptionValues::setoptions,
                &TexOptionValues::noundefined, &TexOptionValues::bboldx, &TexOptionValues::units,
                &TexOptionValues::allowTexHTML, &TexOptionValues::tagformat,
                &TexOptionValues::physics, &TexOptionValues::protectedMacros,
                &TexOptionValues::mathtools, &TexOptionValues::dsfont, &TexOptionValues::active,
                &TexOptionValues::macros, &TexOptionValues::environments, &TexOptionValues::color,
                &TexOptionValues::begingroup, &TexOptionValues::bbm, &TexOptionValues::amscd,
                &TexOptionValues::multlineWidth, &TexOptionValues::ams, &TexOptionValues::digits,
                &TexOptionValues::numberPattern, &TexOptionValues::initialDigit,
                &TexOptionValues::identifierPattern, &TexOptionValues::initialLetter,
                &TexOptionValues::baseURL, &TexOptionValues::internalMath);
}
} // namespace folia::math::tex
