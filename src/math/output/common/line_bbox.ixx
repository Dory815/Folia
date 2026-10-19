/*************************************************************
 * Source port of MathJax 4.1.3 output/common/LineBBox.ts.
 * Copyright (c) 2022-2026 The MathJax Consortium; Apache-2.0.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common.line_bbox;
export import folia.math.util.bbox;
export import folia.math.core.mml_tree.mml_document;
import std;

export namespace folia::math::output::common {
struct IndentPair {
    std::optional<std::u16string> alignment;
    std::optional<std::u16string> shift;
    friend bool operator==(IndentPair const&, IndentPair const&) = default;
};

struct IndentData {
    std::array<IndentPair, 3> lines; // first, normal, last
    friend bool operator==(IndentData const&, IndentData const&) = default;
};
using IndentDataRef = std::shared_ptr<IndentData>;

class LineBBox : public BBox {
public:
    IndentDataRef indentData;
    std::optional<double> lineLeading;
    double originalL = 0;
    bool isFirst = false;
    std::optional<std::array<double, 2>> start;
    std::optional<std::array<double, 2>> end;

    static LineBBox from(BBox const& bbox, std::optional<double> leading,
                         IndentDataRef indent = {});
    explicit LineBBox(BBoxData def = {},
                      std::optional<std::array<double, 2>> start = {});
    void append(LineBBox& child);
    LineBBox copy() const;
    void getIndentData(mml::MmlNode const& node);

protected:
    IndentDataRef copyIndentData(LineBBox const& bbox) const;
};
}
