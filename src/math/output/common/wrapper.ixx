export module folia.math.output.common:wrapper;
export import :types;
export import folia.math.core.tree.wrapper;
export import folia.math.core.mml_tree.mml_document;
export import folia.math.util.bbox;
export import folia.math.util.styles;
export import folia.math.util.style_json;
export import folia.math.util.constants;
export import folia.math.util.lengths;
export import folia.math.output.common.font_data;
export import folia.math.output.common.line_bbox;
export import folia.math.output.scene.builder;
import std;

export namespace folia::math::output::common {
using StringMap = std::map<std::string, std::string, std::less<>>;
using LengthInput = std::variant<double, std::u16string>;
using SceneNode = std::optional<scene::NodeId>;
using SceneNodes = std::vector<SceneNode>;
using SceneParents = std::vector<scene::NodeId>;
struct SceneContainers {
    SceneNodes nodes;
    SceneParents content;
};

struct StyleData {
    std::array<double, 4> margin{};
    std::array<double, 4> padding{};

    struct Border {
        std::array<double, 4> width{};
        std::array<std::string, 4> style{};
        std::array<std::string, 4> color{};
    };

    std::optional<Border> border;
};

struct CommonWrapperClassData {
    style_json::StyleJson styles;
    std::vector<std::string> removeStyles{"fontSize",  "fontFamily",  "fontWeight",
                                          "fontStyle", "fontVariant", "font"};
    std::map<std::string, bool, std::less<>> skipAttributes{
        {"fontfamily", true}, {"fontsize", true},   {"fontweight", true}, {"fontstyle", true},
        {"color", true},      {"background", true}, {"class", true},      {"href", true},
        {"style", true},      {"xmlns", true}};
    std::map<std::string, StringMap, std::less<>> boldVariants;
    std::map<std::string, StringMap, std::less<>> italicVariants;
    static CommonWrapperClassData commonDefaults();
};

struct WrapperBuildArguments {
    CommonWrapperFactory &factory;
    mml::MmlNode &node;
    CommonWrapper *parent;
};

struct CommonWrapperRegistration {
    mml::NodeKind kind;
    tree::FactoryClass<CommonWrapper, WrapperBuildArguments> creator;
    CommonWrapperClassData data;
};

using CommonWrapperCatalog = std::vector<CommonWrapperRegistration>;

class CommonWrapper : public tree::AbstractWrapper<CommonWrapper, mml::MmlNode> {
    friend class CommonTextNode;

  protected:
    virtual void computeBBox(BBox &, bool recompute = false);
    virtual std::shared_ptr<LineBBox> embellishedBBox(std::size_t);
    virtual std::shared_ptr<LineBBox> computeLineBBox(std::size_t);
    virtual std::shared_ptr<LineBBox> getChildLineBBox(CommonWrapper &, std::size_t);
    virtual double sideStyleSize(std::size_t) const;
    virtual void addLeftBorders(BBox &) const;
    virtual void addMiddleBorders(BBox &) const;
    virtual void addRightBorders(BBox &) const;
    virtual void copySkewIC(BBox &) const;
    virtual void getStyles();
    virtual void getStyleData();
    virtual void getVariant();
    virtual std::string explicitVariant(std::string_view family, std::string_view weight,
                                        std::string_view style);
    virtual void getScale();
    virtual void getSpace();
    virtual void getMathMLSpacing();
    virtual void getTeXSpacing(bool isTop, bool hasSpacing);
    virtual bool isTopEmbellished() const;
    virtual std::pair<std::string, double> getAlignShift();
    virtual double getAlignX(double, BBox const &, std::string_view) const;
    virtual double getAlignY(double parentHeight, double parentDepth, double childHeight,
                             double childDepth, std::string_view align) const;
    virtual std::string percent(double) const;
    virtual std::string em(double) const;
    virtual std::string px(double, double minimum = -BIGDIMEN) const;
    virtual double length2em(LengthInput const &, double size = 1,
                             std::optional<double> scale = {}) const;
    virtual std::vector<std::uint32_t> unicodeChars(std::u16string_view,
                                                    std::string_view variant = {}) const;
    virtual CommonMo &createMo(std::u16string_view);

