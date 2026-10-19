/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/ms.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common:ms;
export import :wrapper;
export import folia.math.core.mml_tree.mml_nodes.ms;

export namespace folia::math::output::common {
class CommonMs : public CommonWrapper {
public:
    CommonMs(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    void initializeSource() override;
    CommonWrapper* createText(std::u16string_view text);
};
}
