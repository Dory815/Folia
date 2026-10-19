module folia.math.core.mml_tree.mml_source_allocator;
import folia.math.core.mml_tree.mml_node_context_fields;
import std;

namespace folia::math::mml {
using namespace options;

MmlSourceAllocator::MmlSourceAllocator(Context& context,
        std::shared_ptr<MmlFactory> backend,std::weak_ptr<std::atomic_bool> lease,
        ResolveNode resolveNode,BindProduct bindProduct,
        InitializeTreeBase initializeTreeBase)
    :context_(context),backend_(std::move(backend)),resolveNode_(std::move(resolveNode)),
     bindProduct_(std::move(bindProduct)),initializeTreeBase_(std::move(initializeTreeBase)),
     contextLease_(std::move(lease)){
    if(!backend_||!resolveNode_||!bindProduct_||!initializeTreeBase_)
        throw Exception("TypeError",u"MML Source allocator requires one complete Context binding service");
}
void MmlSourceAllocator::requireActive()const{
    const auto lease=contextLease_.lock();
    if(!lease||!lease->load(std::memory_order_acquire))
        throw Exception("TypeError",u"MML Context allocator has been released");
}
MmlFactory& MmlSourceAllocator::backend()const{requireActive();return *backend_;}
void MmlSourceAllocator::validate()const{requireActive();}
MmlNode& MmlSourceAllocator::initializeReceiver(ObjectRef receiver,
        const tree::FactoryClassRef<MmlNode>& descriptor,Value lexicalFactory,
        Value rawAttributes,Value rawChildren){
    requireActive();
    (void)context_.kind(receiver); // Validate this exact receiver belongs to the Context.
    if(!descriptor||!descriptor->construct||!descriptor->initializeSource)
        throw Exception("TypeError",u"MML Source descriptor has no allocation and initialization capabilities");
    MmlNode* node=nullptr;
    try{node=resolveNode_(receiver);}
    catch(const NativeIdentityResolutionFailure&){}
    initializeTreeBase_(receiver,lexicalFactory);
    if(!node){
        node=backend_->constructInto(receiver,[&]{
            return descriptor->construct(*backend_,tree::FactoryArgs{});
        });
        if(!node)throw Exception("TypeError",u"MML Source descriptor did not allocate a native product");
    }
    // Binding is completion-aware: retry after a later Source initializer
    // failure completes adapters on the same product without allocating again.
    bindProduct_(receiver,*node,backend_);
    descriptor->initializeSource(*backend_,*node,std::move(lexicalFactory),
                                 std::move(rawAttributes),std::move(rawChildren));
    return *node;
}
}