  public:
    CommonWrapper *parent = nullptr;
    BBox bbox;
    std::vector<std::shared_ptr<LineBBox>> lineBBox;
    bool bboxComputed = false;
    std::int64_t _breakCount = -1;
    std::optional<styles::Styles> styles;
    std::optional<StyleData> styleData;
    StringMap removedStyles;
    std::string variant;
    StretchState stretch{NoStretch{}};

    CommonWrapper(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr);
    virtual ~CommonWrapper() = default;
    virtual void initializeSource();
    // Returns the CommonWrapperFactory borrowed by Tree::Wrapper::factory_.
    // No second factory pointer is stored on this object.
    CommonWrapperFactory &factory() const;
    CommonOutputJax &jax() const;
    ExtendedMetrics const &metrics() const;
    virtual double containerWidth() const;
    Linebreaks &linebreaks() const;
    LinebreakOptions const &linebreakOptions() const;

    virtual bool fixesPWidth() const;
    virtual std::size_t breakCount();
    virtual CommonWrapper *breakTop(CommonWrapper *mrow, CommonWrapper *child);
    virtual CommonWrapper &wrapNode(mml::MmlNode &, CommonWrapper *parent = nullptr);
    virtual BBox &getBBox();
    virtual BBox getBBox(bool save);
    virtual BBox getOuterBBox(bool save = true);
    virtual std::array<double, 2> getUnbrokenHD();
    virtual std::shared_ptr<LineBBox> getLineBBox(std::size_t);
    virtual std::array<CommonWrapper *, 2> getBreakNode(LineBBox &);
    virtual bool setChildPWidths(bool recompute, std::optional<double> width = {},
                                 bool clear = true);
    virtual void breakToWidth(double width);
    virtual void invalidateBBox(bool bubble = true);
    virtual CommonWrapper *core();
    virtual CommonWrapper *coreMO();
    virtual double coreRScale() const;
    virtual double getRScale() const;
    virtual std::u16string getText() const;
    virtual bool canStretch(StretchDirection);
    virtual std::pair<std::string, double> processIndent(std::string_view indentalign,
                                                         std::string_view indentshift,
                                                         std::string_view align = {},
                                                         std::string_view shift = {},
                                                         std::optional<double> width = {});
    virtual double getWrapWidth(std::size_t);
    virtual std::string getChildAlign(std::size_t) const;
    virtual std::vector<std::uint32_t> remapChars(std::span<std::uint32_t const>) const;
    virtual mml::TextNode &mmlText(std::u16string);
    virtual mml::MmlNode &mmlNode(mml::NodeKind, mml::AttributeSet,
                                  std::vector<mml::MmlNode *> children = {});
    virtual std::optional<ResolvedFontGlyph> getVariantChar(std::string_view variant,
                                                            std::uint32_t codepoint);

    virtual SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents);
    void placeScene(scene::Builder &, std::span<SceneNode const> nodes, double x, double y,
                    std::optional<scene::NodeId> target = {});
    double sceneDx() const;

  protected:
    virtual SceneContainers createSceneNodes(scene::Builder &,
                                             std::span<scene::NodeId const> parents);
    SceneContainers standardSceneNodes(scene::Builder &, std::span<scene::NodeId const> parents);
    virtual void addSceneChildren(scene::Builder &, std::span<scene::NodeId const> parents);
    std::optional<SceneNodes> toEmbellishedScene(scene::Builder &,
                                                 std::span<scene::NodeId const> parents);
    void handleSceneStyles(scene::Builder &, std::span<SceneNode const>);
    void handleSceneScale(scene::Builder &, std::span<SceneNode const>);
    void handleSceneBorder(scene::Builder &, std::span<SceneNode const>);
    void handleSceneColor(scene::Builder &, std::span<SceneNode const>);
    void handleSceneAttributes(scene::Builder &, std::span<SceneNode const>);
    double handleSceneId(scene::Builder &, std::span<SceneNode const>, double y);
    double emitChar(scene::Builder &, scene::NodeId parent, std::uint32_t codepoint, double x,
                    double y, std::string_view variant, std::u16string &unknown,
                    bool buffer = false);
    double emitUnknownText(scene::Builder &, scene::NodeId parent, double x, double y,
                           std::u16string &unknown, std::string_view variant,
                           CssFontData font = {});
};
} // namespace folia::math::output::common
