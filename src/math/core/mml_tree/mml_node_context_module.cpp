module folia.math.core.mml_tree.mml_node_context_module;
import folia.math.util.property_host;
import std;

namespace folia::math::mml {
using namespace options;
MmlNodeContextModule::MmlNodeContextModule(Context& context)
    :context_(context),lease_(std::make_shared<std::atomic_bool>(true)),
     functions_(context),properties_(context,functions_),
     attributes_(context,functions_,properties_,lease_){}
MmlNodeContextModule::~MmlNodeContextModule(){
    lease_->store(false,std::memory_order_release);
}
void MmlNodeContextModule::active()const{
    if(!lease_->load(std::memory_order_acquire))
        throw Exception("TypeError",u"MML Context module has expired");
}
Context& MmlNodeContextModule::context()const{active();return context_;}
ContextPropertyBindings& MmlNodeContextModule::properties(){active();return properties_;}
AttributesContextBindings& MmlNodeContextModule::attributes(){active();return attributes_;}
tree::TreeNodeContextClasses<MmlNode>& MmlNodeContextModule::treeClasses(){
    active();if(!treeClasses_)throw Exception("TypeError",u"MML tree classes have not been attached to Context identities");
    return *treeClasses_;
}
void MmlNodeContextModule::initializeTreeClasses(tree::TreeNodeContextBindings<MmlNode>& bindings,std::weak_ptr<std::atomic_bool> contextLease){
    active();
    if(treeClasses_){
        if(attachedBindings_!=&bindings)throw Exception("TypeError",u"MML tree classes already use another Context identity registry");
        return;
    }
    auto classes=std::make_unique<tree::TreeNodeContextClasses<MmlNode>>(context_,properties_,bindings,std::move(contextLease));
    treeClasses_=std::move(classes);attachedBindings_=&bindings;
}
void MmlNodeContextModule::detachContextBindings(tree::TreeNodeContextBindings<MmlNode>& bindings)noexcept{
    if(attachedBindings_!=&bindings)return;
    sourceMethods_.reset();treeClasses_.reset();attachedBindings_=nullptr;
}
std::weak_ptr<std::atomic_bool> MmlNodeContextModule::lease()const{return lease_;}
MmlNodeContextPrototypes MmlNodeContextModule::initializeSourceMethods(
        NativeIdentityBindings<MmlNode*>& identities,
        tree::TreeNodeContextBindings<MmlNode>& treeNodes,
        std::weak_ptr<std::atomic_bool> contextLease,
        std::function<bool(Value)> isAbstractMmlNode,
        std::function<bool(Value)> isTextNode){
    active();
    if(!treeClasses_)throw Exception("TypeError",u"MML tree classes must be initialized before Source methods");
    if(!sourceMethods_){
        auto sourceChildren=[](tree::FactoryChildren<MmlNode> children)->Value{
            if(!children)return Null{};
            if(const auto source=children->sourceValue())return *source;
            throw Exception("TypeError",u"MML Source method requires the receiver's Context child array");
        };
        sourceMethods_=std::make_unique<MmlNodeContextMethods>(context_,identities,properties_,
            attributes_,treeNodes,*treeClasses_,std::move(sourceChildren),
            std::move(isAbstractMmlNode),std::move(contextLease),std::move(isTextNode));
    }
    return MmlNodeContextPrototypes{sourceMethods_->nodePrototype(),
        sourceMethods_->mmlNodePrototype(),sourceMethods_->tokenPrototype()};
}
}
