// Candidate native observations for the pinned TextParser source methods.
// This file is deliberately outside the automatic test glob. The caller must
// supply the actual loaded ParserRuntime and actual configured ParseOptions.
// No Context MML identity bridge or synthetic package record is substituted.
module folia.math.input.tex.parser;
import :text_parser;
import :parse_options;
import :runtime;
import folia.math.util.property_host;
import std;

namespace folia::math::tex::candidate {
namespace {
using namespace options;

mml::NodeValue node(TexParser& parser, std::u16string_view kind,
                    mml::Children children = std::make_shared<tree::ChildArray<mml::MmlNode>>(),
                    Property definition = PropertyList{}) {
    const std::array<std::any, 3> arguments{
        std::any(Property(property_host::debugText(kind))), std::any(std::move(children)),
        std::any(std::move(definition))};
    return parser.create(u"node", arguments);
}

std::string nodeKind(const mml::NodeValue& value) {
    return value.requireNode("kind")->kind();
}
}

// The package loader calls TextParser::initializeModule once during module
// setup. This candidate exercises closed typed methods against the real
// ParseOptions, NodeFactory, MmlFactory, ParseUtil, NodeUtil, and resulting MML
// nodes; it does not claim Context-field or package callback acceptance.
std::vector<std::string> runTextParserTypedCandidates(ParserRuntime& runtime,
                                                       ParseOptions& options) {
    if (std::holds_alternative<Undefined>(runtime.textParserClass.data))
        throw Exception("Unimplemented", u"The real TextParser module hook must run before native candidates");

    auto parser = TextParser::make(runtime, u"", PropertyList{{"mathcolor", "red"},
        {"mathsize", "2em"}, {"mathvariant", "bold"}}, options);

    // saveText delegates to the actual ParseUtil.internalText and the actual
    // Stack/NodeFactory ownership. Its Push path then applies token attributes.
    parser->text() = u"sample";
    parser->saveText();
    const mml::Children saved = parser->nodes();
    if (!saved || saved->size() != 1)
        throw Exception("TypeError", u"TextParser.saveText did not push one actual MML node");
    const mml::NodeValue textNode = saved->get(0);
    const auto textAttributes = textNode.requireNode("attributes")->attributes();

    // Exercise addAttributes on a genuine node from the same MmlFactory.
    const mml::NodeValue token = node(*parser, u"mi");
    parser->addAttributes(token);
    const auto tokenAttributes = token.requireNode("attributes")->attributes();

    // Exercise all PushMath wrapper branches using actual MML nodes.
    const mml::NodeValue row = node(*parser, u"mrow");
    parser->PushMath(row);
    const mml::Children pushed = parser->nodes();
    if (!pushed || pushed->size() != 2)
        throw Exception("TypeError", u"TextParser.PushMath did not append one actual MML node");

    std::vector<std::string> result;
    result.push_back("saveText:" + nodeKind(textNode) + ":mathcolor=" +
        property_host::debugText(property_host::string(textAttributes->get("mathcolor"))) +
        ":mathsize=" + property_host::debugText(property_host::string(textAttributes->get("mathsize"))));
    result.push_back("addAttributes:" + nodeKind(token) + ":mathvariant=" +
        property_host::debugText(property_host::string(tokenAttributes->get("mathvariant"))));
    result.push_back("PushMath:" + nodeKind(pushed->get(1)));
    return result;
}
}
