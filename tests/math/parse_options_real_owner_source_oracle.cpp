#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.core.input_jax_host;
import folia.math.core.find_math_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.input.tex.find_tex_host;
import folia.math.input.tex.node_util;
import folia.math.util.regexp_host;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o = folia::math::options;
namespace t = folia::math::tex;
namespace m = folia::math::mml;

struct ParseOptionsGraph {
  o::Context context;
  OptionFunctionBindings functions{context};
  regexp_host::RegExpBindings regexp{context};
  Unicode16Normalization unicode;
  core::InputJaxModule input{context};
  core::FindMathModule findMath{context};
  t::FindTeXModule findTeX{context, findMath, regexp};
  t::ParserRuntime runtime{context, functions, regexp, unicode, input, findTeX};
  std::shared_ptr<m::MmlFactory> factory;
  ParseOptionsGraph() {
    runtime.initializeParserRuntime();
    (void)t::BasePackage::initializeModule(runtime);
  }
  t::TeXRef makeJax() {
    auto jax = t::TeX::make(runtime);
    factory = std::make_shared<m::MmlFactory>();
    jax->setMmlFactory(factory);
    return jax;
  }
};

m::NodeValue makeNode(const std::shared_ptr<m::MmlFactory>& factory,
                      std::string_view kind) {
  return m::NodeValue(factory->create(kind, tree::FactoryArgs{}));
}

