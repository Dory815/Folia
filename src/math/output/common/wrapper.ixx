/*************************************************************
 * MathJax 4.1.3 ts/output/common/Wrapper.ts.
 * The concrete common measurement wrapper; scene emission belongs to the
 * output backend and is deliberately outside this class.
 *************************************************************/
export module folia.math.output.common:wrapper;
export import :types;
export import folia.math.core.tree.wrapper;
export import folia.math.core.mml_tree.mml_node;
export import folia.math.core.math_item;
export import folia.math.util.bbox;
export import folia.math.util.styles;
export import folia.math.util.style_json;
export import folia.math.util.constants;
export import folia.math.output.common.font_data;
export import folia.math.output.common.line_bbox;
export import folia.math.core.dom_adaptor;
export import folia.math.util.map_bindings;
import std;

export namespace folia::math::output::common {
class CommonMo;
class CommonMrow;

using StringMap = std::map<std::string,std::string,std::less<>>;
struct StyleData {
    std::array<double,4> margin{};
    std::array<double,4> padding{};
    struct Border {
        std::array<double,4> width{};
        std::array<std::string,4> style{};
        std::array<std::string,4> color{};
    };
    std::optional<Border> border;
};

struct CommonWrapperClassData {
    std::string kind = "unknown";
    // The exact Source constructor object associated with this same canonical
    // FactoryClassRef. This is metadata, not a second class registry.
    std::optional<options::ObjectRef> sourceConstructor;
    style_json::StyleJson styles;
    std::vector<std::string> removeStyles;
    std::map<std::string,bool,std::less<>> skipAttributes;
    std::map<std::string,StringMap,std::less<>> boldVariants, italicVariants;
    std::function<void(style_json::StyleJsonSheet&,CommonOutputJax&)> addStyles;
};

class CommonWrapper : public tree::AbstractWrapper<CommonWrapper,mml::MmlNode> {
    using Base = tree::AbstractWrapper<CommonWrapper,mml::MmlNode>;
    CommonWrapperFactory* commonFactory_;
    tree::FactoryClassRef<CommonWrapper> selectedClass_;
    enum class SourceInitializationPhase { Uninitialized, Initializing, Complete, Failed };
    SourceInitializationPhase sourceInitializationPhase_ = SourceInitializationPhase::Uninitialized;
protected:
    bool bboxComputed_ = false;
    std::int64_t breakCount_ = -1;

    virtual void computeBBox(BBox&,bool recompute=false);
    virtual std::shared_ptr<LineBBox> embellishedBBox(std::size_t);
    virtual std::shared_ptr<LineBBox> computeLineBBox(std::size_t);
    virtual std::shared_ptr<LineBBox> getChildLineBBox(CommonWrapper&,std::size_t);
    virtual double sideStyleSize(std::size_t) const;
    virtual void addLeftBorders(BBox&) const;
    virtual void addMiddleBorders(BBox&) const;
    virtual void addRightBorders(BBox&) const;
    virtual void copySkewIC(BBox&) const;
    virtual void getStyles();
    virtual void getStyleData();
    virtual void getVariant();
    virtual std::string explicitVariant(std::string_view family,std::string_view weight,std::string_view style);
    virtual void getScale();
    virtual void getSpace();
    virtual void getMathMLSpacing();
    virtual void getTeXSpacing(bool isTop,bool hasSpacing);
    virtual bool isTopEmbellished() const;
    virtual std::pair<std::string,double> getAlignShift();
    virtual double getAlignX(double,const BBox&,std::string_view) const;
    virtual double getAlignY(double,double,double,double,std::string_view) const;
    virtual std::string percent(double) const;
    virtual std::string em(double) const;
    virtual std::string px(double,double minimum=-BIGDIMEN) const;
    virtual double length2em(const Property&,double size=1,std::optional<double> scale={}) const;
    virtual std::shared_ptr<std::vector<std::uint32_t>> unicodeChars(std::u16string_view,std::string_view variant={}) const;
    virtual CommonMo& createMo(std::u16string_view);
public:
    using BBoxRef = std::shared_ptr<BBox>;
    using LineBBoxRef = std::shared_ptr<LineBBox>;
    CommonWrapper* parent = nullptr;
    // Source Wrapper.dom is one mutable array. Keep the adaptor's captured
    // PropertyArray identity and its graph lease together; the lease is not a
    // second child list.
    std::shared_ptr<DOMNodeArray> dom;
    std::shared_ptr<StringMap> removedStyles;
    std::unique_ptr<styles::Styles> styles;
    std::optional<StyleData> styleData;
    std::string variant;
    BBoxRef bbox;
    std::vector<LineBBoxRef> lineBBox;
    DelimiterDataRef stretch = NOSTRETCH;
    std::shared_ptr<FontData> font;

