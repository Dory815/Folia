/*************************************************************
 * MathJax 4.1.3 ts/output/common.ts.
 * Common measurement/output state shared by backend-specific Jax classes.
 *************************************************************/
export module folia.math.output.common:output_jax;
export import :types;
export import :wrapper;
export import :wrapper_factory;
export import :linebreak_visitor;
export import folia.math.core.output_jax;
export import folia.math.core.dom_adaptor;
export import folia.math.core.mml_tree.xml_dom_adaptor;
import std;

export namespace folia::math::output::common {
struct CommonRuntimeBindings;
struct UnknownBBox { double w=0,h=0,d=0; };
using UnknownMap = std::unordered_map<std::u16string,UnknownBBox>;
using UnknownVariantMap = std::unordered_map<std::string,std::shared_ptr<UnknownMap>>;
struct LinebreakOptions {
    bool inlineBreaks = true;
    std::string width = "100%";
    double lineleading = .2;
};

class CommonOutputJax : public core::AbstractOutputJax {
public:
    using NodeMapFactory=std::function<std::shared_ptr<WrapperNodeMap>()>;
    using NodeMapPublisher=std::function<void(options::Value)>;
private:
    std::shared_ptr<LayoutJob> activeJob_;
    std::weak_ptr<LayoutJob> completedJob_;
    CommonRuntimeBindings* runtime_ = nullptr;
    // The Source-visible `font` own data field is the raw Context value.
    // `font` below is only the resolved native cache used by typed layout
    // code; assignments always replace this one source value and refresh or
    // clear that cache.
    options::Value sourceFontValue_ = options::Undefined{};
    AbstractDOMAdaptor* domAdaptor_ = nullptr;
    std::shared_ptr<AbstractDOMAdaptor> domAdaptorOwner_;
    std::shared_ptr<mml::XmlDomAdaptor> htmlAdaptorOwner_;
    std::vector<std::pair<DOMNodeRef,ExtendedMetrics>> metricMap_;
    std::vector<std::pair<DOMNodeRef,DOMNodeRef>> testNodeMap_;
    UnknownVariantMap unknownCache_;
    std::function<void(const CommonWrapper&,const LayoutJob&)> layoutObserver_;
    NodeMapFactory nodeMapFactory_;
    NodeMapPublisher nodeMapPublisher_;
    bool nodeMapPublished_ = false;
protected:
    virtual DOMNodeRef createNode();
    virtual void setScale(DOMNodeRef,CommonWrapper&);
    virtual double getInitialScale() const;
    virtual bool markInlineBreak(bool marked,bool forcebreak,std::string_view linebreak,
                                 mml::MmlNode& parent,mml::MmlNode& child,
                                 mml::MmlNode* mo=nullptr);
    virtual std::vector<std::vector<std::pair<DOMNodeRef,ExtendedMetrics>>> getMetricMaps(core::MathDocumentReference);
    virtual DOMNodeRef getTestElement(DOMNodeRef,bool display);
    virtual ExtendedMetrics measureMetrics(DOMNodeRef,bool getFamily);
    virtual void addFontStyles(style_json::StyleJsonSheet&);
    virtual void addWrapperStyles(style_json::StyleJsonSheet&);
    virtual void addClassStyles(const CommonWrapperClassRef&,style_json::StyleJsonSheet&);
    virtual void setDocument(core::MathDocumentReference);
public:
    inline static constexpr std::string_view FONTPATH = "@mathjax/%%FONT%%-font";
    std::shared_ptr<style_json::StyleJsonSheet> styleJson;
    core::MathDocumentReference document = Missing{};
    core::MathItem* math = nullptr;
    DOMNodeRef container;
    CommonWrapper* table = nullptr;
    double pxPerEm = 16;
    std::shared_ptr<FontData> font;
    std::shared_ptr<CommonWrapperFactory> factory;
    // This is the one Source-created Linebreaks instance for this output Jax.
    // It is owned by the Jax, never shared as a second service identity.
    std::shared_ptr<Linebreaks> linebreaks;
    double containerWidth = 1280;
    // This handle aliases the active LayoutJob map; it is never a copied map.
    std::shared_ptr<WrapperNodeMap> nodeMap;
    DOMNodeRef testInline, testDisplay;

