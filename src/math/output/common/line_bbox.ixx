/*************************************************************
 * Source port of MathJax 4.1.3 output/common/LineBBox.ts.
 * Copyright (c) 2022-2026 The MathJax Consortium; Apache-2.0.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common.line_bbox;
export import folia.math.util.bbox;
export import folia.math.core.mml_tree.mml_node;
import std;

export namespace folia::math::output::common {
using IndentData = std::shared_ptr<PropertyArray>;

class LineBBox : public BBox {
public:
    std::shared_ptr<PropertyArray> indentData;
    std::optional<double> lineLeading;
    double originalL = 0;
    bool isFirst = false;
    std::optional<std::array<double, 2>> start;
    std::optional<std::array<double, 2>> end;

    static LineBBox from(const BBox& bbox, std::optional<double> leading,
                         std::shared_ptr<PropertyArray> indent = {});
    explicit LineBBox(BBoxData def = {},
                      std::optional<std::array<double, 2>> start = {});
    void append(LineBBox& child);
    LineBBox copy() const;
    void getIndentData(const mml::MmlNode& node);

protected:
    std::shared_ptr<PropertyArray> copyIndentData(const LineBBox& bbox) const;
};
}
