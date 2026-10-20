#pragma once
#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.node_util;

using namespace boost::ut;
using namespace folia::math::tex;
using namespace folia::math::mml;

namespace {
struct Warnings final : WarningSink {
    void warn(std::u16string_view) override {}
};
struct Fixture {
    ConfigurationHandler registry;
    MapHandler maps;
    TexSessionState session;
    MacroCallbacks callbacks{{}, {}};
    Warnings warnings;
    ParserConfiguration configuration{registry, maps, session, callbacks, warnings, {}};
    TexOptionValues effective;
    MmlDocument document;
    std::unique_ptr<ParseOptions> options;
    Fixture() {
        configuration.items = baseItemClasses();
        effective.maxBuffer = 10000;
        effective.maxTemplateSubtitutions = 10000;
        effective.maxMacros = 1000;
        effective.tagSide = u"right";
        effective.tagIndent = u"0.8em";
        maps.registerMap(std::make_shared<CharacterMap>(
            CharacterMapInit{{u"not_remap", std::nullopt}, {Token(u"=", std::u16string(u"≠"))}}));
        options =
            std::make_unique<ParseOptions>(configuration, effective, session, document.factory(),
                                           ParseOptions::getVariant(u"TeX"));
        options->tags = std::make_unique<NoTags>(*options);
    }
    StackItemFactory &factory() {
        return *options->itemFactory;
    }
    FrameHandle item(StackKind kind, StackItemInit const &init = NoItemInit{}) {
        return factory().create(kind, init);
    }
    MmlNode &token(NodeKind kind = NodeKind::mi, std::u16string_view text = u"x") {
        return options->nodeFactory.makeToken(kind, text);
    }
    MmlNode &node(NodeKind kind, std::span<MmlNode *const> children = {}) {
        return options->nodeFactory.makeNode(kind, children);
    }
    FrameHandle mml(MmlNode &node) {
        return item(StackKind::mml, MmlItemInit{node});
    }
};
MmlNode &replayNode(CheckResult const &result, std::size_t i = 0) {
    if (auto *node = std::get_if<MmlNode *>(&result.replacements.at(i)))
        return **node;
    return *std::get<FrameHandle>(result.replacements.at(i))->First();
}
std::u16string attr(MmlNode const &node, std::u16string_view name) {
    auto *value = node.attributes().lookup(name);
    return value && std::holds_alternative<std::u16string>(*value)
               ? std::get<std::u16string>(*value)
               : std::u16string{};
}
std::u16string attr(AttributeSet const &attrs, std::u16string_view name) {
    auto *value = attrs.own(name);
    return value && std::holds_alternative<std::u16string>(*value)
               ? std::get<std::u16string>(*value)
               : std::u16string{};
}
template <class Action> void texError(Action action, std::string_view id) {
    try {
        action();
        expect(false) << "Expected TexError";
    } catch (TexError const &error) {
        expect(error.id() == id);
    }
}
struct ArrayProbe : ArrayItem {
    using ArrayItem::ArrayItem;
    using ArrayItem::getEntry;
};
} // namespace
