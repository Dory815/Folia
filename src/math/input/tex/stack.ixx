// MathJax 4.1.3 input/tex/Stack.ts. Apache-2.0.
export module folia.math.input.tex.stack;
export import folia.math.input.tex.stack_item;
export import folia.math.input.tex.stack_array_access;
import std;
export namespace folia::math::tex {
class Stack {
    StackFactoryReference factory_=Missing{};
    Property env_=Missing{};
    StackArrayAccessRef stack_;
protected:
    Stack()=default;
public:
    Property global=Missing{};
    explicit Stack(StackFactoryReference,Property env,Property inner);
    virtual ~Stack()=default;
    Stack(const Stack&)=delete;
    Stack& operator=(const Stack&)=delete;
    void initializeStack(StackFactoryReference,Property env,Property inner);
    virtual void setEnv(Property);
    virtual Property env()const;
    virtual void Push(std::span<const StackEntry>);
    virtual StackEntry Pop();
    virtual StackEntry Top(StackArrayScalar n=Property(Missing{}))const;
    virtual mml::NodeValue Prev(Property noPop=Missing{});
    // Source returns this.stack.length without coercion.
    virtual options::Value height()const;
    virtual std::string toString();
    const StackArrayAccessRef& stackArray()const noexcept{return stack_;}
    void replaceStackArray(StackArrayAccessRef value){stack_=std::move(value);}
    const StackFactoryReference& sourceFactory()const noexcept{return factory_;}
    void setSourceFactory(StackFactoryReference value){factory_=std::move(value);}
    double numericHeight()const{return stack_->number(StackArrayScalar(height()));}
};
}