    CommonOutputJax(OptionFunctionBindings&,options::ObjectRef,
                    FunctionListContextBindings&,AbstractDOMAdaptor* adaptor=nullptr);
    virtual ~CommonOutputJax()=default;
    void initializeCommon(options::Value supplied,options::ObjectRef defaultFactory,
                          options::ObjectRef defaultFont,CommonRuntimeBindings&,
                          std::shared_ptr<void> owner);
    AbstractDOMAdaptor& domAdaptor() const;
    std::shared_ptr<AbstractDOMAdaptor> domAdaptorOwner() const;
    // Resolve a Source-owned DOM-valued field from the wrapper's existing
    // Context identity. The returned handle is the canonical adaptor node;
    // no copied wrapper map or DOM store is introduced.
    DOMNodeRef sourceDOMField(const CommonWrapper&,std::u16string_view) const;
    // Invoke the current Source document adaptor member with canonical
    // Context values. XML measurement uses this for parent/append so live
    // method replacement and Source errors remain observable.
    options::Value sourceDOMValue(DOMNodeRef) const;
    options::Value sourceAdaptorCall(std::u16string_view,std::span<const options::Value>) const;
    // Common XML wrappers use the same retained DOM provider as placement.
    virtual mml::XmlDomAdaptor& htmlAdaptor() const;
    virtual bool forceInlineBreaks() const;
    // Returns the actual active Source layout arena. Context wrapper objects
    // lease this identity so factory products remain alive after typeset.
    std::shared_ptr<LayoutJob> activeLayoutJob() const;
    // Synchronous inspection hook at the actual layout boundary. Arguments
    // are borrowed; observers must not retain the wrapper or job.
    void setLayoutObserver(std::function<void(const CommonWrapper&,const LayoutJob&)> observer);
    // Runs the virtual renderer and publishes its completed wrapper snapshot
    // through the active job observer. Failed renders are never observed.
    void processMathObserved(CommonWrapper&,DOMNodeRef);
    void bindNodeMap(NodeMapFactory factory,NodeMapPublisher publisher);
    void clearPublishedNodeMap();
    virtual std::shared_ptr<LayoutJob> beginLayoutJob(core::MathItem&,core::MathDocumentReference,
                                                       DOMNodeRef container);
    virtual void finishLayoutJob();
    // Private Renderer closes the current request on every exit. This takes
    // either its active job (failed/in-flight layout) or completed job
    // (serialization/result-copy failure) without changing Source session
    // behavior.
    std::shared_ptr<LayoutJob> takeCurrentLayoutJobForPrivateRendererRetirement() noexcept;
    virtual CommonWrapper* adoptWrapper(std::unique_ptr<CommonWrapper>);
    virtual ExtendedMetrics currentMetrics() const;
    virtual LinebreakOptions currentLinebreakOptions() const;
    void setAdaptor(core::OutputAdaptorReference) override;
    core::MathOutput typeset(core::MathItem&,core::MathDocumentReference) override;
    core::MathOutput escaped(core::MathItem&,core::MathDocumentReference) override=0;
    virtual void processMath(CommonWrapper&,DOMNodeRef)=0;
    virtual std::shared_ptr<BBox> getBBox(core::MathItem&,core::MathDocumentReference);
    virtual double getLinebreakWidth();
    virtual void markInlineBreaks(mml::NodeValue);
    virtual void unmarkInlineBreaks(mml::NodeValue);
    void getMetrics(core::MathDocumentReference) override;
    virtual void toDOM(core::MathItem&,DOMNodeRef,core::MathDocumentReference);
    virtual ExtendedMetrics getMetricsFor(DOMNodeRef,bool display);
    virtual core::MathOutput styleSheet(core::MathDocumentReference) override;
    virtual void insertStyles(const style_json::StyleJson&);
    virtual DOMNodeRef html(std::u16string_view type,PropertyList def={},
                            PropertyArray content={},std::u16string_view ns={});
    virtual DOMNodeRef text(std::u16string_view);
    virtual std::string fixed(double,std::uint32_t digits=3) const;
    std::string sourceFixed(double,std::optional<std::uint32_t> digits={}) const;
    virtual DOMNodeRef unknownText(std::u16string_view text,std::string_view variant)=0;
    virtual UnknownBBox measureText(std::u16string_view text,std::string_view variant,
                                    CssFontData font={});
    virtual UnknownBBox measureTextNodeWithCache(DOMNodeRef,std::u16string_view chars,
                                                 std::string_view variant,CssFontData font={});
    UnknownBBox measureTextWithCache(std::u16string_view chars,std::string_view variant,
        CssFontData font,std::function<UnknownBBox()> measure);
    virtual UnknownBBox measureTextNode(DOMNodeRef)=0;
    virtual PropertyList cssFontStyles(const CssFontData&,PropertyList styles={}) const;
    virtual CssFontData getFontData(styles::Styles&);
};
}
