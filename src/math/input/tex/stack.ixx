export module folia.math.input.tex.parser:stack;
export import :stack_item;
import std;

export namespace folia::math::tex {
class Stack {
    StackItemFactory &factory_;
    StackGlobal global_;
    EnvironmentScope env_;
    std::vector<FrameHandle> stack_;

    void accept(FrameHandle);
    void replay(CheckResult, FrameHandle top, FrameHandle incoming);

  public:
    Stack(StackItemFactory &, TexEnvironment environment = {}, bool inner = true);
    ~Stack();
    Stack(Stack const &) = delete;
    Stack &operator=(Stack const &) = delete;
    Stack(Stack &&) = delete;
    Stack &operator=(Stack &&) = delete;

    StackGlobal &global() noexcept;
    StackGlobal const &global() const noexcept;
    TexEnvironment &env() noexcept;
    TexEnvironment const &env() const noexcept;
    EnvironmentScope environmentScope() const noexcept;
    void setEnv(EnvironmentScope);
    void Push(StackEntry);
    void PushAll(std::vector<StackEntry>);
    FrameHandle Pop();
    BaseItem *Top(std::size_t n = 1) const noexcept;
    mml::MmlNode *Prev(bool noPop = false);
    std::size_t height() const noexcept;
    std::string toString() const;
};
} // namespace folia::math::tex
