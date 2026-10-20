module folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.node_defaults;
import folia.math.core.mml_tree.node_properties;
import folia.math.util.js_numeric_input;
import std;

namespace folia::math::mml {
namespace {
constexpr std::u16string_view inheritSentinel = u"_inherit_";
constexpr std::array<std::array<int, 8>, 8> texSpace{{{{0, -1, 2, 3, 0, 0, 0, 1}},
                                                      {{-1, -1, 0, 3, 0, 0, 0, 1}},
                                                      {{2, 2, 0, 0, 2, 0, 0, 2}},
                                                      {{3, 3, 0, 0, 3, 0, 0, 3}},
                                                      {{0, 0, 0, 0, 0, 0, 0, 0}},
                                                      {{0, -1, 2, 3, 0, 0, 0, 1}},
                                                      {{1, 1, 0, 1, 1, 1, 1, 1}},
                                                      {{1, -1, 2, 3, 1, 0, 1, 1}}}};
constexpr std::array<std::u16string_view, 4> texSpaceLengths{u"", u"thinmathspace",
                                                             u"mediummathspace", u"thickmathspace"};

std::u16string ascii16(std::string_view value) {
    std::u16string result;
    result.reserve(value.size());
    for (const unsigned char ch : value)
        result.push_back(static_cast<char16_t>(ch));
    return result;
}

std::string utf8_ascii(std::u16string_view value) {
    std::string result;
    result.reserve(value.size());
    for (const char16_t ch : value) {
        if (ch > 0x7f)
            throw std::invalid_argument("MML attribute name is not ASCII");
        result.push_back(static_cast<char>(ch));
    }
    return result;
}

std::u16string attribute_text(AttributeValue const &value) {
    if (auto p = std::get_if<std::u16string>(&value))
        return *p;
    if (auto p = std::get_if<bool>(&value))
        return *p ? u"true" : u"false";
    if (auto p = std::get_if<double>(&value)) {
        if (std::isnan(*p))
            return u"NaN";
        if (std::isinf(*p))
            return *p < 0 ? u"-Infinity" : u"Infinity";
        if (*p == 0)
            return u"0";
        std::array<char, 64> buffer{};
        const auto [end, error] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), *p);
        if (error != std::errc{})
            throw std::runtime_error("cannot format mathvariant value");
        return ascii16(
            std::string_view(buffer.data(), static_cast<std::size_t>(end - buffer.data())));
    }
    return u"null";
}

bool is_ascii(std::u16string_view value) noexcept {
    return std::all_of(value.begin(), value.end(), [](char16_t ch) { return ch <= 0x7f; });
}

bool truthy(AttributeValue const *value) noexcept {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return false;
    if (auto p = std::get_if<bool>(value))
        return *p;
    if (auto p = std::get_if<double>(value))
        return *p != 0.0 && !std::isnan(*p);
    if (auto p = std::get_if<std::u16string>(value))
        return !p->empty();
    return false;
}

double number(AttributeValue const *value) {
    if (!value || std::holds_alternative<AttributeNull>(*value))
        return 0.0;
    if (auto p = std::get_if<double>(value))
        return *p;
    if (auto p = std::get_if<bool>(value))
        return *p ? 1.0 : 0.0;
    if (auto p = std::get_if<std::u16string>(value))
        return js_input::number(*p);
    return 0.0;
}

bool is_special_inherited_name(std::u16string_view name) noexcept {
    return name == u"displaystyle" || name == u"scriptlevel" || name == u"style";
}

bool excluded(NodeKind origin, NodeKind destination, std::u16string_view name) noexcept {
    for (const auto &rule : AbstractMmlNode::noInherit())
        if (rule.from == origin && rule.to == destination && rule.attribute == name)
            return true;
    return false;
}

bool stops(NodeKind kind, std::u16string_view name) noexcept {
    for (const auto &rule : AbstractMmlNode::stopInherit())
        if (rule.kind == kind && rule.attribute == name)
            return true;
    return false;
}

