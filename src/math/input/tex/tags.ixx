// MathJax 4.1.3 input/tex/Tags.ts; Apache-2.0. Declarations only.
export module folia.math.input.tex.parser:tags;
export import :types;
import std;
export namespace folia::math::tex {
class Label:public NativeObject {
    options::Context* context_=nullptr;
    options::ObjectRef identity_{};
public:
    Label(options::Context&,options::ObjectRef);
    static void initialize(options::Context&,options::ObjectRef,std::span<const options::Value>);
    options::ObjectRef sourceIdentity()const{return identity_;}
    options::Value field(std::u16string_view)const;
    void setField(std::u16string_view,options::Value);
};
class TagInfo:public NativeObject {
    options::Context* context_=nullptr;
    options::ObjectRef identity_{};
public:
    TagInfo(options::Context&,options::ObjectRef);
    static void initialize(options::Context&,options::ObjectRef,std::span<const options::Value>);
    options::ObjectRef sourceIdentity()const{return identity_;}
    options::Value field(std::u16string_view)const;
    void setField(std::u16string_view,options::Value);
};
class Tags:public NativeObject,public std::enable_shared_from_this<Tags> {
protected:
    options::ObjectRef identity_{};
    friend class AbstractTags;
public:
    options::ObjectRef sourceIdentity()const{return identity_;}
    virtual options::Value env()const=0;
    virtual options::Value label()const=0;
    virtual void setLabel(options::Value)=0;
    virtual TagFormat formatTag(options::Value)=0;
    virtual TagFormat formatRef(options::Value)=0;
    virtual options::Value formatUrl(options::Value,options::Value)=0;
    virtual void autoTag()=0;
    virtual mml::NodeValue getTag(options::Value force=options::Undefined{})=0;
    virtual void clearTag()=0;
    virtual void resetTag()=0;
    virtual void reset(options::Value offset=0)=0;
    virtual void startEquation(core::MathItem&)=0;
    virtual void finishEquation(core::MathItem&)=0;
    virtual mml::NodeValue finalize(mml::NodeValue,Property env)=0;
    virtual void start(options::Value,options::Value,options::Value)=0;
    virtual void end()=0;
    virtual void tag(options::Value,options::Value)=0;
    virtual void notag()=0;
};
class AbstractTags:public Tags {
    ParserRuntime& runtime_;
    void makeId();
    mml::NodeValue makeTag();
protected:
    options::ObjectRef identity()const{return identity_;}
    virtual bool noTag()const;
    virtual options::Value formatId(options::Value);
    virtual options::Value formatNumber(options::Value);
public:
    explicit AbstractTags(ParserRuntime&); // allocation only
    void initializeAbstractTags();
    static void initializeModule(ParserRuntime&);
    ParserRuntime& runtime()const;
    options::Value field(std::u16string_view)const;
    void setField(std::u16string_view,options::Value);
    options::Value env()const override;
    options::Value label()const override;
    void setLabel(options::Value)override;
    TagFormat formatTag(options::Value)override;
    TagFormat formatRef(options::Value)override;
    options::Value formatUrl(options::Value,options::Value)override;
    void autoTag()override;
    mml::NodeValue getTag(options::Value force=options::Undefined{})override;
    void clearTag()override;
    void resetTag()override;
    void reset(options::Value offset=0)override;
    void startEquation(core::MathItem&)override;
    void finishEquation(core::MathItem&)override;
    mml::NodeValue finalize(mml::NodeValue,Property env)override;
    void start(options::Value,options::Value,options::Value)override;
    void end()override;
    void tag(options::Value,options::Value)override;
    void notag()override;
};
class NoTags:public AbstractTags {
public:
    using AbstractTags::AbstractTags;
    void autoTag()override;
    mml::NodeValue getTag(options::Value force=options::Undefined{})override;
};
class AllTags:public AbstractTags {
public:
    using AbstractTags::AbstractTags;
    mml::NodeValue finalize(mml::NodeValue,Property env)override;
};
struct TagsClassInfo:NativeObject {
    options::ObjectRef constructor;
    std::function<TagsRef()> allocate;
    std::function<void(Tags&)> initialize;
};
class TagsFactory {
    ParserRuntime& runtime_;
    SourceStringMap<TagsClassRef> tagsMapping_;
    std::u16string defaultTags_=u"none";
public:
    options::Value OPTIONS=options::Undefined{};
    // Created only as ParserRuntime's one loaded-module factory object.
    explicit TagsFactory(ParserRuntime&); // module registration only
    // One real native class descriptor and constructor for Source subclasses.
    // The allocator returns the actual derived object; common initialization
    // runs after its identity is bound, before optional derived Source fields.
    static TagsClassRef registerSubclass(ParserRuntime&,std::u16string className,
        std::function<TagsRef()> allocate,
        std::optional<options::ObjectRef> parentConstructor=std::nullopt,
        std::function<void(Tags&)> initializeDerived={});
    static TagsClassRef registerSourceSubclass(ParserRuntime&,std::u16string className,
        options::ObjectRef parentConstructor,
        std::function<void(Tags&)> initializeDerived,
        std::function<void(options::ObjectRef prototype)> installPrototypeMethods);
    TagsFactory(const TagsFactory&)=delete;
    TagsFactory&operator=(const TagsFactory&)=delete;
    // One Source module load registers the real built-in class constructors.
    // Runtime calls this only after both descriptors and identity bindings exist.
    void initializeModule(TagsClassRef noneClass,TagsClassRef allClass);
    void add(std::u16string,TagsClassRef);
    void addTags(TagsConfigValue);
    TagsRef create(std::u16string_view);
    void setDefault(std::u16string);
    TagsRef getDefault();
};
}
