module folia.math.core.mml_tree.mml_source_allocator;
import folia.math.core.mml_tree.mml_node_context_fields;
import std;

namespace folia::math::mml {
using namespace options;

MmlSourceAllocator::MmlSourceAllocator(Context& context,
        std::shared_ptr<MmlFactory> backend,std::weak_ptr<std::atomic_bool> lease,
        SelectBackend selectBackend,ResolveNode resolveNode,ResolveProductOwner resolveProductOwner,
        BindProduct bindProduct,InitializeTreeBase initializeTreeBase,
        BeginReceiverInitialization beginReceiverInitialization,
        EndReceiverInitialization endReceiverInitialization)
    :context_(context),backend_(std::move(backend)),selectBackend_(std::move(selectBackend)),
     resolveNode_(std::move(resolveNode)),
     resolveProductOwner_(std::move(resolveProductOwner)),bindProduct_(std::move(bindProduct)),
     initializeTreeBase_(std::move(initializeTreeBase)),
     beginReceiverInitialization_(std::move(beginReceiverInitialization)),
     endReceiverInitialization_(std::move(endReceiverInitialization)),
     contextLease_(std::move(lease)){
    if(!backend_||!selectBackend_||!resolveNode_||!resolveProductOwner_||!bindProduct_||
       !initializeTreeBase_||!beginReceiverInitialization_||!endReceiverInitialization_)
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
        Value rawAttributes,Value rawChildren,SourceReceiverProvenance provenance){
    requireActive();
    (void)context_.kind(receiver); // Validate this exact receiver belongs to the Context.
    if(!descriptor||!descriptor->construct||!descriptor->initializeSource)
        throw Exception("TypeError",u"MML Source descriptor has no allocation and initialization capabilities");
    MmlNode* node=nullptr;
    std::shared_ptr<MmlFactory> productOwner;
    try{node=resolveNode_(receiver);}
    catch(const NativeIdentityResolutionFailure&){}
    if(node)productOwner=resolveProductOwner_(receiver,*node);
    const auto receiverScope=beginReceiverInitialization_(receiver,provenance,node==nullptr);
    try{initializeTreeBase_(receiver,lexicalFactory);}
    catch(...){if(receiverScope)endReceiverInitialization_(receiverScope);throw;}
    if(receiverScope)endReceiverInitialization_(receiverScope);
    if(!node){
        productOwner=selectBackend_();
        if(!productOwner)throw Exception("TypeError",u"MML Context has no active Source allocation owner");
        node=productOwner->constructInto(receiver,[&]{
            return descriptor->construct(*productOwner,tree::FactoryArgs{});
        });
        if(!node)throw Exception("TypeError",u"MML Source descriptor did not allocate a native product");
    }
    // Binding is completion-aware: retry after a later Source initializer
    // failure completes adapters on the same product without allocating again.
    bindProduct_(receiver,*node,productOwner);
    descriptor->initializeSource(*productOwner,*node,std::move(lexicalFactory),
                                 std::move(rawAttributes),std::move(rawChildren));
    return *node;
}
}
