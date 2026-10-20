// MathJax 4.1.3 input/tex/bussproofs/BussproofsUtil.ts; Apache-2.0.
module folia.math.input.tex.bussproofs.util;
import folia.math.input.tex.unit_util;
import folia.math.util.js_number;
import folia.math.util.utf16;
import std;

namespace folia::math::tex::bussproofs_util {
namespace {
using mml::MmlNode;
using mml::NodeKind;
using mml::ProofAttribute;
using mml::ProofRuleDirection;

MmlNode &required(MmlNode *node) {
    if (!node)
        throw std::logic_error("Bussproofs requires an existing proof node");
    return *node;
}
mml::ProofNodeProperties &properties(MmlNode *node) {
    return required(node).properties().proof;
}
MmlNode *child(MmlNode *node, std::size_t index) {
    auto children = required(node).children();
    return index < children.size() ? children[index] : nullptr;
}
std::u16string em(double value) {
    return utf16::decode(UnitUtil::instance().em(value)).text;
}
bool truthy(double value) {
    return value != 0 && !std::isnan(value);
}
double orZero(std::optional<double> value) {
    return value && truthy(*value) ? *value : 0;
}
double mathMax(double left, double right) {
    return std::isnan(left) || std::isnan(right) ? std::numeric_limits<double>::quiet_NaN()
                                                 : std::max(left, right);
}
// Each operation borrows the current output and request document only. The
// measurement expression owns no MML; its root points at the measured subtree.
class Geometry {
    TexFilterData &data_;
    core::AbstractMathItem item_;

