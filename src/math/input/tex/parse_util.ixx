// MathJax 4.1.3 input/tex/ParseUtil.ts; Apache-2.0.
export module folia.math.input.tex.parser:parse_util;
export import :types;
import std;
export namespace folia::math::tex {
// Source generics erase to one class identity; conversion uses the canonical
// FunctionRef body, whose result may be any supported native value.
class KeyValueDef:public NativeObject {
public:
    std::u16string name;
    FunctionRef verify,convert;
    KeyValueDef(std::u16string,FunctionRef,FunctionRef);
    static KeyValueType oneof(ParserRuntime&,std::span<const std::u16string>);
};
using KeyValueFn=FunctionRef;
// The exported KeyValueTypes ordinary record belongs to the loaded runtime.
struct ParseUtil {
    static std::u16string cols(std::span<const double>);
    static mml::NodeValue fenced(ParseOptions&,std::u16string,mml::NodeValue,std::u16string,std::u16string big=u"",std::u16string color=u"");
    static mml::NodeValue fixedFence(ParseOptions&,std::u16string,mml::NodeValue,std::u16string);
    static mml::NodeValue mathPalette(ParseOptions&,std::u16string,std::u16string side);
    static void fixInitialMO(ParseOptions&,mml::Children);
    static mml::Children internalMath(TexParser&,std::u16string,Property level=Missing{},Property font=Missing{});
    static mml::NodeValue internalText(TexParser&,std::u16string,Property def);
    static mml::NodeValue underOver(TexParser&,mml::NodeValue,mml::NodeValue,std::u16string pos,bool stack);
    static void checkMovableLimits(mml::NodeValue);
    static ArrayItem& setArrayAlign(ArrayItem&,std::u16string,TexParser* parser=nullptr);
    static std::u16string substituteArgs(TexParser&,options::Value args,std::u16string);
    static std::u16string addArgs(TexParser&,std::u16string,std::u16string);
    static void checkMaxMacros(TexParser&,bool isMacro=true);
    static void checkEqnEnv(TexParser&,bool nestable=true);
    static mml::NodeValue copyNode(mml::NodeValue,TexParser&);
    static std::u16string mmlFilterAttribute(TexParser&,std::u16string,std::u16string);
    static Property getFontDef(TexParser&);
    static Property keyvalOptions(ParserRuntime&,std::u16string,options::Value allowed=options::Null{},bool error=false,bool l3keys=false);
    static bool isLatinOrGreekChar(ParserRuntime&,std::u16string);
};
}
