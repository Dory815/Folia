// MathJax 4.1.3 input/tex/begingroup/BegingroupMethods.ts; Apache-2.0.
module folia.math.input.tex.begingroup.methods;
import std;

namespace folia::math::tex {
namespace {
BegingroupStack &stack(TexParser &parser) {
    if (!parser.configuration.begingroup)
        throw std::logic_error("The begingroup package has not been configured");
    return *parser.configuration.begingroup;
}
}
DispatchOutcome BegingroupMethods::begingroup(TexParser &parser, MacroInvocation const &) {
    stack(parser).push();
    return NoParseResult{};
}
DispatchOutcome BegingroupMethods::endgroup(TexParser &parser, MacroInvocation const &) {
    stack(parser).pop();
    return NoParseResult{};
}
DispatchOutcome BegingroupMethods::reset(TexParser &parser, MacroInvocation const &) {
    stack(parser).reset();
    return NoParseResult{};
}
DispatchOutcome BegingroupMethods::sandbox(TexParser &parser, MacroInvocation const &) {
    stack(parser).sandbox();
    parser.stack.global().is_sandbox = true;
    return NoParseResult{};
}
DispatchOutcome BegingroupMethods::global(TexParser &parser, MacroInvocation const &) {
    const auto position = parser.position();
    std::u16string command;
    if (parser.GetNext() == u"\\") {
        parser.setPosition(parser.position() + 1);
        command = parser.GetCS();
    }
    parser.setPosition(position);
    auto const &allow = *parser.options().begingroup->allowGlobal;
    if (std::ranges::find(allow, command) == allow.end()) {
        const std::array arguments{parser.currentCS()};
        throw TexError("IllegalGlobal", u"Invalid use of %1", arguments);
    }
    parser.stack.env().is_global = true;
    return NoParseResult{};
}
}
