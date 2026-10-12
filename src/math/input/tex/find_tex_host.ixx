// Root actual FindTeX static metadata/allocation binding. No scanner algorithms.
export module folia.math.input.tex.find_tex_host;
export import folia.math.input.tex.find_tex;
export import folia.math.core.find_math_host;
import std;
export namespace folia::math::tex {
class FindTeXModule {
    options::Context& context_;
    regexp_host::RegExpBindings& regexp_;
    options::ObjectRef constructor_;
    options::ObjectRef pair(std::u16string open,std::u16string close){
        auto tuple=context_.array(2);context_.set(tuple,options::Key(u"0"),std::move(open));
        context_.set(tuple,options::Key(u"1"),std::move(close));return tuple;
    }
public:
    // One module per Context/runtime; the RegExp binding owner is shared.
    FindTeXModule(options::Context&context,core::FindMathModule&base,regexp_host::RegExpBindings&regexp)
        :context_(context),regexp_(regexp),constructor_(context.constructor(u"FindTeX")){
        if(&regexp.context()!=&context)throw options::Exception("TypeError",u"FindTeX binding context mismatch");
        context.setPrototype(constructor_,base.abstractClass());
        auto prototype=std::get<options::ObjectRef>(context.get(constructor_,options::Key(u"prototype")).data);
        auto basePrototype=std::get<options::ObjectRef>(context.get(base.abstractClass(),options::Key(u"prototype")).data);
        context.setPrototype(prototype,basePrototype);
        const auto defaults=context.object(),inlines=context.array(1),displays=context.array(2);
        context.set(inlines,options::Key(u"0"),pair(u"\\(",u"\\)"));
        context.set(displays,options::Key(u"0"),pair(u"$$",u"$$"));
        context.set(displays,options::Key(u"1"),pair(u"\\[",u"\\]"));
        context.set(defaults,options::Key(u"inlineMath"),inlines);
        context.set(defaults,options::Key(u"displayMath"),displays);
        context.set(defaults,options::Key(u"processEscapes"),true);
        context.set(defaults,options::Key(u"processEnvironments"),true);
        context.set(defaults,options::Key(u"processRefs"),true);
        context.set(constructor_,options::Key(u"OPTIONS"),defaults);
    }
    FindTeXModule(const FindTeXModule&)=delete;
    FindTeXModule&operator=(const FindTeXModule&)=delete;
    options::ObjectRef constructorIdentity()const{return constructor_;}
    template<class T=FindTeX>std::shared_ptr<T> make(options::Value supplied){
        static_assert(std::is_base_of_v<FindTeX,T>);
        auto result=core::makeFindMath<T>(context_,constructor_,std::move(supplied),regexp_);
        result->initializeFindTeX();return result;
    }
};
}
