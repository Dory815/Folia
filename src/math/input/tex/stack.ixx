// MathJax 4.1.3 input/tex/Stack.ts. Apache-2.0.
export module folia.math.input.tex.stack;
export import folia.math.input.tex.stack_item;
import std;
export namespace folia::math::tex {
class Stack {
    StackFactoryReference factory_=Missing{};
    Property env_=Missing{};
    // This is the Source parser stack. Entries retain actual BaseItem owners;
    // each BaseItem separately owns its existing MML child-array handle.
    std::vector<StackItemRef> stack_;
protected:
    Stack()=default;
public:
    Property global=Missing{};
    explicit Stack(StackFactoryReference,Property env,Property inner);
    virtual ~Stack()=default;
    Stack(const Stack&)=delete;
    Stack& operator=(const Stack&)=delete;
    // Source phase for actual subclasses: allocate before invoking virtuals.
    void initializeStack(StackFactoryReference,Property env,Property inner);
    virtual void setEnv(Property);
    virtual Property env()const;
    virtual void Push(std::span<const StackEntry>);
    virtual StackEntry Pop();
    virtual StackEntry Top(Property n=Missing{})const;
    virtual mml::NodeValue Prev(Property noPop=Missing{});
    virtual double height()const;
    virtual std::string toString()const;
};
}
