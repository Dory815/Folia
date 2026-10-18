// Per-render-session cache for actual Source module owners. It is never static
// and retains no Context; the session clears/destroys it before Context.
export module folia.math.util.runtime_modules;
export import folia.math.util.option_types;
import std;

export namespace folia::math::util {
class RuntimeModules {
    struct Entry {
        std::type_index type{typeid(void)};
        std::shared_ptr<void> owner;
    };
    options::Context* context_;
    std::unordered_map<std::u16string,Entry> owners_;
public:
    explicit RuntimeModules(options::Context& context):context_(&context){}
    RuntimeModules(const RuntimeModules&)=delete;
    RuntimeModules& operator=(const RuntimeModules&)=delete;
    options::Context& context()const{return *context_;}

    template<class T,class Factory>
    std::shared_ptr<T> getOrCreate(std::u16string identity,Factory&& create){
        auto found=owners_.find(identity);
        if(found!=owners_.end()){
            if(found->second.type!=std::type_index(typeid(T)))
                throw options::Exception("TypeError",u"Source module identity reused for a different native owner");
            return std::static_pointer_cast<T>(found->second.owner);
        }
        // A thrown module initializer is not cached. This matches the pinned
        // loader's removal of a failed module from its cache.
        auto owner=std::invoke(std::forward<Factory>(create));
        if(!owner)throw options::Exception("TypeError",u"Source module initializer returned no owner");
        if constexpr(requires(const T& value){value.context();}){
            if(&owner->context()!=context_)
                throw options::Exception("TypeError",u"Source module owner belongs to a different Context");
        }
        auto [slot,inserted]=owners_.emplace(std::move(identity),Entry{std::type_index(typeid(T)),owner});
        if(!inserted)throw options::Exception("TypeError",u"Source module cache changed during initialization");
        return owner;
    }
    template<class T> std::shared_ptr<T> find(std::u16string_view identity)const{
        const auto found=owners_.find(std::u16string(identity));
        if(found==owners_.end())return {};
        if(found->second.type!=std::type_index(typeid(T)))
            throw options::Exception("TypeError",u"Source module identity has a different native owner type");
        return std::static_pointer_cast<T>(found->second.owner);
    }
    void clear(std::u16string_view identity){owners_.erase(std::u16string(identity));}
    void clear(){owners_.clear();}
    std::size_t size()const noexcept{return owners_.size();}
};
}
