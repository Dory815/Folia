// MathJax 4.1.3 input/tex/ColumnParser.ts; Apache-2.0.
// Source column-template parser with native per-array state.
export module folia.math.input.tex.parser:column_parser;
export import :base_items;
export import :types;
export import folia.math.input.tex.session;
import std;

export namespace folia::math::tex {
class TexParser;

// A Source ColumnState.  The four arrays that alias ArrayItem in Source are
// references here too; the other vectors are per-process scratch state.
struct ColumnState {
    TexParser &parser;
    std::u16string template_text;
    std::size_t i{};
    std::u16string c;
    std::size_t j{};
    std::vector<std::u16string> calign;
    std::vector<std::u16string> cwidth;
    std::vector<std::u16string> cspace;
    std::vector<std::u16string> clines;
    std::vector<std::u16string> &cstart;
    std::vector<std::u16string> &cend;
    std::vector<bool> &cextra;
    std::vector<ArrayColumnAlignment> &ralign;
    ColumnState(TexParser &, std::u16string, ArrayItem &);
};

class ColumnParser {
  public:
    using Handler = void (*)(ColumnParser &, ColumnState &);
    using BuiltinEntry = std::pair<char16_t, Handler>;
    double MAXCOLUMNS{10000};

  private:
    ColumnMacroDefinitions &macro_definitions_;
    static std::span<BuiltinEntry const> builtinHandlers() noexcept;

  protected:
    virtual void processColumn(ColumnState &, std::u16string);
    virtual void setColumnAlign(ColumnState &, ArrayItem &);
    virtual void setColumnWidths(ColumnState &, ArrayItem &);
    virtual void setColumnSpacing(ColumnState &, ArrayItem &);
    virtual void setColumnLines(ColumnState &, ArrayItem &);
    virtual void setPadding(ColumnState &, ArrayItem &);

  public:
    explicit ColumnParser(ColumnMacroDefinitions &);
    virtual void process(TexParser &, std::u16string, ArrayItem &);
    virtual void getColumn(ColumnState &, std::u16string ralign, std::u16string calign = u"left");
    virtual std::u16string getDimen(ColumnState &);
    virtual std::u16string getAlign(ColumnState &);
    virtual std::u16string getBraces(ColumnState &);
    virtual void macroColumn(ColumnState &, std::u16string macro, double argument_count);
    virtual void addRule(ColumnState &, std::u16string rule);
    virtual void addAt(ColumnState &, std::u16string macro);
    virtual void addBang(ColumnState &, std::u16string macro);
    virtual void repeat(ColumnState &);
    virtual void brace(ColumnState &);
};
} // namespace folia::math::tex
