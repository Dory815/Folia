export module folia.math.core.mml_tree.node_kind;
import std;

export namespace folia::math::mml {
enum class NodeKind : std::uint8_t {
    math,
    mi,
    mn,
    mo,
    mtext,
    mspace,
    ms,
    mrow,
    inferred_mrow,
    mfrac,
    msqrt,
    mroot,
    mstyle,
    merror,
    mpadded,
    mphantom,
    mfenced,
    menclose,
    maction,
    msub,
    msup,
    msubsup,
    munder,
    mover,
    munderover,
    mmultiscripts,
    mprescripts,
    none,
    mtable,
    mlabeledtr,
    mtr,
    mtd,
    maligngroup,
    malignmark,
    mglyph,
    semantics,
    annotation,
    annotation_xml,
    tex_atom,
    math_choice,
    text,
    xml,
    html
};

inline constexpr std::array<NodeKind, 43> allNodeKinds{{
    NodeKind::math,
    NodeKind::mi,
    NodeKind::mn,
    NodeKind::mo,
    NodeKind::mtext,
    NodeKind::mspace,
    NodeKind::ms,
    NodeKind::mrow,
    NodeKind::inferred_mrow,
    NodeKind::mfrac,
    NodeKind::msqrt,
    NodeKind::mroot,
    NodeKind::mstyle,
    NodeKind::merror,
    NodeKind::mpadded,
    NodeKind::mphantom,
    NodeKind::mfenced,
    NodeKind::menclose,
    NodeKind::maction,
    NodeKind::msub,
    NodeKind::msup,
    NodeKind::msubsup,
    NodeKind::munder,
    NodeKind::mover,
    NodeKind::munderover,
    NodeKind::mmultiscripts,
    NodeKind::mprescripts,
    NodeKind::none,
    NodeKind::mtable,
    NodeKind::mlabeledtr,
    NodeKind::mtr,
    NodeKind::mtd,
    NodeKind::maligngroup,
    NodeKind::malignmark,
    NodeKind::mglyph,
    NodeKind::semantics,
    NodeKind::annotation,
    NodeKind::annotation_xml,
    NodeKind::tex_atom,
    NodeKind::math_choice,
    NodeKind::text,
    NodeKind::xml,
    NodeKind::html
}};

constexpr std::string_view nodeKindName(NodeKind kind) noexcept {
    switch (kind) {
        case NodeKind::math: return "math";
        case NodeKind::mi: return "mi";
        case NodeKind::mn: return "mn";
        case NodeKind::mo: return "mo";
        case NodeKind::mtext: return "mtext";
        case NodeKind::mspace: return "mspace";
        case NodeKind::ms: return "ms";
        case NodeKind::mrow: return "mrow";
        case NodeKind::inferred_mrow: return "inferredMrow";
        case NodeKind::mfrac: return "mfrac";
        case NodeKind::msqrt: return "msqrt";
        case NodeKind::mroot: return "mroot";
        case NodeKind::mstyle: return "mstyle";
        case NodeKind::merror: return "merror";
        case NodeKind::mpadded: return "mpadded";
        case NodeKind::mphantom: return "mphantom";
        case NodeKind::mfenced: return "mfenced";
        case NodeKind::menclose: return "menclose";
        case NodeKind::maction: return "maction";
        case NodeKind::msub: return "msub";
        case NodeKind::msup: return "msup";
        case NodeKind::msubsup: return "msubsup";
        case NodeKind::munder: return "munder";
        case NodeKind::mover: return "mover";
        case NodeKind::munderover: return "munderover";
        case NodeKind::mmultiscripts: return "mmultiscripts";
        case NodeKind::mprescripts: return "mprescripts";
        case NodeKind::none: return "none";
        case NodeKind::mtable: return "mtable";
        case NodeKind::mlabeledtr: return "mlabeledtr";
        case NodeKind::mtr: return "mtr";
        case NodeKind::mtd: return "mtd";
        case NodeKind::maligngroup: return "maligngroup";
        case NodeKind::malignmark: return "malignmark";
        case NodeKind::mglyph: return "mglyph";
        case NodeKind::semantics: return "semantics";
        case NodeKind::annotation: return "annotation";
        case NodeKind::annotation_xml: return "annotation-xml";
        case NodeKind::tex_atom: return "TeXAtom";
        case NodeKind::math_choice: return "MathChoice";
        case NodeKind::text: return "text";
        case NodeKind::xml: return "XML";
        case NodeKind::html: return "html";
    }
    return {};
}

constexpr std::optional<NodeKind> nodeKindFromName(std::string_view name) noexcept {
    for (const auto kind : allNodeKinds) {
        if (nodeKindName(kind) == name) {
            return kind;
        }
    }
    return std::nullopt;
}
}
