module folia.math.input.tex.stack;
import folia.math.input.tex.stack_host;
import folia.math.util.property_host;
import folia.math.util.js_number;
import std;
namespace folia::math::tex {
namespace {
StackEntry entryFromArray(const StackArrayStorageValue& value){
    if(const auto* entry=std::get_if<StackEntry>(&value))return *entry;
    return StackEntry(std::get<options::Value>(value));
}
StackArrayStorageValue arrayValue(const StackEntry& entry){return StackArrayStorageValue(entry);}
std::string entryString(const StackArrayStorageValue& value){
    const StackEntry entry=entryFromArray(value);
    if(const auto* property=std::get_if<Property>(&entry)){
        if(property_host::nullish(*property))return {};
        return property_host::debugText(property_host::string(*property));
    }
    if(const auto* raw=std::get_if<options::Value>(&entry)){
        if(std::holds_alternative<options::Undefined>(raw->data)||std::holds_alternative<options::Null>(raw->data))return {};
        if(const auto* text=std::get_if<std::u16string>(&raw->data))return property_host::debugText(*text);
        if(const auto* n=std::get_if<double>(&raw->data))return js_number::toString(*n);
        if(const auto* b=std::get_if<bool>(&raw->data))return *b?"true":"false";
    }
    return stack_host::item(entry,"toString").toString();
}
bool rawTruthy(const options::Value& value){return options::truthy(value);}
}
void Stack::initializeStack(StackFactoryReference factory,Property environment,Property inner){
    factory_=std::move(factory);env_=environment;
    auto storage=std::make_shared<StackArrayStorage>();
    stack_=std::make_shared<NativeStackArrayAccess>(storage,
        [](const options::Value& value){return StackEntry(value);},entryString);
    global=PropertyList{{"isInner",std::move(inner)}};
    const StackEntry start=stack_host::create(factory_,"start",std::any(global));
    stack_->pushEntry(start);
    if(property_host::truthy(environment)){
        BaseItem& first=stack_host::item(start,"env");first.setEnv(environment);
    }
    BaseItem& first=stack_host::item(start,"env");
    const Property firstEnvironment=first.env();this->setEnv(firstEnvironment);
}
void Stack::setEnv(Property value){env_=std::move(value);}
Property Stack::env()const{return env_;}
options::Value Stack::height()const{return stack_->readLength();}
StackEntry Stack::Top(StackArrayScalar n)const{
    if(const auto* property=std::get_if<Property>(&n);property&&property->missing())n=Property(1);
    if(const auto* value=std::get_if<options::Value>(&n);value&&std::holds_alternative<options::Undefined>(value->data))n=Property(1);
    const auto comparisonBase=stack_;
    const options::Value comparisonLength=comparisonBase->readLength();
    if(comparisonBase->lessThan(comparisonLength,n))return StackEntry(Property(PropertyNull{}));
    const auto indexedBase=stack_;
    const auto lengthBase=stack_;
    const options::Value indexLength=lengthBase->readLength();
    const StackArrayScalar index=indexedBase->subtract(indexLength,n);
    return indexedBase->getIndex(index);
}
void Stack::Push(std::span<const StackEntry> values){
    const std::vector<StackEntry> arguments(values.begin(),values.end());
    for(const StackEntry& input:arguments){
        if(!stack_host::truthy(input))continue;
        StackEntry current=stack_->materialize(input);
        if(stack_host::isNode(current)){
            const mml::NodeValue node=std::get<mml::NodeValue>(current);
            current=stack_host::create(factory_,"mml",std::any(node));
        }
        BaseItem& item=stack_host::item(current,"global");item.global=global;
        bool succeeded=true,checkedTop=false;
        std::variant<Missing,PropertyNull,CheckList> topValue=PropertyNull{};
        const options::Value length=stack_->readLength();
        if(rawTruthy(length)){
            const StackEntry topEntry=this->Top();
            BaseItem& top=stack_host::item(stack_->materialize(topEntry),"checkItem");
            const CheckType checked=top.checkItem(&item);
            const auto tuple=stack_host::tuple(checked);topValue=tuple->items;
            checkedTop=true;succeeded=property_host::truthy(tuple->success);
        }
        if(!succeeded)continue;
        if(checkedTop&&stack_host::replacement(topValue)){
            // Source evaluates Pop() before spreading the live replacement
            // value. Keep those observable steps sequenced explicitly.
            this->Pop();const std::vector<StackEntry> replacement=stack_host::spread(topValue);
            this->Push(replacement);continue;
        }
        if(!item.isKind("null")){
            const StackArrayAccessRef receiver=stack_;
            const StackArrayMethod method=receiver->getMethod(u"push");
            const std::array<StackArrayStorageValue,1> pushed{arrayValue(current)};
            (void)receiver->callMethod(method,pushed);
        }
        const Property itemEnvironment=item.env();
        if(property_host::truthy(itemEnvironment)){
            const bool shouldCopy=item.copyEnv();
            if(shouldCopy){
                // Object.assign(item.env, this.env): Source resolves the
                // receiver first, then reads this.env for the argument.
                const Property target=item.env();
                const Property source=this->env();
                stack_host::assign(target,source);
            }
            const Property nextEnvironment=item.env();
            this->setEnv(nextEnvironment);
        }else{
            // Source reads this.env before assigning item.env.
            const Property source=this->env();
            item.setEnv(source);
        }
    }
}
StackEntry Stack::Pop(){
    const StackArrayAccessRef base=stack_;
    const StackArrayMethod method=base->getMethod(u"pop");
    const StackArrayStorageValue raw=base->callMethod(method,{});
    // The `pop` call belongs to the array captured before the call. A getter
    // or replacement triggered by that call must not redirect conversion of
    // its return value through a later `this.stack` owner.
    StackEntry item=base->materialize(entryFromArray(raw));
    BaseItem& popped=stack_host::item(item,"isOpen");
    if(!popped.isOpen())stack_host::deleteEnv(popped);
    const options::Value length=stack_->readLength();
    if(rawTruthy(length)){
        const StackEntry top=this->Top();BaseItem& current=stack_host::item(stack_->materialize(top),"env");
        this->setEnv(current.env());
    }else this->setEnv(PropertyList{});
    return item;
}
mml::NodeValue Stack::Prev(Property noPop){
    const StackEntry top=this->Top();
    if(property_host::truthy(noPop))return stack_host::item(stack_->materialize(top),"First").First();
    return stack_host::item(stack_->materialize(top),"Pop").Pop();
}
std::string Stack::toString(){
    const StackArrayAccessRef receiver=stack_;
    const StackArrayMethod method=receiver->getMethod(u"join");
    const std::array<StackArrayStorageValue,1> args{StackArrayStorageValue(options::Value(u"\n  "))};
    const StackArrayStorageValue joined=receiver->callMethod(method,args);
    const std::u16string text=receiver->toStringValue(joined);
    return "stack[\n  "+property_host::debugText(text)+"\n]";
}
}
