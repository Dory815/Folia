/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mi.ts.
 *************************************************************/
export module folia.math.output.common:mi;
export import :wrapper;
import std;
export namespace folia::math::output::common {
class CommonMi:public CommonWrapper {
public:
    using CommonWrapper::CommonWrapper;
protected:
    void computeBBox(BBox&,bool recompute=false)override;
};
}
