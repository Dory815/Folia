// Native host for the string-keyed Map owners used by the TeX parser family.
// This is one insertion-ordered owner; ordinary option records use Context.
export module folia.math.util.string_map;
export import folia.math.util.map_bindings;
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

// The TeX package-data and ParseOptions variant maps are ordinary Source
// Maps. Their native algorithms use string keys, while Context exposes the
// same MapStorage through MapContextBindings. Keeping this specialization on
// that storage makes Context mutation and typed parser reads share one map.
template<> class SourceStringMap<options::Value> {
    options::MapStorageRef storage_=std::make_shared<options::ValueMapStorage>();
public:
    void set(std::u16string key,options::Value value){
        storage_->set(options::Value(std::move(key)),std::move(value));
    }
    std::optional<options::Value> get(std::u16string_view key)const{
        const options::Value sourceKey{std::u16string(key)};
        if(!storage_->has(sourceKey))return std::nullopt;
        return storage_->get(sourceKey);
    }
    bool has(std::u16string_view key)const{
        return storage_->has(options::Value(std::u16string(key)));
    }
    std::size_t size()const{return storage_->size();}
    options::MapStorageRef storage()const{return storage_;}
    void useStorage(options::MapStorageRef storage){
        if(!storage)throw options::Exception("TypeError",u"Source Map storage is null");
        storage_=std::move(storage);
    }
    class KeyIterator {
        options::MapStorageRef storage_;
        std::size_t index_=0;
        bool done_=false;
    public:
        explicit KeyIterator(options::MapStorageRef storage):storage_(std::move(storage)){}
        std::optional<std::u16string> next(){
            if(done_)return std::nullopt;
            const auto& entries=storage_->entries();
            while(index_<entries.size()){
                const auto& entry=entries[index_++];
                if(!entry.live)continue;
                if(const auto* key=std::get_if<std::u16string>(&entry.key.data))return *key;
                throw options::Exception("TypeError",u"Source string Map contains a non-string key");
            }
            done_=true;
            storage_.reset();
            return std::nullopt;
        }
    };
    KeyIterator keys()const{return KeyIterator(storage_);}
};
}
