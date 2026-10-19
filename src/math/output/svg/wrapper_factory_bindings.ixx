/*************************************************************
 * Source-order extraction of SvgWrapperFactory.defaultNodes.
 * The resulting typed creator map is consumed by the existing common
 * factory; this helper owns no class registry or factory state.
 *************************************************************/
export module folia.math.output.svg.wrapper_factory_bindings;
export import folia.math.output.svg.wrapper_factory;
export import folia.math.output.common;
export import folia.math.util.option_types;
import std;

export namespace folia::math::output::svg {
using WrapperClassResolver =
    std::function<common::CommonWrapperClassRef(options::ObjectRef)>;

// Mirrors AbstractFactory's constructor-time operation sequence:
// read this.constructor.defaultNodes once, snapshot Object.keys, then Get each
// key only when it is visited. The class resolver must return the canonical
// typed class descriptor for that exact Context constructor object.
common::CommonWrapperClassMap readDefaultNodes(
    options::Context&, options::ObjectRef actualConstructor,
    const WrapperClassResolver&);
common::CommonWrapperClassMap readDefaultNodes(
    options::Context&, options::Value actualNodes,
    const WrapperClassResolver&);

// Per-Context module owner for the actual SvgWrapperFactory constructor.
// `actualSvgWrappers` is the Source module's ordinary class-map object; it is
// installed by reference on the constructor and read live for every instance.
class SvgWrapperFactoryContextBindings {
    struct State;
    std::shared_ptr<State> state_;
public:
    SvgWrapperFactoryContextBindings(options::Context&,
        common::CommonFactoryContextBindings&);
    ~SvgWrapperFactoryContextBindings();
    SvgWrapperFactoryContextBindings(const SvgWrapperFactoryContextBindings&)=delete;
    SvgWrapperFactoryContextBindings& operator=(const SvgWrapperFactoryContextBindings&)=delete;
    options::Context& context()const;
    options::ObjectRef registerConstructor(options::ObjectRef actualSvgWrappers,
        common::CommonFactoryContextBindings& commonBindings,
        std::optional<options::ObjectRef> parentConstructor={});
    std::shared_ptr<SvgWrapperFactory> construct(
        std::span<const options::Value> args={});
    std::shared_ptr<SvgWrapperFactory> resolve(options::ObjectRef)const;
};
}
