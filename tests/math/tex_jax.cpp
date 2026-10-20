#include "math/support/tex_fixture.hpp"
import folia.math.input.tex.filter_util;

namespace {
struct JaxFixture {
    ConfigurationHandler registry;
    MapHandler maps;
    MacroCallbacks callbacks{{}, {}};
    Warnings warnings;
    MmlDocument document;
    std::unique_ptr<TeX> jax;
    void configure(TexOptionValues supplied = {}) {
        jax = std::make_unique<TeX>(registry, maps, callbacks, warnings, document.factory(),
                                    std::move(supplied));
    }
    MmlNode &compile(std::u16string text, bool display = true) {
        folia::math::core::AbstractMathItem math(std::move(text), jax.get());
        math.setDisplay(display);
        return jax->compile(math, document);
    }
};
std::vector<MmlNode *> findNodes(MmlNode &root, NodeKind kind) {
    std::vector<MmlNode *> result;
    root.walkTree([&](auto &node) {
        if (node.kind() == kind)
            result.push_back(&node);
    });
    return result;
}
std::u16string tokenText(MmlNode &root) {
    std::u16string text;
    root.walkTree([&](auto &node) {
        if (node.isToken())
            text += NodeUtil::getText(node);
    });
    return text;
}
void mark(TexFilterData &data, std::u16string_view step) {
    auto &trace = data.data.options.baseURL;
    trace = trace.value_or(u"") + std::u16string(step);
}
void userPre(TexFilterData &data) {
    mark(data, u"user-pre;");
    data.math.math = u"x^2";
}
void configuredPre(TexFilterData &data) {
    mark(data, u"config-pre;");
}
void packagePre(TexFilterData &data) {
    mark(data, u"package-pre;");
}
void userPost(TexFilterData &data) {
    mark(data, u"user-post;");
}
void packagePost(TexFilterData &data) {
    mark(data, u"package-post;");
}
void configurePackage(ParserConfiguration &, TeX &jax) {
    jax.options.baseURL = jax.options.baseURL.value_or(u"") + u"config;";
    jax.preFilters.add(configuredPre);
}
void failPre(TexFilterData &) {
    throw std::runtime_error("preprocessor exception");
}
bool stopPost(TexFilterData &data) {
    mark(data, u"stop;");
    return false;
}
MmlNode &customError(TeX &jax, TexError const &error) {
    return jax.parseOptions().nodeFactory.makeToken(NodeKind::mtext,
                                                    std::u16string(error.message()).substr(0, 10));
}
void dynamicInit(ParserConfiguration &config) {
    config.options.maxMacros = config.options.maxMacros.value_or(0) + 1;
}
void dynamicConfig(ParserConfiguration &, TeX &jax) {
    jax.options.baseURL = jax.options.baseURL.value_or(u"") + u"dynamic;";
}
MmlNode &dynamicNode(NodeFactory &factory, NodeKind kind, std::span<MmlNode *const> children,
                     NodeDefinition definition, MmlNode *text) {
    definition.attributes.set(u"data-created", u"dynamic");
    return NodeFactory::createNode(factory, kind, children, std::move(definition), text);
}
} // namespace

