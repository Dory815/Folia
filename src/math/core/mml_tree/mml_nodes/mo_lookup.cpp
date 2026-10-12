// MathJax 4.1.3 MmlNodes/mo.ts lookup and operator-table methods.
// Apache-2.0; see src/math/LICENSE-MathJax.txt.
module folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mo_host;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.core.mml_tree.operator_dictionary;
import std;

namespace folia::math::mml {
namespace {
std::string propertyKey(const Property& value) {
    return host::debugText(host::string(value));
}

std::string propertyKey(std::u16string_view value) {
    return host::debugText(value);
}

PropertyArray formOrder(std::string_view first, std::string_view second,
                        std::string_view third) {
    return PropertyArray{Property(std::string(first)), Property(std::string(second)),
                         Property(std::string(third))};
}
}

Property MmlMo::texClass() {
    tree::TreeCallGuard<MmlNode> guard;
    if (_texClass == Property(PropertyNull{})) {
        const std::u16string text = getText();
        const Property definition = getOperatorDef(text);
        return host::get(definition, "2");
    }
    return _texClass;
}

bool MmlMo::hasSpacingAttributes() {
    tree::TreeCallGuard<MmlNode> guard;
    const auto firstAttributes = host::moAttributes(*this);
    if (firstAttributes->isSet("lspace")) return true;
    const auto secondAttributes = host::moAttributes(*this);
    return secondAttributes->isSet("rspace");
}

Property MmlMo::getOperatorDef(std::u16string_view mo) {
    tree::TreeCallGuard<MmlNode> guard;

    const PropertyArray orderedForms = handleExplicitForm(getForms());
    const std::vector<Property> forms = host::moHead(Property(orderedForms), 3);
    const Property form1 = forms[0];
    const Property form2 = forms[1];
    const Property form3 = forms[2];

    host::moAttributes(*this)->setInherited("form", form1);

    const MoClassServices& services = constructorOperators();
    const Property operatorTable = host::moStatic<Property>(services.table);
    Property formTable = host::get(operatorTable, propertyKey(form1));
    const std::string moKey = propertyKey(mo);
    Property definition = host::get(formTable, moKey);
    if (!host::truthy(definition)) {
        formTable = host::get(operatorTable, propertyKey(form2));
        definition = host::get(formTable, moKey);
    }
    if (!host::truthy(definition)) {
        formTable = host::get(operatorTable, propertyKey(form3));
        definition = host::get(formTable, moKey);
    }
    if (host::truthy(definition)) return definition;

    setProperty("noDictDef", true);
    const Property limits = host::moAttributes(*this)->get("movablelimits");
    const MoPatternHandle opPattern = host::moStatic<MoPatternHandle>(services.opPattern);
    const bool isOperatorName = host::moMatch(opPattern, mo);
    const bool operatorOrLimits = isOperatorName ? true : host::truthy(limits);
    if (operatorOrLimits) {
        const Property texClassProperty = getProperty("texClass");
        if (texClassProperty.missing()) {
            return Property(operator_dict::OPDEF(
                Property(1.0), Property(2.0), Property(static_cast<double>(TEXCLASS.OP))));
        }
    }

    const Property range = operator_dict::getRange(mo);
    const Property spacingTable = host::moStatic<Property>(services.spacing);
    const Property spacingClass = host::get(range, "2");
    const Property spacing = host::get(spacingTable, propertyKey(spacingClass));
    const std::vector<Property> spaces = host::moHead(spacing, 2);
    const Property texClass = host::get(range, "2");
    return Property(operator_dict::OPDEF(spaces[0], spaces[1], texClass));
}

void MmlMo::checkOperatorTable(std::u16string_view mo) {
    tree::TreeCallGuard<MmlNode> guard;
    const Property definition = getOperatorDef(mo);
    if (getProperty("texClass").missing()) {
        setTexClass(host::get(definition, "2"));
    }

    Property definitionProperties = host::get(definition, "3");
    if (!host::truthy(definitionProperties)) definitionProperties = Property(PropertyList{});
    const std::vector<std::string> names = host::moKeys(definitionProperties);
    for (const std::string& name : names) {
        const auto currentAttributes = host::moAttributes(*this);
        const Property currentProperties = host::get(definition, "3");
        const Property value = host::get(currentProperties, name);
        currentAttributes->setInherited(name, value);
    }

    const Property left = host::get(definition, "0");
    lspace = host::number(left) / 18.0;
    const Property right = host::get(definition, "1");
    rspace = host::number(right) / 18.0;
}

PropertyArray MmlMo::getForms() {
    tree::TreeCallGuard<MmlNode> guard;
    MmlNode* core = nullptr;
    NodeValue parentValue = this->getParent();
    NodeValue logicalParentValue = this->Parent();
    tree::TreeWalkBudget<MmlNode> walkBudget;
    while (MmlNode* const logicalParent = logicalParentValue.nodeOrNull()) {
        if (!logicalParent->isEmbellished()) break;
        walkBudget.step();
        core = parentValue.nodeOrNull();
        parentValue = logicalParent->getParent();
        logicalParentValue = logicalParent->Parent();
    }
    if (!core) core = this;

    MmlNode* const parent = parentValue.nodeOrNull();
    if (parent && parent->isKind("mrow")) {
        auto* const row = dynamic_cast<MmlMrow*>(parent);
        if (!row) host::typeError("parent.nonSpaceLength is not a function");
        if (row->nonSpaceLength() != 1) {
            if (row->firstNonSpace() == core) {
                return PropertyArray{"prefix", "infix", "postfix"};
            }
            if (row->lastNonSpace() == core) {
                return PropertyArray{"postfix", "infix", "prefix"};
            }
        }
    }
    return PropertyArray{"infix", "prefix", "postfix"};
}

PropertyArray MmlMo::handleExplicitForm(PropertyArray forms) {
    tree::TreeCallGuard<MmlNode> guard;
    const auto firstAttributes = host::moAttributes(*this);
    if (!firstAttributes->isSet("form")) return forms;

    const Property form = host::moAttributes(*this)->get("form");
    const std::size_t length = forms.size();
    const Property formsValue(forms);
    const PropertyList formRecord = forms.record();
    PropertyArray result;
    result.push(form);
    for (std::size_t index = 0; index < length; ++index) {
        const std::string key = std::to_string(index);
        if (!formRecord.has(key)) continue;
        const Property name = host::get(formsValue, key);
        if (!(name == form)) result.push(name);
    }
    return result;
}
}
