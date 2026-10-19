export module folia.math.core.mml_tree.node_traits;
export import folia.math.core.mml_tree.node_kind;
import std;

export namespace folia::math::mml {
enum class NodeArityKind : std::uint8_t {
    any,
    at_least_one,
    exact,
    inferred_row
};

struct NodeTraits final {
    NodeArityKind arityKind;
    std::uint8_t exactArity;
};

constexpr NodeTraits nodeTraits(NodeKind kind) noexcept {
    using K = NodeKind;
    using A = NodeArityKind;
    switch (kind) {
        case K::math: return {A::inferred_row, 0};
        case K::mi: return {A::any, 0};
        case K::mn: return {A::any, 0};
        case K::mo: return {A::any, 0};
        case K::mtext: return {A::any, 0};
        case K::mspace: return {A::exact, 0};
        case K::ms: return {A::any, 0};
        case K::mrow: return {A::any, 0};
        case K::inferred_mrow: return {A::any, 0};
        case K::mfrac: return {A::exact, 2};
        case K::msqrt: return {A::inferred_row, 0};
        case K::mroot: return {A::exact, 2};
        case K::mstyle: return {A::inferred_row, 0};
        case K::merror: return {A::inferred_row, 0};
        case K::mpadded: return {A::inferred_row, 0};
        case K::mphantom: return {A::inferred_row, 0};
        case K::mfenced: return {A::any, 0};
        case K::menclose: return {A::inferred_row, 0};
        case K::maction: return {A::at_least_one, 1};
        case K::msub: return {A::exact, 2};
        case K::msup: return {A::exact, 2};
        case K::msubsup: return {A::exact, 3};
        case K::munder: return {A::exact, 2};
        case K::mover: return {A::exact, 2};
        case K::munderover: return {A::exact, 3};
        case K::mmultiscripts: return {A::at_least_one, 1};
        case K::mprescripts: return {A::exact, 0};
        case K::none: return {A::exact, 0};
        case K::mtable: return {A::any, 0};
        case K::mlabeledtr: return {A::at_least_one, 1};
        case K::mtr: return {A::any, 0};
        case K::mtd: return {A::inferred_row, 0};
        case K::maligngroup: return {A::inferred_row, 0};
        case K::malignmark: return {A::exact, 0};
        case K::mglyph: return {A::any, 0};
        case K::semantics: return {A::at_least_one, 1};
        case K::annotation: return {A::any, 0};
        case K::annotation_xml: return {A::any, 0};
        case K::tex_atom: return {A::inferred_row, 0};
        case K::math_choice: return {A::exact, 4};
        case K::text: return {A::exact, 0};
        case K::xml: return {A::exact, 0};
        case K::html: return {A::exact, 0};
    }
    return {A::any, 0};
}
}