bool always_inherited(std::u16string_view name) noexcept {
    return std::find(AbstractMmlNode::alwaysInherit().begin(),
                     AbstractMmlNode::alwaysInherit().end(),
                     name) != AbstractMmlNode::alwaysInherit().end();
}

AttributeSet base_defaults() {
    AttributeSet result;
    for (const auto name : {u"mathbackground", u"mathcolor", u"mathsize", u"dir"})
        result.setDefault(std::u16string(name), std::u16string(inheritSentinel));
    return result;
}

void copy_attribute_layers(AttributeSet const &from, AttributeSet &to, bool copy_own,
                           bool keep_ids = true) {
    if (copy_own)
        for (const auto &[name, value] : from.ownEntries())
            if (name != u"id" || keep_ids)
                to.set(name, value);
    for (const auto &[name, value] : from.inheritedEntries())
        to.setInherited(name, value);
    for (const auto &[name, value] : from.defaultEntries())
        to.setDefault(name, value);
    for (const auto &[name, value] : from.globalDefaultEntries())
        to.setGlobalDefault(name, value);
}

void initialize_values(MmlNode &node, AttributeSet supplied) {
    AttributeSet combined = nodeDefaultAttributes(node.kind());
    copy_attribute_layers(supplied, combined, true);
    node.attributes() = std::move(combined);
}
} // namespace

MmlNode::MmlNode(MmlDocument &document, NodeKind kind) : owner_(&document), kind_(kind) {}

void MmlNode::copyNativeStateTo(MmlNode &, MmlClone &) const {}

NodeKind MmlNode::kind() const noexcept {
    return kind_;
}

bool MmlNode::isKind(NodeKind kind) const {
    return document().nodeIsKind(*this, kind);
}

MmlDocument &MmlNode::document() const noexcept {
    return *owner_;
}

tree::NodeOwner<MmlNode, NodeKind> &MmlNode::treeOwner() const noexcept {
    return document();
}

std::u16string_view MmlNode::text() const noexcept {
    return text_;
}

void MmlNode::setText(std::u16string value) {
    text_ = std::move(value);
}

AttributeSet &MmlNode::attributes() noexcept {
    return attributes_;
}

AttributeSet const &MmlNode::attributes() const noexcept {
    return attributes_;
}

NodeProperties &MmlNode::properties() noexcept {
    return properties_;
}

NodeProperties const &MmlNode::properties() const noexcept {
    return properties_;
}

MmlIntrinsicState &MmlNode::state() noexcept {
    return state_;
}

MmlIntrinsicState const &MmlNode::state() const noexcept {
    return state_;
}

AbstractMmlNode::AbstractMmlNode(MmlDocument &document, NodeKind kind)
    : tree::AbstractNode<MmlNode, NodeKind, MmlNode>(document, kind) {}

AttributeSet AbstractMmlNode::defaults() {
    return base_defaults();
}

std::span<InheritanceExclusion const> AbstractMmlNode::noInherit() {
    static constexpr std::array rules{
        InheritanceExclusion{NodeKind::mstyle, NodeKind::mpadded, u"width"},
        InheritanceExclusion{NodeKind::mstyle, NodeKind::mpadded, u"height"},
        InheritanceExclusion{NodeKind::mstyle, NodeKind::mpadded, u"depth"},
        InheritanceExclusion{NodeKind::mstyle, NodeKind::mpadded, u"lspace"},
        InheritanceExclusion{NodeKind::mstyle, NodeKind::mpadded, u"voffset"},
        InheritanceExclusion{NodeKind::mstyle, NodeKind::mtable, u"width"},
        InheritanceExclusion{NodeKind::mstyle, NodeKind::mtable, u"height"},
        InheritanceExclusion{NodeKind::mstyle, NodeKind::mtable, u"depth"},
        InheritanceExclusion{NodeKind::mstyle, NodeKind::mtable, u"align"},
        InheritanceExclusion{NodeKind::maligngroup, NodeKind::mrow, u"groupalign"},
        InheritanceExclusion{NodeKind::maligngroup, NodeKind::mtable, u"groupalign"},
        InheritanceExclusion{NodeKind::mtr, NodeKind::msqrt, u"data-vertical-align"},
        InheritanceExclusion{NodeKind::mtr, NodeKind::mroot, u"data-vertical-align"},
        InheritanceExclusion{NodeKind::mlabeledtr, NodeKind::msqrt, u"data-vertical-align"},
        InheritanceExclusion{NodeKind::mlabeledtr, NodeKind::mroot, u"data-vertical-align"}};
    return rules;
}

