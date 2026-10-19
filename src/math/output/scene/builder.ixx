/*************************************************************
 * Source-mapped scene construction primitives.
 * This builder records completed geometry/paint; it performs no MML layout.
 *************************************************************/
export module folia.math.output.scene.foundation:builder;
export import :types;
export import folia.math.output.common;
import folia.math.util.property_host;
import std;

export namespace folia::math::output::scene {
// Optional synchronous diagnostics at the real Source-equivalent scene
// operations. It names the imported canonical CommonWrapper type directly;
// Builder/WrapperFactory borrow this observer only during the render call and
// never copy it into a completed Scene.
struct LayoutEventObserver {
    std::function<void(common::CommonWrapper&,double,double,const NodeRef&,bool)> place;
    std::function<std::size_t(common::CommonWrapper&,std::uint32_t,double,double,
        const NodeRef&,std::string_view,std::optional<bool>)> beginPlaceChar;
    std::function<void(std::size_t,double)> finishPlaceChar;
    std::function<void(common::CommonWrapper*,std::u16string_view,
        const PropertyList&,std::size_t)> paint;
};
using LayoutEventObserverRef=std::shared_ptr<LayoutEventObserver>;

class Builder {
    std::vector<NodeRef> roots_;
    std::vector<std::shared_ptr<const GlyphOutline>> resources_;
    LayoutEventObserverRef observer_;
    common::CommonWrapper* currentWrapper_=nullptr;
    static bool reaches(const NodeRef&,const Node*);
public:
    void setLayoutObserver(LayoutEventObserverRef observer){observer_=std::move(observer);}
    LayoutEventObserverRef exchangeLayoutObserver(LayoutEventObserverRef observer){
        return std::exchange(observer_,std::move(observer));
    }
    const LayoutEventObserverRef& layoutObserver()const noexcept{return observer_;}
    common::CommonWrapper* exchangeCurrentWrapper(common::CommonWrapper* wrapper)noexcept{
        return std::exchange(currentWrapper_,wrapper);
    }
    NodeRef node(std::u16string kind,PropertyList attributes={},std::u16string text={},
                 std::size_t sourceChildCount=0);
    // Capture a node that was already created by a Source DOM operation.
    // This preserves the current DOM state without reporting another paint
    // creation event to the layout observer.
    NodeRef adoptExisting(std::u16string kind,PropertyList attributes={},std::u16string text={});
    void geometry(const NodeRef&,Geometry);
    void transform(const NodeRef&,Transform);
    // Validate the Scene-only acyclicity constraint without mutating either
    // node. Source append dispatch can call this after its live property reads
    // and before the canonical children array performs its single push.
    void validateAppend(const NodeRef& parent,const NodeRef& child)const;
    void append(const NodeRef& parent,const NodeRef& child);
    void addRoot(NodeRef root){
        if(!root)folia::math::property_host::typeError("Cannot add null scene root");
        roots_.push_back(std::move(root));
    }
    void retain(const std::shared_ptr<const GlyphOutline>& owner){
        if(!owner)folia::math::property_host::typeError("Cannot retain null scene resource");
        resources_.push_back(owner);
    }
    Scene finish()&&;
};
}
