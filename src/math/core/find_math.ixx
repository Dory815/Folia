// MathJax 4.1.3 core/FindMath.ts; Apache-2.0.
export module folia.math.core.find_math;
export import folia.math.core.math_item;
export import folia.math.util.options;
import std;
export namespace folia::math::core {
class FindMath:public NativeObject {
public:
    virtual ProtoMathItems findMath(std::any where)=0;
};
class AbstractFindMath:public FindMath {
    options::Context& context_;
    options::ObjectRef identity_;
protected:
    options::Value options_=options::Undefined{};
    AbstractFindMath(options::Context& context,options::ObjectRef identity)
        :context_(context),identity_(identity){}
public:
    AbstractFindMath(const AbstractFindMath&)=delete;
    AbstractFindMath& operator=(const AbstractFindMath&)=delete;
    // Actual Source base phase, before FindTeX.getPatterns.
    void initializeFindMath(options::Value supplied);
    options::Context& optionsContext()const{return context_;}
    options::ObjectRef instanceIdentity()const{return identity_;}
};
}
