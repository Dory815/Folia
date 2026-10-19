// Canonical Context identities for actual common FontData class and instances.
export module folia.math.output.common.font_data_bindings;
export import folia.math.output.common.font_data;
export import folia.math.util.native_identities;
import std;

export namespace folia::math::output::common {
class FontDataContextBindings {
public:
    using Allocator=std::function<std::shared_ptr<FontData>(
        FontDataClass&,FontLoadHost&,options::Context&,options::Value)>;
    using InstanceInitializer=std::function<void(
        const std::shared_ptr<FontData>&,options::ObjectRef)>;
private:
    struct State;
    std::shared_ptr<State> state_;
public:
    explicit FontDataContextBindings(options::Context&);
    FontDataContextBindings(const FontDataContextBindings&)=delete;
    FontDataContextBindings& operator=(const FontDataContextBindings&)=delete;

    // The class owner and host are the actual Source class data and output-jax
    // service. Their anchors are retained by the Context constructor closure;
    // allocation always returns the requested derived FontData owner.
    options::ObjectRef registerClass(std::u16string name,
        std::shared_ptr<FontDataClass> classOwner,
        FontLoadHost& host,std::shared_ptr<void> hostAnchor,Allocator,
        std::optional<options::ObjectRef> parentConstructor={},
        InstanceInitializer initializeInstance={});
    std::shared_ptr<FontData> resolve(options::ObjectRef instance)const;
    std::shared_ptr<FontDataClass> resolveClass(options::ObjectRef constructor)const;
    // Return the existing canonical Context handles for the exact native
    // owners; these are used for Source `.constructor` property reads/writes.
    std::optional<options::ObjectRef> identity(const FontData*)const;
    std::optional<options::ObjectRef> constructor(const FontDataClass*)const;
    bool isFontData(options::Value value)const;
    options::Context& context()const;
};
}
