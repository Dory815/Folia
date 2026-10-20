// MathJax 4.1.3 util/Entities.ts; native string tables and local shard ownership.
export module folia.math.util.entities;
import std;
export namespace folia::math::entities {
using EntityEntry = std::pair<std::u16string, std::u16string>;
using EntityTable = std::map<std::u16string, std::u16string, std::less<>>;
using LoadedFiles = std::set<std::u16string, std::less<>>;
struct EntityOptions {
    bool loadMissingEntities{true};
};
std::u16string numeric(std::u16string_view);
class EntityModule {
    EntityOptions options_;
    EntityTable entities_;
    LoadedFiles loaded_;

  public:
    EntityModule();
    EntityOptions &options() noexcept {
        return options_;
    }
    EntityOptions const &options() const noexcept {
        return options_;
    }
    EntityTable const &table() const noexcept {
        return entities_;
    }
    LoadedFiles const &loadedFiles() const noexcept {
        return loaded_;
    }
    void add(std::span<EntityEntry const>, std::u16string_view file);
    void remove(std::u16string_view entity);
    bool installShard(std::u16string_view file);
    std::u16string translate(std::u16string_view text);
};
} // namespace folia::math::entities
