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
constexpr std::array<std::string_view, 4> TEXSPACELENGTH = {
    "", "thinmathspace", "mediummathspace", "thickmathspace"
};
constexpr std::array<std::array<int, 8>, 8> TEXSPACE = {{
    {{ 0,-1, 2, 3, 0, 0, 0, 1}},
    {{-1,-1, 0, 3, 0, 0, 0, 1}},
    {{ 2, 2, 0, 0, 2, 0, 0, 2}},
    {{ 3, 3, 0, 0, 3, 0, 0, 3}},
    {{ 0, 0, 0, 0, 0, 0, 0, 0}},
    {{ 0,-1, 2, 3, 0, 0, 0, 1}},
    {{ 1, 1, 0, 1, 1, 1, 1, 1}},
    {{ 1,-1, 2, 3, 1, 0, 1, 1}}
}};

PropertyList class_info_defaults(const tree::FactoryClassRef<MmlNode>& nodeClass,
                                 bool required) {
    if (!nodeClass) host::typeError("Missing registered MML node class");
    const auto* info = std::any_cast<std::shared_ptr<MmlClassInfo>>(&nodeClass->staticData);
    if (!info || !*info) host::typeError("MML class has no native static metadata");
    if (!(*info)->defaults) {
        if (required) host::typeError("Math node class has no defaults");
        return {};
    }
    return *(*info)->defaults;
}

MmlNode* child_at(const Children& children, std::size_t index, std::string_view operation) {
    if (!children || index >= children->size()) host::typeError(operation);
    auto* child = (*children)[index];
    if (!child) host::typeError(operation);
    return child;
}

std::optional<std::size_t> class_index(double value, std::size_t count) {
    if (!std::isfinite(value) || value < 0 || value >= static_cast<double>(count) ||
        std::trunc(value) != value) return std::nullopt;
    return static_cast<std::size_t>(value);
}

bool property_set_contains(const Property& value) {
    return std::any_of(MATHVARIANTS.begin(), MATHVARIANTS.end(),
        [&](const Property& candidate) { return host::sameValueZero(candidate, value); });
}

bool forbidden_inheritance(std::string_view origin, std::string_view destination,
                           std::string_view name) {
    auto from = PropertyList::object(AbstractMmlNode::noInherit().get(origin));
    if (!from) return false;
    auto to = PropertyList::object(from->get(destination));
    return to && host::truthy(to->get(name));
}

bool stopped_inheritance(std::string_view destination, std::string_view name) {
    auto to = PropertyList::object(AbstractMmlNode::stopInherit().get(destination));
    return to && host::truthy(to->get(name));
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
    tree::TreeCallGuard<MmlNode> guard;
    Generic::initializeNode({});

    Children children = std::make_shared<std::vector<MmlNode*>>();
    if (args.size() > 1 && args[1].has_value()) {
        if (const auto* supplied = std::any_cast<Children>(&args[1])) children = *supplied;
        else if (std::any_cast<std::nullptr_t>(&args[1])) children.reset();
        else host::typeError("MML children argument has no child-array binding");
    }

    if (arity() < 0) {
        auto* inferred = this->factory.create("inferredMrow", {});
        this->childNodes = std::make_shared<std::vector<MmlNode*>>();
        this->childNodes->push_back(inferred);
        if (!inferred) host::typeError("Cannot set parent on null inferred row");
        inferred->parent = this;
    }
    setChildren(std::move(children));

    const auto ownDefaults = class_info_defaults(this->factory.getNodeClass(kind()), false);
    const auto mathDefaults = class_info_defaults(this->factory.getNodeClass("math"), true);
    setAttributes(std::make_shared<Attributes>(ownDefaults, mathDefaults));

    if (!args.empty() && args[0].has_value()) {
        if (const auto* supplied = std::any_cast<PropertyList>(&args[0])) {
            auto attrs = attributes();
            if (!attrs) host::typeError("MML attributes setter produced null handle");
            attrs->setList(*supplied);
        } else if (!std::any_cast<std::nullptr_t>(&args[0])) {
            host::typeError("MML attributes argument has no PropertyList binding");
        }
    }
}

