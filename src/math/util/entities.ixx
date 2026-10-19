// MathJax 4.1.3 util/Entities.ts.  The entity records and loaded-file state
// belong to one module instance; shard data is only installed on demand.
export module folia.math.util.entities;
export import folia.math.core.tree.property;
export import folia.math.util.promise;
import folia.math.util.regexp_host;
import std;

export namespace folia::math::entities {
using EntityLoader = std::function<async::PromiseRef(std::string)>;

class EntityModule {
    PropertyList options_;
    PropertyList entities_;
    PropertyList loaded_;
    async::QueueRef queue_;
    EntityLoader asyncLoad_;

    void requestShard(std::u16string_view file);
public:
    explicit EntityModule(async::QueueRef queue = {}, EntityLoader loader = {});
    const async::QueueRef& queue() const noexcept { return queue_; }
    void setAsyncLoad(EntityLoader loader) { asyncLoad_ = std::move(loader); }

    PropertyList& options() noexcept { return options_; }
    const PropertyList& options() const noexcept { return options_; }
    PropertyList& table() noexcept { return entities_; }
    const PropertyList& table() const noexcept { return entities_; }
    const PropertyList& loadedFiles() const noexcept { return loaded_; }

    // Source exports `entities` as one mutable object. `add` mutates that same
    // object and marks the source file loaded after Object.assign succeeds.
    void add(const PropertyList& additions, std::u16string_view file);
    void remove(std::u16string_view entity);
    // Native build-time shard loader entry. Host code calls this for the
    // requested generated source file; it does not preinstall other shards.
    bool installShard(std::u16string_view file);
    std::u16string translate(std::u16string_view text);
    std::u16string numeric(std::u16string_view entity) const;
};

}
