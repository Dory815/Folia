// MathJax 4.1.3 input/tex/Tags.ts; Apache-2.0. Declarations only.
export module folia.math.input.tex.parser:tags;
export import :types;
import std;
export namespace folia::math::tex {
class Label:public NativeObject {
public:
    std::u16string tag,id;
    explicit Label(std::u16string tag=u"???",std::u16string id=u"");
};
class TagInfo:public NativeObject {
public:
    std::u16string env;
    bool taggable=false,defaultTags=false;
    options::Value tag=options::Null{};
    std::u16string tagId;
    TagFormat tagFormat=u"";
    bool noTag=false;
    std::u16string labelId;
    explicit TagInfo(std::u16string env=u"",bool taggable=false,bool defaultTags=false,
        options::Value tag=options::Null{},std::u16string tagId=u"",
        TagFormat tagFormat=u"",bool noTag=false,std::u16string labelId=u"");
};
class Tags:public NativeObject {
public:
    ParserBorrowed<ParseOptions> configuration=PropertyNull{};
    options::Value ids=options::Undefined{},allIds=options::Undefined{};
    options::Value labels=options::Undefined{},allLabels=options::Undefined{};
    bool redo=false,refUpdate=false;
    ParserShared<TagInfo> currentTag=Missing{};
    // Own function field in Source; do not replace by a virtual method.
    FunctionRef enTag;
    virtual std::u16string env()const=0;
    virtual std::u16string label()const=0;
    virtual void setLabel(std::u16string)=0;
    virtual TagFormat formatTag(options::Value)=0;
    virtual TagFormat formatRef(options::Value)=0;
    virtual std::u16string formatUrl(std::u16string,std::u16string)=0;
    virtual void autoTag()=0;
    virtual mml::NodeValue getTag(bool force=false)=0;
    virtual void clearTag()=0;
    virtual void resetTag()=0;
    virtual void reset(double offset=0)=0;
    virtual void startEquation(core::MathItem&)=0;
    virtual void finishEquation(core::MathItem&)=0;
    virtual mml::NodeValue finalize(mml::NodeValue,Property env)=0;
    virtual void start(std::u16string,bool,bool)=0;
    virtual void end()=0;
    virtual void tag(options::Value,bool)=0;
    virtual void notag()=0;
};
class AbstractTags:public Tags {
    ParserRuntime& runtime_;
    ParserShared<std::vector<TagInfoRef>> stack=Missing{};
    void makeId();
    mml::NodeValue makeTag();
protected:
    double counter=0,allCounter=0;
    ParserShared<std::vector<TagInfoRef>> history=Missing{};
    virtual bool noTag()const;
    virtual std::u16string formatId(std::u16string);
    virtual std::u16string formatNumber(double);
public:
    explicit AbstractTags(ParserRuntime&); // allocation only
    void initializeAbstractTags();
    ParserRuntime& runtime()const;
    std::u16string env()const override;
    std::u16string label()const override;
    void setLabel(std::u16string)override;
    TagFormat formatTag(options::Value)override;
    TagFormat formatRef(options::Value)override;
    std::u16string formatUrl(std::u16string,std::u16string)override;
    void autoTag()override;
    mml::NodeValue getTag(bool force=false)override;
    void clearTag()override;
    void resetTag()override;
    void reset(double offset=0)override;
    void startEquation(core::MathItem&)override;
    void finishEquation(core::MathItem&)override;
    mml::NodeValue finalize(mml::NodeValue,Property env)override;
    void start(std::u16string,bool,bool)override;
    void end()override;
    void tag(options::Value,bool)override;
    void notag()override;
};
class NoTags:public AbstractTags {
public:
    using AbstractTags::AbstractTags;
    void autoTag()override;
    mml::NodeValue getTag(bool force=false)override;
};
class AllTags:public AbstractTags {
public:
    using AbstractTags::AbstractTags;
    mml::NodeValue finalize(mml::NodeValue,Property env)override;
};
struct TagsClassInfo:NativeObject {
    options::ObjectRef constructor;
    std::function<TagsRef()> allocate;
};
class TagsFactory {
    ParserRuntime& runtime_;
    SourceStringMap<TagsClassRef> tagsMapping_;
    std::u16string defaultTags_=u"none";
public:
    options::Value OPTIONS=options::Undefined{};
    // Created only as ParserRuntime's one loaded-module factory object.
    explicit TagsFactory(ParserRuntime&); // module registration only
    TagsFactory(const TagsFactory&)=delete;
    TagsFactory&operator=(const TagsFactory&)=delete;
    void add(std::u16string,TagsClassRef);
    void addTags(TagsConfigValue);
    TagsRef create(std::u16string_view);
    void setDefault(std::u16string);
    TagsRef getDefault();
};
}