MmlNode* AbstractMmlNode::copy(bool keepIds) {
    tree::TreeCallGuard<MmlNode> guard;
    auto* copied = dynamic_cast<AbstractMmlNode*>(this->factory.create(kind(), {}));
    if (!copied) host::typeError("MML copy creator did not produce an AbstractMmlNode");

    copied->properties = PropertyList{};
    copied->properties.assign(this->properties);
    auto source = attributes();
    if (source) {
        const auto sourceAttributes = source->getAllAttributes();
        for (const auto& name : sourceAttributes.keys()) {
            if (name != "id" || keepIds) {
                auto target = copied->attributes();
                if (!target) host::typeError("Copied MML node has no attributes");
                target->set(name, sourceAttributes.getOwn(name));
            }
        }
    }
    if (this->childNodes && !this->childNodes->empty()) {
        Children children = this->childNodes;
        if (children->size() == 1 && child_at(children, 0, "Cannot read inferred child")->isInferred()) {
            children = child_at(children, 0, "Cannot read inferred child")->childNodes;
        }
        if (!children) host::typeError("Cannot iterate null MML children");
        for (std::size_t index = 0; index < children->size(); ++index) {
            MmlNode* child = index < children->size() ? (*children)[index] : nullptr;
            if (child) copied->appendChild(child->copy());
            else if (copied->childNodes) copied->childNodes->push_back(nullptr);
            else host::typeError("Cannot append null copied child");
        }
    }
    return copied;
}

std::optional<double> AbstractMmlNode::texClass() const { return texclass_; }
void AbstractMmlNode::setTexClass(std::optional<double> value) { texclass_ = value; }
std::optional<double> AbstractMmlNode::prevClass() const { return prevClass_; }
void AbstractMmlNode::setPrevClass(std::optional<double> value) { prevClass_ = value; }
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
    if (!children) host::typeError("Cannot iterate null MML children");
    for (std::size_t index = 0; index < children->size(); ++index) {
        MmlNode* child = (*children)[index];
        if (child && !child->isEmpty()) return false;
    }
    return true;
}

double AbstractMmlNode::arity() const { return std::numeric_limits<double>::infinity(); }
bool AbstractMmlNode::isInferred() const { return false; }
bool AbstractMmlNode::notParent() const { return false; }

MmlNode* AbstractMmlNode::Parent() const {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode* parent = this->parent;
    while (parent && parent->notParent()) parent = parent->Parent();
    return parent;
}

void AbstractMmlNode::setChildren(Children children) {
    tree::TreeCallGuard<MmlNode> guard;
    if (arity() < 0) {
        child_at(this->childNodes, 0, "Cannot route children through an empty inferred row")
            ->setChildren(std::move(children));
        return;
    }
    Generic::setChildren(std::move(children));
}

MmlNode* AbstractMmlNode::appendChild(MmlNode* child) {
    tree::TreeCallGuard<MmlNode> guard;
    if (arity() < 0) {
        child_at(this->childNodes, 0, "Cannot route child through an empty inferred row")
            ->appendChild(child);
        return child;
    }
    if (!child) host::typeError("Cannot read isInferred from null child");
    if (child->isInferred()) {
        if (arity() == std::numeric_limits<double>::infinity()) {
            const auto original = child->childNodes;
            if (!original) host::typeError("Cannot read forEach from null inferred children");
            const auto length = original->size();
            for (std::size_t i = 0; i < length; ++i) {
                if (i < original->size()) Generic::appendChild((*original)[i]);
            }
            return child;
        }
        auto* original = child;
        child = this->factory.create("mrow", {});
        child->setChildren(original->childNodes);
        child->setAttributes(original->attributes());
        for (const auto& name : original->getPropertyNames())
            child->setProperty(name, original->getProperty(name));
    }
    return Generic::appendChild(child);
}

MmlNode* AbstractMmlNode::replaceChild(MmlNode* newChild, MmlNode* oldChild) {
    tree::TreeCallGuard<MmlNode> guard;
    if (arity() < 0) {
        child_at(this->childNodes, 0, "Cannot route replacement through an empty inferred row")
            ->replaceChild(newChild, oldChild);
        return newChild;
    }
    return Generic::replaceChild(newChild, oldChild);
}

MmlNode* AbstractMmlNode::core() { return this; }
MmlNode* AbstractMmlNode::coreMO() { return this; }
std::size_t AbstractMmlNode::coreIndex() { return 0; }

std::optional<std::size_t> AbstractMmlNode::childPosition() {
    MmlNode* child = nullptr;
    MmlNode* parent = this->parent;
    while (parent && parent->notParent()) {
        child = parent;
        parent = parent->parent;
    }
    if (!child) child = this;
    if (parent) {
        const auto siblings = parent->childNodes;
        if (!siblings) host::typeError("Cannot iterate null parent children");
        for (std::size_t i = 0; i < siblings->size(); ++i)
            if ((*siblings)[i] == child) return i;
    }
    return std::nullopt;
}

MmlNode* AbstractMmlNode::setTeXclass(MmlNode* previous) {
    tree::TreeCallGuard<MmlNode> guard;
    getPrevClass(previous);
    return texClass().has_value() ? this : previous;
}

