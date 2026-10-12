module folia.math.input.tex.stack;
import folia.math.input.tex.stack_host;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
void Stack::initializeStack(StackFactoryReference factory,Property environment,Property inner){
    factory_=std::move(factory);
    env_=environment;
    stack_.clear();
    global=PropertyList{{"isInner",std::move(inner)}};
    const StackEntry start=stack_host::create(factory_,"start",std::any(global));
    stack_={std::get<StackItemRef>(start)};
    if(property_host::truthy(environment)){
        BaseItem& first=stack_host::item(stack_.front(),"env");
        first.setEnv(environment);
    }
    BaseItem& first=stack_host::item(stack_.front(),"env");
    const Property firstEnvironment=first.env();
    this->setEnv(firstEnvironment);
}
void Stack::setEnv(Property value){env_=std::move(value);}
Property Stack::env()const{return env_;}
void Stack::Push(std::span<const StackEntry> values){
    const std::vector<StackEntry> arguments(values.begin(),values.end());
    for(const StackEntry& input:arguments){
        if(!stack_host::truthy(input))continue;

        StackEntry current=input;
        if(stack_host::isNode(input)){
            const mml::NodeValue node=std::get<mml::NodeValue>(input);
            current=stack_host::create(factory_,"mml",std::any(node));
        }
        BaseItem& item=stack_host::item(current,"global");
        item.global=global;

        bool succeeded=true;
        bool checkedTop=false;
        std::variant<Missing,PropertyNull,CheckList> topValue=PropertyNull{};
        if(!stack_.empty()){
            const StackEntry topEntry=this->Top();
            BaseItem& top=stack_host::item(topEntry,"checkItem");
            const CheckType checked=top.checkItem(&item);
            const auto tuple=stack_host::tuple(checked);
            topValue=tuple->items;
            const Property success=tuple->success;
            checkedTop=true;
            succeeded=property_host::truthy(success);
        }
        if(!succeeded)continue;

        if(checkedTop&&stack_host::replacement(topValue)){
            this->Pop();
            const std::vector<StackEntry> replacement=stack_host::spread(topValue);
            this->Push(replacement);
            continue;
        }

        const bool nullItem=item.isKind("null");
        if(!nullItem){
            if(auto* reference=std::get_if<StackItemRef>(&current))stack_.push_back(*reference);
            else property_host::typeError("Native Stack item reflection is unbound");
        }

        const Property itemEnvironment=item.env();
        if(property_host::truthy(itemEnvironment)){
            const bool copy=item.copyEnv();
            if(copy){
                const Property target=item.env();
                const Property source=this->env();
                stack_host::assign(target,source);
            }
            const Property nextEnvironment=item.env();
            this->setEnv(nextEnvironment);
        }else{
            const Property nextEnvironment=this->env();
            item.setEnv(nextEnvironment);
        }
    }
}
StackEntry Stack::Pop(){
    StackEntry item=Property(Missing{});
    if(!stack_.empty()){
        item=stack_.back();
        stack_.pop_back();
    }
    BaseItem& popped=stack_host::item(item,"isOpen");
    if(!popped.isOpen())stack_host::deleteEnv(popped);

    if(!stack_.empty()){
        const StackEntry top=this->Top();
        BaseItem& current=stack_host::item(top,"env");
        const Property nextEnvironment=current.env();
        this->setEnv(nextEnvironment);
    }else{
        this->setEnv(PropertyList{});
    }
    return item;
}
StackEntry Stack::Top(Property n)const{
    if(n.missing())n=1;
    const double lengthForCompare=static_cast<double>(stack_.size());
    const double countForCompare=property_host::number(n);
    if(lengthForCompare<countForCompare)return Property(PropertyNull{});
    const double lengthForIndex=static_cast<double>(stack_.size());
    const double countForIndex=property_host::number(n);
    return stack_host::index(stack_,lengthForIndex-countForIndex);
}
mml::NodeValue Stack::Prev(Property noPop){
    const StackEntry top=this->Top();
    if(property_host::truthy(noPop))return stack_host::item(top,"First").First();
    return stack_host::item(top,"Pop").Pop();
}
double Stack::height()const{return static_cast<double>(stack_.size());}
std::string Stack::toString()const{
    const std::string contents=stack_host::join(stack_,"\n  ");
    return "stack[\n  "+contents+"\n]";
}
}
