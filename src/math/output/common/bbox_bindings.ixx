// Context identity and live source-property projection for the real BBox owner.
export module folia.math.output.common.bbox_context_bindings;
export import folia.math.util.bbox;
export import folia.math.output.common.line_bbox;
export import folia.math.util.context_property_bindings;
import std;

export namespace folia::math::output::common {
class BBoxContextBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    BBoxContextBindings(options::Context&,ContextPropertyBindings&);
    ~BBoxContextBindings();
    BBoxContextBindings(const BBoxContextBindings&)=delete;
    BBoxContextBindings& operator=(const BBoxContextBindings&)=delete;
    options::Context& context()const;
    options::ObjectRef constructor();
    options::ObjectRef lineConstructor();
    options::ObjectRef bind(const std::shared_ptr<BBox>&);
    options::ObjectRef bind(const std::shared_ptr<LineBBox>&);
    std::shared_ptr<BBox> resolve(options::ObjectRef)const;
    std::shared_ptr<LineBBox> resolveLine(options::ObjectRef)const;
};
}