void AbstractMmlNode::updateTeXclass(MmlNode* coreNode) {
    if (coreNode) {
        setPrevClass(coreNode->prevClass());
        setPrevLevel(coreNode->prevLevel());
        // Right-associative TS assignment clears prevLevel before prevClass.
        coreNode->setPrevLevel(PropertyNull{});
        coreNode->setPrevClass(std::nullopt);
        setTexClass(coreNode->texClass());
    }
}

void AbstractMmlNode::getPrevClass(MmlNode* previous) {
    if (previous) {
        setPrevClass(previous->texClass());
        auto attrs = previous->attributes();
        if (!attrs) host::typeError("Cannot read scriptlevel from null attributes");
        setPrevLevel(attrs->get("scriptlevel"));
    }
}

Property AbstractMmlNode::texSpacing() {
    const double previous = prevClass().value_or(TeXClasses::NONE);
    const double current = texClass().value_or(0);
    const double normalized = current == 0 || std::isnan(current) ? 0 : current;
    if (previous == TeXClasses::NONE || normalized == TeXClasses::NONE) return std::string{};
    const auto row = class_index(previous, TEXSPACE.size());
    if (!row) host::typeError("Invalid TeX spacing row");
    const auto column = class_index(normalized, TEXSPACE[*row].size());
    const std::optional<int> space = column
        ? std::optional<int>{TEXSPACE[*row][*column]} : std::nullopt;
    const bool previousScript = host::greaterThanZero(prevLevel());
    bool currentScript = false;
    if (!previousScript) {
        auto attrs = attributes();
        if (!attrs) host::typeError("Cannot read scriptlevel from null attributes");
        currentScript = host::greaterThanZero(attrs->get("scriptlevel"));
    }
    if (!space) return Missing{};
    if ((previousScript || currentScript) && *space >= 0) return std::string{};
    return std::string(TEXSPACELENGTH[static_cast<std::size_t>(std::abs(*space))]);
}

bool AbstractMmlNode::hasSpacingAttributes() {
    tree::TreeCallGuard<MmlNode> guard;
    if (!isEmbellished()) return false;
    auto* mo = coreMO();
    if (!mo) host::typeError("Cannot read spacing attributes from null core mo");
    return mo->hasSpacingAttributes();
}

void AbstractMmlNode::setInheritedAttributes(AttributeList inherited, Property display,
                                              Property level, Property prime) {
    tree::TreeCallGuard<MmlNode> guard;
    auto attrs = attributes();
    if (!attrs) host::typeError("Cannot read defaults from null attributes");
    const auto defaults = attrs->getAllDefaults();
    for (const auto& key : inherited.keys()) {
        if (defaults.hasOwn(key) || alwaysInherit().hasOwn(key)) {
            const auto [origin, value] = unpackAttribute(inherited.getOwn(key));
            if (!forbidden_inheritance(origin, kind(), key)) {
                attrs = attributes();
                if (!attrs) host::typeError("Cannot inherit into null attributes");
                attrs->setInherited(key, value);
            }
        }
        if (stopped_inheritance(kind(), key)) {
            AttributeList updated;
            updated.assign(inherited);
            updated.erase(key);
            inherited = std::move(updated);
        }
    }
    attrs = attributes();
    if (!attrs) host::typeError("Cannot read inherited display attributes");
    if (attrs->getExplicit("displaystyle").missing()) {
        attrs = attributes();
        if (!attrs) host::typeError("Cannot inherit displaystyle into null attributes");
        attrs->setInherited("displaystyle", display);
    }
    attrs = attributes();
    if (!attrs) host::typeError("Cannot read inherited script attributes");
    if (attrs->getExplicit("scriptlevel").missing()) {
        attrs = attributes();
        if (!attrs) host::typeError("Cannot inherit scriptlevel into null attributes");
        attrs->setInherited("scriptlevel", level);
    }
    if (host::truthy(prime)) setProperty("texprimestyle", prime);

    const double expected = arity();
    if (expected >= 0 && expected != std::numeric_limits<double>::infinity()) {
        if (!this->childNodes) host::typeError("Cannot read child count during inherited attributes");
        if ((expected == 1 && this->childNodes->empty()) ||
            (expected != 1 && static_cast<double>(this->childNodes->size()) != expected)) {
            if (expected < static_cast<double>(this->childNodes->size())) {
                this->childNodes = std::make_shared<std::vector<MmlNode*>>(
                    this->childNodes->begin(),
                    this->childNodes->begin() + static_cast<std::ptrdiff_t>(expected));
            } else {
                while (static_cast<double>(this->childNodes->size()) < expected)
                    appendChild(this->factory.create("mrow", {}));
            }
        }
    }

    if (linebreakContainer() && !isEmbellished()) {
        const auto align = linebreakAlign();
        if (!align.empty()) {
            attrs = attributes();
            if (!attrs) host::typeError("Cannot read linebreak attributes");
            Property indent = attrs->get(align);
            if (!host::truthy(indent)) indent = "left";
            PropertyList added{{"indentalign", indent}, {"indentshift", "0"},
                               {"indentalignfirst", indent}, {"indentshiftfirst", "0"},
                               {"indentalignlast", "indentalign"},
                               {"indentshiftlast", "indentshift"}};
            inherited = addInheritedAttributes(std::move(inherited), std::move(added));
        }
    }
    setChildInheritedAttributes(std::move(inherited), std::move(display), std::move(level), std::move(prime));
}

