// MathJax 4.1.3 input/tex/TokenMap.ts; Apache-2.0.
export module folia.math.input.tex.parser:token_map;
export import :macro;
export import folia.math.util.regexp_host;
import std;

export namespace folia::math::tex {
using TokenLookup = std::variant<std::monostate, std::u16string_view,
                                 Token const*, Macro const*, CommandDefinition const*>;
DispatchOutcome parseResult(DispatchOutcome);

class TokenMap {
public:
    virtual ~TokenMap() = default;
    virtual std::u16string_view name() const noexcept = 0;
    virtual std::optional<TokenMapParser> parser() const = 0;
    virtual void setParser(std::optional<TokenMapParser>) = 0;
    virtual std::optional<TokenMapParser> parserFor(std::u16string_view) const = 0;
    virtual bool contains(std::u16string_view) const = 0;
    virtual TokenLookup lookup(std::u16string_view) const = 0;
    virtual DispatchOutcome parse(TexParser&, std::u16string_view) = 0;
};
using TokenMapRef = std::shared_ptr<TokenMap>;
struct TokenMapInit {
    std::u16string name;
    std::optional<TokenMapParser> parser;
};
struct RegExpMapInit { TokenMapInit base; regexp_host::RegExpRef regexp; };
struct CharacterMapInit { TokenMapInit base; std::vector<Token> entries; };
struct MacroMapInit { TokenMapInit base; std::vector<Macro> entries; };
struct SessionMacroMapInit {
    TokenMapInit base;
    TexSessionState& session;
    MacroCallbacks const& callbacks;
    MacroHandler expansion;
};
struct CommandMapInit { MacroMapInit macros; };
struct EnvironmentMapInit { MacroMapInit macros; EnvironmentHandler parser; };

class AbstractTokenMap : public TokenMap {
    std::u16string name_;
    std::optional<TokenMapParser> parser_;
protected:
    explicit AbstractTokenMap(TokenMapInit);
    static DispatchOutcome parseValue(TexParser&, TokenMapParser const&, TokenInput const&);
public:
    std::u16string_view name() const noexcept override { return name_; }
    std::optional<TokenMapParser> parser() const override { return parser_; }
    void setParser(std::optional<TokenMapParser>) override;
    std::optional<TokenMapParser> parserFor(std::u16string_view) const override;
    DispatchOutcome parse(TexParser&, std::u16string_view) override;
};
class RegExpMap : public AbstractTokenMap {
    regexp_host::RegExpRef regexp_;
public:
    explicit RegExpMap(RegExpMapInit);
    bool contains(std::u16string_view) const override;
    TokenLookup lookup(std::u16string_view) const override;
};

template<class Entry> class AbstractParseMap : public AbstractTokenMap {
    std::map<std::u16string, Entry, std::less<>> entries_;
protected:
    AbstractParseMap(TokenMapInit init, std::vector<Entry> entries)
        : AbstractTokenMap(std::move(init)) {
        for (auto& entry : entries) {
            auto name = entry.token;
            add(std::move(name), std::move(entry));
        }
    }
public:
    bool contains(std::u16string_view token) const override { return entries_.contains(token); }
    TokenLookup lookup(std::u16string_view token) const override {
        auto entry = entries_.find(token);
        return entry == entries_.end() ? TokenLookup{} : TokenLookup{&entry->second};
    }
    void add(std::u16string key, Entry value) { entries_.insert_or_assign(std::move(key), std::move(value)); }
    void add(Entry value) { auto key = value.token; add(std::move(key), std::move(value)); }
    void remove(std::u16string_view token) { auto entry = entries_.find(token); if (entry != entries_.end()) entries_.erase(entry); }
};
class CharacterMap : public AbstractParseMap<Token> {
public:
    explicit CharacterMap(CharacterMapInit);
};
class DelimiterMap : public CharacterMap {
public:
    explicit DelimiterMap(CharacterMapInit);
    DispatchOutcome parse(TexParser&, std::u16string_view) override;
};
class MacroMap : public AbstractParseMap<Macro> {
    TexSessionState* session_{};
    MacroCallbacks const* callbacks_{};
    MacroHandler expansion_{};
protected:
    DispatchOutcome invoke(TexParser&, TokenLookup const&, TokenMapParser const&,
                           std::u16string_view input, std::u16string_view prefix);
public:
    explicit MacroMap(MacroMapInit);
    explicit MacroMap(SessionMacroMapInit);
    bool contains(std::u16string_view) const override;
    TokenLookup lookup(std::u16string_view) const override;
    std::optional<TokenMapParser> parserFor(std::u16string_view) const override;
    DispatchOutcome parse(TexParser&, std::u16string_view) override;
};
// Commands preserve their input name separately from the definition's name.
class CommandMap : public MacroMap {
public:
    explicit CommandMap(CommandMapInit);
    explicit CommandMap(SessionMacroMapInit);
    DispatchOutcome parse(TexParser&, std::u16string_view) override;
};
class EnvironmentMap : public MacroMap {
public:
    explicit EnvironmentMap(EnvironmentMapInit);
    DispatchOutcome parse(TexParser&, std::u16string_view) override;
};
}
