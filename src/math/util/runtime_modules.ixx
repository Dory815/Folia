// Context-indexed cache for actual Source module owners. A facade strongly
// owns the shared state; Context keeps only a weak lookup entry, so independent
// ParserRuntime instances share one live graph without extending its lifetime.
export module folia.math.util.runtime_modules;
export import folia.math.util.option_types;
import std;

export namespace folia::math::util {
class RuntimeModules {
    struct Entry {
        std::type_index type{typeid(void)};
        std::shared_ptr<void> owner;
    };
    struct State {
        options::Context* context;
        std::unordered_map<std::u16string,Entry> owners;
        explicit State(options::Context& value):context(&value){}
    };
    std::shared_ptr<State> state_;
public:
    explicit RuntimeModules(options::Context& context)
        :state_(context.getOrCreateService<State>(u"math/RuntimeModules",
            [&]{return std::make_shared<State>(context);})){ }
    RuntimeModules(const RuntimeModules&)=default;
    RuntimeModules& operator=(const RuntimeModules&)=default;
    options::Context& context()const{return *state_->context;}

    template<class T,class Factory>
    std::shared_ptr<T> getOrCreate(std::u16string identity,Factory&& create){
        auto found=state_->owners.find(identity);
        if(found!=state_->owners.end()){
            if(found->second.type!=std::type_index(typeid(T)))
                throw options::Exception("TypeError",u"Source module identity reused for a different native owner");
            return std::static_pointer_cast<T>(found->second.owner);
        }
        // A thrown module initializer is not cached. This matches the pinned
        // loader's removal of a failed module from its cache.
        auto owner=std::invoke(std::forward<Factory>(create));
        if(!owner)throw options::Exception("TypeError",u"Source module initializer returned no owner");
        if constexpr(requires(const T& value){value.context();}){
            if(&owner->context()!=state_->context)
                throw options::Exception("TypeError",u"Source module owner belongs to a different Context");
        }
        auto [slot,inserted]=state_->owners.emplace(std::move(identity),Entry{std::type_index(typeid(T)),owner});
        if(!inserted)throw options::Exception("TypeError",u"Source module cache changed during initialization");
        return owner;
    }
    template<class T> std::shared_ptr<T> find(std::u16string_view identity)const{
        const auto found=state_->owners.find(std::u16string(identity));
        if(found==state_->owners.end())return {};
        if(found->second.type!=std::type_index(typeid(T)))
            throw options::Exception("TypeError",u"Source module identity has a different native owner type");
        return std::static_pointer_cast<T>(found->second.owner);
    }
    void clear(std::u16string_view identity){state_->owners.erase(std::u16string(identity));}
    void clear(){state_->owners.clear();}
    std::size_t size()const noexcept{return state_->owners.size();}
};
}
