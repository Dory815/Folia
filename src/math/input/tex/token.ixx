// Root declarations for MathJax 4.1.3 Token.ts::Token. Apache-2.0.
export module folia.math.input.tex.token;
export import folia.math.core.tree.property;
import std;
export namespace folia::math::tex {
class Token {
    Property token_, character_, attributes_;
public:
    Token(Property token,Property character,Property attributes=Missing{});
    virtual ~Token()=default;
    virtual Property token() const;
    virtual Property character() const;
    virtual Property attributes() const;
};
using TokenRef=std::shared_ptr<Token>;
}
