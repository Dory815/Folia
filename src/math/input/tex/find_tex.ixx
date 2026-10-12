// MathJax 4.1.3 input/tex/FindTeX.ts; Apache-2.0.
export module folia.math.input.tex.find_tex;
export import folia.math.core.find_math;
export import folia.math.util.regexp_binding;
import std;
export namespace folia::math::tex {
class FindTeX:public core::AbstractFindMath {
protected:
    // Source local starts array has one native typed owner. It is not an option
    // dictionary or a synchronized copy of a Context array.
    using StringArray=std::shared_ptr<std::vector<std::u16string>>;
    using Delims=options::Value;
    using EndItem=options::Value;
    regexp_host::RegExpBindings& regexp_;
    options::Value start_=options::Undefined{},end_=options::Undefined{};
    options::Value hasPatterns_=options::Undefined{},env_=options::Undefined{},sub_=options::Undefined{};
    virtual void getPatterns();
    virtual void addPattern(StringArray starts,Delims delims,bool display);
    virtual options::Value endPattern(options::Value end,options::Value endp=options::Undefined{});
    virtual Property findEnd(options::Value text,double n,const regexp_host::Match&start,EndItem end);
    virtual void findMathInString(core::ProtoMathItems math,double n,options::Value text);
public:
    // Allocation/borrowed binding only; actual Source phases are explicit.
    FindTeX(options::Context&context,options::ObjectRef identity,regexp_host::RegExpBindings&regexp)
        :AbstractFindMath(context,identity),regexp_(regexp){
        if(&regexp.context()!=&context)throw options::Exception("TypeError",u"FindTeX bindings belong to another Context");
    }
    void initializeFindTeX();
    core::ProtoMathItems findMath(std::any strings)override;
};
}