std::span<InheritanceStop const> AbstractMmlNode::stopInherit() {
    static constexpr std::array rules{InheritanceStop{NodeKind::mtd, u"columnalign"},
                                      InheritanceStop{NodeKind::mtd, u"rowalign"},
                                      InheritanceStop{NodeKind::mtd, u"groupalign"}};
    return rules;
}

std::span<std::u16string_view const> AbstractMmlNode::alwaysInherit() {
    static constexpr std::array<std::u16string_view, 3> names{
        u"scriptminsize", u"scriptsizemultiplier", u"infixlinebreakstyle"};
    return names;
}

VerifyOptions AbstractMmlNode::verifyDefaults() {
    return {};
}

void AbstractMmlNode::initializeMml(AttributeSet supplied, std::span<MmlNode *const> children) {
    state() = initialIntrinsicState(kind());
    properties() = NodeProperties{};
    initializeNodeProperties(kind(), properties());
    if (arity() < 0) {
        MmlNode &inferred = document().factory().make(NodeKind::inferred_mrow);
        // Source's constructor seeds childNodes directly before setChildren;
        // appendChild would recurse through the negative-arity forwarding path.
        document().append(*this, inferred);
    }
    setChildren(children);
    initialize_values(*this, std::move(supplied));
}

bool AbstractMmlNode::isToken() const {
    return false;
}

bool AbstractMmlNode::isEmbellished() const {
    return false;
}

bool AbstractMmlNode::isSpacelike() const {
    return false;
}

bool AbstractMmlNode::linebreakContainer() const {
    return false;
}

std::string AbstractMmlNode::linebreakAlign() const {
    return "data-align";
}

bool AbstractMmlNode::isEmpty() const {
    return std::all_of(children().begin(), children().end(),
                       [](MmlNode const *child) { return child->isEmpty(); });
}

double AbstractMmlNode::arity() const {
    return std::numeric_limits<double>::infinity();
}

bool AbstractMmlNode::isInferred() const {
    return false;
}

bool AbstractMmlNode::notParent() const {
    return false;
}

MmlNode *AbstractMmlNode::Parent() const noexcept {
    MmlNode *current = parent();
    while (current && current->notParent())
        current = current->Parent();
    return current;
}

void AbstractMmlNode::setChildren(std::span<MmlNode *const> values) {
    if (arity() < 0) {
        if (children().empty())
            throw std::logic_error("inferred MML node has no inferred row");
        children().front()->setChildren(values);
        return;
    }
    // Source clears the existing list, then re-adds through MML appendChild;
    // this is observable for inferred rows and cannot be replaced by Tree's
    // raw bulk setter. Copy first because callers can pass our current span.
    const std::vector<MmlNode *> incoming(values.begin(), values.end());
    document().setChildren(*this, {});
    for (MmlNode *child : incoming)
        appendChild(*child);
}

MmlNode &AbstractMmlNode::appendChild(MmlNode &child) {
    if (arity() < 0) {
        if (children().empty())
            throw std::logic_error("inferred MML node has no inferred row");
        return children().front()->appendChild(child);
    }
    if (!child.isInferred()) {
        document().append(*this, child);
        return child;
    }
    if (std::isinf(arity()) && arity() > 0) {
        const std::vector<MmlNode *> grandchildren(child.children().begin(),
                                                   child.children().end());
        for (MmlNode *grandchild : grandchildren)
            document().append(*this, *grandchild);
        return child;
    }
    MmlNode &replacement =
        document().factory().make(NodeKind::mrow, child.attributes(), child.children());
    replacement.properties() = child.properties();
    document().append(*this, replacement);
    return replacement;
}

