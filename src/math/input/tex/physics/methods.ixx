// MathJax 4.1.3 input/tex/physics/PhysicsMethods.ts; Apache-2.0.
export module folia.math.input.tex.physics.methods;
export import folia.math.input.tex.physics.items;
export import folia.math.input.tex.base_methods;
import std;
export namespace folia::math::tex {
struct PhysicsMethods {
    static DispatchOutcome Quantity(TexParser &, MacroInvocation const &);
    static DispatchOutcome Eval(TexParser &, MacroInvocation const &);
    static DispatchOutcome Commutator(TexParser &, MacroInvocation const &);
    static DispatchOutcome VectorBold(TexParser &, MacroInvocation const &);
    static DispatchOutcome StarMacro(TexParser &, MacroInvocation const &);
    static DispatchOutcome OperatorApplication(TexParser &, MacroInvocation const &);
    static DispatchOutcome VectorOperator(TexParser &, MacroInvocation const &);
    static DispatchOutcome Expression(TexParser &, MacroInvocation const &);
    static DispatchOutcome Qqtext(TexParser &, MacroInvocation const &);
    static DispatchOutcome Differential(TexParser &, MacroInvocation const &);
    static DispatchOutcome Derivative(TexParser &, MacroInvocation const &);
    static DispatchOutcome Bra(TexParser &, MacroInvocation const &);
    static DispatchOutcome Ket(TexParser &, MacroInvocation const &);
    static DispatchOutcome BraKet(TexParser &, MacroInvocation const &);
    static DispatchOutcome KetBra(TexParser &, MacroInvocation const &);
    static DispatchOutcome Expectation(TexParser &, MacroInvocation const &);
    static DispatchOutcome MatrixElement(TexParser &, MacroInvocation const &);
    static DispatchOutcome MatrixQuantity(TexParser &, MacroInvocation const &);
    static DispatchOutcome IdentityMatrix(TexParser &, MacroInvocation const &);
    static DispatchOutcome XMatrix(TexParser &, MacroInvocation const &);
    static DispatchOutcome PauliMatrix(TexParser &, MacroInvocation const &);
    static DispatchOutcome DiagonalMatrix(TexParser &, MacroInvocation const &);
    static DispatchOutcome AutoClose(TexParser &, MacroInvocation const &);
    static DispatchOutcome Vnabla(TexParser &, MacroInvocation const &);
    static DispatchOutcome DiffD(TexParser &, MacroInvocation const &);
    static constexpr auto Macro = &BaseMethods::Macro;
    static constexpr auto NamedFn = &BaseMethods::NamedFn;
    static constexpr auto Array = &BaseMethods::Array;
};
} // namespace folia::math::tex