suite parseOptionsRealOwner = [] {
  "parse_options_real_owner_parser_list_mutations_and_tree_filtering"_test = [] {
    ParseOptionsGraph graph;
    auto jax = graph.makeJax();
    auto parsed = jax->parseOptions();
    const auto* owner = std::get_if<t::ParseOptionsRef>(&parsed);
    expect(owner != nullptr && *owner);
    if (!owner || !*owner) return;
    auto options = *owner;

    const auto parserArrayBefore = options->parsers;
    auto parser = t::TexParser::make(u"", PropertyList{}, *options);
    const auto duringConstruction = options->parser();
    const auto* constructionOwner = std::get_if<t::TexParserRef>(&duringConstruction);
    expect(constructionOwner != nullptr && *constructionOwner == parser);
    (void)parser->mml();
    expect(std::holds_alternative<Missing>(options->parser()));
    options->pushParser(parser);
    const auto current = options->parser();
    const auto* currentOwner = std::get_if<t::TexParserRef>(&current);
    expect(currentOwner != nullptr && *currentOwner == parser);
    options->popParser();
    const auto afterPop = options->parser();
    expect(std::holds_alternative<Missing>(afterPop));
    expect(options->parsers == parserArrayBefore);

    const auto factory = graph.factory;
    expect(factory != nullptr);
    if (!factory) return;
    const auto root = makeNode(factory, "mrow");
    const auto live = makeNode(factory, "mi");
    const auto detached = makeNode(factory, "mn");
    root.requireNode()->appendChild(live);
    options->root = root;

    const Property custom(std::string("custom"));
    options->addNode(custom, live);
    options->addNode(custom, detached);
    options->addNode(custom, live);
    expect(bool(property_host::string(t::NodeUtil::instance().getProperty(live, "in-lists")) == u"custom,custom"));
    expect(bool(property_host::string(t::NodeUtil::instance().getProperty(detached, "in-lists")) == u"custom"));
    auto filtered = options->getList(u"custom");
    // getList is the public path through the private inTree helper: the real
    // root and its child survive, while an unattached canonical MML node is pruned.
    expect(filtered->size() == 2u);
    if (filtered->size() == 2u) {
      expect(filtered->get(0) == live);
      expect(filtered->get(1) == live);
    }
    auto oneRemoved = std::make_shared<tree::ChildArray<m::MmlNode>>();
    oneRemoved->push_back(live);
    options->removeFromList(u"custom", oneRemoved);
    expect(options->getList(u"custom")->size() == 1u);
    expect(options->getList(u"custom")->get(0) == live);

    const auto parseIdentity = graph.runtime.parseOptionsIdentities.find(options);
    expect(parseIdentity.has_value());
    if (!parseIdentity) return;
    const auto listsValue = graph.context.get(o::Value(*parseIdentity), o::Key(u"nodeLists"));
    const auto* listsObject = std::get_if<o::ObjectRef>(&listsValue.data);
    expect(listsObject != nullptr);
    if (!listsObject) return;
    const auto liveIdentity = graph.runtime.mmlNodeIdentities.find(live.requireNode());
    const auto detachedIdentity = graph.runtime.mmlNodeIdentities.find(detached.requireNode());
    expect(liveIdentity.has_value() && detachedIdentity.has_value());
    if (!liveIdentity || !detachedIdentity) return;
    const auto replacement = graph.context.array(2);
    graph.context.set(replacement, o::Key(u"0"), o::Value(*detachedIdentity));
    graph.context.set(replacement, o::Key(u"1"), o::Value(*liveIdentity));
    graph.context.set(o::Value(*listsObject), o::Key(u"custom"), o::Value(replacement));
    auto removeDetached = std::make_shared<tree::ChildArray<m::MmlNode>>();
    removeDetached->push_back(detached);
    options->removeFromList(u"custom", removeDetached);
    expect(graph.context.get(o::Value(replacement), o::Key(u"length")) == o::Value(1.0));
    expect(graph.context.get(o::Value(replacement), o::Key(u"0")) == o::Value(*liveIdentity));
  };

  "parse_options_real_owner_clear_retains_persistent_fields_and_writes_before_throw"_test = [] {
    ParseOptionsGraph graph;
    auto jax = graph.makeJax();
    auto parsed = jax->parseOptions();
    const auto* owner = std::get_if<t::ParseOptionsRef>(&parsed);
    expect(owner != nullptr && *owner);
    if (!owner || !*owner) return;
    auto options = *owner;
    auto parser = t::TexParser::make(u"", PropertyList{}, *options);
    (void)parser->mml();
    expect(std::holds_alternative<Missing>(options->parser()));
    options->pushParser(parser);

    const auto parseIdentity = graph.runtime.parseOptionsIdentities.find(options);
    expect(parseIdentity.has_value());
    if (!parseIdentity) return;
    const auto parseValue = o::Value(*parseIdentity);
    const auto optionsValue = graph.context.get(parseValue, o::Key(u"options"));
    const auto packageDataValue = graph.context.get(parseValue, o::Key(u"packageData"));
    const auto nodeFactoryValue = graph.context.get(parseValue, o::Key(u"nodeFactory"));
    const auto handlersValue = graph.context.get(parseValue, o::Key(u"handlers"));
    const auto tagsValue = graph.context.get(parseValue, o::Key(u"tags"));
    graph.context.set(optionsValue, o::Key(u"reviewSentinel"), o::Value(17.0));
    options->error = true;
    options->root = makeNode(graph.factory, "mrow");
    const auto oldParsers = std::get<o::ObjectRef>(options->parsers.data);
    options->clear();
    expect(std::get<o::ObjectRef>(options->parsers.data) != oldParsers);
    expect(graph.context.get(options->parsers, o::Key(u"length")) == o::Value(0.0));
    expect(options->root.kind() == tree::ChildKind::Null);
    expect(!options->error);
    expect(graph.context.get(optionsValue, o::Key(u"reviewSentinel")) == o::Value(17.0));
    expect(graph.context.get(parseValue, o::Key(u"options")) == optionsValue);
    expect(graph.context.get(parseValue, o::Key(u"packageData")) == packageDataValue);
    expect(graph.context.get(parseValue, o::Key(u"nodeFactory")) == nodeFactoryValue);
    expect(graph.context.get(parseValue, o::Key(u"handlers")) == handlersValue);
    expect(graph.context.get(parseValue, o::Key(u"tags")) == tagsValue);

    const auto* tags = std::get_if<o::ObjectRef>(&tagsValue.data);
    expect(tags != nullptr);
    if (!tags) return;
    int resetGetterReads = 0;
    const auto throwingResetGetter = graph.context.function(u"get resetTag",
        [&resetGetterReads](o::Context&, o::Value, std::span<const o::Value>) -> o::Value {
          ++resetGetterReads;
          throw o::Exception("WitnessError", u"reset-witness");
        });
    graph.context.define(*tags, o::Key(u"resetTag"),
        o::Descriptor::access(throwingResetGetter, std::nullopt));
    auto throwingParser = t::TexParser::make(u"", PropertyList{}, *options);
    (void)throwingParser->mml();
    expect(std::holds_alternative<Missing>(options->parser()));
    options->pushParser(throwingParser);
    options->root = makeNode(graph.factory, "mrow");
    options->error = true;
    bool witnessError = false;
    try { options->clear(); }
    catch (const o::Exception& error) {
      witnessError = error.name == "WitnessError" && property_host::debugText(error.message) == "reset-witness";
    }
    expect(witnessError);
    expect(resetGetterReads == 1);
    expect(std::holds_alternative<Missing>(options->parser()));
    expect(options->root.kind() == tree::ChildKind::Null);
    expect(options->error == false);
  };
};
}