MmlNode &AbstractMmlNode::replaceChild(MmlNode &replacement, MmlNode &oldChild) {
    if (arity() < 0) {
        if (children().empty())
            throw std::logic_error("inferred MML node has no inferred row");
        return children().front()->replaceChild(replacement, oldChild);
    }
    if (!childIndex(oldChild))
        return replacement;
    document().replace(*this, oldChild, replacement);
    return replacement;
}

MmlNode *AbstractMmlNode::core() {
    return this;
}

MmlNode *AbstractMmlNode::coreMO() {
    return this;
}

std::size_t AbstractMmlNode::coreIndex() {
    return 0;
}

std::optional<std::size_t> AbstractMmlNode::childPosition() {
    MmlNode *child = this;
    MmlNode *ancestor = parent();
    while (ancestor && ancestor->notParent()) {
        child = ancestor;
        ancestor = ancestor->parent();
    }
    if (!ancestor)
        return std::nullopt;
    const auto index = ancestor->childIndex(*child);
    return index;
}

MmlNode *AbstractMmlNode::setTeXclass(MmlNode *previous) {
    getPrevClass(previous);
    return texClass().has_value() ? this : previous;
}

void AbstractMmlNode::updateTeXclass(MmlNode *coreNode) {
    if (!coreNode)
        return;
    setPrevClass(coreNode->prevClass());
    setPrevLevel(coreNode->prevLevel());
    coreNode->setPrevLevel(std::nullopt);
    coreNode->setPrevClass(std::nullopt);
    setTexClass(coreNode->texClass());
}

void AbstractMmlNode::getPrevClass(MmlNode *previous) {
    if (!previous)
        return;
    setPrevClass(previous->texClass());
    setPrevLevel(number(previous->attributes().lookup(u"scriptlevel")));
}

std::optional<TexClass> AbstractMmlNode::texClass() {
    return state().working_tex_class;
}

void AbstractMmlNode::setTexClass(std::optional<TexClass> value) {
    state().working_tex_class = value;
}

std::optional<TexClass> AbstractMmlNode::prevClass() const noexcept {
    return state().previous_tex_class;
}

void AbstractMmlNode::setPrevClass(std::optional<TexClass> value) {
    state().previous_tex_class = value;
}

std::optional<double> AbstractMmlNode::prevLevel() const noexcept {
    return state().previous_script_level;
}

void AbstractMmlNode::setPrevLevel(std::optional<double> value) {
    state().previous_script_level = value;
}

std::string AbstractMmlNode::texSpacing() {
    const auto previous = prevClass().value_or(TexClass::none);
    const auto current = texClass().value_or(TexClass::ord);
    if (previous == TexClass::none || current == TexClass::none)
        return {};
    const auto previousIndex = static_cast<int>(previous);
    const auto currentIndex = static_cast<int>(current);
    if (previousIndex < 0 || previousIndex >= 8 || currentIndex < 0 || currentIndex >= 8)
        throw std::logic_error("invalid TeX class index");
    const int spacing =
        texSpace[static_cast<std::size_t>(previousIndex)][static_cast<std::size_t>(currentIndex)];
    const bool previousScript = prevLevel().value_or(0.0) > 0.0;
    const bool currentScript = !previousScript && number(attributes().lookup(u"scriptlevel")) > 0.0;
    if ((previousScript || currentScript) && spacing >= 0)
        return {};
    const int magnitude = std::abs(spacing);
    if (magnitude < 0 || magnitude >= static_cast<int>(texSpaceLengths.size()))
        throw std::logic_error("invalid TeX spacing index");
    return utf8_ascii(texSpaceLengths[static_cast<std::size_t>(magnitude)]);
}

