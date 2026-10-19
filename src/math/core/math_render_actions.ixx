// MathJax 4.1.3 core/MathDocument.ts RenderList.
// Render actions retain their canonical Context callback and RenderData objects.
export module folia.math.core.math_render_actions;
export import folia.math.core.math_item;
export import folia.math.util.prioritized_list;
export import folia.math.util.option_types;
import std;

export namespace folia::math::core {
// Actual MathItem providers implement this identity surface. RenderList uses
// it only at the Source call point; it never manufactures an item object.
class MathItemSourceIdentity {
public:
    virtual ~MathItemSourceIdentity()=default;
    virtual options::Context& sourceContext()const=0;
    virtual options::ObjectRef sourceIdentity()const=0;
};

struct RenderData final : NativeObject {
    options::ObjectRef identity{};
};
using RenderDataRef=std::shared_ptr<RenderData>;

// The same prioritized storage drives execution and iteration. Context values
// supplied to create() remain live while action tuples are read.
class RenderActions final : public PrioritizedList<RenderDataRef>,
                            public MathRenderActionsService {
    options::Context& context_;
    options::ObjectRef identity_;
    RenderDataRef makeData(std::u16string id,options::Value renderDoc,
                           options::Value renderMath,options::Value convert);
    options::ObjectRef methodAction(std::u16string method,bool documentMethod);
    bool invoke(const RenderDataRef&,bool documentMethod,MathItem*,
                MathDocumentService&,bool& result);
public:
    RenderActions(options::Context&,options::ObjectRef identity);
    RenderActions(const RenderActions&)=delete;
    RenderActions& operator=(const RenderActions&)=delete;

    // Source RenderList.create over the actual enumerable keys and action
    // values of the supplied ordinary object.
    void create(options::Value actions);
    std::pair<RenderDataRef,options::Value> action(std::u16string id,
                                                    options::Value definition);
    void renderDoc(MathDocumentService&,Property start=Missing{});
    void renderMath(MathItem&,MathDocumentReference,Property start)override;
    void renderConvert(MathItem&,MathDocumentReference,Property end)override;
    options::Value findID(std::u16string_view id);
    options::ObjectRef sourceIdentity()const noexcept{return identity_;}
    options::Context& sourceContext()const noexcept{return context_;}
};
}
