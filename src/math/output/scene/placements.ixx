// Source registration for implemented backend-independent SVG placements.
export module folia.math.output.scene:placements;
export import :wrapper;
export import :mrow;
export import :mo;
export import :mphantom;
export import :mpadded;
export import :mtable;
export import :mtr;
export import :mtd;
export import :menclose;
export import :text_node;
export import :math;
export import :mfenced;
export import :merror;
export import :maction;
export import :semantics;
export import :tex_atom;
export import :mglyph;
export import :scriptbase;
export import :msubsup;
export import :munderover;
export import :mmultiscripts;
export import :xml_node;
export import :inherited_svg_wrapper;
import std;

export namespace folia::math::output::scene {
void registerScenePlacements(WrapperFactory&);
}
