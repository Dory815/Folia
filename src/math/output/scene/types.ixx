/*************************************************************
 * Backend-independent completed scene values for MathJax output.
 * Geometry stays in source math units until a consumer serializes/draws it.
 *************************************************************/
export module folia.math.output.scene.foundation:types;
export import :html_types;
export import folia.math.core.tree.property;
export import folia.math.util.option_types;
import std;

export namespace folia::math::output::scene {
struct Node;
using NodeRef=std::shared_ptr<Node>;

// A mutable Scene node has one current child owner. The default owner is a
// backend-neutral vector; an input adapter may provide another live owner.
// FrozenNode always copies this list into its own ordinary vector.
class ChildOwner {
public:
    virtual ~ChildOwner()=default;
    virtual std::size_t size()const=0;
    virtual bool hasSlot(std::size_t index)const{return index<size();}
    virtual NodeRef at(std::size_t)const=0;
    virtual std::vector<NodeRef> snapshot()const{
        std::vector<NodeRef> out;const auto count=size();out.reserve(count);
        for(std::size_t i=0;i<count;++i){
            if(!hasSlot(i))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'kind')");
            auto child=at(i);
            if(!child)throw options::Exception("TypeError",u"Scene child has no canonical node mapping");
            out.push_back(std::move(child));
        }
        return out;
    }
    virtual void set(std::size_t,NodeRef)=0;
    virtual void append(NodeRef)=0;
    virtual void insert(std::size_t,NodeRef)=0;
    // Structural erase shifts later slots. deleteSlot preserves array length
    // and leaves a hole, matching JavaScript DeleteProperty on children[i].
    virtual void erase(std::size_t)=0;
    virtual void deleteSlot(std::size_t index){if(index<size())throw std::logic_error("child owner does not support sparse slots");}
    virtual void resize(std::size_t)=0;
    virtual void assign(const std::vector<NodeRef>& values){
        resize(0);
        for(const auto& value:values)append(value);
    }
    virtual void reserve(std::size_t){}
};
class ChildList {
    class VectorOwner final:public ChildOwner {
        std::vector<std::optional<NodeRef>> values_;
    public:
        std::size_t size()const override{return values_.size();}
        bool hasSlot(std::size_t i)const override{return i<values_.size()&&values_[i].has_value();}
        NodeRef at(std::size_t i)const override{return values_.at(i).value_or(NodeRef{});}
        void set(std::size_t i,NodeRef value)override{values_.at(i)=std::move(value);}
        void append(NodeRef value)override{values_.emplace_back(std::move(value));}
        void insert(std::size_t i,NodeRef value)override{values_.insert(values_.begin()+static_cast<std::ptrdiff_t>(i),std::move(value));}
        void erase(std::size_t i)override{values_.erase(values_.begin()+static_cast<std::ptrdiff_t>(i));}
        void deleteSlot(std::size_t i)override{if(i<values_.size())values_[i].reset();}
        void resize(std::size_t n)override{values_.resize(n);}
        void reserve(std::size_t n)override{values_.reserve(n);}
        std::vector<NodeRef> take(){
            std::vector<NodeRef> out;out.reserve(values_.size());
            for(const auto& slot:values_){
                if(!slot||!*slot)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'kind')");
                out.push_back(*slot);
            }
            values_.clear();return out;
        }
    };
    std::shared_ptr<ChildOwner> owner_=std::make_shared<VectorOwner>();
    void ensure(){if(!owner_)owner_=std::make_shared<VectorOwner>();}
