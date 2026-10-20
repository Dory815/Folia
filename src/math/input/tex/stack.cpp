// MathJax 4.1.3 Stack.ts; Apache-2.0.
module folia.math.input.tex.parser;
import :stack_item_factory;
import :stack;
import std;
namespace folia::math::tex {
Stack::Stack(StackItemFactory &factory, TexEnvironment environment, bool inner)
    : factory_(factory), global_{.is_inner = inner},
      env_(std::make_shared<TexEnvironment>(std::move(environment))) {
    auto start = factory_.create(StackKind::start, StartItemInit{global_});
    start->setEnv(env_);
    stack_.push_back(std::move(start));
}
Stack::~Stack() = default;
StackGlobal &Stack::global() noexcept {
    return global_;
}
StackGlobal const &Stack::global() const noexcept {
    return global_;
}
TexEnvironment &Stack::env() noexcept {
    return *env_;
}
TexEnvironment const &Stack::env() const noexcept {
    return *env_;
}
EnvironmentScope Stack::environmentScope() const noexcept {
    return env_;
}
void Stack::setEnv(EnvironmentScope value) {
    if (!value)
        throw std::invalid_argument("Stack requires an environment");
    env_ = std::move(value);
}
void Stack::accept(FrameHandle item) {
    if (!item->isKind(StackKind::null_item))
        stack_.push_back(item);
    if (auto local = item->environmentScope()) {
        if (item->copyEnv())
            overlayEnvironment(*local, *env_);
        env_ = std::move(local);
    } else
        item->setEnv(env_);
}
void Stack::replay(CheckResult result, FrameHandle top, FrameHandle incoming) {
    Pop();
    for (auto const &entry : result.replacements)
        std::visit(
            [&](auto const &value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, ReuseTop>)
                    Push(top);
                else if constexpr (std::is_same_v<T, ReuseIncoming>)
                    Push(incoming);
                else
                    Push(value);
            },
            entry);
}
void Stack::Push(StackEntry entry) {
    FrameHandle incoming;
    if (auto node = std::get_if<mml::MmlNode *>(&entry)) {
        if (!*node)
            return;
        incoming = factory_.create(StackKind::mml, MmlItemInit{**node});
    } else if (auto frame = std::get_if<FrameHandle>(&entry))
        incoming = *frame;
    if (!incoming)
        return;
    incoming->setGlobal(global_);
    auto top = stack_.empty() ? FrameHandle{} : stack_.back();
    auto result = top ? top->checkItem(incoming) : CheckResult(CheckAction::accept);
    if (result.action == CheckAction::drop)
        return;
    if (result.action == CheckAction::replay) {
        replay(std::move(result), std::move(top), std::move(incoming));
        return;
    }
    accept(std::move(incoming));
}
void Stack::PushAll(std::vector<StackEntry> values) {
    for (auto &value : values)
        Push(std::move(value));
}
FrameHandle Stack::Pop() {
    if (stack_.empty())
        throw std::out_of_range("Cannot pop an empty TeX stack");
    auto item = std::move(stack_.back());
    stack_.pop_back();
    // Source delete item.env leaves the inherited accessor backing _env intact.
    env_ = stack_.empty() ? std::make_shared<TexEnvironment>() : stack_.back()->environmentScope();
    return item;
}
BaseItem *Stack::Top(std::size_t n) const noexcept {
    return n == 0 || n > stack_.size() ? nullptr : stack_[stack_.size() - n].get();
}
mml::MmlNode *Stack::Prev(bool noPop) {
    auto top = Top();
    return !top ? nullptr : noPop ? top->First() : top->Pop();
}
std::size_t Stack::height() const noexcept {
    return stack_.size();
}
std::string Stack::toString() const {
    std::string text = "stack[\n  ";
    for (std::size_t i = 0; i < stack_.size(); ++i) {
        if (i)
            text += "\n  ";
        text += stack_[i]->toString();
    }
    return text + "\n]";
}
} // namespace folia::math::tex
