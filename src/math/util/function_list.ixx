// Complete source FunctionList declaration; body ownership is root-constrained.
export module folia.math.util.function_list;
export import folia.math.util.function_types;
export import folia.math.util.options;
import std;
export namespace folia::math {
using FunctionListItemHandle=PrioritizedListItemHandle<FunctionValue>;
using FunctionListItemsAccess=PrioritizedListItemsAccess<FunctionValue>;
using FunctionListItemsAccessRef=PrioritizedListItemsAccessRef<FunctionValue>;
// Borrowed collection view, not another array owner. Native tables and option
// arrays supply current length/indexed reads to the one addList algorithm.
class FunctionDefinitionsSource {
public:
    virtual ~FunctionDefinitionsSource()=default;
    virtual std::size_t length()=0;
    virtual FunctionDefinition read(std::size_t)=0;
};
class FunctionList:public PrioritizedList<FunctionValue> {
    using Base=PrioritizedList<FunctionValue>;
    void appendDefinitions(FunctionDefinitionsSource&,std::optional<double> defaultPriority=std::nullopt);
public:
    using StorageRef=Base::StorageRef;
    using ItemRef=FunctionListItemHandle;
    using iterator=Base::iterator;
    FunctionList()=default;
    virtual ~FunctionList()=default;
    FunctionList(const FunctionList&)=delete;
    FunctionList& operator=(const FunctionList&)=delete;
    FunctionList(FunctionList&&)=delete;
    FunctionList& operator=(FunctionList&&)=delete;
    void initializeFunctions(FunctionDefinitionsRef list=nullptr);
    virtual void addList(FunctionDefinitionsRef);
    // Typed host entry for an already captured Source array. No snapshot.
    void addList(FunctionDefinitionsSource&);
    // Context bridges pass the shared per-module PrioritizedList default while
    // reusing this class's one insertion algorithm.
    void addListWithDefault(FunctionDefinitionsRef,double);
    void addListWithDefault(FunctionDefinitionsSource&,double);
    // Preserve omittedness until the actual virtual callee resolves its default.
    virtual FunctionValue add(FunctionValue value,std::optional<double> priority=std::nullopt){return Base::add(std::move(value),priority.value_or(DEFAULTPRIORITY));}
    void remove(FunctionValue value){Base::remove(value);}
    virtual bool execute(FunctionArgs args={});
    virtual async::PromiseRef asyncExecute(const async::QueueRef&,FunctionArgs args={});
    // Context identity bindings retain and expose this exact array owner.
    // Rebinding is limited to another storage returned by the same binding
    // registry; ordinary typed calls never create a shadow collection.
    StorageRef storageRef()const{return Base::storageRef();}
    void rebindStorage(StorageRef storage);
    // Shared iteration boundary for the Context bridge. The callback performs
    // only the host-specific invocation; capture, mutation visibility, stop,
    // and item lifetime are the FunctionList's one existing algorithm.
    bool executeUsing(const std::function<bool(const ItemRef&)>& callback);
    void bindItemsAccess(FunctionListItemsAccessRef value){Base::bindItemsAccess(std::move(value));}
    FunctionListItemsAccessRef itemsAccess()const{return Base::itemsAccess();}
};
template<class T=FunctionList>std::shared_ptr<T> makeFunctionList(FunctionDefinitionsRef list=nullptr){
    static_assert(std::is_base_of_v<FunctionList,T>);
    auto result=std::make_shared<T>();
    result->initializeFunctions(std::move(list));
    return result;
}
}