public:
    std::size_t size()const{return owner_?owner_->size():0;}
    bool empty()const{return size()==0;}
    bool hasSlot(std::size_t i)const{return owner_&&owner_->hasSlot(i);}
    NodeRef at(std::size_t i)const{return owner_->at(i);}
    NodeRef operator[](std::size_t i)const{return at(i);}
    NodeRef front()const{return at(0);}
    NodeRef back()const{return at(size()-1);}
    void push_back(NodeRef value){ensure();owner_->append(std::move(value));}
    void insert(std::size_t i,NodeRef value){ensure();owner_->insert(i,std::move(value));}
    void erase(std::size_t i){owner_->erase(i);}
    void deleteSlot(std::size_t i){owner_->deleteSlot(i);}
    void resize(std::size_t n){ensure();owner_->resize(n);}
    void clear(){resize(0);}
    void reserve(std::size_t n){if(owner_)owner_->reserve(n);}
    std::vector<NodeRef> snapshot()const{return owner_?owner_->snapshot():std::vector<NodeRef>{};}
    void replaceOwner(std::shared_ptr<ChildOwner> value){owner_=std::move(value);ensure();}
    std::shared_ptr<ChildOwner> owner()const{return owner_;}
    ChildList& operator=(std::vector<NodeRef> values){
        owner_=std::make_shared<VectorOwner>();
        owner_->assign(values);return *this;
    }
    std::vector<NodeRef> releaseNative(){
        auto native=std::dynamic_pointer_cast<VectorOwner>(owner_);
        if(!native)throw std::logic_error("cannot move a non-native Scene child owner");
        auto values=native->take();owner_=std::make_shared<VectorOwner>();return values;
    }
};

// AttributeOwner is the attribute counterpart of ChildOwner. It contains no
// Context or SVG concepts; adapters can retain a live property-storage owner.
class AttributeOwner {
public:
    virtual ~AttributeOwner()=default;
    virtual Property get(std::string_view)const=0;
    virtual Property getOwn(std::string_view)const=0;
    virtual bool hasOwn(std::string_view)const=0;
    virtual void set(std::string_view,Property)=0;
    virtual void setOwn(std::string_view,Property)=0;
    virtual void erase(std::string_view)=0;
    virtual std::vector<std::string> keys()const=0;
};
class AttributeList {
    class PropertyOwner final:public AttributeOwner {
        PropertyList values_;
    public:
        explicit PropertyOwner(PropertyList values={}):values_(std::move(values)){}
        Property get(std::string_view name)const override{return values_.get(name);}
        Property getOwn(std::string_view name)const override{return values_.getOwn(name);}
        bool hasOwn(std::string_view name)const override{return values_.hasOwn(name);}
        void set(std::string_view name,Property value)override{values_.set(name,std::move(value));}
        void setOwn(std::string_view name,Property value)override{values_.setOwn(name,std::move(value));}
        void erase(std::string_view name)override{values_.erase(name);}
        std::vector<std::string> keys()const override{return values_.keys();}
    };
    std::shared_ptr<AttributeOwner> owner_=std::make_shared<PropertyOwner>();
public:
    AttributeList()=default;
    AttributeList(PropertyList value):owner_(std::make_shared<PropertyOwner>(std::move(value))){}
    Property get(std::string_view name)const{return owner_->get(name);}
    Property getOwn(std::string_view name)const{return owner_->getOwn(name);}
    bool hasOwn(std::string_view name)const{return owner_->hasOwn(name);}
    void set(std::string_view name,Property value){owner_->set(name,std::move(value));}
    void setOwn(std::string_view name,Property value){owner_->setOwn(name,std::move(value));}
    void erase(std::string_view name){owner_->erase(name);}
    std::vector<std::string> keys()const{return owner_->keys();}
    void replaceOwner(std::shared_ptr<AttributeOwner> value){owner_=std::move(value);}
    std::shared_ptr<AttributeOwner> owner()const{return owner_;}
    AttributeList& operator=(PropertyList value){owner_=std::make_shared<PropertyOwner>(std::move(value));return *this;}
};

