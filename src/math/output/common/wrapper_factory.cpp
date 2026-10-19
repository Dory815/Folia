/*************************************************************
 * MathJax 4.1.3 ts/output/common/WrapperFactory.ts.
 *************************************************************/
module folia.math.output.common;
import :wrapper_factory;
import :output_jax;

namespace folia::math::output::common {

CommonWrapperFactory::CommonWrapperFactory() : Base(std::optional<tree::FactoryClassMap<CommonWrapper>>(defaultNodes())) {}
CommonWrapperFactory::CommonWrapperFactory(CommonWrapperClassMap defaults)
    : Base(std::optional<tree::FactoryClassMap<CommonWrapper>>(std::move(defaults))) {}
CommonWrapperFactory::CommonWrapperFactory(CommonOutputJax& output) : CommonWrapperFactory() { bindJax(output); }

CommonWrapperFactory::ClassConstructionScope::ClassConstructionScope(
    CommonWrapperFactory& factory,tree::FactoryClassRef<CommonWrapper> selected)
    :factory_(&factory){
    if(!selected)throw options::Exception("TypeError",u"Wrapper class descriptor is missing");
    factory_->constructionClasses_.push_back(std::move(selected));
}
CommonWrapperFactory::ClassConstructionScope::~ClassConstructionScope(){
    if(factory_)factory_->constructionClasses_.pop_back();
}
tree::FactoryClassRef<CommonWrapper> CommonWrapperFactory::selectedClass()const{
    return constructionClasses_.empty()?tree::FactoryClassRef<CommonWrapper>{}:constructionClasses_.back();
}

CommonWrapper* CommonWrapperFactory::wrap(mml::MmlNode* node,tree::FactoryArgs args) {
    return Base::wrap(node,args);
}
CommonWrapper* CommonWrapperFactory::createWrapper(std::string_view kind,tree::FactoryArgs args) {
    return Base::create(kind,args);
}
CommonWrapper* CommonWrapperFactory::adopt(std::unique_ptr<CommonWrapper> product) {
    if(!product)throw options::Exception("TypeError",u"Cannot adopt an empty CommonWrapper");
    auto& output=jax();
    const auto job=output.activeLayoutJob();
    if(!job)throw options::Exception("TypeError",u"Cannot adopt a wrapper without an active layout job");
    auto* raw=output.adoptWrapper(std::move(product));
    const auto selected=raw->selectedClass();
    if(!wrapperInstanceBinder_)
        throw options::Exception("Unimplemented",u"Common wrapper Context instance binding is not installed");
    (void)wrapperInstanceBinder_(*raw,selected,job);
    raw->runSourceInitialization();
    return raw;
}
void CommonWrapperFactory::bindWrapperInstances(WrapperInstanceBinder binder,WrapperIdentityLookup lookup){
    if(!binder||!lookup)throw options::Exception("TypeError",u"Common wrapper identity callbacks must be callable");
    wrapperInstanceBinder_=std::move(binder);
    wrapperIdentityLookup_=std::move(lookup);
}
options::ObjectRef CommonWrapperFactory::objectFor(const CommonWrapper& wrapper)const{
    if(!wrapperIdentityLookup_)throw options::Exception("Unimplemented",u"Common wrapper Context identity lookup is not installed");
    return wrapperIdentityLookup_(wrapper);
}
void CommonWrapperFactory::bindJax(CommonOutputJax& output) {
    jaxValue=options::Value(output.instanceIdentity());
}
CommonOutputJax& CommonWrapperFactory::jax()const {
    if(!jaxResolver_)
        throw options::Exception("Unimplemented",u"CommonWrapperFactory.jax has no canonical output-Jax resolver");
    return jaxResolver_(jaxValue);
}
tree::FactoryCreatorView<CommonWrapper> CommonWrapperFactory::wrappers() const {
    // AbstractFactory exposes this exact ordinary creator object, not a copied
    // map of registered wrapper classes.
    return static_cast<const Base&>(*this).creatorView();
}
std::vector<std::string> CommonWrapperFactory::getKinds() const { return Base::getKinds(); }
CommonWrapperClassRef CommonWrapperFactory::getWrapperClass(std::string_view kind) const { return Base::getNodeClass(kind); }
void CommonWrapperFactory::setWrapperClass(std::string kind,CommonWrapperClassRef wrapperClass) {
    Base::setNodeClass(std::move(kind),std::move(wrapperClass));
}
CommonWrapperClassMap CommonWrapperFactory::defaultNodes() { return {}; }

}
