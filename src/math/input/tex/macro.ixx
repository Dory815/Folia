// Root declarations for MathJax 4.1.3 Token.ts::Macro. Apache-2.0.
export module folia.math.input.tex.macro;
export import folia.math.input.tex.types;
import std;
export namespace folia::math::tex {
class Macro:public NativeObject {
    Property token_;
    ParseMethod func_;
    Property args_;
public:
    Macro(Property token,ParseMethod func,Property args=Missing{});
    virtual ~Macro()=default;
    virtual Property token() const;
    virtual ParseMethod func() const;
    virtual Property args() const;
    Property sourceTokenField()const{return token_;}
    void sourceTokenField(Property value){token_=std::move(value);}
    ParseMethod sourceFuncField()const{return func_;}
    void sourceFuncField(ParseMethod value){func_=std::move(value);}
    Property sourceArgsField()const{return args_;}
    void sourceArgsField(Property value){args_=std::move(value);}
};
using MacroRef=std::shared_ptr<Macro>;
}
