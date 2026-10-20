// MathJax 4.1.3 input/tex/MapHandler.ts; Apache-2.0.
export module folia.math.input.tex.parser:map_handler;
export import :token_map;
export import folia.math.util.function_list;
import std;

export namespace folia::math::tex {
class WarningSink {
public:
    virtual ~WarningSink() = default;
    virtual void warn(std::u16string_view) = 0;
};
struct HandlerRegistration {
    HandlerKind kind;
    std::vector<std::u16string> maps;
    FallbackHandler fallback{};
    double priority{5};
};

class MapHandler final {
    std::map<std::u16string, TokenMapRef, std::less<>> maps_;
public:
    MapHandler() = default;
    MapHandler(MapHandler const&) = delete;
    MapHandler& operator=(MapHandler const&) = delete;
    void registerMap(TokenMapRef fullyInitializedMap);
    TokenMapRef getMap(std::u16string_view) const;
};
class SubHandler {
    MapHandler& registry_;
    WarningSink& warnings_;
    PrioritizedList<TokenMapRef> configuration_;
    FunctionList<TexParser&, std::u16string_view> fallback_;
public:
    SubHandler(MapHandler& registry, WarningSink& warnings) : registry_(registry), warnings_(warnings) {}
    void add(std::span<std::u16string const>, FallbackHandler = nullptr, double priority = 5);
    void remove(std::span<std::u16string const>, FallbackHandler = nullptr);
    DispatchOutcome parse(TexParser&, std::u16string_view);
    TokenLookup lookup(std::u16string_view) const;
    bool contains(std::u16string_view) const;
    std::u16string toString() const;
    TokenMapRef applicable(std::u16string_view) const;
    TokenMapRef retrieve(std::u16string_view) const;
};
class SubHandlers {
    MapHandler& registry_;
    WarningSink& warnings_;
    std::array<std::unique_ptr<SubHandler>, 4> handlers_;
    std::vector<HandlerKind> insertionOrder_;
public:
    SubHandlers(MapHandler& registry, WarningSink& warnings) : registry_(registry), warnings_(warnings) {}
    void add(std::span<HandlerRegistration const>);
    void remove(std::span<HandlerRegistration const>);
    void set(HandlerKind, std::unique_ptr<SubHandler>);
    SubHandler* get(HandlerKind) noexcept;
    SubHandler const* get(HandlerKind) const noexcept;
    TokenMapRef retrieve(std::u16string_view) const;
    std::span<HandlerKind const> keys() const noexcept { return insertionOrder_; }
};
}
