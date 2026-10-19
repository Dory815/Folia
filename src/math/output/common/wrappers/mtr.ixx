/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mtr.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common:mtr;
export import :wrapper;
export import folia.math.core.mml_tree.mml_nodes.mtr;

export namespace folia::math::output::common {
class CommonMtr : public CommonWrapper {
  public:
    using CommonWrapper::CommonWrapper;
    virtual std::size_t numCells() const;

    virtual bool labeled() const {
        return false;
    }

    virtual std::span<CommonWrapper *const> tableCells() const;
    virtual CommonWrapper *getChild(std::size_t) const;
    virtual std::vector<BBox> getChildBBoxes() const;
    virtual void stretchChildren(std::optional<std::array<double, 2>> HD = {});

    bool fixesPWidth() const override {
        return false;
    }
};

class CommonMlabeledtr : public CommonMtr {
  public:
    using CommonMtr::CommonMtr;
    std::size_t numCells() const override;

    bool labeled() const override {
        return true;
    }

    std::span<CommonWrapper *const> tableCells() const override;
    CommonWrapper *getChild(std::size_t) const override;
    std::vector<BBox> getChildBBoxes() const override;
};
} // namespace folia::math::output::common
