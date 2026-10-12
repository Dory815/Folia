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
    if (!children) host::typeError("Cannot read properties of null (reading '0')");
    const NodeValue child = children->get(index);
    return child.requireNode(operation);
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
    // These are the source field-initializer values. Establish them before
    // factory creation or virtual child hooks can observe this object.
    prevClass_ = PropertyNull{};
    prevLevel_ = PropertyNull{};
    texclass_ = PropertyNull{};

    Property inputAttributes = Missing{};
    if (!args.empty() && args[0].has_value()) {
        if (const auto* value = std::any_cast<Property>(&args[0])) {
            inputAttributes = *value;
        } else if (const auto* value = std::any_cast<PropertyList>(&args[0])) {
            inputAttributes = *value;
        } else if (std::any_cast<std::nullptr_t>(&args[0])) {
            inputAttributes = PropertyNull{};
        } else if (std::any_cast<PropertyNull>(&args[0])) {
            inputAttributes = PropertyNull{};
        } else {
            host::typeError("Unsupported native MML constructor properties argument");
        }
    }

    Children inputChildren = std::make_shared<tree::ChildArray<MmlNode>>();
    if (args.size() > 1 && args[1].has_value()) {
        if (const auto* value = std::any_cast<Children>(&args[1])) {
            inputChildren = *value;
        } else if (const auto* value = std::any_cast<Property>(&args[1])) {
            if (value->missing()) {
                inputChildren = std::make_shared<tree::ChildArray<MmlNode>>();
            } else if (host::nullish(*value)) {
                inputChildren.reset();
            } else {
                host::typeError("Unsupported native MML constructor children argument");
            }
        } else if (std::any_cast<std::nullptr_t>(&args[1]) ||
                   std::any_cast<PropertyNull>(&args[1])) {
            inputChildren.reset();
        } else {
            host::typeError("Unsupported native MML constructor children argument");
        }
    }

    if (arity() < 0) {
        MmlNode* const inferred = factory.create("inferredMrow", {});
        Children inferredChildren = std::make_shared<tree::ChildArray<MmlNode>>();
        inferredChildren->push_back(NodeValue(inferred));
        this->childNodes = std::move(inferredChildren);
        const NodeValue inferredChild = this->childNodes->get(0);
        inferredChild.requireNodeForWrite("parent")->setParent(
            NodeValue(static_cast<MmlNode*>(this)));
    }
    this->setChildren(std::move(inputChildren));

    const auto nodeClass = factory.getNodeClass(this->kind());
    PropertyList nodeDefaults = class_info_defaults(nodeClass, false);
    const auto mathClass = factory.getNodeClass("math");
    PropertyList mathDefaults = class_info_defaults(mathClass, false);
    auto newAttributes = std::make_shared<Attributes>(nodeDefaults, mathDefaults);
    this->setAttributes(std::move(newAttributes));

    // The CJS Object.assign used by Attributes.setList ignores nullish input,
    // while copying enumerable scalar/object properties for supported values.
    const PropertyList assignedAttributes = host::objectAssign(inputAttributes);
    const auto currentAttributes = this->attributes();
    if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'setList')");
    currentAttributes->setList(assignedAttributes);
}

PropertyList AbstractMmlNode::constructorDefaults()const {
    if(!constructorInfo_)host::typeError("Missing native constructor-class binding");
    auto defaults=constructorInfo_->getDefaults();
    if(!defaults)host::typeError("Constructor class has no defaults");
    return *defaults;
}