suite tex_jax = [] {
    "TeX input jax compiles base syntax through all original postfilters"_test = [] {
        JaxFixture f;
        f.configure();
        expect(f.jax->name() == "TeX");
        expect(f.jax->processStrings());
        expect(f.jax->options.maxBuffer.value() == 5120_u);
        expect(f.jax->options.maxTemplateSubtitutions.value() == 10000_u);
        expect(f.jax->postFilters.entries().size() == 8_u);
        auto &root = f.compile(u"\\mathbb{R}+\\frac{1}{\\sqrt[3]{x}}+x^2");
        expect(root.kind() == NodeKind::math);
        expect(attr(root, u"display") == u"block");
        expect(findNodes(root, NodeKind::mfrac).size() == 1_u);
        expect(findNodes(root, NodeKind::mroot).size() == 1_u);
        expect(findNodes(root, NodeKind::msup).size() == 1_u);
        expect(findNodes(root, NodeKind::msubsup).empty());
        auto identifiers = findNodes(root, NodeKind::mi);
        expect(std::ranges::any_of(identifiers, [](auto node) {
            return NodeUtil::getText(*node) == u"R" &&
                   attr(*node, u"mathvariant") == u"double-struck";
        }));
        expect(!f.jax->parseOptions().error);
        expect(f.jax->parseOptions().parsers.empty());
    };
    "TeX input jax preserves supplied config and package filter registration order"_test = [] {
        JaxFixture f;
        ConfigurationDefinition package;
        package.config = ProcessorSpec<ConfigMethod>{configurePackage};
        package.preprocessors.emplace_back(FilterList::Callback{packagePre});
        package.postprocessors.emplace_back(FilterList::Callback{packagePost});
        Configuration::create(f.registry, u"test-filter-order", std::move(package));
        TexOptionValues supplied;
        supplied.packages =
            std::vector<PackageSelection>{{u"base", {}}, {u"test-filter-order", {}}};
        supplied.preFilters = std::vector<FilterList::Item>{{5, userPre}};
        supplied.postFilters = std::vector<FilterList::Item>{{5, userPost}};
        f.configure(supplied);
        auto &root = f.compile(u"replaced", false);
        expect(findNodes(root, NodeKind::msup).size() == 1_u);
        expect(attr(root, u"data-latex") == u"x^2");
        expect(f.jax->options.baseURL.value() ==
               u"config;user-pre;config-pre;package-pre;user-post;package-post;");
    };
    "TeX input jax converts only TexError and preserves configured formatter"_test = [] {
        JaxFixture f;
        f.configure();
        auto &root = f.compile(u"\\notACommand");
        expect(f.jax->parseOptions().error) << "TexError marks the expression";
        expect(findNodes(root, NodeKind::merror).size() == 1_u) << "Default formatter returns merror";
        expect(tokenText(root).find(u"Undefined control sequence") != std::u16string::npos)
            << "Default formatter retains the diagnostic text";
        expect(f.jax->parseOptions().parsers.empty()) << "Parser scope unwinds after TexError";
        f.jax->options.formatError = customError;
        auto &custom = f.compile(u"\\notACommand");
        expect(findNodes(custom, NodeKind::merror).empty()) << "Custom formatter replaces merror";
        expect(findNodes(custom, NodeKind::mtext).size() == 1_u) << "Custom formatter creates mtext";
        f.jax->preFilters.add(failPre);
        expect(throws<std::runtime_error>([&] { f.compile(u"x"); }));
        f.jax->preFilters.remove(failPre);
        expect(!findNodes(f.compile(u"x"), NodeKind::mi).empty());
        expect(!f.jax->parseOptions().error);
        expect(tokenText(f.jax->formatError(TexError("Multiline", u"first\nsecond"))) == u"first");
        const std::array<std::pair<std::u16string_view, std::u16string_view>, 6> messages{{
            {u"first\nsecond\nthird", u"first\nthird"},
            {u"first\nsecond\rthird", u"first\rthird"},
            {u"first\nsecond\u2028third", u"first\u2028third"},
            {u"first\nsecond\u2029third", u"first\u2029third"},
            {u"first\rsecond", u"first\rsecond"}, {u"\n", u""}}};
        for (auto const& [message, expected] : messages)
            expect(tokenText(f.jax->formatError(TexError("Multiline", std::u16string(message)))) == expected);
    };
    "TeX input jax applies hsize after filtering and stops filters on false"_test = [] {
        JaxFixture f;
        f.configure();
        auto &root = f.compile(u"\\hsize{5em} x");
        expect(attr(root, u"maxwidth") == u"5em");
        expect(attr(root, u"overflow") == u"linebreak");
        f.jax->postFilters.add(stopPost, -10);
        f.jax->postFilters.add(userPost, -9);
        f.compile(u"x");
        expect(f.jax->options.baseURL.value() == u"stop;");
    };
    "TeX input jax rebinds document ownership and clears per-expression lists"_test = [] {
        JaxFixture f;
        f.configure();
        std::size_t expectedCount{};
        for (std::size_t i = 0; i < 64; ++i) {
            MmlDocument document;
            folia::math::core::AbstractMathItem math(u"x_i+\\frac{1}{2}", f.jax.get());
            auto &root = f.jax->compile(math, document);
            expect(&root.document() == &document);
            expect(&f.jax->parseOptions().nodeFactory.mmlFactory() == &document.factory());
            expect(f.jax->parseOptions().root == &root);
            expect(f.jax->parseOptions().parsers.empty());
            if (i == 0)
                expectedCount = document.size();
            expect(document.size() == expectedCount);
            expect(f.jax->parseOptions().getList(u"msub").size() == 1_u);
        }
        f.jax->setMmlFactory(f.document.factory());
        f.jax->reset(9);
        expect(f.jax->parseOptions().root == nullptr);
        expect(f.jax->parseOptions().nodeLists.empty());
        expect(f.jax->session().counters().equation == 9);
    };
    "configuration runtime add installs original creators defaults tags and filters"_test = [] {
        JaxFixture f;
        f.configure();
        ConfigurationDefinition package;
        package.init = ProcessorSpec<InitMethod>{dynamicInit};
        package.config = ProcessorSpec<ConfigMethod>{dynamicConfig};
        package.options.maxMacros = 99;
        package.nodes.node = dynamicNode;
        package.preprocessors.emplace_back(FilterList::Callback{packagePre}, 2);
        Configuration::create(f.registry, u"test-dynamic", std::move(package));
        TexOptionValues supplied;
        supplied.maxMacros = 123;
        f.jax->configuration().add(u"test-dynamic", *f.jax, supplied);
        expect(f.jax->options.maxMacros.value() == 123_u);
        expect(f.jax->configuration().options.maxMacros.value() == 100_u);
        expect(f.jax->options.baseURL.value() == u"dynamic;");
        auto &root = f.compile(u"x");
        expect(attr(root, u"data-created") == u"dynamic");
        expect(f.jax->options.baseURL.value() == u"dynamic;package-pre;");
        expect(
            throws<std::invalid_argument>([&] { f.jax->configuration().add(u"absent", *f.jax); }));
    };
};
