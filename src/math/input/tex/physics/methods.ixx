// MathJax 4.1.3 input/tex/physics/PhysicsMethods.ts.
export module folia.math.input.tex.physics.methods;
export import folia.math.input.tex.parser;
export import folia.math.input.tex.physics.items;
import std;
export namespace folia::math::tex {
struct PhysicsMethodSet {
    Property methods=Missing{};
    Property get(std::u16string_view name)const;
};
struct PhysicsMethods { static PhysicsMethodSet create(ParserRuntime&); };
}