bool AbstractMmlNode::hasSpacingAttributes() {
    if (!isEmbellished())
        return false;
    MmlNode *node = coreMO();
    return node && node->hasSpacingAttributes();
}

void AbstractMmlNode::setInheritedAttributes(AttributeList const &attrs, bool display, double level,
                                             bool prime) {
    inheritAttributes(attrs, display, level, prime);
}

void AbstractMmlNode::inheritAttributes(AttributeList const &attrs, bool display, double level,
                                        bool prime) {
    AttributeList inherited = attrs;
    for (auto item = inherited.begin(); item != inherited.end();) {
        if ((attributes().hasDefault(item->name) || always_inherited(item->name)) &&
            !excluded(item->origin, kind(), item->name))
            attributes().setInherited(item->name, item->value);
        if (stops(kind(), item->name))
            item = inherited.erase(item);
        else
            ++item;
    }

    if (!attributes().own(u"displaystyle"))
        attributes().setInherited(u"displaystyle", display);
    if (!attributes().own(u"scriptlevel"))
        attributes().setInherited(u"scriptlevel", level);
    if (prime)
        properties().tex.prime_style = true;

    const double expected = arity();
    if (expected >= 0.0 && std::isfinite(expected)) {
        std::size_t count = children().size();
        if ((expected == 1.0 && count == 0) ||
            (expected != 1.0 && static_cast<double>(count) != expected)) {
            if (expected < static_cast<double>(count)) {
                document().setChildren(*this, children().first(static_cast<std::size_t>(expected)));
            } else {
                while (static_cast<double>(children().size()) < expected) {
                    MmlNode &row = document().factory().make(NodeKind::mrow);
                    appendChild(row);
                }
            }
        }
    }

    if (linebreakContainer() && !isEmbellished()) {
        const auto alignName = ascii16(linebreakAlign());
        if (!alignName.empty()) {
            AttributeValue const *align = attributes().lookup(alignName);
            const AttributeValue alignValue =
                truthy(align) ? *align : AttributeValue{std::u16string(u"left")};
            AttributeAdditions additions{{u"indentalign", alignValue},
                                         {u"indentshift", std::u16string(u"0")},
                                         {u"indentalignfirst", alignValue},
                                         {u"indentshiftfirst", std::u16string(u"0")},
                                         {u"indentalignlast", std::u16string(u"indentalign")},
                                         {u"indentshiftlast", std::u16string(u"indentshift")}};
            inherited = addInheritedAttributes(inherited, additions);
        }
    }
    setChildInheritedAttributes(inherited, display, level, prime);
}

void AbstractMmlNode::setChildInheritedAttributes(AttributeList const &inherited, bool display,
                                                  double level, bool prime) {
    for (MmlNode *child : children())
        child->setInheritedAttributes(inherited, display, level, prime);
}

AttributeList AbstractMmlNode::addInheritedAttributes(AttributeList const &current,
                                                      AttributeAdditions const &additions) const {
    AttributeList updated = current;
    for (const auto &[name, value] : additions)
        if (!is_special_inherited_name(name)) {
            const auto found =
                std::find_if(updated.begin(), updated.end(),
                             [&](InheritedAttribute const &item) { return item.name == name; });
            if (found == updated.end())
                updated.push_back({name, kind(), value});
            else
                *found = {name, kind(), value};
        }
    return updated;
}

void AbstractMmlNode::inheritAttributesFrom(MmlNode &source) {
    AttributeList inherited;
    if (source.attributes().isSet(u"mathsize"))
        if (auto value = source.attributes().lookup(u"mathsize"))
            inherited.push_back({u"mathsize", NodeKind::math, *value});
    const bool display = truthy(source.attributes().lookup(u"displaystyle"));
    const double level = number(source.attributes().lookup(u"scriptlevel"));
    const bool prime = source.properties().tex.prime_style.value_or(false);
    setInheritedAttributes(inherited, display, level, prime);
}

