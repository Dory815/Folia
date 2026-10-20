/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mtr.ts.
 * Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common:mtr;
export import :wrapper;
export import folia.math.core.mml_tree.mml_nodes.mtr;

export namespace folia::math::output::common {
class CommonMtd;
struct TableRowGeometry {
    double h{}, d{}, t_space{}, b_space{}, t_line{}, b_line{};
};
struct TableCellPlacement {
    double x{}, y{}, l_space{}, width{}, r_space{}, l_line{}, r_line{};
};
struct TableRowScene {
    SceneNodes nodes;
    SceneNodes label_nodes;
};
class CommonMtr : public CommonWrapper {
  public:
    using CommonWrapper::CommonWrapper;
    virtual TableRowScene emitTableScene(scene::Builder &, std::span<scene::NodeId const> parents,
                                         TableRowGeometry const &,
                                         std::optional<scene::NodeId> label_parent = {});
    double placeSceneCell(scene::Builder &, CommonMtd &, std::span<SceneNode const>,
                          TableRowGeometry const &, TableCellPlacement const &);
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
    TableRowScene emitTableScene(scene::Builder &, std::span<scene::NodeId const> parents,
                                 TableRowGeometry const &,
                                 std::optional<scene::NodeId> label_parent = {}) override;
    std::size_t numCells() const override;

    bool labeled() const override {
        return true;
    }

    std::span<CommonWrapper *const> tableCells() const override;
    CommonWrapper *getChild(std::size_t) const override;
    std::vector<BBox> getChildBBoxes() const override;
};
} // namespace folia::math::output::common
