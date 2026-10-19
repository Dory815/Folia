/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mtable.ts.
 *************************************************************/
export module folia.math.output.common:mtable;
export import :mtr;
import folia.math.core.mml_tree.mml_nodes.mtable;
import std;

export namespace folia::math::output::common {
struct TableData {
    std::vector<double> H, D, W, NH, ND;
    double L{};
};

struct AutomaticColumnWidth {};

using ColumnWidth = std::variant<AutomaticColumnWidth, double, std::string>;
using ColumnWidths = std::vector<ColumnWidth>;

class CommonMtable : public CommonWrapper {
  public:
    std::size_t numCols = 0, numRows = 0;
    bool hasLabels = false, isTop = false, frame = false, fframe = false;
    CommonWrapper *container = nullptr;
    std::size_t containerI = 0;
    double fLine = 0, pWidth = 0;
    std::array<double, 4> fSpace{};
    std::vector<double> cSpace, rSpace, cLines, rLines;
    ColumnWidths cWidths;
    std::optional<TableData> data;
    std::vector<std::pair<CommonWrapper *, std::size_t>> pwidthCells;
    explicit CommonMtable(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr);
    void initializeSource() override;
    std::vector<CommonMtr *> tableRows() const;
    CommonWrapper *findContainer();
    double getPercentageWidth();
    void stretchRows();
    void stretchColumns();
    void stretchColumn(std::size_t, std::optional<double>);
    void breakColumn(std::size_t, double, std::string_view);
    TableData &getTableData();
    void updateHDW(CommonWrapper &, std::size_t, std::size_t, std::string_view, TableData &,
                   std::vector<double> *widths = nullptr);
    void recordPWidthCell(CommonWrapper &, std::size_t);
    void setColumnPWidths();
    std::array<double, 2> getBBoxHD(double) const;
    std::array<double, 2> getBBoxLR();
    std::tuple<double, std::string, double> getPadAlignShift(std::string_view);
    double getWidth();
    void adjustWideTable();
    double naturalWidth();
    double getEqualRowHeight();
    std::vector<double> getComputedWidths();
    ColumnWidths getColumnWidths();
    ColumnWidths getEqualColumns(std::string_view);
    ColumnWidths getColumnWidthsAuto(const std::vector<std::string> &);
    ColumnWidths getColumnWidthsPercent(const std::vector<std::string> &);
    ColumnWidths getColumnWidthsFixed(const std::vector<std::string> &, double);
    void adjustColumnWidths(double);
    double getVerticalPosition(double, std::string_view);
    std::array<double, 4> getFrameSpacing();
    std::vector<std::string> getEmHalfSpacing(const std::array<double, 4> &,
                                              const std::vector<double> &, double = 1) const;
    std::vector<double> getRowHalfSpacing() const;
    std::vector<double> getColumnHalfSpacing() const;
    std::pair<std::string, std::optional<double>> getAlignmentRow() const;
    std::optional<std::vector<std::string>> getColumnAttributes(std::string_view,
                                                                std::size_t = 1) const;
    std::optional<std::vector<std::string>> getRowAttributes(std::string_view,
                                                             std::size_t = 1) const;
    std::vector<std::string> getAttributeArray(std::string_view) const;
    std::optional<std::vector<std::string>> addEm(const std::vector<double> &, double = 1) const;
    std::optional<std::vector<double>> convertLengths(const std::vector<std::string> &) const;
    void getStyles() override;
    std::string getChildAlign(std::size_t) const override;
    double getWrapWidth(std::size_t) override;

    bool fixesPWidth() const override {
        return false;
    }

    bool setChildPWidths(bool, std::optional<double> width = {}, bool clear = true) override;
    std::pair<std::string, double> getAlignShift() override;

  protected:
    void computeBBox(BBox &, bool = false) override;
};
} // namespace folia::math::output::common
