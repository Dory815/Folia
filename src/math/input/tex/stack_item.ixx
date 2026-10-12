// MathJax 4.1.3 input/tex/StackItem.ts, MmlStack. Apache-2.0.
export module folia.math.input.tex.stack_item;
export import folia.math.input.tex.stack_services;
import std;
export namespace folia::math::tex {
class MmlStack {
    mml::Children nodes_;
protected:
    explicit MmlStack(mml::Children nodes):nodes_(std::move(nodes)){}
    virtual mml::Children nodes()const{return nodes_;}
    virtual StackFactoryReference factory()const=0;
public:
    std::u16string startStr;
    double startI=0,stopI=0;
    virtual ~MmlStack()=default;
    MmlStack(const MmlStack&)=delete;
    MmlStack& operator=(const MmlStack&)=delete;
    virtual void Push(std::span<const mml::NodeValue> nodes);
    virtual mml::NodeValue Pop();
    virtual mml::NodeValue First()const;
    virtual void setFirst(mml::NodeValue);
    virtual mml::NodeValue Last()const;
    virtual void setLast(mml::NodeValue);
    virtual mml::Children Peek(Property count=Missing{})const;
    virtual double Size()const;
    virtual void Clear();
    virtual mml::NodeValue toMml(Property inferred=Missing{},Property forceRow=Missing{});
    virtual mml::NodeValue create(std::string_view kind,FunctionArgs rest={});
};
}
