// MathJax 4.1.3 input/tex/base/BaseItems.ts; all 26 real subclasses.
// Allocation precedes existing initializeBaseItem and derived Source phases.
export module folia.math.input.tex.parser:base_items;
export import :types;
import std;
export namespace folia::math::tex {
class StartItem:public BaseItem {
public:
    void initializeStartItem(Property global);
    std::string kind()const override;
    bool isOpen()const override;
    CheckType checkItem(BaseItem*)override;
};
class StopItem:public BaseItem {
public:
    std::string kind()const override;
    bool isClose()const override;
};
class OpenItem:public BaseItem {
public:
    static Property errors;
    std::string kind()const override;
    bool isOpen()const override;
    CheckType checkItem(BaseItem*)override;
};
class CloseItem:public BaseItem {
public:
    std::string kind()const override;
    bool isClose()const override;
};
class NullItem:public BaseItem {public:std::string kind()const override;};
class PrimeItem:public BaseItem {
public:std::string kind()const override;CheckType checkItem(BaseItem*)override;
};
class SubsupItem:public BaseItem {
public:
    static Property errors;
    std::string kind()const override;
    CheckType checkItem(BaseItem*)override; // includes Source null result
};
class OverItem:public BaseItem {
public:
    void initializeOverItem();
    std::string kind()const override;
    bool isClose()const override;
    CheckType checkItem(BaseItem*)override;
    std::string toString()const override;
};
class LeftItem:public BaseItem {
public:
    static Property errors;
    void initializeLeftItem(Property delim);
    std::string kind()const override;
    bool isOpen()const override;
    CheckType checkItem(BaseItem*)override;
};
class Middle:public BaseItem {
public:
    void initializeMiddle(Property delim,Property color);
    std::string kind()const override;
    bool isClose()const override;
};
class RightItem:public BaseItem {
public:
    void initializeRightItem(Property delim,Property color);
    std::string kind()const override;
    bool isClose()const override;
};
class BreakItem:public BaseItem {
public:
    void initializeBreakItem(Property linebreak,Property insert);
    std::string kind()const override;
    CheckType checkItem(BaseItem*)override;
};
class BeginItem:public BaseItem {
public:
    std::string kind()const override;
    bool isOpen()const override;
    CheckType checkItem(BaseItem*)override;
};
class EndItem:public BaseItem {
public:std::string kind()const override;bool isClose()const override;
};
class StyleItem:public BaseItem {
public:std::string kind()const override;CheckType checkItem(BaseItem*)override;
};
class PositionItem:public BaseItem {
public:std::string kind()const override;CheckType checkItem(BaseItem*)override;
};
class CellItem:public BaseItem {
public:std::string kind()const override;bool isClose()const override;
};
class MmlItem:public BaseItem {
public:std::string kind()const override;bool isFinal()const override;
};
class FnItem:public BaseItem {
public:std::string kind()const override;CheckType checkItem(BaseItem*)override;
};
class NotItem:public BaseItem {
protected:
    ParserShared<TokenMap> remap=Missing{};
public:
    void initializeNotItem();
    std::string kind()const override;
    CheckType checkItem(BaseItem*)override;
};
class NonscriptItem:public BaseItem {
public:std::string kind()const override;CheckType checkItem(BaseItem*)override;
};
class DotsItem:public BaseItem {
public:std::string kind()const override;CheckType checkItem(BaseItem*)override;
};
class ArrayItem:public BaseItem {
protected:
    virtual mml::NodeValue handleFrame(mml::NodeValue);
    virtual options::Value getEntry(); // actual [prefix,entry,terminator,found] array
public:
    ParserShared<tree::ChildArray<mml::MmlNode>> table=Missing{},row=Missing{};
    options::Value frame=options::Undefined{},hfill=options::Undefined{};
    options::Value arraydef=options::Undefined{},cstart=options::Undefined{};
    options::Value cend=options::Undefined{},cextra=options::Undefined{};
    options::Value ralign=options::Undefined{},breakAlign=options::Undefined{};
    Property atEnd=Missing{},templateSubs=Missing{};
    ParserBorrowed<TexParser> parser=Missing{};
    void initializeArrayItem(ParserRuntime&);
    std::string kind()const override;
    bool isOpen()const override;
    bool copyEnv()const override;
    CheckType checkItem(BaseItem*)override;
    virtual mml::NodeValue createMml();
    virtual void StartEntry();
    virtual void EndEntry();
    virtual void EndRow();
    virtual void EndTable();
    virtual void checkLines();
    virtual void addRowSpacing(std::u16string);
};
class EqnArrayItem:public ArrayItem {
protected:
    virtual void extendArray(std::u16string,double max);
    virtual void addIndentshift();
public:
    Property maxrow=Missing{};
    void initializeEqnArrayItem(FunctionArgs);
    std::string kind()const override;
    void EndEntry()override;
    void EndRow()override;
    void EndTable()override;
};
class MstyleItem:public BeginItem {
public:
    Property attrList=Missing{}; // actual supplied MML PropertyList handle
    void initializeMstyleItem(Property attr,Property name);
    std::string kind()const override;
    CheckType checkItem(BaseItem*)override;
};
class EquationItem:public BaseItem {
public:
    void initializeEquationItem(FunctionArgs);
    std::string kind()const override;
    bool isOpen()const override;
    CheckType checkItem(BaseItem*)override;
};
}
