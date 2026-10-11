// Native owned dictionary of ParseMethod/FunctionValue slots; no JS evaluator.
// Functions share the existing FunctionRef identity, scalar slots use Property.
export module folia.math.util.method_config;
export import folia.math.util.function_types;
import std;
export namespace folia::math {
class MethodConfig {
    struct State {
        std::vector<std::pair<std::string,FunctionValue>> entries;
        std::shared_ptr<State> prototype;
        bool ordinary=true;
    };
    std::shared_ptr<State> state_=std::make_shared<State>();
public:
    MethodConfig()=default;
    static MethodConfig nullObject(){MethodConfig r;r.state_->ordinary=false;return r;}
    static MethodConfig withPrototype(const MethodConfig&p){MethodConfig r;r.state_->prototype=p.state_;return r;}
    FunctionValue get(std::string_view key)const{
        bool ordinary=true;
        for(auto p=state_;p;p=p->prototype){
            for(const auto&[name,value]:p->entries)if(name==key)return value;
            ordinary=p->ordinary;
        }
        return Property(ordinary?PropertyList{}.get(key):Property(Missing{}));
    }
    void setOwn(std::string_view key,FunctionValue value){
        for(auto&[name,old]:state_->entries)if(name==key){old=std::move(value);return;}
        state_->entries.emplace_back(std::string(key),std::move(value));
    }
    void erase(std::string_view key){std::erase_if(state_->entries,[&](const auto&e){return e.first==key;});}
};
}