struct Point {double x=0,y=0;};
enum class RectField {X,Y,Width,Height};
struct RectGeometry {
    double x=0,y=0,width=0,height=0;
    // Geometry owns every coordinate value. This order/count records only the
    // Source attribute-presence and insertion order for the SVG serializer.
    std::array<RectField,4> fields{RectField::X,RectField::Y,RectField::Width,RectField::Height};
    std::size_t fieldCount=4;
    std::optional<double> rx,ry;
};
enum class ViewportAlignment {
    None,XMinYMin,XMidYMin,XMaxYMin,XMinYMid,XMidYMid,XMaxYMid,
    XMinYMax,XMidYMax,XMaxYMax
};
enum class ViewportFit {Meet,Slice};
struct ViewportGeometry {
    double x=0,y=0,width=0,height=0;
    double viewX=0,viewY=0,viewWidth=0,viewHeight=0;
    bool hasViewBox=true;
    bool emitRect=true;
    ViewportAlignment alignment=ViewportAlignment::XMidYMid;
    ViewportFit fit=ViewportFit::Meet;
    // Nested SVG viewports clip by default. Source styles explicitly make
    // the outer result and table label/content viewports overflow-visible.
    // The hierarchy composes this rectangle with all enclosing clips.
    bool clipToViewport=true;
    // A relative extent is a fraction of the enclosing viewport; omitted
    // nested SVG width/height are both 100%. No CSS parsing is needed later.
    bool widthRelative=false,heightRelative=false;
    bool emitFit=false; // preserve an explicitly written meet/slice token
};
struct LineGeometry {Point from,to;double width=0;bool emitStrokeWidth=true;};
struct PolygonGeometry {std::vector<Point> points;};
struct EllipseGeometry {double cx=0,cy=0,rx=0,ry=0;};
struct PathCommand {char operation=0;std::vector<double> arguments;};
struct GlyphOutline {
    std::string sourcePath;
    std::vector<PathCommand> commands;
    // FontData path coordinates use 1000 units per MathJax em. This explicit
    // scale lets a non-SVG renderer consume commands without parsing `d`.
    double coordinateUnitsPerEm=1000;
};
// Font glyphs and stretch delimiters retain the exact outline path supplied by
// the source FontData.  The scene may also expose decoded commands to a future
// drawing consumer, but SVG serialization must not reconstruct them from DOM.
struct PathGeometry {std::shared_ptr<const GlyphOutline> outline;};
struct UseGeometry {std::string href;double x=0,y=0;bool xlink=false;};
using Geometry=std::variant<std::monostate,RectGeometry,ViewportGeometry,LineGeometry,PolygonGeometry,
                            PathGeometry,UseGeometry,ForeignObjectGeometry,EllipseGeometry>;

// A mutable input projection may provide current typed geometry from its real
// attribute owner. Frozen geometry remains a value with no backend references.
class GeometryOwner {
public:
    virtual ~GeometryOwner()=default;
    virtual std::shared_ptr<const GeometryOwner> cloneFor(const NodeRef& target)const=0;
    virtual void apply(Geometry& geometry,std::optional<Transform>& transform,
        std::span<const std::pair<std::string,std::string>> attributes)const=0;
};

enum class LineCap {Butt,Round,Square};
enum class LineJoin {Miter,Round,Bevel};
struct StrokeDash {
    std::vector<double> lengths;
    double offset=0;
    std::string separator=" "; // Source spelling; numeric lengths own the data
};
struct ShapePaint {
    // Unspecified values inherit from the parent. CSS paint tokens preserve
    // currentColor/none and Source-defined colors without an SVG parser.
    std::optional<std::string> fill,stroke;
    std::optional<double> strokeWidth;
    std::optional<LineCap> lineCap;
    std::optional<LineJoin> lineJoin;
    // Lengths and offset use the same mathematical em as geometry. A
    // one-element SVG pattern retains that length; consumers repeat it.
    std::optional<StrokeDash> dash;
    // Resolved class/stylesheet paint is available to drawing backends but
    // does not introduce attributes absent from the Source SVG call.
    bool emitAttributes=true;
};

// A mutable input projection may provide current typed paint from the same
// captured attribute values used by geometry. Frozen paint contains no backend
// owner or source identity.
class PaintOwner {
public:
    virtual ~PaintOwner()=default;
    virtual std::shared_ptr<const PaintOwner> cloneFor(const NodeRef& target)const=0;
    virtual void apply(std::optional<ShapePaint>& paint,const Geometry& geometry,
        std::span<const std::pair<std::string,std::string>> attributes)const=0;
};

