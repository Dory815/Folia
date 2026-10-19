/*************************************************************
 * MathJax 4.1.3 ts/output/common/WrapperFactory.ts.
 *************************************************************/
export module folia.math.output.common:wrapper_factory;
export import :types;
export import :wrapper;
export import folia.math.core.tree.wrapper_factory;
import std;

export namespace folia::math::output::common {
class CommonWrapperFactory
    : public tree::AbstractWrapperFactory<CommonWrapper,mml::MmlNode> {
    using Base = tree::AbstractWrapperFactory<CommonWrapper,mml::MmlNode>;
public:
    // Source-visible field: this is the only stored value. The accessor
    // resolves the current value through its canonical Context payload.
    options::Value jaxValue = options::Null{};
    using JaxResolver=std::function<CommonOutputJax&(const options::Value&)>;
private:
    JaxResolver jaxResolver_;
    std::vector<tree::FactoryClassRef<CommonWrapper>> constructionClasses_;
public:
    using WrapperInstanceBinder = std::function<options::ObjectRef(
        CommonWrapper&,tree::FactoryClassRef<CommonWrapper>,std::shared_ptr<LayoutJob>)>;
    using WrapperIdentityLookup = std::function<options::ObjectRef(const CommonWrapper&)>;
private:
    WrapperInstanceBinder wrapperInstanceBinder_;
    WrapperIdentityLookup wrapperIdentityLookup_;
public:
    class ClassConstructionScope {
        CommonWrapperFactory* factory_ = nullptr;
    public:
        ClassConstructionScope(CommonWrapperFactory&,tree::FactoryClassRef<CommonWrapper>);
        ClassConstructionScope(const ClassConstructionScope&)=delete;
        ClassConstructionScope& operator=(const ClassConstructionScope&)=delete;
        ~ClassConstructionScope();
    };
    CommonWrapperFactory();
    explicit CommonWrapperFactory(CommonWrapperClassMap defaults);
    explicit CommonWrapperFactory(CommonOutputJax&);
    virtual ~CommonWrapperFactory()=default;

    CommonWrapper* wrap(mml::MmlNode*,tree::FactoryArgs args={}) override;
    CommonWrapper* adopt(std::unique_ptr<CommonWrapper>) override;
    CommonWrapper* createWrapper(std::string_view kind,tree::FactoryArgs args={});
    template<class BuildArguments>
    CommonWrapper* createWrapperWithArguments(std::string_view kind,BuildArguments&& builder){
        return Base::createWithArguments(kind,std::forward<BuildArguments>(builder));
    }
    void bindJax(CommonOutputJax&);
    void bindJaxResolver(JaxResolver resolver){jaxResolver_=std::move(resolver);}
    void bindWrapperInstances(WrapperInstanceBinder binder,WrapperIdentityLookup lookup);
    options::ObjectRef objectFor(const CommonWrapper&) const;
    CommonOutputJax& jax()const;
    tree::FactoryClassRef<CommonWrapper> selectedClass() const;
    tree::FactoryCreatorView<CommonWrapper> wrappers() const;
    std::vector<std::string> getKinds() const;
    CommonWrapperClassRef getWrapperClass(std::string_view kind) const;
    void setWrapperClass(std::string kind,CommonWrapperClassRef);
    static CommonWrapperClassMap defaultNodes();
};
}