MmlNode* AbstractMmlNode::copy(bool keepIds) {
    tree::TreeCallGuard<MmlNode> guard;

    MmlNode* const node = factory.create(this->kind(), {});
    if (!node) host::typeError("Cannot set properties of null (setting 'properties')");
    auto* const normalNode = dynamic_cast<AbstractMmlNode*>(node);
    if (!normalNode) {
        host::typeError("MML copy factory product does not implement normal MML property storage");
    }

    // Replace the store handle, rather than assigning into a factory-owned
    // record that constructor observers may already have retained.
    PropertyList copiedProperties;
    copiedProperties.assign(this->properties);
    normalNode->properties = std::move(copiedProperties);

    if (this->attributes()) {
        const auto sourceAttributes = this->attributes();
        if (!sourceAttributes) {
            host::typeError("Cannot read properties of null (reading 'getAllAttributes')");
        }
        const PropertyList attributes = sourceAttributes->getAllAttributes();
        const std::vector<std::string> names = attributes.keys();
        for (const auto& name : names) {
            if (name != "id" || keepIds) {
                const auto destinationAttributes = normalNode->attributes();
                if (!destinationAttributes) {
                    host::typeError("Cannot read properties of null (reading 'set')");
                }
                const Property value = attributes.get(name);
                destinationAttributes->set(name, value);
            }
        }
    }

    // Preserve the source's condition reads before capturing its iteration
    // handle: a factory/attribute callback may have changed childNodes.
    if (this->childNodes && this->childNodes->size()) {
        Children children = this->childNodes;
        if (!children) host::typeError("Cannot read properties of null (reading 'length')");
        if (children->size() == 1) {
            const NodeValue firstForInference = children->get(0);
            if (firstForInference.requireNode("isInferred")->isInferred()) {
                const NodeValue firstForChildren = children->get(0);
                children = firstForChildren.requireNode("childNodes")->childNodes;
            }
        }
        if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");

        for (std::size_t index = 0; index < children->size(); ++index) {
            const NodeValue child = children->get(index);
            if (child.isNode()) {
                MmlNode* const copiedChild = child.requireNode("copy")->copy();
                normalNode->appendChild(NodeValue(copiedChild));
            } else {
                const Children destination = normalNode->childNodes;
                if (!destination) {
                    host::typeError("Cannot read properties of null (reading 'push')");
                }
                destination->push_back(NodeValue(PropertyNull{}));
            }
        }
    }
    return node;
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
    tree::TreeCallGuard<MmlNode> guard;
    const auto children = this->childNodes;
    if (!children) throw Error("TypeError",u"object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t i = 0; i < children->size(); ++i) {
        const NodeValue child = children->get(i);
        if (MmlNode* const node = child.nodeOrNull(); node && !node->isEmpty()) {
            return false;
        }
    }
    return true;
}

double AbstractMmlNode::arity() const { return std::numeric_limits<double>::infinity(); }
bool AbstractMmlNode::isInferred() const { return false; }
Property AbstractMmlNode::notParent() const { return false; }

NodeValue AbstractMmlNode::Parent() const {
    tree::TreeCallGuard<MmlNode> guard;
    NodeValue parent = getParent();
    while (auto* node = parent.nodeOrNull()) {
        if (!host::truthy(node->notParent())) break;
        parent = node->Parent();
    }
    return parent;
}

void AbstractMmlNode::setChildren(Children children) {
    if (arity() < 0) {
        child_at(this->childNodes, 0, "setChildren")->setChildren(std::move(children));
        return;
    }
    Generic::setChildren(std::move(children));
}

NodeValue AbstractMmlNode::appendChild(NodeValue child) {
    if (arity() < 0) {
        child_at(this->childNodes, 0, "appendChild")->appendChild(child);
        return child;
    }

    MmlNode* const input = child.requireNode("isInferred");
    if (input->isInferred()) {
        if (arity() == std::numeric_limits<double>::infinity()) {
            const Children originalChildren = input->childNodes;
            if (!originalChildren) {
                host::typeError("Cannot read properties of null (reading 'forEach')");
            }
            originalChildren->forEach([&](NodeValue node, std::size_t, tree::ChildArray<MmlNode>&) {
                Generic::appendChild(node);
            });
            return child;
        }

        MmlNode* const original = input;
        MmlNode* const replacement = factory.create("mrow", {});
        if (!replacement) host::typeError("Cannot read properties of null (reading 'setChildren')");
        replacement->setChildren(original->childNodes);
        replacement->setAttributes(original->attributes());
        for (const auto& name : original->getPropertyNames()) {
            replacement->setProperty(name, original->getProperty(name));
        }
        return Generic::appendChild(NodeValue(replacement));
    }
    return Generic::appendChild(child);
}

