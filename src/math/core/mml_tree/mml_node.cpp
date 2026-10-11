/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of the normal MML node behavior from
 * .reference/MathJax-src/ts/core/MmlTree/MmlNode.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 * TS SHA256: cc0594eb5782928412c4307637866d9f80ac147af577297edd48ee88b63b8656
 * Pinned CJS SHA256: 86c5eb572dcf2652c3c52a60b66c720d231e17f600cc9aeef3ace4aeafc81d45
 *************************************************************/
module folia.math.core.mml_tree.mml_node;
import folia.math.core.mml_tree.mml_node_host;
import std;

namespace folia::math::mml {
namespace {
const PropertyArray TEXSPACELENGTH = {
    "", "thinmathspace", "mediummathspace", "thickmathspace"
};
const PropertyArray TEXSPACE = {
    PropertyArray{ 0,-1, 2, 3, 0, 0, 0, 1},
    PropertyArray{-1,-1, 0, 3, 0, 0, 0, 1},
    PropertyArray{ 2, 2, 0, 0, 2, 0, 0, 2},
    PropertyArray{ 3, 3, 0, 0, 3, 0, 0, 3},
    PropertyArray{ 0, 0, 0, 0, 0, 0, 0, 0},
    PropertyArray{ 0,-1, 2, 3, 0, 0, 0, 1},
    PropertyArray{ 1, 1, 0, 1, 1, 1, 1, 1},
    PropertyArray{ 1,-1, 2, 3, 1, 0, 1, 1}
};

PropertyList class_info_defaults(const tree::FactoryClassRef<MmlNode>& nodeClass,
                                 bool required) {
    if (!nodeClass) host::typeError("Missing registered MML node class");
    const auto* info = std::any_cast<std::shared_ptr<MmlClassInfo>>(&nodeClass->staticData);
    if (!info || !*info) host::typeError("MML class has no native static metadata");
    const auto defaults=(*info)->getDefaults();
    if (!defaults) {
        if (required) host::typeError("Math node class has no defaults");
        return {};
    }
    return *defaults;
}

MmlNode* child_at(const Children& children, std::size_t index, std::string_view operation) {
    // P23f Root stub: child_at.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

bool property_set_contains(const Property& value) {
    return std::any_of(MATHVARIANTS.begin(), MATHVARIANTS.end(),
        [&](const Property& candidate) { return host::sameValueZero(candidate, value); });
}

bool forbidden_inheritance(const Property& origin, MmlNode& node,
                           std::string_view name) {
    const auto from = host::get(AbstractMmlNode::noInherit(),host::debugText(host::string(origin)));
    if (host::nullish(from)) return false;
    const auto to = host::get(from,node.kind());
    return !host::nullish(to) && host::truthy(host::get(to,name));
}

bool stopped_inheritance(std::string_view destination, std::string_view name) {
    const auto to = host::get(AbstractMmlNode::stopInherit(),destination);
    return !host::nullish(to) && host::truthy(host::get(to,name));
}
}

std::vector<Property> MATHVARIANTS = {
    "normal", "bold", "italic", "bold-italic", "double-struck", "fraktur",
    "bold-fraktur", "script", "bold-script", "sans-serif", "bold-sans-serif",
    "sans-serif-italic", "sans-serif-bold-italic", "monospace", "inital",
    "tailed", "looped", "stretched"
};

std::vector<std::string> indentAttributes = {
    "indentalign", "indentalignfirst", "indentshift", "indentshiftfirst"
};

PropertyList& AbstractMmlNode::noInherit() {
    static PropertyList value{
        {"mstyle", PropertyList{
            {"mpadded", PropertyList{{"width", true}, {"height", true}, {"depth", true},
                                      {"lspace", true}, {"voffset", true}}},
            {"mtable", PropertyList{{"width", true}, {"height", true}, {"depth", true}, {"align", true}}}
        }},
        {"maligngroup", PropertyList{
            {"mrow", PropertyList{{"groupalign", true}}},
            {"mtable", PropertyList{{"groupalign", true}}}
        }},
        {"mtr", PropertyList{
            {"msqrt", PropertyList{{"data-vertical-align", true}}},
            {"mroot", PropertyList{{"data-vertical-align", true}}}
        }},
        {"mlabeledtr", PropertyList{
            {"msqrt", PropertyList{{"data-vertical-align", true}}},
            {"mroot", PropertyList{{"data-vertical-align", true}}}
        }}
    };
    return value;
}

PropertyList& AbstractMmlNode::stopInherit() {
    static PropertyList value{{"mtd", PropertyList{
        {"columnalign", true}, {"rowalign", true}, {"groupalign", true}
    }}};
    return value;
}

PropertyList& AbstractMmlNode::alwaysInherit() {
    static PropertyList value{{"scriptminsize", true}, {"scriptsizemultiplier", true},
                              {"infixlinebreakstyle", true}};
    return value;
}

PropertyList& AbstractMmlNode::verifyDefaults() {
    static PropertyList value{{"checkArity", true}, {"checkAttributes", false},
                              {"checkMathvariants", true}, {"fullErrors", false},
                              {"fixMmultiscripts", true}, {"fixMtables", true}};
    return value;
}

void AbstractMmlNode::initializeMml(tree::FactoryArgs args) {
    // P23f Root stub: AbstractMmlNode::initializeMml.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

PropertyList AbstractMmlNode::constructorDefaults()const {
    if(!constructorInfo_)host::typeError("Missing native constructor-class binding");
    auto defaults=constructorInfo_->getDefaults();
    if(!defaults)host::typeError("Constructor class has no defaults");
    return *defaults;
}

MmlNode* AbstractMmlNode::copy(bool keepIds) {
    // P23f Root stub: AbstractMmlNode::copy.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

Property AbstractMmlNode::texClass() { return texclass_; }
void AbstractMmlNode::setTexClass(Property value) { texclass_ = std::move(value); }
Property AbstractMmlNode::prevClass() { return prevClass_; }
void AbstractMmlNode::setPrevClass(Property value) { prevClass_ = std::move(value); }
Property AbstractMmlNode::prevLevel() const { return prevLevel_; }
void AbstractMmlNode::setPrevLevel(Property value) { prevLevel_ = std::move(value); }
std::shared_ptr<Attributes> AbstractMmlNode::attributes() const { return attributes_; }
void AbstractMmlNode::setAttributes(std::shared_ptr<Attributes> value) { attributes_ = std::move(value); }

bool AbstractMmlNode::isToken() const { return false; }
bool AbstractMmlNode::isEmbellished() const { return false; }
bool AbstractMmlNode::isSpacelike() const { return false; }
bool AbstractMmlNode::linebreakContainer() const { return false; }
std::string AbstractMmlNode::linebreakAlign() const { return "data-align"; }

bool AbstractMmlNode::isEmpty() const {
    // P23f Root stub: AbstractMmlNode::isEmpty.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

double AbstractMmlNode::arity() const { return std::numeric_limits<double>::infinity(); }
bool AbstractMmlNode::isInferred() const { return false; }
Property AbstractMmlNode::notParent() const { return false; }

NodeValue AbstractMmlNode::Parent() const {
    // P23f Root stub: AbstractMmlNode::Parent.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void AbstractMmlNode::setChildren(Children children) {
    // P23f Root stub: AbstractMmlNode::setChildren.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

NodeValue AbstractMmlNode::appendChild(NodeValue child) {
    // P23f Root stub: AbstractMmlNode::appendChild.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

NodeValue AbstractMmlNode::replaceChild(NodeValue newChild, NodeValue oldChild) {
    // P23f Root stub: AbstractMmlNode::replaceChild.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

NodeValue AbstractMmlNode::core() { return this; }
NodeValue AbstractMmlNode::coreMO() { return this; }
std::size_t AbstractMmlNode::coreIndex() { return 0; }

std::optional<std::size_t> AbstractMmlNode::childPosition() {
    // P23f Root stub: AbstractMmlNode::childPosition.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

NodeValue AbstractMmlNode::setTeXclass(NodeValue previous) {
    // P23f Root stub: AbstractMmlNode::setTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void AbstractMmlNode::updateTeXclass(NodeValue coreNode) {
    // P23f Root stub: AbstractMmlNode::updateTeXclass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void AbstractMmlNode::getPrevClass(NodeValue previous) {
    // P23f Root stub: AbstractMmlNode::getPrevClass.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

Property AbstractMmlNode::texSpacing() {
    const Property firstPrevious = prevClass();
    const Property previous = !host::nullish(firstPrevious) ? prevClass() : Property(TEXCLASS.NONE);
    const Property rawCurrent = texClass();
    const Property current = host::truthy(rawCurrent) ? rawCurrent : Property(TEXCLASS.ORD);
    if (previous == Property(TEXCLASS.NONE) || current == Property(TEXCLASS.NONE)) return std::string{};
    const auto rowKey = host::debugText(host::string(previous));
    const Property row = host::get(TEXSPACE, rowKey);
    const auto columnKey = host::debugText(host::string(current));
    const Property space = host::get(row, columnKey);
    const bool previousScript = host::greaterThanZero(prevLevel());
    bool currentScript = false;
    if (!previousScript) {
        auto attrs = attributes();
        if (!attrs) host::typeError("Cannot read scriptlevel from null attributes");
        currentScript = host::greaterThanZero(attrs->get("scriptlevel"));
    }
    if ((previousScript || currentScript) && host::number(space) >= 0) return std::string{};
    const double magnitude = std::abs(host::number(space));
    const auto lengthKey = host::debugText(host::string(magnitude));
    return host::get(TEXSPACELENGTH, lengthKey);
}

bool AbstractMmlNode::hasSpacingAttributes() {
    // P23f Root stub: AbstractMmlNode::hasSpacingAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void AbstractMmlNode::inheritAttributes(AttributeList inherited, Property display,
                                              Property level, Property prime) {
    // P23f Root stub: AbstractMmlNode::inheritAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void AbstractMmlNode::setChildInheritedAttributes(AttributeList inherited, Property display,
                                                   Property level, Property prime) {
    // P23f Root stub: AbstractMmlNode::setChildInheritedAttributes.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

AttributeList AbstractMmlNode::addInheritedAttributes(AttributeList current, Property additions) {
    auto updated = host::objectAssign(current);
    for (const auto& name : host::ownKeys(additions)) {
        if (name != "displaystyle" && name != "scriptlevel" && name != "style")
            updated.set(name, attributeTuple(kind(), host::get(additions,name)));
    }
    return updated;
}

void AbstractMmlNode::inheritAttributesFrom(NodeValue node) {
    // P23f Root stub: AbstractMmlNode::inheritAttributesFrom.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void AbstractMmlNode::verifyTree(std::optional<PropertyList> options) {
    // P23f Root stub: AbstractMmlNode::verifyTree.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void AbstractMmlNode::verifyAttributes(PropertyList options) {
    if (host::truthy(options.get("checkAttributes"))) {
        auto attrs = attributes();
        if (!attrs) host::typeError("Cannot verify null MML attributes");
        std::vector<std::string> bad;
        for (const auto& name : attrs->getExplicitNames()) {
            const bool data = name.size() >= 5 && name.substr(0, 5) == "data-";
            const bool allowed = name == "class" || name == "style" || name == "id" ||
                                 name == "href" || name == "xlink:href";
            if (!data && attrs->getDefault(name).missing() && !allowed) bad.push_back(name);
        }
        if (!bad.empty()) {
            std::string joined;
            for (std::size_t i = 0; i < bad.size(); ++i) {
                if (i) joined += ", ";
                joined += bad[i];
            }
            mError("Unknown attributes for " + kind() + " node: " + joined, options, false);
        }
    }
    if (host::truthy(options.get("checkMathvariants"))) {
        auto attrs = attributes();
        if (!attrs) host::typeError("Cannot verify null MML attributes");
        const Property variant = attrs->getExplicit("mathvariant");
        if (host::truthy(variant) && !property_set_contains(variant) &&
            !host::truthy(getProperty("ignore-variant"))) {
            mError("Invalid mathvariant: " + host::debugText(host::string(variant)), options, true);
        }
    }
}

void AbstractMmlNode::verifyChildren(PropertyList options) {
    // P23f Root stub: AbstractMmlNode::verifyChildren.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

MmlNode* AbstractMmlNode::mError(std::string message, ErrorOptions options, bool shortError) {
    // P23f Root stub: AbstractMmlNode::mError.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}
} // namespace folia::math::mml
