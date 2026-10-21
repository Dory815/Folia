/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * MathJax 4.1.3 input/tex/FilterUtil.ts; Apache-2.0.
 *************************************************************/
module folia.math.input.tex.filter_util;
import folia.math.input.tex.node_util;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import std;

namespace folia::math::tex {
namespace {
using mml::AttributeValue;
using mml::MmlNode;
using mml::NodeKind;
bool _isEqual(AttributeValue const *first, AttributeValue const *second) {
    return first && second ? *first == *second : !first && !second;
}
bool truthy(AttributeValue const *value) {
    if (!value || std::holds_alternative<mml::AttributeNull>(*value))
        return false;
    if (auto boolean = std::get_if<bool>(value))
        return *boolean;
    if (auto number = std::get_if<double>(value))
        return *number != 0 && !std::isnan(*number);
    return !std::get<std::u16string>(*value).empty();
}
std::u16string text(AttributeValue const *value) {
    if (auto string = value ? std::get_if<std::u16string>(value) : nullptr)
        return *string;
    return {};
}
void _copyExplicit(std::span<std::u16string_view const> names, MmlNode &destination,
                   MmlNode const &source) {
    for (auto name : names)
        if (auto value = source.attributes().own(name);
            value && !std::holds_alternative<mml::AttributeNull>(*value))
            destination.attributes().set(std::u16string(name), *value);
}
bool _compareExplicit(MmlNode const &first, MmlNode const &second) {
    auto names = [](mml::AttributeSet const &attributes, std::u16string_view space) {
        std::vector<std::u16string_view> result;
        for (auto const &[name, value] : attributes.ownEntries())
            if (name != space && (name != u"stretchy" || truthy(&value)) && name != u"data-latex" &&
                name != u"data-latex-item")
                result.push_back(name);
        return result;
    };
    auto one = names(first.attributes(), u"lspace"), two = names(second.attributes(), u"rspace");
    if (one.size() != two.size())
        return false;
    for (auto name : one)
        if (!_isEqual(first.attributes().own(name), second.attributes().own(name)))
            return false;
    return true;
}
MmlNode *child(MmlNode &node, std::size_t index) {
    return index < node.children().size() ? node.children()[index] : nullptr;
}
void _cleanSubSup(ParseOptions &options, bool underover) {
    auto list = underover ? u"munderover" : u"msubsup";
    std::vector<MmlNode *> removed;
    for (auto node : options.getList(list)) {
        std::size_t base{}, low{}, up{};
        if (underover) {
            auto &scripts = dynamic_cast<mml::MmlMunderover &>(*node);
            base = scripts.base();
            low = scripts.under();
            up = scripts.over();
        } else {
            auto &scripts = dynamic_cast<mml::MmlMsubsup &>(*node);
            base = scripts.base();
            low = scripts.sub();
            up = scripts.sup();
        }
        auto lower = child(*node, low), upper = child(*node, up);
        if (lower && upper)
            continue;
        auto parent = node->parent();
        auto kind = underover ? (lower ? NodeKind::munder : NodeKind::mover)
                              : (lower ? NodeKind::msub : NodeKind::msup);
        const std::array children{child(*node, base), lower ? lower : upper};
        auto &replacement = options.request().nodeFactory.makeNode(kind, children);
        NodeUtil::copyAttributes(*node, replacement);
        parent->replaceChild(replacement, *node);
        removed.push_back(node);
    }
    options.removeFromList(list, removed);
}
void _moveLimits(ParseOptions &options, std::u16string_view list, NodeKind replacementKind) {
    std::vector<MmlNode *> removed;
    for (auto node : options.getList(list)) {
        if (truthy(node->attributes().lookup(u"displaystyle")))
            continue;
        auto &scripts = dynamic_cast<mml::MmlMunderover &>(*node);
        auto base = child(*node, scripts.base());
        auto core = base->coreMO();
        if (base->properties().tex.movable_limits.value_or(false) &&
            !core->attributes().own(u"movablelimits")) {
            auto children =
                std::vector<MmlNode *>(node->children().begin(), node->children().end());
            auto &replacement = options.request().nodeFactory.makeNode(replacementKind, children);
            NodeUtil::copyAttributes(*node, replacement);
            node->parent()->replaceChild(replacement, *node);
            removed.push_back(node);
        }
    }
    options.removeFromList(list, removed);
}
} // namespace

void FilterUtil::cleanSubSup(TexFilterData &arg) {
    if (arg.data.request().error)
        return;
    _cleanSubSup(arg.data, false);
    _cleanSubSup(arg.data, true);
}
void FilterUtil::setInherited(TexFilterData &arg) {
    arg.data.request().root->setInheritedAttributes({}, arg.math.isDisplay(), 0, false);
}
void FilterUtil::checkScriptlevel(TexFilterData &arg) {
    auto &options = arg.data;
    std::vector<MmlNode *> removed;
    for (auto node : options.getList(u"mstyle")) {
        auto first = child(*node, 0);
        if (first->children().size() != 1)
            continue;
        auto &attributes = node->attributes();
        for (auto key : {u"displaystyle", u"scriptlevel"})
            if (_isEqual(attributes.own(key), attributes.effectiveInherited(key)))
                attributes.erase(key);
        auto names = std::vector<mml::AttributeSet::Entry>(attributes.ownEntries());
        if (std::any_of(names.begin(), names.end(),
                        [](auto const &entry) { return !entry.first.starts_with(u"data-latex"); }))
            continue;
        auto content = first->children()[0];
        for (auto const &[name, value] : names)
            content->attributes().set(name, *attributes.lookup(name));
        node->parent()->replaceChild(*content, *node);
        removed.push_back(node);
    }
    options.removeFromList(u"mstyle", removed);
}
void FilterUtil::moveLimits(TexFilterData &arg) {
    _moveLimits(arg.data, u"munderover", NodeKind::msubsup);
    _moveLimits(arg.data, u"munder", NodeKind::msub);
    _moveLimits(arg.data, u"mover", NodeKind::msup);
}
void FilterUtil::cleanStretchy(TexFilterData &arg) {
    for (auto node : arg.data.getList(u"fixStretchy"))
        if (node->properties().tex.fix_stretchy.value_or(false)) {
            if (auto symbol = NodeUtil::getForm(*node))
                for (auto const &[name, value] : symbol->properties)
                    if (name == u"stretchy" && truthy(&value))
                        node->attributes().set(u"stretchy", false);
            node->properties().tex.fix_stretchy.reset();
        }
}
void FilterUtil::cleanAttributes(TexFilterData &arg) {
    arg.data.request().root->walkTree([](MmlNode &node) {
        auto const &keep = node.properties().tex.keep_attributes.value_or(u"");
        std::unordered_set<std::u16string_view> retained;
        std::size_t begin{};
        while (begin <= keep.size()) {
            auto end = keep.find(u' ', begin);
            if (end == keep.npos)
                end = keep.size();
            retained.insert(std::u16string_view(keep).substr(begin, end - begin));
            if (end == keep.size())
                break;
            begin = end + 1;
        }
        auto &attributes = node.attributes();
        attributes.erase(u"data-latex-item");
        auto entries = std::vector<mml::AttributeSet::Entry>(attributes.ownEntries());
        for (auto const &[name, value] : entries)
            if (!retained.contains(name) &&
                _isEqual(attributes.lookup(name), attributes.effectiveInherited(name)))
                attributes.erase(name);
    });
}
void FilterUtil::combineRelations(TexFilterData &arg) {
    std::vector<MmlNode *> removed;
    for (auto mo : arg.data.getList(u"mo")) {
        auto parent = mo->parent();
        if (mo->properties().tex.relations_combined.value_or(false) || !parent ||
            !parent->isKind(NodeKind::mrow) || mo->texClass() != mml::TexClass::rel)
            continue;
        auto next = parent->childIndex(*mo).value() + 1;
        auto variant = mo->properties().tex.variant_form;
        while (next < parent->children().size()) {
            auto other = parent->children()[next];
            if (!other || !other->isKind(NodeKind::mo) || other->texClass() != mml::TexClass::rel)
                break;
            if (variant == other->properties().tex.variant_form && _compareExplicit(*mo, *other)) {
                auto children =
                    std::vector<MmlNode *>(other->children().begin(), other->children().end());
                for (auto child : children)
                    mo->appendChild(*child);
                constexpr std::array<std::u16string_view, 2> names{u"stretchy", u"rspace"};
                _copyExplicit(names, *mo, *other);
                mml::mergeNodeProperties(other->properties(), mo->properties());
                if (auto latex = other->attributes().lookup(u"data-latex"); truthy(latex)) {
                    auto first = mo->attributes().lookup(u"data-latex");
                    mo->attributes().set(u"data-latex",
                                         (first ? text(first) : u"undefined") + text(latex));
                }
                parent->removeChild(*other);
                removed.push_back(other);
                other->properties().tex.relations_combined = true;
                mo->properties().tex.explicit_tex_class = mml::TexClass::rel;
            } else {
                if (!mo->attributes().own(u"rspace"))
                    mo->attributes().set(u"rspace", u"0pt");
                if (!other->attributes().own(u"lspace"))
                    other->attributes().set(u"lspace", u"0pt");
                break;
            }
        }
        auto forms = dynamic_cast<mml::MmlMo &>(*mo).getForms();
        mo->attributes().setInherited(u"form", forms.front());
    }
    arg.data.removeFromList(u"mo", removed);
}
} // namespace folia::math::tex
