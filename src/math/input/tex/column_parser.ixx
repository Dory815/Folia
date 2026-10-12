// MathJax 4.1.3 input/tex/ColumnParser.ts; Apache-2.0.
export module folia.math.input.tex.parser:column_parser;
export import :types;
import std;
export namespace folia::math::tex {
struct ColumnState:NativeObject {
    ParserBorrowed<TexParser> parser=Missing{};
    std::u16string templateText,c;
    double i=0,j=0;
    options::Value calign=options::Undefined{},cwidth=options::Undefined{};
    options::Value cspace=options::Undefined{},clines=options::Undefined{};
    options::Value cstart=options::Undefined{},cend=options::Undefined{};
    options::Value cextra=options::Undefined{},ralign=options::Undefined{};
};
class ColumnParser:public NativeObject {
    ParserRuntime& runtime_;
protected:
    virtual void processColumn(ColumnState&,std::u16string);
    virtual void setColumnAlign(ColumnState&,ArrayItem&);
    virtual void setColumnWidths(ColumnState&,ArrayItem&);
    virtual void setColumnSpacing(ColumnState&,ArrayItem&);
    virtual void setColumnLines(ColumnState&,ArrayItem&);
    virtual void setPadding(ColumnState&,ArrayItem&);
public:
    options::Value columnHandler=options::Undefined{};
    double MAXCOLUMNS=10000;
    explicit ColumnParser(ParserRuntime&); // allocation only
    void initializeColumnParser();
    ParserRuntime& runtime()const;
    virtual void process(TexParser&,std::u16string,ArrayItem&);
    virtual void getColumn(ColumnState&,std::u16string ralign,std::u16string calign=u"left");
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
