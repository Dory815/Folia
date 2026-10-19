// Request-owned Common mathematical services and layout lifecycle.
module folia.math.output.common;
import :output_jax;
import :wrapper_factory;
import :linebreak_visitor;
import std;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mspace;
import folia.math.util.lengths;
import folia.math.util.js_number;
import folia.math.util.utf16;

namespace folia::math::output::common {

CommonOutputJax::CommonOutputJax(mml::MmlDocument &document, CommonOutputJaxConfig config,
                                 MeasurementTarget &measurement,
                                 CommonWrapperCatalog wrapperCatalog,
                                 LinebreakFactory linebreakFactory)
    : document_(document), fonts_(config.fonts), face_(fonts_.face(config.faceName)),
      metrics_(std::move(config.metrics)), options_(std::move(config.options)),
      inlineBreakPolicy_(config.inlineBreakPolicy), measurement_(measurement) {
    if (!std::isfinite(metrics_.em) || metrics_.em <= 0 || !std::isfinite(metrics_.ex) ||
        metrics_.ex < 0) {
        throw std::invalid_argument(
            "CommonOutputJax requires positive em and nonnegative ex metrics");
    }

    // Declaration order intentionally makes the visitor die before the
    // factory arena (and its wrappers).
    factory_ = std::make_unique<CommonWrapperFactory>(*this, std::move(wrapperCatalog));
    if (!linebreakFactory) {
        throw std::invalid_argument("CommonOutputJax requires a typed Linebreaks factory");
    }
    linebreaks_ = linebreakFactory(*factory_);
    if (!linebreaks_) {
        throw std::runtime_error("Linebreaks factory returned no visitor");
    }
}

CommonWrapperFactory &CommonOutputJax::factory() noexcept {
    return *factory_;
}

mml::MmlDocument &CommonOutputJax::document() const noexcept {
    return document_;
}

double CommonOutputJax::pxPerEm() const {
    const double xHeight = face_.parameters.required("x_height");
    if (!std::isfinite(xHeight) || xHeight <= 0) {
        throw std::domain_error("CommonOutputJax font x_height must be positive");
    }
    return metrics_.ex / xHeight;
}

double CommonOutputJax::containerWidth() const noexcept {
    return containerWidth_;
}

CommonWrapperFactory const &CommonOutputJax::factory() const noexcept {
    return *factory_;
}

FontRepository const &CommonOutputJax::fonts() const noexcept {
    return fonts_;
}

FontFaceData const &CommonOutputJax::face() const noexcept {
    return face_;
}

ExtendedMetrics const &CommonOutputJax::currentMetrics() const noexcept {
    return metrics_;
}

CommonOptions const &CommonOutputJax::options() const noexcept {
    return options_;
}

LinebreakOptions const &CommonOutputJax::currentLinebreakOptions() const noexcept {
    return options_.linebreaks;
}

Linebreaks &CommonOutputJax::linebreaks() const noexcept {
    return *linebreaks_;
}

CommonWrapper *CommonOutputJax::wrapperFor(mml::MmlNode const &node) const noexcept {
    const auto found = wrappers_.find(const_cast<mml::MmlNode *>(&node));
    return found == wrappers_.end() ? nullptr : found->second;
}

void CommonOutputJax::registerWrapper(mml::MmlNode &node, CommonWrapper &wrapper) {
    if (&node.document() != &document_) {
        throw std::invalid_argument("Cannot index a wrapper for another MML document");
    }
    if (wrapper.node != &node || &wrapper.factory() != factory_.get()) {
        throw std::invalid_argument("Wrapper index entry does not match its node and factory");
    }
    // Source nodeMap.set selects the latest completed wrapper for this node.
    // The factory still owns every constructed wrapper for the request.
    wrappers_.insert_or_assign(&node, &wrapper);
}

CommonWrapper *CommonOutputJax::table() const noexcept {
    return table_;
}

std::vector<CommonDiagnostic> const &CommonOutputJax::diagnostics() const noexcept {
    return diagnostics_;
}

void CommonOutputJax::recordUnknownMathVariant(mml::NodeKind nodeKind, std::string value) {
    diagnostics_.push_back(
        {CommonDiagnosticCode::unknown_math_variant, nodeKind, std::move(value)});
}

void CommonOutputJax::setTable(CommonWrapper &table) {
    if (&table.factory() != factory_.get()) {
        throw std::invalid_argument("Table wrapper belongs to another layout job");
    }
    table_ = &table;
}

LayoutJob::LayoutJob(mml::MmlDocument &document, CommonOutputJaxConfig config,
                     MeasurementTarget &measurement, CommonWrapperCatalog wrapperCatalog,
                     LinebreakFactory linebreakFactory)
    : jax_(document, std::move(config), measurement, std::move(wrapperCatalog), linebreakFactory) {
}

mml::MmlDocument &LayoutJob::document() const noexcept {
    return jax_.document();
}

CommonOutputJax &LayoutJob::jax() noexcept {
    return jax_;
}

CommonOutputJax const &LayoutJob::jax() const noexcept {
    return jax_;
}

CommonWrapperFactory &LayoutJob::factory() noexcept {
    return jax_.factory();
}

CommonWrapper &LayoutJob::build() {
    mml::MmlNode *const root = document().root();
    if (!root) {
        throw std::logic_error("Cannot build a Common layout without an MML root");
    }
    root->attributes().setInherited(u"overflow",
                                    utf16::decode(jax_.options().displayOverflow).text);
    const auto *overflow = root->attributes().lookup(u"overflow");
    const auto *overflowText = overflow ? std::get_if<std::u16string>(overflow) : nullptr;
    if (overflowText && *overflowText == u"linebreak")
        (void)jax_.getLinebreakWidth();

    const bool makeBreaks =
        jax_.options().linebreaks.inlineBreaks && !jax_.currentMetrics().display;
    auto &layout = root->properties().layout;
    if (layout.inline_marked.value_or(false) &&
        (!makeBreaks || jax_.forceInlineBreaks() != layout.inline_forced.value_or(false))) {
        jax_.unmarkInlineBreaks(*root);
        layout.inline_marked.reset();
        layout.inline_forced.reset();
    }
    if (makeBreaks && !layout.inline_marked.value_or(false)) {
        const auto children = root->children();
        if (!children.empty())
            jax_.markInlineBreaks(*children.front());
        layout.inline_marked = true;
        layout.inline_forced = jax_.forceInlineBreaks();
    }
    root->setTeXclass(nullptr);
    return factory().wrap(*root);
}

namespace {

std::u16string attributeText(mml::AttributeSet const &attributes, std::u16string_view name,
                             std::u16string_view fallback = {}) {
    const auto *value = attributes.lookup(name);
    if (const auto *text = value ? std::get_if<std::u16string>(value) : nullptr)
        return *text;
    if (const auto *boolean = value ? std::get_if<bool>(value) : nullptr)
        return *boolean ? u"true" : u"false";
    if (const auto *number = value ? std::get_if<double>(value) : nullptr) {
        return utf16::decode(js_number::toString(*number)).text;
    }
    if (value && std::holds_alternative<mml::AttributeNull>(*value))
        return u"null";
    return std::u16string(fallback);
}

bool truthy(mml::AttributeSet const &attributes, std::u16string_view name) {
    const auto *value = attributes.lookup(name);
    if (!value || std::holds_alternative<mml::AttributeNull>(*value))
        return false;
    if (const auto *boolean = std::get_if<bool>(value))
        return *boolean;
    if (const auto *number = std::get_if<double>(value))
        return *number != 0 && !std::isnan(*number);
    if (const auto *text = std::get_if<std::u16string>(value))
        return !text->empty();
    return false;
}

bool has(std::optional<bool> const &value) {
    return value.value_or(false);
}

void setStyle(style_json::StyleJson &styles, std::u16string name, std::u16string value) {
    const auto found =
        std::find_if(styles.begin(), styles.end(),
                     [&](style_json::StyleEntry const &entry) { return entry.name == name; });
    if (found == styles.end())
        styles.push_back({std::move(name), std::move(value)});
    else
        found->value = std::move(value);
}

void clearBreakFlags(mml::MmlNode &node) {
    auto &layout = node.properties().layout;
    layout.force_break.reset();
    layout.breakable.reset();
}

void markBreak(mml::MmlNode &parent, mml::MmlNode &child, mml::MmlNode *mo, bool &marked,
               bool force, std::u16string_view linebreak) {
    auto &childLayout = child.properties().layout;
    childLayout.breakable = true;
    if (force && linebreak != u"newline") {
        childLayout.force_break = true;
        if (mo)
            mo->properties().layout.force_break = true;
    } else {
        childLayout.force_break.reset();
        if (mo)
            mo->properties().layout.force_break.reset();
        if (linebreak == u"newline")
            childLayout.newline = true;
    }
    if (!marked) {
        parent.properties().layout.process_breaks = true;
        if (auto *outer = parent.parent())
            outer->properties().layout.process_breaks = true;
        marked = true;
    }
}

} // namespace

bool CommonOutputJax::forceInlineBreaks() const {
    return inlineBreakPolicy_ == InlineBreakPolicy::forced && options_.linebreaks.inlineBreaks;
}

double CommonOutputJax::getLinebreakWidth() {
    const auto *root = document_.root();
    if (!root)
        throw std::logic_error("CommonOutputJax requires an MML root for line width");
    const auto &attributes = root->attributes();
    std::u16string selected;
    if (const auto *maximum = attributes.lookup(u"maxwidth"); maximum) {
        bool selectedMaximum = false;
        if (const auto *text = std::get_if<std::u16string>(maximum)) {
            selectedMaximum = !text->empty();
            if (selectedMaximum)
                selected.assign(*text);
        } else if (const auto *number = std::get_if<double>(maximum)) {
            selectedMaximum = *number != 0 && !std::isnan(*number);
            if (selectedMaximum) {
                selected = utf16::decode(js_number::toString(*number)).text;
            }
        } else if (const auto *boolean = std::get_if<bool>(maximum)) {
            selectedMaximum = *boolean;
            if (selectedMaximum)
                selected = u"true";
        }
        if (!selectedMaximum)
            selected = utf16::decode(options_.linebreaks.width).text;
    } else {
        selected = utf16::decode(options_.linebreaks.width).text;
    }
    const double relativeWidth = metrics_.containerWidth / pxPerEm();
    lengths::LengthContext context;
    context.size_em = relativeWidth;
    context.scale = 1;
    context.em_pixels = pxPerEm();
    context.ex_em = metrics_.ex / metrics_.em;
    containerWidth_ = lengths::length2em(utf16::encode(selected), context);
    return containerWidth_;
}

void CommonOutputJax::markInlineBreaks(mml::MmlNode &node) {
    const bool force = forceInlineBreaks();
    bool marked = false;
    bool postBreak = false;
    std::u16string markNext;
    const auto children = node.children();
    for (std::size_t i = 0; i < children.size(); ++i) {
        auto &child = *children[i];
        if (!markNext.empty()) {
            markBreak(node, child, nullptr, marked, force, markNext);
            markNext.clear();
            postBreak = false;
        } else if (child.isEmbellished()) {
            if (i == 0)
                continue;
            auto *moNode = child.coreMO();
            auto *mo = dynamic_cast<mml::MmlMo *>(moNode);
            if (!mo)
                throw std::logic_error("Embellished MML operator has no MmlMo core");
            const auto texClass = mo->texClass();
            const auto &attributes = mo->attributes();
            const auto linebreak = attributeText(attributes, u"linebreak", u"auto");
            const auto style = attributeText(attributes, u"linebreakstyle", u"after");
            const bool eligible = texClass == mml::TexClass::bin ||
                                  texClass == mml::TexClass::rel ||
                                  (texClass == mml::TexClass::ord && mo->hasSpacingAttributes()) ||
                                  linebreak != u"auto";
            if (eligible && linebreak != u"nobreak") {
                if (style == u"before") {
                    if (!postBreak || linebreak != u"auto") {
                        markBreak(node, child, mo, marked, force, linebreak);
                    }
                } else {
                    markNext.assign(linebreak);
                }
            }
            postBreak = linebreak == u"newline" && style == u"after";
        } else if (child.kind() == mml::NodeKind::mspace) {
            auto *space = dynamic_cast<mml::MmlMspace *>(&child);
            if (!space)
                throw std::logic_error("mspace kind is not MmlMspace");
            const auto &attributes = space->attributes();
            const auto linebreak = attributeText(attributes, u"linebreak", u"auto");
            if (linebreak != u"nobreak" && space->canBreak()) {
                markBreak(node, child, nullptr, marked, force, linebreak);
            }
            postBreak = linebreak == u"newline";
        } else {
            postBreak = false;
            const auto &attributes = child.attributes();
            const auto nested = child.children();
            const auto kind = child.kind();
            if ((kind == mml::NodeKind::mstyle && !truthy(attributes, u"style") &&
                 !attributes.own(u"mathbackground")) ||
                kind == mml::NodeKind::semantics) {
                if (!nested.empty())
                    markInlineBreaks(*nested.front());
                if (has(child.properties().layout.process_breaks)) {
                    child.properties().layout.inline_breaks = true;
                    if (!nested.empty())
                        nested.front()->properties().layout.inline_breaks = true;
                    if (auto *outer = node.parent())
                        outer->properties().layout.process_breaks = true;
                }
            } else if (kind == mml::NodeKind::mrow && truthy(attributes, u"data-semantic-added")) {
                markInlineBreaks(child);
                if (has(child.properties().layout.process_breaks)) {
                    child.properties().layout.inline_breaks = true;
                    if (auto *outer = node.parent())
                        outer->properties().layout.process_breaks = true;
                }
            }
        }
    }
}

void CommonOutputJax::unmarkInlineBreaks(mml::MmlNode &node) {
    clearBreakFlags(node);
    if (auto *mo = node.coreMO())
        mo->properties().layout.force_break.reset();
    auto &layout = node.properties().layout;
    if (!has(layout.process_breaks))
        return;
    layout.process_breaks.reset();
    for (auto *child : node.children())
        unmarkInlineBreaks(*child);
}

ExtendedMetrics CommonOutputJax::getMetricsFor(MetricRequest const &request) {
    auto result = measurement_.measureMetrics(request);
    result.display = request.display;
    return result;
}

UnknownBBox CommonOutputJax::measureText(std::u16string_view text, std::string_view variant,
                                         CssFontData font) {
    return measureTextWithCache({std::u16string(text), std::string(variant), std::move(font)});
}

UnknownBBox CommonOutputJax::measureTextWithCache(UnknownTextRequest const &request) {
    std::string key = request.variant;
    if (request.variant == "-explicitFont") {
        key = request.font.family + "-" + (request.font.italic ? "T" : "F") + "-" +
              (request.font.bold ? "T" : "F") + "-";
    }
    auto &textCache = unknownCache_[key];
    if (const auto found = textCache.find(request.text); found != textCache.end())
        return found->second;
    const auto measured = measurement_.measureUnknownText(request);
    textCache.emplace(request.text, measured);
    return measured;
}

UnknownBBox CommonOutputJax::measureTextNodeWithCache(UnknownTextRequest const &request) {
    return measureTextWithCache(request);
}

style_json::StyleJson CommonOutputJax::cssFontStyles(CssFontData const &font,
                                                     style_json::StyleJson styles) const {
    setStyle(styles, u"font-family", utf16::decode(face_.getFamily(font.family)).text);
    if (font.italic)
        setStyle(styles, u"font-style", u"italic");
    if (font.bold)
        setStyle(styles, u"font-weight", u"bold");
    return styles;
}

CssFontData CommonOutputJax::getFontData(styles::Styles const &source) {
    return {face_.getFamily(utf16::encode(source.get(u"font-family"))),
            source.get(u"font-style") == u"italic", source.get(u"font-weight") == u"bold"};
}

void CommonOutputJax::addFontStyles(style_json::StyleJsonSheet &target) {
    target.addStyles(face_.styles);
}

void CommonOutputJax::addWrapperStyles(style_json::StyleJsonSheet &target) {
    for (const auto kind : factory_->getKinds())
        addClassStyles(kind, target);
}

void CommonOutputJax::addClassStyles(mml::NodeKind kind, style_json::StyleJsonSheet &target) {
    target.addStyles(factory_->classData(kind).styles);
}

} // namespace folia::math::output::common
