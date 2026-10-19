/*************************************************************
 * Source-identity registration of native wrapper implementations.
 * This module is intentionally separate from the frozen SVG Jax entrypoint;
 * it does not invent descriptors for Source kinds without a typed owner.
 *************************************************************/
export module folia.math.output.svg.wrapper_classes;
export import folia.math.output.common;
import std;

export namespace folia::math::output::svg {
// Read the actual Source SvgWrappers object in its own enumerable-key order,
// bind each supported constructor identity to one canonical Common factory
// descriptor, and return the exact initial creator map. A missing native kind
// fails explicitly; no generic CommonWrapper substitution is made.
common::CommonWrapperClassMap registerImplementedWrapperClasses(
    options::Context&, options::ObjectRef actualSvgWrappers,
    common::CommonFactoryContextBindings&, folia::math::ContextPropertyBindings&);
}
