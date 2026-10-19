/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/TeXAtom.ts placement.
 *************************************************************/
export module folia.math.output.scene:tex_atom;
export import :wrapper;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class TeXAtomPlacement final : public SvgWrapperPlacement {
protected:
    void placeContent(common::CommonWrapper&,Builder&,WrapperFactory&,
                      std::span<const NodeRef>) override;
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                               std::span<const NodeRef>) override;
};
void registerTeXAtomPlacement(WrapperFactory&);
}