NodeValue AbstractMmlNode::replaceChild(NodeValue newChild, NodeValue oldChild) {
    tree::TreeCallGuard<MmlNode> guard;
    const double nodeArity = arity();
    if (nodeArity < 0) {
        const Children children = this->childNodes;
        if (!children) host::typeError("Cannot read properties of null (reading '0')");
        const NodeValue first = children->get(0);
        first.requireNode("replaceChild")->replaceChild(newChild, oldChild);
        return newChild;
    }
    return Generic::replaceChild(newChild, oldChild);
}

NodeValue AbstractMmlNode::core() { return this; }
NodeValue AbstractMmlNode::coreMO() { return this; }
std::size_t AbstractMmlNode::coreIndex() { return 0; }

std::optional<std::size_t> AbstractMmlNode::childPosition() {
    tree::TreeCallGuard<MmlNode> guard;
    NodeValue child;
    NodeValue parent = getParent();
    while (auto* node = parent.nodeOrNull()) {
        if (!host::truthy(node->notParent())) break;
        child = parent;
        parent = node->getParent();
    }
    if (!child.isNode()) child = NodeValue(this);
    if (auto* node = parent.nodeOrNull()) {
        const auto children = node->childNodes;
        if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
        for (std::size_t index = 0; index < children->size(); ++index) {
            if (children->get(index) == child) return index;
        }
    }
    return {};
}

NodeValue AbstractMmlNode::setTeXclass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    return !host::nullish(texClass()) ? NodeValue(this) : previous;
}

void AbstractMmlNode::updateTeXclass(NodeValue coreNode) {
    tree::TreeCallGuard<MmlNode> guard;
    auto* core = coreNode.nodeOrNull();
    if (!core) return;
    setPrevClass(core->prevClass());
    setPrevLevel(core->prevLevel());
    core->setPrevLevel(PropertyNull{});
    core->setPrevClass(PropertyNull{});
    setTexClass(core->texClass());
}

void AbstractMmlNode::getPrevClass(NodeValue previous) {
    tree::TreeCallGuard<MmlNode> guard;
    auto* node = previous.nodeOrNull();
    if (!node) return;
    setPrevClass(node->texClass());
    auto previousAttributes = node->attributes();
    if (!previousAttributes) host::typeError("Cannot read properties of null (reading 'get')");
    setPrevLevel(previousAttributes->get("scriptlevel"));
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
    tree::TreeCallGuard<MmlNode> guard;
    host::inheritanceDefaults(inherited, display, level, prime);

    const auto defaultsAttributes = this->attributes();
    if (!defaultsAttributes) host::typeError("Cannot read properties of null (reading 'getAllDefaults')");
    const PropertyList defaults = defaultsAttributes->getAllDefaults();
    const std::vector<std::string> keys = host::ownKeys(inherited);
    for (const auto& key : keys) {
        if (defaults.hasOwn(key) || AbstractMmlNode::alwaysInherit().hasOwn(key)) {
            const Property tuple = host::get(inherited, key);
            const auto [origin, value] = unpackAttribute(tuple);
            if (!forbidden_inheritance(origin, *this, key)) {
                const auto currentAttributes = this->attributes();
                if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'setInherited')");
                currentAttributes->setInherited(key, value);
            }
        }
        if (stopped_inheritance(this->kind(), key)) {
            PropertyList stopped = host::objectAssign(inherited);
            stopped.erase(key);
            inherited = Property(std::move(stopped));
        }
    }

    const auto displayAttributes = this->attributes();
    if (!displayAttributes) host::typeError("Cannot read properties of null (reading 'getExplicit')");
    const Property displaystyle = displayAttributes->getExplicit("displaystyle");
    if (displaystyle.missing()) {
        const auto currentAttributes = this->attributes();
        if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'setInherited')");
        currentAttributes->setInherited("displaystyle", display);
    }

    const auto levelAttributes = this->attributes();
    if (!levelAttributes) host::typeError("Cannot read properties of null (reading 'getExplicit')");
    const Property scriptlevel = levelAttributes->getExplicit("scriptlevel");
    if (scriptlevel.missing()) {
        const auto currentAttributes = this->attributes();
        if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'setInherited')");
        currentAttributes->setInherited("scriptlevel", level);
    }

    if (host::truthy(prime)) this->setProperty("texprimestyle", prime);

    const double expectedArity = this->arity();
    if (expectedArity >= 0 && expectedArity != std::numeric_limits<double>::infinity()) {
        const auto children = this->childNodes;
        if (!children) host::typeError("Cannot read properties of null (reading 'length')");
        const double childLength = static_cast<double>(children->size());
        if ((expectedArity == 1 && childLength == 0) ||
            (expectedArity != 1 && childLength != expectedArity)) {
            if (expectedArity < childLength) {
                this->childNodes = children->slice(0, static_cast<std::int64_t>(expectedArity));
            } else {
                while (true) {
                    const auto currentChildren = this->childNodes;
                    if (!currentChildren) host::typeError("Cannot read properties of null (reading 'length')");
                    if (!(static_cast<double>(currentChildren->size()) < expectedArity)) break;
                    MmlNode* const row = this->factory.create("mrow", {});
                    this->appendChild(NodeValue(row));
                }
            }
        }
    }

    if (this->linebreakContainer() && !this->isEmbellished()) {
        const std::string align = this->linebreakAlign();
        if (!align.empty()) {
            const auto currentAttributes = this->attributes();
            if (!currentAttributes) host::typeError("Cannot read properties of null (reading 'get')");
            Property indentalign = currentAttributes->get(align);
            if (!host::truthy(indentalign)) indentalign = "left";
            PropertyList additions{
                {"indentalign", indentalign},
                {"indentshift", "0"},
                {"indentalignfirst", indentalign},
                {"indentshiftfirst", "0"},
                {"indentalignlast", "indentalign"},
                {"indentshiftlast", "indentshift"}
            };
            inherited = this->addInheritedAttributes(std::move(inherited), Property(additions));
        }
    }

    this->setChildInheritedAttributes(std::move(inherited), display, level, prime);
}

