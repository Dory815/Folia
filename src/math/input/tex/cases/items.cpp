// Native port of CasesBeginItem from MathJax 4.1.3 CasesConfiguration.ts.
module folia.math.input.tex.cases.items;
import folia.math.input.tex.parser;
import folia.math.input.tex.stack_host;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
std::string CasesBeginItem::kind() const { return "cases-begin"; }

CheckType CasesBeginItem::checkItem(BaseItem* raw) {
    if (!raw) property_host::typeError("Cannot read properties of null (reading 'isKind')");
    BaseItem& item=*raw;
    if (item.isKind("end") &&
        stack_property_host::scalar(item.getName()) == stack_property_host::scalar(this->getName()) &&
        stack_property_host::truthy(this->getProperty("end"))) {
        this->setProperty("end",false);
        auto result=std::make_shared<CheckTuple>();
        result->items=std::make_shared<std::vector<StackEntry>>();
        result->success=true;
        return result;
    }
    return BeginItem::checkItem(raw);
}

tree::FactoryClassRef<BaseItem> CasesBeginItem::initializeModule(ParserRuntime& runtime) {
    // CasesConfiguration imports BeginItem from BaseItems.  StackItem's
    // module only exports the abstract BaseItem constructor.
    const auto* module=std::get_if<options::ObjectRef>(&runtime.baseItemsModule.data);
    if(!module)throw options::Exception("TypeError",u"BaseItems module exports are unavailable");
    const auto baseValue=runtime.context().get(options::Value(*module),options::Key(u"BeginItem"));
    const auto* base=std::get_if<options::ObjectRef>(&baseValue.data);
    if(!base)throw options::Exception("TypeError",u"BeginItem constructor is unavailable");
    return StackItemFactoryBindings::registerSubclass(runtime,u"CasesBeginItem","cases-begin",0,typeid(CasesBeginItem),
        []{return std::make_unique<CasesBeginItem>();},[](BaseItem&,tree::FactoryArgs){},
        [](const BaseItem* item){return dynamic_cast<const CasesBeginItem*>(item)!=nullptr;},
        []{return BaseItem::baseErrors();},*base,true);
}
}