void AbstractMmlNode::setChildInheritedAttributes(AttributeList inherited, Property display,
                                                   Property level, Property prime) {
    const auto children = this->childNodes;
    if (!children) host::typeError("Cannot iterate null MML children");
    for (std::size_t i = 0; i < children->size(); ++i) {
        auto* child = (*children)[i];
        if (!child) host::typeError("Cannot set inherited attributes on null child");
        child->setInheritedAttributes(inherited, display, level, prime);
    }
}

AttributeList AbstractMmlNode::addInheritedAttributes(AttributeList current, PropertyList additions) {
    AttributeList updated;
    updated.assign(current);
    for (const auto& name : additions.keys()) {
        if (name != "displaystyle" && name != "scriptlevel" && name != "style")
            updated.set(name, attributeTuple(kind(), additions.getOwn(name)));
    }
    return updated;
}

void AbstractMmlNode::inheritAttributesFrom(MmlNode* node) {
    tree::TreeCallGuard<MmlNode> guard;
    if (!node) host::typeError("Cannot read attributes from null MML node");
    auto source = node->attributes();
    if (!source) host::typeError("Cannot read inherited attributes from null source attributes");
    const Property display = source->get("displaystyle");
    const Property level = source->get("scriptlevel");
    AttributeList defaults;
    if (source->isSet("mathsize")) defaults.set("mathsize", attributeTuple("math", source->get("mathsize")));
    Property prime = node->getProperty("texprimestyle");
    if (!host::truthy(prime)) prime = false;
    setInheritedAttributes(std::move(defaults), display, level, prime);
}

void AbstractMmlNode::verifyTree(std::optional<PropertyList> options) {
    if (!options) return;
    tree::TreeCallGuard<MmlNode> guard;
    verifyAttributes(*options);
    const double expected = arity();
    if (host::truthy(options->get("checkArity")) && expected >= 0 &&
        expected != std::numeric_limits<double>::infinity()) {
        if (!this->childNodes) host::typeError("Cannot read child count while verifying MML node");
        if ((expected == 1 && this->childNodes->empty()) ||
            (expected != 1 && static_cast<double>(this->childNodes->size()) != expected))
            mError("Wrong number of children for \"" + kind() + "\" node", *options, true);
    }
    verifyChildren(*options);
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
    const auto children = this->childNodes;
    if (!children) host::typeError("Cannot iterate null MML children");
    for (std::size_t i = 0; i < children->size(); ++i) {
        auto* child = (*children)[i];
        if (!child) host::typeError("Cannot verify null MML child");
        child->verifyTree(options);
    }
}

MmlNode* AbstractMmlNode::mError(std::string message, PropertyList options, bool shortError) {
    if (this->parent && this->parent->isKind("merror")) return nullptr;
    auto* error = this->factory.create("merror", {});
    auto errorAttributes = error->attributes();
    if (!errorAttributes) host::typeError("MML error node has null attributes");
    errorAttributes->set("data-mjx-message", message);
    if (host::truthy(options.get("fullErrors")) || shortError) {
        auto* textContainer = this->factory.create("mtext", {});
        auto* text = dynamic_cast<TextNode*>(this->factory.create("text", {}));
        if (!text) host::typeError("Text factory did not produce TextNode");
        const auto content = host::truthy(options.get("fullErrors")) ? message : kind();
        text->setText(host::text(content));
        textContainer->appendChild(text);
        error->appendChild(textContainer);
        if (!this->parent) host::typeError("Cannot replace detached MML node with error");
        this->parent->replaceChild(error, this);
        if (!host::truthy(options.get("fullErrors"))) {
            auto titleAttributes = error->attributes();
            if (!titleAttributes) host::typeError("MML error node has null attributes after replacement");
            titleAttributes->set("title", message);
        }
    } else {
        if (!this->parent) host::typeError("Cannot replace detached MML node with error");
        this->parent->replaceChild(error, this);
        error->appendChild(this);
    }
    return error;
}
} // namespace folia::math::mml