void AbstractMmlNode::setChildInheritedAttributes(AttributeList inherited, Property display,
                                                   Property level, Property prime) {
    const Children children = this->childNodes;
    if (!children) host::typeError("object null is not iterable (cannot read property Symbol(Symbol.iterator))");
    for (std::size_t index = 0; index < children->size(); ++index) {
        children->get(index).requireNode("setInheritedAttributes")
            ->setInheritedAttributes(inherited, display, level, prime);
    }
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
    tree::TreeCallGuard<MmlNode> guard;

    MmlNode* const source = node.requireNode("attributes");
    const auto attributes = source->attributes();
    if (!attributes) host::typeError("Cannot read properties of null (reading 'get')");

    const Property display = attributes->get("displaystyle");
    const Property level = attributes->get("scriptlevel");

    PropertyList defaults;
    if (attributes->isSet("mathsize")) {
        const Property mathsize = attributes->get("mathsize");
        defaults.set("mathsize", attributeTuple("math", mathsize));
    }

    Property prime = source->getProperty("texprimestyle");
    if (!host::truthy(prime)) prime = false;

    setInheritedAttributes(Property(std::move(defaults)), display, level, prime);
}

void AbstractMmlNode::verifyTree(Property options) {
    // P23f Root stub: AbstractMmlNode::verifyTree.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

void AbstractMmlNode::verifyAttributes(Property options) {
    if (host::truthy(host::get(options,"checkAttributes"))) {
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
    if (host::truthy(host::get(options,"checkMathvariants"))) {
        auto attrs = attributes();
        if (!attrs) host::typeError("Cannot verify null MML attributes");
        const Property variant = attrs->getExplicit("mathvariant");
        if (host::truthy(variant) && !property_set_contains(variant) &&
            !host::truthy(getProperty("ignore-variant"))) {
            mError("Invalid mathvariant: " + host::debugText(host::string(variant)), options, true);
        }
    }
}

void AbstractMmlNode::verifyChildren(Property options) {
    // P23f Root stub: AbstractMmlNode::verifyChildren.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}

MmlNode* AbstractMmlNode::mError(std::string message, Property options, bool shortError) {
    // P23f Root stub: AbstractMmlNode::mError.
    throw Error("Unimplemented",u"Root-frozen tagged MML operation");
}
} // namespace folia::math::mml