// Semantic payloads survive SVG serialization and can be consumed by a
// non-DOM renderer without reverse-parsing SVG tags or attributes.
struct GlyphPrimitive {
    std::uint32_t codepoint=0;
    std::string variant;
    std::string font;
    std::shared_ptr<const GlyphOutline> outline;
};
struct TextRunPrimitive {
    std::u16string text;
    std::string variant;
    std::string fontFamily;
    bool italic=false,bold=false;
    // Font em size in the node's mathematical coordinate system. Missing
    // means inherited (for example the #text leaf inside a text element).
    std::optional<double> fontSize;
};
struct CommentRunPrimitive {std::u16string text;};
class SourceTextOwner {
public:
    virtual ~SourceTextOwner()=default;
    virtual std::u16string text()const=0;
};
struct SourceTextPrimitive {
    std::u16string text;bool comment=false;
    std::shared_ptr<const SourceTextOwner> owner;
    std::u16string currentText()const{return owner?owner->text():text;}
};
// Marks a mutable Lite DOM node projected into the live Scene owner. Kind,
// attributes, parent, and children live on Node; this carries no second copy.
struct SourceDomPrimitive {};
struct ClipPrimitive {std::string id;};
struct LinkPrimitive {std::u16string href;};
struct ImagePrimitive {std::u16string href;};
// Source SVG.handleInlineBreaks retains flow information alongside each
// completed line. Other drawing backends can consume this without HTML.
struct InlineBreakPrimitive {double space=0;bool forced=false,prebreak=false;};
using Semantic=std::variant<std::monostate,GlyphPrimitive,TextRunPrimitive,
                            CommentRunPrimitive,SourceTextPrimitive,SourceDomPrimitive,ClipPrimitive,LinkPrimitive,ImagePrimitive,HtmlNodePrimitive,
                            InlineBreakPrimitive>;

// Decode font path data once, when a glyph crosses from FontData into the
// completed scene.  The original spelling remains authoritative for SVG;
// commands are the backend-independent geometry view.
std::shared_ptr<const GlyphOutline> decodeGlyphOutline(std::string sourcePath);

struct Node {
    // SVG-compatible primitive/semantic kind, or the reserved "#text" kind.
    std::u16string kind;
    AttributeList attributes;
    Geometry geometry;
    std::shared_ptr<const GeometryOwner> geometryOwner;
    void applyGeometryOwner(Geometry& value,std::optional<Transform>& transformValue,
        std::span<const std::pair<std::string,std::string>> currentAttributes)const{
        value=geometry;
        transformValue=transform;
        if(geometryOwner)geometryOwner->apply(value,transformValue,currentAttributes);
    }
    Semantic semantic;
    // Pure subscription names for output hosts. This metadata never carries
    // Source callbacks or Context identities and is not an SVG attribute.
    std::vector<std::u16string> eventTypes;
    std::optional<ShapePaint> paint;
    std::shared_ptr<const PaintOwner> paintOwner;
    std::optional<Transform> transform;
    // Source DOM attribute insertion order, independent of numeric geometry.
    std::optional<std::size_t> transformAttributeIndex;
    // A raw transform copied from an explicit MathML attribute can survive
    // Source place() after a generated translation is prepended to it.
    std::optional<std::u16string> transformSuffix;
    // Preserve the explicit Source field value separately from this weak
    // structural edge, avoiding a shared_ptr parent/children cycle.
    options::Value parentValue;
    std::weak_ptr<Node> parent;
    ChildList children;
};

// A completed scene uses a separate recursive type so neither the root list
// nor a child slot can leak a mutable Builder node through shared_ptr<Node>.
struct FrozenNode;
using FrozenNodeRef=std::shared_ptr<const FrozenNode>;
struct FrozenNode {
    std::u16string kind;
    std::vector<std::pair<std::string,std::string>> attributes;
    Geometry geometry;
    Semantic semantic;
    std::vector<std::u16string> eventTypes;
    std::optional<ShapePaint> paint;
    std::optional<Transform> transform;
    std::optional<std::size_t> transformAttributeIndex;
    std::optional<std::u16string> transformSuffix;
    std::vector<FrozenNodeRef> children;
};

class Builder;

// A completed scene owns all nodes and all external resources it references.
// The resources vector is for actual immutable font/path owners; it never owns
// a layout job or a CommonWrapper arena.
class Scene {
    std::vector<FrozenNodeRef> roots_;
    std::vector<std::shared_ptr<const GlyphOutline>> resources_;
    friend class Builder;
    // Only Builder completion may populate a Scene.  The resulting scene has
    // no public mutation path and contains no mutable node or attribute handle.
    void freeze(std::span<const NodeRef>,
                std::span<const std::shared_ptr<const GlyphOutline>>);
public:
    const std::vector<FrozenNodeRef>& roots()const noexcept{return roots_;}
    const std::vector<std::shared_ptr<const GlyphOutline>>& resources()const noexcept{return resources_;}
};
}
