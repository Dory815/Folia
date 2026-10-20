// MathJax 4.1.3 StackItem.ts; Apache-2.0.
module folia.math.input.tex.parser;
import :stack_item_factory;
import :base_items;
import std;
namespace folia::math::tex {
CheckResult::CheckResult(CheckAction value) : action(value) {}
CheckResult::CheckResult(std::vector<ReplayEntry> values)
    : action(CheckAction::replay), replacements(std::move(values)) {}
CheckResult::~CheckResult() = default;
CheckResult::CheckResult(CheckResult &&) noexcept = default;
CheckResult &CheckResult::operator=(CheckResult &&) noexcept = default;
MmlStack::MmlStack(std::vector<mml::MmlNode *> values) : nodes_(std::move(values)) {}
std::vector<mml::MmlNode *> &MmlStack::nodes() noexcept {
    return nodes_;
}
std::vector<mml::MmlNode *> const &MmlStack::nodes() const noexcept {
    return nodes_;
}
std::u16string_view MmlStack::sourceAtCreation() const noexcept {
    return startStr ? std::u16string_view(*startStr) : std::u16string_view{};
}
void MmlStack::Push(std::span<mml::MmlNode *const> values) {
    nodes_.insert(nodes_.end(), values.begin(), values.end());
}
mml::MmlNode *MmlStack::Pop() noexcept {
    if (nodes_.empty())
        return nullptr;
    auto node = nodes_.back();
    nodes_.pop_back();
    return node;
}
mml::MmlNode *MmlStack::First() const noexcept {
    return nodes_.empty() ? nullptr : nodes_.back();
}
void MmlStack::setFirst(mml::MmlNode &node) {
    if (nodes_.empty())
        throw std::out_of_range("Empty MML stack has no first node");
    nodes_.back() = &node;
}
mml::MmlNode *MmlStack::Last() const noexcept {
    return nodes_.empty() ? nullptr : nodes_.front();
}
void MmlStack::setLast(mml::MmlNode &node) {
    if (nodes_.empty())
        nodes_.push_back(&node);
    else
        nodes_.front() = &node;
}
std::vector<mml::MmlNode *> MmlStack::Peek(std::size_t count) const {
    // Source slice(size-count): a negative start counts back from size again.
    auto start =
        count <= nodes_.size()
            ? nodes_.size() - count
            : (count - nodes_.size() < nodes_.size() ? nodes_.size() - (count - nodes_.size()) : 0);
    return {nodes_.begin() + static_cast<std::ptrdiff_t>(start), nodes_.end()};
}
std::size_t MmlStack::Size() const noexcept {
    return nodes_.size();
}
void MmlStack::Clear() noexcept {
    nodes_.clear();
}
mml::MmlNode &MmlStack::toMml(bool inferred, bool forceRow) {
    if (nodes_.size() == 1 && !forceRow)
        return *nodes_.front();
    return create(inferred ? mml::NodeKind::inferred_mrow : mml::NodeKind::mrow, nodes_);
}
mml::MmlNode &MmlStack::create(mml::NodeKind kind, std::span<mml::MmlNode *const> children,
                               mml::AttributeSet attributes) {
    return factory().configuration().nodeFactory.makeNode(kind, children, std::move(attributes));
}
mml::MmlNode &MmlStack::createToken(mml::NodeKind kind, std::u16string_view text,
                                    mml::AttributeSet attributes) {
    return factory().configuration().nodeFactory.makeToken(kind, text, std::move(attributes));
}
BaseItem::BaseItem(StackItemFactory &owner, bool open, std::vector<mml::MmlNode *> values)
    : MmlStack(std::move(values)), factory_(owner),
      env_(open ? std::make_shared<TexEnvironment>() : nullptr) {}
StackItemFactory &BaseItem::factory() const {
    return factory_;
}
StackKind BaseItem::kind() const {
    return StackKind::base;
}
void BaseItem::setGlobal(StackGlobal &global) noexcept {
    global_ = &global;
}
StackGlobal &BaseItem::global() const {
    if (!global_)
        throw std::logic_error("Stack frame has not been activated");
    return *global_;
}
EnvironmentScope BaseItem::environmentScope() const noexcept {
    return env_;
}
void BaseItem::setEnv(EnvironmentScope value) {
    env_ = std::move(value);
}
TexEnvironment &BaseItem::env() {
    if (!env_)
        throw std::logic_error("Stack frame has no environment");
    return *env_;
}
TexEnvironment const &BaseItem::env() const {
    if (!env_)
        throw std::logic_error("Stack frame has no environment");
    return *env_;
}
void BaseItem::clearEnv() {
    env() = {};
}
bool BaseItem::copyEnv() const noexcept {
    return true;
}
bool BaseItem::isOpen() const noexcept {
    return false;
}
bool BaseItem::isClose() const noexcept {
    return false;
}
bool BaseItem::isFinal() const noexcept {
    return false;
}
bool BaseItem::isKind(StackKind value) const {
    return kind() == value;
}
std::optional<std::u16string_view> BaseItem::getName() const noexcept {
    return std::nullopt;
}
std::optional<StackErrorDefinition> BaseItem::getErrors(StackErrorKey key) const {
    switch (key) {
    case StackErrorKey::end:
        return StackErrorDefinition{"MissingBeginExtraEnd",
                                    u"Missing \\begin{%1} or extra \\end{%1}"};
    case StackErrorKey::close:
        return StackErrorDefinition{"ExtraCloseMissingOpen",
                                    u"Extra close brace or missing open brace"};
    case StackErrorKey::right:
        return StackErrorDefinition{"MissingLeftExtraRight", u"Missing \\left or extra \\right"};
    case StackErrorKey::middle:
        return StackErrorDefinition{"ExtraMiddle", u"Extra \\middle"};
    default:
        return std::nullopt;
    }
}
CheckResult BaseItem::checkItem(FrameHandle const &incoming) {
    if (incoming->isKind(StackKind::over) && isOpen()) {
        dynamic_cast<OverItem &>(*incoming).numerator = &toMml(false);
        Clear();
    }
    if (incoming->isKind(StackKind::cell) && isOpen()) {
        if (dynamic_cast<CellItem &>(*incoming).linebreak.value_or(false))
            return CheckAction::drop;
        std::u16string name(incoming->getName().value_or(u"undefined"));
        throw TexError("Misplaced", u"Misplaced %1", {&name, 1});
    }
    std::optional<StackErrorKey> key;
    switch (incoming->kind()) {
    case StackKind::end:
        key = StackErrorKey::end;
        break;
    case StackKind::close:
        key = StackErrorKey::close;
        break;
    case StackKind::right:
        key = StackErrorKey::right;
        break;
    case StackKind::middle:
        key = StackErrorKey::middle;
        break;
    case StackKind::stop:
        key = StackErrorKey::stop;
        break;
    default:
        break;
    }
    if (incoming->isClose() && key)
        if (auto error = getErrors(*key)) {
            std::u16string name(incoming->getName().value_or(u"undefined"));
            throw TexError(std::move(error->id), std::move(error->message), {&name, 1});
        }
    if (!incoming->isFinal())
        return CheckAction::accept;
    if (auto node = incoming->First())
        Push({&node, 1});
    return CheckAction::drop;
}
void BaseItem::addLatexItem(mml::MmlNode &node, std::u16string_view prefix) {
    auto source = sourceAtCreation();
    auto start = std::min(startI, source.size()), stop = std::min(stopI, source.size());
    auto part = source.substr(start, stop > start ? stop - start : 0);
    if (part.empty())
        return;
    std::u16string value(prefix);
    value += part;
    node.attributes().set(u"data-latex-item", value);
    if (value != u"}")
        node.attributes().set(u"data-latex", std::move(value));
}
std::string BaseItem::toString() const {
    static constexpr std::array names{
        "base",      "start",     "stop",   "open",       "close",    "null",
        "prime",     "subsup",    "over",   "left",       "middle",   "right",
        "break",     "begin",     "end",    "style",      "position", "cell",
        "mml",       "fn",        "not",    "nonscript",  "dots",     "array",
        "eqnarray",  "equation",  "mstyle", "casesBegin", "multline", "flalign",
        "multlined", "proofTree", "braket", "beginEnv",   "autoOpen", "dummy"};
    std::string result(names.at(static_cast<std::size_t>(kind())));
    result += '[';
    for (std::size_t i = 0; i < nodes().size(); ++i) {
        if (i)
            result += "; ";
        result += nodes()[i]->toString();
    }
    return result + ']';
}
} // namespace folia::math::tex
