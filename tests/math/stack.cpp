#include "math/support/tex_fixture.hpp"

suite stack = [] {
    "stack open frames copy environment and release after the last activation"_test = [] {
        Fixture fixture;
        TexEnvironment environment;
        environment.font = u"normal";
        Stack stack(fixture.factory(), environment, true);
        auto frame = fixture.item(StackKind::open);
        std::weak_ptr<BaseItem> lifetime = frame;
        stack.Push(frame);
        frame.reset();
        expect(!lifetime.expired());
        expect(stack.height() == 2_u);
        stack.env().color = u"red";
        auto popped = stack.Pop();
        expect(popped->env().color.value() == u"red");
        expect(!stack.env().color);
        expect(stack.env().font.value() == u"normal");
        popped.reset();
        expect(lifetime.expired());
        expect(stack.height() == 1_u);
    };
    "stack preserves two activations of the same begin frame and its shared state"_test = [] {
        Fixture fixture;
        Stack stack(fixture.factory());
        auto begin = fixture.item(StackKind::begin);
        std::weak_ptr<BaseItem> lifetime = begin;
        stack.Push(begin);
        stack.Push(begin);
        expect(stack.height() == 3_u);
        expect(stack.Top() == stack.Top(2));
        stack.Top()->env().bold_symbol = true;
        expect(stack.Top(2)->env().bold_symbol.value());
        begin.reset();
        auto first = stack.Pop();
        first.reset();
        expect(!lifetime.expired());
        auto second = stack.Pop();
        expect(second->env().bold_symbol.value());
        second.reset();
        expect(lifetime.expired());
    };
    "stack consumes final frames without retaining discarded products in the factory"_test = [] {
        Fixture fixture;
        Stack stack(fixture.factory());
        auto &token = fixture.token();
        auto frame = fixture.mml(token);
        std::weak_ptr<BaseItem> lifetime = frame;
        stack.Push(frame);
        frame.reset();
        expect(lifetime.expired());
        expect(stack.height() == 1_u);
        expect(stack.Prev(true) == &token);
        expect(stack.Prev() == &token);
        expect(stack.Prev(true) == nullptr);
    };
    "stack replays fraction completion and the incoming stop in Source order"_test = [] {
        Fixture fixture;
        Stack stack(fixture.factory());
        auto &numerator = fixture.token(NodeKind::mi, u"a");
        auto &denominator = fixture.token(NodeKind::mi, u"b");
        stack.Push(&numerator);
        auto over = fixture.item(StackKind::over);
        std::weak_ptr<BaseItem> lifetime = over;
        stack.Push(over);
        over.reset();
        stack.Push(&denominator);
        stack.Push(fixture.item(StackKind::stop));
        expect(lifetime.expired());
        expect(stack.height() == 1_u);
        expect(stack.Top()->kind() == StackKind::mml);
        auto root = stack.Top()->First();
        expect(root->kind() == NodeKind::mfrac);
        expect(root->children()[0] == &numerator);
        expect(root->children()[1] == &denominator);
    };
};
