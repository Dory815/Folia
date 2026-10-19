/*************************************************************
 * MathJax 4.1.3 output/svg/Wrapper.ts glyph and text paint primitives.
 *************************************************************/
export module folia.math.output.scene:glyph_paint;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
struct GlyphResult { double width=0; bool emitted=false; };
std::shared_ptr<std::vector<std::uint32_t>> glyphUnicodeChars(
    common::CommonWrapper&,std::u16string_view,std::string_view variant);
Property glyphSlot(const common::CharDataArray&,std::size_t);
GlyphResult emitGlyph(common::CommonWrapper&,Builder&,WrapperFactory&,const NodeRef&,
    std::uint32_t,double,double,std::string_view,unsigned depth=0,std::optional<bool> buffer={},
    bool observeCall=true,const common::CharDataArray* sourceData=nullptr);
NodeRef emitTextRun(common::CommonWrapper&,Builder&,const NodeRef&,std::u16string,
    std::string_view,double,double,bool place=true);
void placeGlyphText(common::CommonWrapper&,Builder&,WrapperFactory&,const NodeRef&,
    std::span<const std::uint32_t>,std::string_view,bool buffer=false);
void emitGlyphToRoots(common::CommonWrapper&,Builder&,WrapperFactory&,
    std::span<const NodeRef>,std::uint32_t,double,double,std::string_view,
    double scaleX=1,double scaleY=1);
void emitGlyphExtender(common::CommonWrapper&,Builder&,WrapperFactory&,
    std::span<const NodeRef>,std::uint32_t,std::string_view,bool,double,double,
    double,double,double,double,double,double,double);
NodeRef cloneSceneNode(const NodeRef&);
}