void AbstractMmlNode::verifyTree(std::optional<VerifyOptions> const &options) {
    if (!options)
        return;
    verifyAttributes(*options);
    const double expected = arity();
    if (options->checkArity && expected >= 0.0 && !(std::isinf(expected) && expected > 0.0)) {
        const bool wrong = expected == 1.0 ? children().empty()
                                           : static_cast<double>(children().size()) != expected;
        if (wrong)
            (void)mError(ascii16("Wrong number of children for \"" +
                                 std::string(nodeKindName(kind())) + "\" node"),
                         *options, true);
    }
    verifyChildren(*options);
}

void AbstractMmlNode::verifyAttributes(VerifyOptions const &options) {
    if (options.checkAttributes) {
        std::vector<std::u16string> bad;
        for (const auto &[name, value] : attributes().ownEntries()) {
            (void)value;
            const bool data = name.starts_with(u"data-");
            const bool allowed = name == u"class" || name == u"style" || name == u"id" ||
                                 name == u"href" || name == u"xlink:href";
            if (!data && !allowed && !attributes().hasDefault(name))
                bad.push_back(name);
        }
        if (!bad.empty()) {
            std::u16string message = u"Unknown attributes for ";
            message += ascii16(nodeKindName(kind()));
            message += u" node: ";
            for (std::size_t i = 0; i < bad.size(); ++i) {
                if (i)
                    message += u", ";
                message += bad[i];
            }
            (void)mError(std::move(message), options, false);
        }
    }
    if (options.checkMathvariants) {
        const auto *variant = attributes().own(u"mathvariant");
        if (truthy(variant) && !properties().tex.ignore_variant.value_or(false)) {
            const auto *value = std::get_if<std::u16string>(variant);
            const bool valid = value && is_ascii(*value) &&
                               std::find(MATHVARIANTS.begin(), MATHVARIANTS.end(),
                                         utf8_ascii(*value)) != MATHVARIANTS.end();
            if (!valid)
                (void)mError(u"Invalid mathvariant: " + attribute_text(*variant), options, true);
        }
    }
}

void AbstractMmlNode::verifyChildren(VerifyOptions const &options) {
    const auto childOptions = std::optional<VerifyOptions>{options};
    for (MmlNode *child : children())
        child->verifyTree(childOptions);
}

MmlNode *AbstractMmlNode::mError(std::u16string message, VerifyOptions const &options,
                                 bool shortError) {
    if (parent() && parent()->isKind(NodeKind::merror))
        return nullptr;
    MmlNode &error = document().factory().make(NodeKind::merror);
    error.attributes().set(u"data-mjx-message", message);
    if (options.fullErrors || shortError) {
        MmlNode &mtext = document().factory().make(NodeKind::mtext);
        MmlNode &text = document().factory().make(NodeKind::text);
        text.setText(options.fullErrors ? message : ascii16(nodeKindName(kind())));
        mtext.appendChild(text);
        error.appendChild(mtext);
        if (!parent())
            throw std::logic_error("mError requires a parent node");
        parent()->replaceChild(error, *this);
        if (!options.fullErrors)
            error.attributes().set(u"title", message);
    } else {
        if (!parent())
            throw std::logic_error("mError requires a parent node");
        parent()->replaceChild(error, *this);
        error.appendChild(*this);
    }
    return &error;
}

MmlNode &AbstractMmlNode::copy(bool keepIds) const {
    AttributeSet own;
    for (const auto &[name, value] : attributes().ownEntries())
        if (keepIds || name != u"id")
            own.set(name, value);
    MmlNode &result = document().factory().make(kind(), std::move(own));
    copyNodeProperties(properties(), result.properties());
    auto childrenToCopy = children();
    if (childrenToCopy.size() == 1 && childrenToCopy.front()->isInferred())
        childrenToCopy = childrenToCopy.front()->children();
    for (MmlNode *child : childrenToCopy)
        if (child)
            result.appendChild(child->copy(false));
        else
            document().setChildSlot(result, result.children().size(), nullptr);
    return result;
}
} // namespace folia::math::mml
