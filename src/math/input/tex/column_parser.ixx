// MathJax 4.1.3 input/tex/ColumnParser.ts; Apache-2.0.
export module folia.math.input.tex.parser:column_parser;
export import :types;
import std;
export namespace folia::math::tex {
// This is a narrow native view of the one ordinary Source state record.
// All observable fields live in record; the proxy contains no mirrored values.
struct ColumnState:NativeObject {
    ParserRuntime* runtime=nullptr;
    options::Value record=options::Undefined{};
    ColumnState(ParserRuntime&,options::Value);
    options::Value get(std::u16string_view)const;
    void set(std::u16string_view,options::Value);
};
class ColumnParser:public NativeObject {
    ParserRuntime& runtime_;
    options::ObjectRef identity_{};
protected:
    virtual void processColumn(ColumnState&,std::u16string);
    virtual void setColumnAlign(ColumnState&,options::Value array);
    virtual void setColumnWidths(ColumnState&,options::Value array);
    virtual void setColumnSpacing(ColumnState&,options::Value array);
    virtual void setColumnLines(ColumnState&,options::Value array);
    virtual void setPadding(ColumnState&,options::Value array);
public:
    explicit ColumnParser(ParserRuntime&); // allocation only
    static void initializeModule(ParserRuntime&);
    void initializeColumnParser(std::shared_ptr<void> actualOwner);
    ParserRuntime& runtime()const;
    options::ObjectRef sourceIdentity()const;
    virtual void process(TexParser&,std::u16string,ArrayItem&);
    virtual void getColumn(ColumnState&,options::Value ralign,options::Value calign=options::Undefined{});
    virtual std::u16string getDimen(ColumnState&);
    virtual std::u16string getAlign(ColumnState&);
    virtual std::u16string getBraces(ColumnState&);
    virtual void macroColumn(ColumnState&,std::u16string,double n);
    virtual void addRule(ColumnState&,std::u16string);
    virtual void addAt(ColumnState&,std::u16string);
    virtual void addBang(ColumnState&,std::u16string);
    virtual void repeat(ColumnState&);
    virtual void brace(ColumnState&);
};
}
