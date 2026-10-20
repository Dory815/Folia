// MathJax 4.1.3 input/tex/braket/BraketMethods.ts; Apache-2.0.
module folia.math.input.tex.braket.methods;
import std;

namespace folia::math::tex {
DispatchOutcome BraketMethods::Braket(TexParser &parser, MacroInvocation const &invocation) {
    const auto position = parser.position();
    (void)parser.GetArgument(invocation.dispatch_token);
    parser.setPosition(position);
    bool single = true;
    if (parser.GetNext() == u"{") {
        parser.setPosition(parser.position() + 1);
        single = false;
    }
    auto arguments = std::get<std::span<TexArgument const>>(invocation.arguments);
    auto frame = parser.itemFactory().create(StackKind::braket);
    auto &item = dynamic_cast<BraketItem &>(*frame);
    item.bar_count = 0;
    item.open = std::get<std::u16string>(arguments[0]);
    item.close = std::get<std::u16string>(arguments[1]);
    item.stretchy = std::get<bool>(arguments[2]);
    item.bar_max = std::get<double>(arguments[3]);
    item.single = single;
    item.space = arguments.size() > 4 ? std::get<bool>(arguments[4]) : false;
    parser.Push(frame);
    item.env().braket_item_depth = parser.stack.height() - 1;
    return NoParseResult{};
}
DispatchOutcome BraketMethods::Bar(TexParser &parser, MacroInvocation const &invocation) {
    auto depth = parser.stack.env().braket_item_depth;
    const auto height = parser.stack.height();
    if (!depth || *depth >= height)
        return ContinueDispatch{};
    auto top = dynamic_cast<BraketItem *>(parser.stack.Top(height - *depth));
    if (!top || top->bar_count >= top->bar_max)
        return ContinueDispatch{};
    std::u16string mark = invocation.dispatch_token == u"|" ? u"|" : u"\u2016";
    if (mark == u"|" && parser.GetNext() == u"|") {
        parser.setPosition(parser.position() + 1);
        mark = u"\u2016";
    }
    NodeDefinition definition;
    definition.attributes.set(u"stretchy", top->stretchy);
    definition.attributes.set(u"data-braketbar", true);
    definition.properties.tex.explicit_tex_class =
        top->stretchy ? mml::TexClass::bin : mml::TexClass::ord;
    if (!top->stretchy) {
        parser.Push(&parser.createToken(mml::NodeKind::mo, mark, std::move(definition)));
        return Handled{};
    }
    auto close = parser.itemFactory().create(StackKind::close);
    dynamic_cast<CloseItem &>(*close).braket_bar = true;
    parser.Push(close);
    NodeDefinition atom;
    atom.properties.tex.explicit_tex_class = mml::TexClass::close;
    top->barNodes.push_back(&parser.create(mml::NodeKind::tex_atom, {}, atom));
    top->barNodes.push_back(&parser.createToken(mml::NodeKind::mo, mark, std::move(definition)));
    atom.properties.tex.explicit_tex_class = mml::TexClass::open;
    top->barNodes.push_back(&parser.create(mml::NodeKind::tex_atom, {}, std::move(atom)));
    ++top->bar_count;
    return Handled{};
}
} // namespace folia::math::tex
