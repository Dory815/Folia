// MathJax 4.1.3 input/tex/begingroup/BegingroupStack.ts.
export module folia.math.input.tex.begingroup.stack;
export import folia.math.input.tex.parser;
import std;
export namespace folia::math::tex {
class BegingroupStack final:public GlobalDefinitionScope,
                            public std::enable_shared_from_this<BegingroupStack> {
    ParserRuntime& runtime_;
    options::Value i_=-100.0,top_=-100.0,base_=-100.0;
    options::Value marker_=options::Undefined{};
    std::shared_ptr<SubHandlers> handlers_;
    std::array<TokenMapRef,3> global_{}; // delimiter, environment, command
    void getGlobal();
public:
    static constexpr double newcommandPriority=-100;
    static void initializeModule(ParserRuntime&);
    BegingroupStack(ParserRuntime&,ParseOptions&);
    void initializeStack(ParseOptions&);
    double index()const;
    double top()const;
    double base()const;
    options::Value marker()const{return marker_;}
    void bindSourceFields(options::ObjectRef);
    std::shared_ptr<SubHandlers> handlers()const{return handlers_;}
    TokenMapRef global(std::u16string_view)const;
    std::vector<TokenMapRef> checkGlobal(PropertyArray tokens,PropertyArray maps)override;
    virtual void push();
    virtual void pop();
    virtual void finish();
    virtual void remove();
    virtual void reset();
    virtual void sandbox();
};
using BegingroupStackRef=std::shared_ptr<BegingroupStack>;
BegingroupStackRef begingroupStack(ParseOptions&);
BegingroupStackRef begingroupStack(TexParser&);
}
