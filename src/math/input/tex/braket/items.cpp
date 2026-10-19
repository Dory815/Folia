// Native port of MathJax 4.1.3 input/tex/braket/BraketItems.ts.
module folia.math.input.tex.braket.items;
import folia.math.input.tex.parser;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.tex_constants;
import folia.math.util.lengths;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Property scalar(const StackProperty& value){return stack_property_host::scalar(value);}
bool truth(const StackProperty& value){return stack_property_host::truthy(value);}
double number(const StackProperty& value){return property_host::number(scalar(value));}
std::u16string string(const StackProperty& value){return property_host::string(scalar(value));}
mml::Children childArray(std::span<const mml::NodeValue> items){auto out=std::make_shared<tree::ChildArray<mml::MmlNode>>();for(const auto& item:items)out->push_back(item);return out;}
mml::NodeValue node(BaseItem& item,std::string_view kind,mml::Children children,Property attributes=PropertyList{}){
    const std::array<std::any,3> args{std::any(Property(std::string(kind))),std::any(std::move(children)),std::any(std::move(attributes))};
    return item.create("node",FunctionArgs(args));
}
mml::NodeValue token(BaseItem& item,std::string_view kind,Property attributes,Property text){
    const std::array<std::any,3> args{std::any(Property(std::string(kind))),std::any(std::move(attributes)),std::any(std::move(text))};
    return item.create("token",FunctionArgs(args));
}
CheckType complete(BaseItem& item,mml::NodeValue value){
    const auto factoryValue=item.sourceFactory();
    const auto* factory=std::get_if<StackItemFactoryService*>(&factoryValue);
    if(!factory||!*factory)property_host::typeError("Cannot read properties of undefined (reading 'create')");
    const std::any argument(value);auto* raw=(*factory)->itemFactory().create("mml",tree::FactoryArgs(&argument,1));
    if(!raw)property_host::typeError("Cannot read properties of null (reading 'isKind')");
    auto list=std::make_shared<std::vector<StackEntry>>();list->emplace_back((*factory)->reference(raw));
    auto result=std::make_shared<CheckTuple>();result->items=std::move(list);result->success=true;return result;
}
}
std::string BraketItem::kind()const{return "braket";}
bool BraketItem::isOpen()const{return true;}
CheckType BraketItem::checkItem(BaseItem* item){
    if(item&&item->isKind("close")){
        const bool braketbar=truth(item->getProperty("braketbar"));
        if(braketbar){
            const auto saved=BaseItem::toMml(true,true);
            if(saved.isNode()&&saved.requireNode("braketbar row")->childNodes)
                for(std::size_t i=0;i<saved.requireNode("braketbar row")->childNodes->size();++i)
                    barNodes_.push_back(saved.requireNode("braketbar row")->childNodes->get(i));
            Clear();return BaseItem::fail;
        }
        const auto result=toMml();
        return complete(*this,result);
    }
    if(item&&item->isKind("mml")){
        const std::array<mml::NodeValue,1> child{item->toMml()};Push(child);
        if(truth(getProperty("single")))return complete(*this,toMml());
        return BaseItem::fail;
    }
    return BaseItem::checkItem(item);
}
mml::NodeValue BraketItem::toMml(Property inferred,Property forceRow){
    const bool isInferred=inferred.missing()||property_host::truthy(inferred);
    auto inner=BaseItem::toMml(Property(isInferred),std::move(forceRow));
    if(!isInferred)return inner;
    const auto open=string(getProperty("open")),close=string(getProperty("close"));
    if(!barNodes_.empty()){
        auto children=barNodes_;children.push_back(inner);
        inner=node(*this,"inferredMrow",childArray(children));
    }
    if(truth(getProperty("stretchy"))){
        if(truth(getProperty("space"))){
            const auto thin=lengths::em(std::ranges::find_if(lengths::MATHSPACE,[](const auto& entry){return entry.first=="thinmathspace";})->second);
            const auto spaceAttrs=Property(PropertyList{{"width",Property(thin)}});
            const std::array<std::any,2> leftArgs{std::any(Property("mspace")),std::any(spaceAttrs)};
            const std::array<std::any,2> rightArgs{std::any(Property("mspace")),std::any(spaceAttrs)};
            const std::array<mml::NodeValue,3> spaced{
                create("token",FunctionArgs(leftArgs)),
                inner,
                create("token",FunctionArgs(rightArgs))};
            inner=node(*this,"inferredMrow",childArray(spaced));
        }
        const auto factoryValue=sourceFactory();
        const auto* factory=std::get_if<StackItemFactoryService*>(&factoryValue);
        if(!factory||!*factory)property_host::typeError("Cannot read properties of undefined (reading 'configuration')");
        const auto configuration=(*factory)->configuration;
        const auto* configurationRef=std::get_if<StackItemFactoryConfiguration*>(&configuration);
        auto* config=configurationRef&&*configurationRef?dynamic_cast<ParseOptions*>(*configurationRef):nullptr;
        if(!config)property_host::typeError("Cannot read properties of undefined (reading 'configuration')");
        auto result=ParseUtil::fenced(*config,open,inner,close);
        return result;
    }
    const Property openAttrs(PropertyList{{"fence",true},{"stretchy",false},{"symmetric",true},{"texClass",mml::TEXCLASS.OPEN}});
    const Property closeAttrs(PropertyList{{"fence",true},{"stretchy",false},{"symmetric",true},{"texClass",mml::TEXCLASS.CLOSE}});
    const auto openNode=token(*this,"mo",openAttrs,property_host::debugText(open));
    const auto closeNode=token(*this,"mo",closeAttrs,property_host::debugText(close));
    const std::array<mml::NodeValue,3> children{openNode,inner,closeNode};
    auto result=node(*this,"mrow",childArray(children),Property(PropertyList{{"open",property_host::debugText(open)},{"close",property_host::debugText(close)}}));
    return result;
}
tree::FactoryClassRef<BaseItem> BraketItem::initializeModule(ParserRuntime& runtime){
    // StackItem is initialized once by the loaded parser module. Braket only
    // registers its subclass against that same exported BaseItem constructor.
    if(std::holds_alternative<Undefined>(runtime.stackItemModule.data))
        StackItemFactoryBindings::initializeModule(runtime);
    const auto* module=std::get_if<ObjectRef>(&runtime.stackItemModule.data);
    if(!module)throw Exception("TypeError",u"StackItem module exports are unavailable");
    const auto baseValue=runtime.context().get(Value(*module),Key(u"BaseItem"));
    const auto* base=std::get_if<ObjectRef>(&baseValue.data);
    if(!base)throw Exception("TypeError",u"BaseItem constructor is unavailable");
    return StackItemFactoryBindings::registerSubclass(runtime,u"BraketItem","braket",0,typeid(BraketItem),
        []{return std::make_unique<BraketItem>();},[](BaseItem&,tree::FactoryArgs){},
        [](const BaseItem* item){return dynamic_cast<const BraketItem*>(item)!=nullptr;},
        []{return BaseItem::baseErrors();},*base,true);
}
}
