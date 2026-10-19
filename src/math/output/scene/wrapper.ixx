/*************************************************************
 * SVG-source placement lifted to the backend-independent Scene.
 * Per-kind algorithms are supplied by source-mapped scene wrapper modules.
 *************************************************************/
export module folia.math.output.scene:wrapper;
export import folia.math.output.scene.foundation;
export import folia.math.output.common;
import std;

export namespace folia::math::output::scene {
class WrapperFactory;
class Placement {
public:
    virtual ~Placement()=default;
    virtual std::vector<NodeRef> build(common::CommonWrapper&,Builder&,WrapperFactory&,
                                       std::span<const NodeRef> parents)=0;
};

// Shared standard SVG wrapper pipeline. Per-kind classes implement only the
// placement/paint-content equations; they cannot duplicate common BBox work
// or silently fall back to a generic layout when a kind is unregistered.
class SvgWrapperPlacement : public Placement {
protected:
    virtual void placeContent(common::CommonWrapper&,Builder&,
                              WrapperFactory&,std::span<const NodeRef> roots)=0;
public:
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,
                               WrapperFactory&,std::span<const NodeRef> parents) override;
};

class WrapperFactory {
    std::map<std::string,std::shared_ptr<Placement>,std::less<>> placements_;
    LayoutEventObserverRef observer_;
    std::function<void(common::CommonMath&)> displayHandler_;
    std::function<std::optional<std::string>(common::CommonMo&,std::uint32_t,
        std::string_view,std::string,Builder&)> glyphCache_;
    bool useXlink_=false;
    std::function<common::UnknownBBox(const NodeRef&,std::u16string_view,std::string_view)> textMeasurement_;
    std::function<NodeRef(Builder&,NodeRef,const PropertyList&,DOMNodeRef)> xmlSvgNodeCreator_;
public:
    using XmlSvgNodeCreator=std::function<NodeRef(Builder&,NodeRef,const PropertyList&,DOMNodeRef)>;
    XmlSvgNodeCreator exchangeXmlSvgNodeCreator(XmlSvgNodeCreator creator){
        return std::exchange(xmlSvgNodeCreator_,std::move(creator));
    }
    NodeRef createXmlSvgNode(Builder& builder,NodeRef shell,const PropertyList& attributes,
                             DOMNodeRef html)const{
        if(!xmlSvgNodeCreator_)
            throw options::Exception("Unimplemented",u"SVG XML node creator is not bound");
        return xmlSvgNodeCreator_(builder,std::move(shell),attributes,std::move(html));
    }
    using MactionAction = std::function<void(common::CommonMaction&,Builder&,
        std::span<const NodeRef>)>;
    void setLayoutObserver(LayoutEventObserverRef observer){observer_=std::move(observer);}
    const LayoutEventObserverRef& layoutObserver()const noexcept{return observer_;}
    void setDisplayHandler(std::function<void(common::CommonMath&)> handler){
        displayHandler_=std::move(handler);
    }
    void handleDisplay(common::CommonMath& wrapper){
        if(!displayHandler_)throw options::Exception("Unimplemented",u"SVG math display handler is not bound");
        displayHandler_(wrapper);
    }
    void registerKind(std::string kind,std::shared_ptr<Placement> placement);
    void setGlyphCache(std::function<std::optional<std::string>(common::CommonMo&,
        std::uint32_t,std::string_view,std::string,Builder&)> callback,bool useXlink){
        glyphCache_=std::move(callback);useXlink_=useXlink;
    }
    std::optional<std::string> cacheGlyph(common::CommonMo& wrapper,std::uint32_t cp,
        std::string_view variant,std::string path,Builder& builder){
        if(!glyphCache_)return {};
        return glyphCache_(wrapper,cp,variant,std::move(path),builder);
    }
    bool useXlink()const noexcept{return useXlink_;}
    void setTextMeasurement(std::function<common::UnknownBBox(const NodeRef&,
        std::u16string_view,std::string_view)> callback){textMeasurement_=std::move(callback);}
    common::UnknownBBox measureTextNode(const NodeRef& node,std::u16string_view chars,std::string_view variant){
        if(!textMeasurement_)throw options::Exception("Unimplemented",u"Source text-node measurement is not bound");
        return textMeasurement_(node,chars,variant);
    }
    void setMactionAction(MactionAction action){mactionAction_=std::move(action);}
    void runMactionAction(common::CommonMaction& wrapper,Builder& builder,
                          std::span<const NodeRef> roots)const{
        if(!mactionAction_)throw options::Exception("Unimplemented",u"SVG Maction action bridge is not bound");
        mactionAction_(wrapper,builder,roots);
    }
    std::vector<NodeRef> build(common::CommonWrapper&,Builder&,std::span<const NodeRef> parents);
private:
    MactionAction mactionAction_;
};

// Exact base SvgWrapper.addChildren flow for wrapper kinds that inherit that
// Source method. Specialized kinds call this only from their corresponding
// Source-mapped placement body; Mrow and stretchy Mo keep their own algorithms.
void addSvgBaseChildren(common::CommonWrapper&,Builder&,WrapperFactory&,
                        std::span<const NodeRef> parents);
std::optional<std::vector<NodeRef>> tryEmbellishedSvg(
    common::CommonWrapper&,Builder&,WrapperFactory&,std::span<const NodeRef> parents);
}
