// Complete source FunctionList declaration; body ownership is root-constrained.
export module folia.math.util.function_list;
export import folia.math.util.function_types;
import std;
export namespace folia::math {
class FunctionList:public PrioritizedList<FunctionValue> {
    using Base=PrioritizedList<FunctionValue>;
public:
    FunctionList()=default;
    virtual ~FunctionList()=default;
    FunctionList(const FunctionList&)=delete;
    FunctionList& operator=(const FunctionList&)=delete;
    FunctionList(FunctionList&&)=delete;
    FunctionList& operator=(FunctionList&&)=delete;
    void initializeFunctions(FunctionDefinitionsRef list=nullptr);
    virtual void addList(FunctionDefinitionsRef);
    virtual FunctionValue add(FunctionValue value,double priority=DEFAULTPRIORITY){return Base::add(std::move(value),priority);}
    virtual bool execute(FunctionArgs args={});
    virtual async::PromiseRef asyncExecute(const async::QueueRef&,FunctionArgs args={});
};
template<class T=FunctionList>std::shared_ptr<T> makeFunctionList(FunctionDefinitionsRef list=nullptr){
    static_assert(std::is_base_of_v<FunctionList,T>);
    auto result=std::make_shared<T>();
    result->initializeFunctions(std::move(list));
    return result;
}
}