    CommonWrapper(CommonWrapperFactory&,mml::MmlNode&,CommonWrapper* parent=nullptr);
    virtual ~CommonWrapper()=default;
    // Runs after the complete selected derived wrapper is allocated,
    // adopted, and bound to its canonical Source identity.
    virtual void initializeSource();
    // The factory enters this phase exactly once after publishing the real
    // wrapper identity. Failed initialization remains observable and is never
    // replayed against the partially initialized object.
    void runSourceInitialization();

    CommonWrapperFactory& factory() const;
    tree::FactoryClassRef<CommonWrapper> selectedClass() const { return selectedClass_; }
    // Narrow Source-method entry points for the Context prototype bridge. They
    // delegate to the same virtual implementations used by native layout.
    std::string sourcePercent(double value) const { return percent(value); }
    std::string sourceEm(double value) const { return em(value); }
    std::string sourcePx(double value,double minimum) const { return px(value,minimum); }
    double sourceLength2em(const Property& value,double size=1,
                           std::optional<double> scale={}) const {
        return length2em(value,size,scale);
    }
    // Context prototype adapters call these on this same live wrapper. Keep
    // the Source method surface as forwarding calls rather than copied state.
    CommonWrapper* sourceBreakTop(CommonWrapper* row,CommonWrapper* child) {
        return breakTop(row,child);
    }
    void sourceComputeBBox(BBox& target,bool recompute=false) { computeBBox(target,recompute); }
    LineBBoxRef sourceEmbellishedBBox(std::size_t index) { return embellishedBBox(index); }
    LineBBoxRef sourceComputeLineBBox(std::size_t index) { return computeLineBBox(index); }
    LineBBoxRef sourceGetChildLineBBox(CommonWrapper& child,std::size_t index) {
        return getChildLineBBox(child,index);
    }
    double sourceSideStyleSize(std::size_t side) const { return sideStyleSize(side); }
    void sourceAddLeftBorders(BBox& target) const { addLeftBorders(target); }
    void sourceAddMiddleBorders(BBox& target) const { addMiddleBorders(target); }
    void sourceAddRightBorders(BBox& target) const { addRightBorders(target); }
    void sourceCopySkewIC(BBox& target) const { copySkewIC(target); }
    std::pair<std::string,double> sourceGetAlignShift() { return getAlignShift(); }
    void sourceGetStyles() { getStyles(); }
    void sourceGetStyleData() { getStyleData(); }
    void sourceGetVariant() { getVariant(); }
    std::string sourceExplicitVariant(std::string_view family,std::string_view weight,
                                      std::string_view style) {
        return explicitVariant(family,weight,style);
    }
    void sourceGetScale() { getScale(); }
    void sourceGetSpace() { getSpace(); }
    void sourceGetMathMLSpacing() { getMathMLSpacing(); }
    void sourceGetTeXSpacing(bool isTop,bool hasSpacing) { getTeXSpacing(isTop,hasSpacing); }
    bool sourceIsTopEmbellished() const { return isTopEmbellished(); }
    double sourceGetAlignX(double width,const BBox& box,std::string_view align) const {
        return getAlignX(width,box,align);
    }
    double sourceGetAlignY(double height,double depth,double total,double axis,
                           std::string_view align) const {
        return getAlignY(height,depth,total,axis,align);
    }
    std::shared_ptr<std::vector<std::uint32_t>> sourceUnicodeChars(
        std::u16string_view chars,std::string_view variant={}) const {
        return unicodeChars(chars,variant);
    }
    std::shared_ptr<std::vector<std::uint32_t>> sourceRemapChars(
        std::shared_ptr<std::vector<std::uint32_t>> chars) const {
        return remapChars(std::move(chars));
    }
    mml::TextNode* sourceMmlText(std::u16string text) { return mmlText(std::move(text)); }
    mml::MmlNode* sourceMmlNode(std::string_view kind,PropertyList properties={},
                                std::vector<mml::MmlNode*> children={}) {
        return mmlNode(kind,std::move(properties),std::move(children));
    }
    CommonMo& sourceCreateMo(std::u16string_view text) { return createMo(text); }
    CharDataArray sourceGetVariantChar(std::string_view variant,std::uint32_t codepoint) {
        return getVariantChar(variant,codepoint);
    }
    DOMNodeRef sourceHtml(std::u16string_view type,PropertyList definition={},
                          PropertyArray content={});
    CommonOutputJax& jax() const;
    AbstractDOMAdaptor& adaptor() const;
    const ExtendedMetrics& metrics() const;
    double containerWidth() const;
    Linebreaks& linebreaks() const;
    options::Value linebreakOptions() const;
    virtual bool fixesPWidth() const;
    virtual std::size_t breakCount();
    virtual CommonWrapper* breakTop(CommonWrapper* mrow,CommonWrapper* child);
    CommonWrapper* wrap(mml::MmlNode*,tree::FactoryArgs args={}) override;
    virtual CommonWrapper* wrapNode(mml::MmlNode&,CommonWrapper* parent=nullptr);
    virtual BBoxRef getBBox(bool save=true);
    virtual BBoxRef getOuterBBox(bool save=true);
    virtual std::array<double,2> getUnbrokenHD();
    virtual LineBBoxRef getLineBBox(std::size_t);
    virtual std::array<CommonWrapper*,2> getBreakNode(LineBBox&);
    virtual bool setChildPWidths(bool recompute,std::optional<double> width={},bool clear=true);
    virtual void breakToWidth(double width);
    virtual void invalidateBBox(bool bubble=true);
    virtual CommonWrapper* core();
    virtual CommonMo* coreMO();
    virtual double coreRScale() const;
    virtual double getRScale() const;
    virtual std::u16string getText() const;
    virtual bool canStretch(std::string_view direction);
    virtual std::pair<std::string,double> processIndent(std::string_view indentalign,std::string_view indentshift,
                                                        std::string_view align={},std::string_view shift={},
                                                        std::optional<double> width={});
    virtual double getWrapWidth(std::size_t index);
    virtual std::string getChildAlign(std::size_t index) const;
    virtual std::shared_ptr<std::vector<std::uint32_t>> remapChars(std::shared_ptr<std::vector<std::uint32_t>> chars) const;
    virtual mml::TextNode* mmlText(std::u16string text);
    virtual mml::MmlNode* mmlNode(std::string_view kind,PropertyList properties={},
                                  std::vector<mml::MmlNode*> children={});
    virtual CharDataArray getVariantChar(std::string_view variant,std::uint32_t codepoint);
};

// Concrete wrapper kinds live in source-aligned partitions.

// Source's nodeMap is a real ECMAScript Map.  Native lookups and the Context
// projection share this one insertion-ordered storage; the conversion
// callbacks resolve the existing MML/wrapper identities and never create
// proxy nodes or a parallel pointer table.
class WrapperNodeMap {
public:
    using NodeIdentity = std::function<options::ObjectRef(mml::MmlNode&)>;
    using WrapperIdentity = std::function<options::ObjectRef(CommonWrapper&)>;
    using WrapperResolver = std::function<CommonWrapper&(options::ObjectRef)>;
private:
    options::Context* context_ = nullptr;
    options::MapStorageRef storage_;
    options::ObjectRef object_{};
    NodeIdentity nodeIdentity_;
    WrapperIdentity wrapperIdentity_;
    WrapperResolver wrapperResolver_;
public:
    WrapperNodeMap()=default;
    WrapperNodeMap(options::Context&,options::MapContextBindings&,
                   NodeIdentity,WrapperIdentity,WrapperResolver);
    void initialize(options::Context&,options::MapContextBindings&,
                    NodeIdentity,WrapperIdentity,WrapperResolver);
    const options::MapStorageRef& storage() const noexcept { return storage_; }
    options::ObjectRef contextObject() const;
    void insert_or_assign(mml::MmlNode*,CommonWrapper*);
    void clear();
    CommonWrapper* find(mml::MmlNode*) const;
    void forEachValue(const std::function<void(CommonWrapper&)>&) const;
};

// The factory persists across jobs, while all products and the MML-to-wrapper
// relation are scoped to one Source layout operation.
class LayoutJob {
    std::vector<std::unique_ptr<CommonWrapper>> wrappers_;
public:
    core::MathItem* math = nullptr;
    core::MathDocumentReference document = Missing{};
    DOMNodeRef container;
    std::shared_ptr<WrapperNodeMap> nodeMap = std::make_shared<WrapperNodeMap>();
    CommonWrapper* table = nullptr;
    LayoutJob(core::MathItem&,core::MathDocumentReference,DOMNodeRef);
    LayoutJob(const LayoutJob&)=delete;
    LayoutJob& operator=(const LayoutJob&)=delete;
    CommonWrapper* adopt(std::unique_ptr<CommonWrapper>);
    std::size_t wrapperCount() const { return wrappers_.size(); }
};
}
