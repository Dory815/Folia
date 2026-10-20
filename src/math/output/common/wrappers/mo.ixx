// Candidate CommonMo declaration against common-contract-refresh/headers.
export module folia.math.output.common:mo;
export import :wrapper;
import std;
import folia.math.core.mml_tree.mml_nodes.mo;

export namespace folia::math::output::common {
struct FixedStretchSize {
    std::size_t index{};
};

struct AssemblyStretchSize {};

class CommonMo : public CommonWrapper {
  public:
    // Source null / fixed-size index / -1 (assembly), with no numeric JS bag.
    std::optional<std::variant<FixedStretchSize, AssemblyStretchSize>> size;
    bool isAccent{};
    std::string breakStyle;
    CommonMo *multChar{}; // borrowed from the same factory-owned layout job

    CommonMo(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr);
    void initializeSource() override;
    SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;
    std::size_t breakCount() override;
    std::size_t embellishedBreakCount() const;
    std::string embellishedBreakStyle() const;
    void setBreakStyle(std::string_view linebreak = {});
    std::string getBreakStyle(std::string_view linebreak = {}) const;
    void getMultChar();
    void protoBBox(BBox &);
    void computeBBox(BBox &, bool recompute = false) override;
    std::shared_ptr<LineBBox> computeLineBBox(std::size_t) override;
    LineBBox moLineBBox(std::size_t, std::string_view, BBox const *obox = nullptr);
    double getAccentOffset();
    double getCenterOffset(BBox *bbox = nullptr);
    void getStretchedVariant(std::span<double const> dimensions, bool exact = false);
    double getSize(std::string_view name, double fallback);
    double getWH(std::span<double const> dimensions) const;
    void getStretchBBox(std::span<double const> requested, double dimension,
                        DelimiterRecipe const &);
    std::array<double, 2> getBaseline(std::span<double const> requested, double dimension,
                                      DelimiterRecipe const &) const;
    double checkExtendedHeight(double dimension, DelimiterRecipe const &) const;
    void getVariant() override;
    std::vector<std::uint32_t> remapChars(std::span<std::uint32_t const> chars) const override;
    bool canStretch(StretchDirection) override;

  private:
    void setDelimSize(std::uint32_t character, std::size_t index);
    mml::MmlMo &mo() const;
    ActiveStretch &activeStretch();
    ActiveStretch const &activeStretch() const;
};
} // namespace folia::math::output::common
