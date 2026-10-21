#include "math/support/tex_fixture.hpp"

namespace {
DispatchOutcome literal(TexParser &parser, TokenInput const &input) {
    auto text = std::get<std::u16string_view>(input.value);
    parser.Push(&parser.createToken(NodeKind::mi, text));
    return Handled{};
}
DispatchOutcome failCharacter(TexParser &, TokenInput const &) {
    throw TexError("Fixture", u"Character callback failed");
}
std::shared_ptr<RegExpMap> install(Fixture &fixture) {
    auto map = std::make_shared<RegExpMap>(
        RegExpMapInit{{u"literal", TokenMapParser{TokenInputHandler{literal}}},
                      std::make_shared<folia::math::regexp_host::RegExp>(u"[\\s\\S]")});
    fixture.maps.registerMap(map);
    const std::array registrations{HandlerRegistration{HandlerKind::character, {u"literal"}}};
    fixture.options->handlers.add(registrations);
    return map;
}
std::u16string shape(MmlNode const &node) {
    if (node.kind() == NodeKind::text)
        return u"text(" + std::u16string(node.text()) + u")";
    auto name = nodeKindName(node.kind());
    std::u16string result(name.begin(), name.end());
    result += u'(';
    for (std::size_t i = 0; i < node.children().size(); ++i) {
        if (i)
            result += u',';
        if (node.children()[i])
            result += shape(*node.children()[i]);
    }
    result += u')';
    return result;
}
} // namespace

suite tex_parser = [] {
    "tex parser uses real configured dispatch and matches Source token trees"_test = [] {
        Fixture fixture;
        install(fixture);
        struct Case {
            std::u16string source, expected;
        };
        const std::array cases{Case{u"", u"inferredMrow()"},
                               Case{u"abc", u"inferredMrow(mi(text(a)),mi(text(b)),mi(text(c)))"},
                               Case{u"α𝛼", u"inferredMrow(mi(text(α)),mi(text(𝛼)))"}};
        for (auto const &item : cases) {
            TexParser parser(item.source, ParserEnvironmentSeed{}, *fixture.options);
            expect(fixture.options->request().parsers.size() == 1_u);
            auto node = parser.mml();
            expect(node != nullptr);
            if (!node)
                continue;
            expect(shape(*node) == item.expected);
            expect(attr(*node, u"data-latex") == item.source);
            expect(fixture.options->request().parsers.empty());
        }
    };
    "tex parser construction exceptions remove the exact request scope"_test = [] {
        Fixture fixture;
        auto map = install(fixture);
        TexParser outer(u"", ParserEnvironmentSeed{}, *fixture.options);
        map->setParser(TokenMapParser{TokenInputHandler{failCharacter}});
        texError([&] { TexParser failed(u"x", ParserEnvironmentSeed{}, *fixture.options); },
                 "Fixture");
        expect(fixture.options->request().parsers.size() == 1_u);
        expect(fixture.options->parser() == &outer);
        map->setParser(TokenMapParser{TokenInputHandler{literal}});
        TexParser inner(u"x", ParserEnvironmentSeed{}, *fixture.options);
        expect(fixture.options->request().parsers.size() == 2_u);
        expect(inner.mml() != nullptr);
        expect(fixture.options->parser() == &outer);
        expect(outer.mml() != nullptr);
        expect(fixture.options->request().parsers.empty());
    };
    "tex parser provenance snapshots survive source replacement without retaining parser objects"_test =
        [] {
            Fixture fixture;
            install(fixture);
            TexParser parser(u"a", ParserEnvironmentSeed{}, *fixture.options);
            auto snapshot = parser.sourceSnapshot();
            auto frame = fixture.item(StackKind::open);
            parser.setString(u"later");
            parser.setPosition(2);
            parser.Push(frame);
            expect(*snapshot == u"a");
            expect(frame->sourceAtCreation() == u"later");
            parser.setString(u"replacement");
            expect(frame->sourceAtCreation() == u"later");
            expect(frame->stopI == 2_u);
            // A constructor's saved source and the activation's source are values,
            // while frame and MML ownership remains on their existing owners.
        };
    "ParseOptions filters duplicate node registrations by current root ancestry"_test = [] {
        Fixture fixture;
        auto &options = *fixture.options;
        auto &live = fixture.token();
        auto &dead = fixture.token();
        std::array children{&live};
        auto &root = fixture.node(NodeKind::mrow, children);
        options.addNode(u"postprocess", live);
        options.addNode(u"postprocess", live);
        options.addNode(u"postprocess", dead);
        options.request().root = &root;
        expect(options.getList(u"postprocess").size() == 2_u);
        expect(live.properties().tex.in_lists->size() == 2_u);
        expect(dead.properties().tex.in_lists->size() == 1_u);
        const std::array remove{&live};
        options.removeFromList(u"postprocess", remove);
        expect(options.getList(u"postprocess").size() == 1_u);
        root.removeChild(live);
        expect(options.getList(u"postprocess").empty());
        auto &clone = fixture.document.cloneSubtree(live);
        expect(
            std::ranges::equal(*clone.properties().tex.in_lists, *live.properties().tex.in_lists));
        fixture.effective.maxBuffer = 123;
        fixture.session.defineMacro(u"persist", MacroDefinition{.replacement_source = u"x"});
        options.request().error = true;
        options.clear();
        expect(options.request().root == nullptr);
        expect(options.request().nodeLists.empty());
        expect(!options.request().error);
        expect(fixture.effective.maxBuffer.value() == 123_u);
        expect(fixture.session.findMacro(u"persist") != nullptr);
    };
};
