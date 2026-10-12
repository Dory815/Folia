// Native host for the string-keyed Map owners used by the TeX parser family.
// This is one insertion-ordered owner; ordinary option records use Context.
export module folia.math.util.string_map;
import std;
export namespace folia::math {
template<class T>class SourceStringMap {
    using Storage=std::vector<std::pair<std::u16string,T>>;
    std::shared_ptr<Storage> storage_=std::make_shared<Storage>();
public:
    // Copy is a handle alias, matching assignment of the same Source Map.
    void set(std::u16string key,T value){
        for(auto&entry:*storage_)if(entry.first==key){entry.second=std::move(value);return;}
        storage_->emplace_back(std::move(key),std::move(value));
    }
    std::optional<T> get(std::u16string_view key)const{
        for(const auto&entry:*storage_)if(entry.first==key)return entry.second;
        return std::nullopt;
    }
    bool has(std::u16string_view key)const{
        return std::ranges::any_of(*storage_,[&](const auto&entry){return entry.first==key;});
    }
    std::size_t size()const{return storage_->size();}
    class KeyIterator {
        std::shared_ptr<Storage> storage_;
        std::size_t index_=0;
        bool done_=false;
    public:
        explicit KeyIterator(std::shared_ptr<Storage> storage):storage_(std::move(storage)){}
        std::optional<std::u16string> next(){
            if(done_)return std::nullopt;
            if(index_>=storage_->size()){done_=true;return std::nullopt;}
            return (*storage_)[index_++].first;
        }
    };
    KeyIterator keys()const{return KeyIterator(storage_);}
};
}