  public:
    explicit Geometry(TexFilterData &data) : data_(data), item_(u"", nullptr, data.math.display) {}
    double getBBox(MmlNode *node) {
        item_.root = &required(node);
        if (!data_.output)
            throw std::logic_error("Bussproofs measurement requires the current output service");
        return data_.output->getBBox(item_, data_.document).w;
    }
};
MmlNode *getRule(MmlNode *node) {
    std::size_t index = 0;
    while (node && !node->isKind(NodeKind::mtable)) {
        if (node->isKind(NodeKind::text))
            return nullptr;
        if (node->isKind(NodeKind::mrow)) {
            node = child(node, 0);
            index = 0;
            continue;
        }
        node = child(node->parent(), index++);
    }
    return node;
}
MmlNode *getPremises(MmlNode *rule, std::optional<ProofRuleDirection> direction) {
    return child(
        child(child(child(child(rule, direction == ProofRuleDirection::up ? 1 : 0), 0), 0), 0), 0);
}
MmlNode *getPremise(MmlNode *premises, std::size_t index) {
    return child(child(child(premises, index), 0), 0);
}
MmlNode *getConclusion(MmlNode *rule, std::optional<ProofRuleDirection> direction) {
    return child(child(child(child(rule, direction == ProofRuleDirection::up ? 0 : 1), 0), 0), 0);
}
MmlNode *getColumn(MmlNode *node) {
    while (node && !node->isKind(NodeKind::mtd))
        node = node->parent();
    return node;
}
MmlNode *nextSibling(MmlNode *node) {
    auto &parent = required(required(node).parent());
    auto children = parent.children();
    auto found = std::ranges::find(children, node);
    return found != children.end() && ++found != children.end() ? *found : nullptr;
}
MmlNode *getParentInf(MmlNode *node) {
    while (node && !properties(node).premise_count().has_value())
        node = node->parent();
    return node;
}
double getSpaces(Geometry &geometry, MmlNode *inference, MmlNode *rule, bool right = false) {
    double result = 0;
    if (inference == rule)
        return result;
    auto *parent = required(rule).parent();
    if (inference != parent) {
        auto &props = properties(inference);
        result += orZero(right ? props.right_space : props.left_space);
        inference = parent;
    }
    if (inference == rule)
        return result;
    auto children = required(inference).children();
    auto *edge = children.empty() ? nullptr : children[right ? children.size() - 1 : 0];
    if (edge != rule)
        result += geometry.getBBox(edge);
    return result;
}
double adjustValue(Geometry &geometry, MmlNode *inference, bool right = false) {
    auto *rule = getRule(inference);
    auto *conclusion = getConclusion(rule, properties(rule).rule_direction());
    const double space = getSpaces(geometry, inference, rule, right);
    const double ruleWidth = geometry.getBBox(rule);
    const double conclusionWidth = geometry.getBBox(conclusion);
    return space + (ruleWidth - conclusionWidth) / 2;
}
void addSpace(ParseOptions &config, MmlNode *inference, double space, bool right = false) {
    if (space == 0)
        return;
    auto &initial = properties(inference);
    if (initial.rule_direction() || initial.labelled_rule()) {
        auto &row = config.nodeFactory.makeNode(NodeKind::mrow);
        required(required(inference).parent()).replaceChild(row, *inference);
        row.appendChild(*inference);
        mml::moveProofProperties(initial, row.properties().proof);
        inference = &row;
    }
    auto &props = properties(inference);
    auto &margin = right ? props.right_space : props.left_space;
    margin = orZero(margin) + space;
    std::u16string style;
    for (bool side : {false, true}) {
        const auto value = side ? props.right_space : props.left_space;
        if (value && truthy(*value)) {
            if (!style.empty())
                style += u"; ";
            style += side ? u"margin-right: " : u"margin-left: ";
            style += em(*value);
        }
    }
    inference->attributes().set(u"style", std::move(style));
}
void addSequentSpace(ParseOptions &config, MmlNode *sequent, std::size_t position, bool right,
                     double width) {
    NodeDefinition definition;
    definition.attributes.set(u"width", em(width));
    auto &space = config.nodeFactory.makeNode(NodeKind::mspace, {}, std::move(definition));
    auto *cell = child(sequent, position);
    if (right)
        required(cell).appendChild(space);
    else {
        auto &row = required(child(cell, 0));
        row.document().insert(row, 0, space);
    }
    auto &props = properties(required(sequent).parent());
    if (right)
        props.setSequentAdjustRight(width);
    else
        props.setSequentAdjustLeft(width);
}
void adjustSequentPairwise(Geometry &geometry, ParseOptions &config,
                           std::vector<MmlNode *> sequents) {
    auto *top = sequents.back();
    sequents.pop_back();
    while (!sequents.empty()) {
        auto *bottom = sequents.back();
        sequents.pop_back();
        // Preserve the original getBBox call order: right pair, then left pair.
        const double topRight = geometry.getBBox(child(top, 2));
        const double bottomRight = geometry.getBBox(child(bottom, 2));
        const double topLeft = geometry.getBBox(child(top, 0));
        const double bottomLeft = geometry.getBBox(child(bottom, 0));
        const double left = topLeft - bottomLeft, right = topRight - bottomRight;
        if (properties(required(top).parent()).axiom().value_or(false)) {
            addSequentSpace(config, left < 0 ? top : bottom, 0, false, std::abs(left));
            addSequentSpace(config, right < 0 ? top : bottom, 2, true, std::abs(right));
        }
        top = bottom;
    }
}
void adjustSequents(Geometry &geometry, ParseOptions &config) {
    auto found = config.nodeLists.find(u"sequent");
    if (found == config.nodeLists.end())
        return;
    auto const &sequents = found->second;
    for (auto index = sequents.size(); index != 0;) {
        auto *sequent = sequents[--index];
        if (!sequent)
            break;
        auto &props = properties(sequent);
        if (props.sequent_processed().value_or(false)) {
            props.erase(ProofAttribute::sequent_processed);
            continue;
        }
        auto *inference = getParentInf(sequent);
        if (properties(inference).premise_count() != 1)
            continue;
        std::vector<MmlNode *> collect{sequent};
        while (properties(inference).premise_count() == 1) {
            inference = getRule(inference);
            auto *premise =
                getPremise(getPremises(inference, properties(inference).rule_direction()), 0);
            auto direction = properties(premise).rule_direction();
            auto *next = direction ? getConclusion(premise, direction) : premise;
            if (properties(next).sequent().value_or(false)) {
                sequent = child(next, 0);
                collect.push_back(sequent);
                properties(sequent).setSequentProcessed(true);
            }
            inference = premise;
        }
        adjustSequentPairwise(geometry, config, std::move(collect));
    }
}
std::u16string semantics(mml::ProofNodeProperties const &props) {
    std::u16string result;
    auto add = [&](std::u16string_view key, std::u16string value) {
        if (!result.empty())
            result += u";";
        result += u"bspr_";
        result += key;
        result += u":";
        result += value;
    };
    auto boolean = [](bool value) -> std::u16string { return value ? u"true" : u"false"; };
    auto number = [](double value) { return utf16::decode(js_number::toString(value)).text; };
    for (auto key : props.orderedKeys()) {
        switch (key) {
        case ProofAttribute::axiom:
            add(u"axiom", boolean(*props.axiom()));
            break;
        case ProofAttribute::proof:
            add(u"proof", boolean(*props.proof()));
            break;
        case ProofAttribute::sequent:
            add(u"sequent", boolean(*props.sequent()));
            break;
        case ProofAttribute::sequent_processed:
            add(u"sequentProcessed", boolean(*props.sequent_processed()));
            break;
        case ProofAttribute::premise_count:
            add(u"inference", number(static_cast<double>(*props.premise_count())));
            break;
        case ProofAttribute::rule_direction:
            add(u"inferenceRule",
                *props.rule_direction() == ProofRuleDirection::up ? u"up" : u"down");
            break;
        case ProofAttribute::labelled_rule: {
            const auto labels = *props.labelled_rule();
            add(u"labelledRule", labels == mml::ProofLabels::both   ? u"both"
                                 : labels == mml::ProofLabels::left ? u"left"
                                                                    : u"right");
            break;
        }
        case ProofAttribute::proof_label:
            add(u"prooflabel", *props.proof_label() == mml::ProofSide::left ? u"left" : u"right");
            break;
        case ProofAttribute::sequent_adjust_left:
            add(u"sequentAdjust_left", number(*props.sequent_adjust_left()));
            break;
        case ProofAttribute::sequent_adjust_right:
            add(u"sequentAdjust_right", number(*props.sequent_adjust_right()));
            break;
        }
    }
    return result;
}
} // namespace

void saveDocument(TexFilterData &data) {
    if (!data.output)
        throw std::runtime_error(
            "The bussproofs extension requires an output jax with a getBBox() method");
}
void clearDocument(TexFilterData &) {
    // There is no retained document in the native module. Filter borrows expire
    // with the calling compile invocation, including when another filter throws.
}
void makeBsprAttributes(TexFilterData &data) {
    required(data.data.root).walkTree([](MmlNode &node) {
        auto value = semantics(node.properties().proof);
        if (!value.empty())
            node.attributes().set(u"semantics", std::move(value));
    });
}
void balanceRules(TexFilterData &data) {
    auto &config = data.data;
    Geometry geometry(data);
    adjustSequents(geometry, config);
    auto found = config.nodeLists.find(u"inference");
    if (found == config.nodeLists.end())
        return;
    double maxAdjust = 0;
    for (auto *inference : found->second) {
        const bool proof = properties(inference).proof().value_or(false);
        auto *rule = getRule(inference);
        auto *premises = getPremises(rule, properties(rule).rule_direction());
        auto *first = getPremise(premises, 0);
        double leftAdjust = 0;
        auto count = properties(first).premise_count();
        if (count && *count != 0) {
            const double adjust = adjustValue(geometry, first);
            if (truthy(adjust)) {
                addSpace(config, first, -adjust);
                const double width = getSpaces(geometry, inference, rule);
                addSpace(config, inference, adjust - width);
                leftAdjust = adjust - width;
            }
        }
        auto const children = required(premises).children();
        auto *last = getPremise(premises, children.empty() ? 0 : children.size() - 1);
        if (!properties(last).premise_count().has_value())
            continue;
        const double adjust = adjustValue(geometry, last, true);
        addSpace(config, last, -adjust, true);
        const double width = getSpaces(geometry, inference, rule, true);
        const double ruleWidth = geometry.getBBox(rule);
        const double premisesWidth = geometry.getBBox(required(premises).parent());
        const double delta = (ruleWidth - premisesWidth) / 2;
        addSpace(config, inference, delta < leftAdjust ? -delta : -leftAdjust);
        maxAdjust = mathMax(0, mathMax(0, maxAdjust + adjust - width) - delta);
        auto *column = proof ? nullptr : getColumn(inference);
        if (proof || !column) {
            auto *owner = properties(inference).proof().value_or(false)
                              ? inference
                              : required(inference).parent();
            addSpace(config, owner, maxAdjust, true);
            continue;
        }
        if (auto *sibling = nextSibling(column)) {
            NodeDefinition definition;
            definition.attributes.set(u"width", em(maxAdjust));
            auto &space = config.nodeFactory.makeNode(NodeKind::mspace, {}, std::move(definition));
            sibling->appendChild(space);
            maxAdjust = 0;
            continue;
        }
        if (!getParentInf(column))
            continue;
    }
}
} // namespace folia::math::tex::bussproofs_util
