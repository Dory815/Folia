// Root test stimuli and observation encoding; no expected-result algorithm.
#include "support/folia_test.hpp"
import folia.math.util.function_list;
namespace {
using namespace boost::ut;
using namespace folia::math;
namespace a=folia::math::async;
namespace h {std::string debugText(std::u16string s){return {s.begin(),s.end()};}}
Property clone(const Property&v){if(auto x=PropertyArray::array(v)){PropertyArray r;for(std::size_t i=0;i<x->size();++i)r.push(clone(x->get(i)));return r;}if(auto x=PropertyList::object(v)){PropertyList r;for(const auto&k:x->keys())r.setOwn(k,clone(x->getOwn(k)));return r;}return v;}
bool deep(const Property&x,const Property&y){if(auto n=std::get_if<double>(&x.value))if(auto m=std::get_if<double>(&y.value))return(std::isnan(*n)&&std::isnan(*m))||(*n==*m&&std::signbit(*n)==std::signbit(*m));auto p=PropertyList::record(x),q=PropertyList::record(y);if(p&&q){if(PropertyArray::array(x).has_value()!=PropertyArray::array(y).has_value()||p->keys()!=q->keys())return false;for(const auto&k:p->keys())if(!deep(p->getOwn(k),q->getOwn(k)))return false;return true;}return x==y;}
Property value(const std::any&v){if(auto p=std::any_cast<Property>(&v))return *p;if(auto p=std::any_cast<bool>(&v))return *p;if(auto p=std::any_cast<std::exception_ptr>(&v)){try{std::rethrow_exception(*p);}catch(const tree::FactoryError&e){return PropertyList{{"error",e.name}};}}return Missing{};}
void tick(const a::QueueRef&q){bool marker=false;q->post([&]{marker=true;});while(!marker){if(!q->runOne())throw std::runtime_error("Missing microtask marker");}}
struct Probe:FunctionList{StorageRef&storage(){return itemStorage();}};
struct Scenario {
 a::QueueRef queue=std::make_shared<a::JobQueue>(),foreignQueue=std::make_shared<a::JobQueue>();
 a::PromiseRef input=a::Promise::create(queue),second=a::Promise::create(queue),foreign=a::Promise::create(foreignQueue);
 Probe list;PropertyList box{{"value",7}};std::vector<std::any>args{Property(box),Property(17)};
 PropertyArray calls;std::vector<std::shared_ptr<FunctionListItem>> recordIds;
 std::map<FunctionRef,std::string> ids;FunctionRef fa,fb,fc,fd;Probe::StorageRef old;
 std::string kind,action;bool triggered=false;
 Scenario(std::string k,std::string ac):kind(std::move(k)),action(std::move(ac)){
  foreign->resolve(false);second->catchError([](const auto&){return std::any{};});
  fd=fn("d",[](auto&){return std::any(true);});
  fb=fn("b",[this](auto&)->std::any{if(kind=="second")return second;if(kind=="continuation_throw")throw a::ThrownValue{Property(PropertyList{{"reason","continuation"}})};return true;});
  fc=fn("c",[](auto&){return std::any(true);});
  fa=fn("a",[this](auto&r)->std::any{if(!triggered){triggered=true;mutate(action);if(action=="receiver"){r.item=fd;r.priority=81;}if(action=="reentrant"){std::array<std::any,2>data{Property(box),Property(23)};list.execute(data);}}return result();});
  list.add(fa,0);list.add(fb,10);list.add(fc,20);old=list.storage();for(const auto&r:*old)rid(r);
 }
 int rid(const std::shared_ptr<FunctionListItem>&r){auto it=std::find(recordIds.begin(),recordIds.end(),r);if(it!=recordIds.end())return int(it-recordIds.begin())+1;recordIds.push_back(r);return int(recordIds.size());}
 int rid(FunctionListItem&r){for(const auto&p:recordIds)if(p.get()==&r)return rid(p);for(const auto&s:{old,list.storage()})if(s)for(const auto&p:*s)if(p.get()==&r)return rid(p);throw std::runtime_error("Unregistered callback record");}
 std::string name(const FunctionValue&v){auto p=std::get_if<FunctionRef>(&v);return p&&ids.contains(*p)?ids[*p]:"raw";}
 FunctionRef fn(std::string n,std::function<std::any(FunctionListItem&)>body){auto f=std::make_shared<Function>();ids[f]=n;f->call=[this,n,body=std::move(body)](const FunctionReceiver&receiver,FunctionArgs data){const auto record=std::dynamic_pointer_cast<FunctionListItem>(std::get<NativeObjectRef>(receiver));if(!record)throw std::runtime_error("Invalid Source priority receiver");auto&r=*record;const auto object=data.empty()?Property(Missing{}):value(data[0]);auto rec=PropertyList::record(object);calls.push(PropertyList{{"name",n},{"record",rid(r)},{"item",name(r.item)},{"priority",r.priority},{"same",object==Property(box)},{"box",rec?rec->getOwn("value"):Property(Missing{})},{"arg",data.size()>1?value(data[1]):Property(Missing{})},{"argc",int(data.size())}});return body(r);};return f;}
 std::any result(){
  if(kind=="promise"||kind=="second"||kind=="continuation_throw")return input;
  if(kind=="fulfilled"){auto p=a::Promise::create(queue);p->resolve(false);return p;}
  if(kind=="rejected"){auto p=a::Promise::create(queue);p->reject(Property(PropertyList{{"reason","input"}}));return p;}
  if(kind=="foreign")return foreign;
  if(kind=="throw")throw a::ThrownValue{Property(PropertyList{{"reason","callback"}})};
  if(kind=="typeerror")throw tree::FactoryError("TypeError",u"native");
  if(kind=="symbol")return folia::math::options::SymbolRef{7,8};
  if(kind=="thenable")return Property(PropertyList{{"then",BuiltinProperty::functionCall}});
  if(kind=="missing")return Property(Missing{});if(kind=="null")return Property(PropertyNull{});
  if(kind=="false")return Property(false);if(kind=="true")return Property(true);
  if(kind=="zero")return Property(0);if(kind=="minuszero")return Property(-0.0);
  if(kind=="nan")return Property(std::numeric_limits<double>::quiet_NaN());
  if(kind=="empty")return Property("");if(kind=="string")return Property("false");
  if(kind=="array")return Property(PropertyArray{});return Property(PropertyList{{"a",false}});
 }
 void mutate(std::string which){
  if(which=="remove_self")list.remove(fa);if(which=="remove_next")list.remove(fb);
  if(which=="append")list.add(fd,30);if(which=="insert_before")list.add(fd,-10);
  if(which=="rebind"||which=="rebind_append")list.storage()=std::make_shared<Probe::Storage>(Probe::Storage{std::make_shared<FunctionListItem>(FunctionListItem{40,fd})});
  if(which=="rebind_append")old->push_back(std::make_shared<FunctionListItem>(FunctionListItem{50,fd}));
  if(which=="clear")old->clear();if(which=="null_next")(*old)[1]=nullptr;
  if(which=="replace_next")(*old)[1]->item=fd;if(which=="priority_next")(*old)[1]->priority=-99;
  if(which=="args"){box.setOwn("value",99);args[1]=Property(77);}
 }
 Property items(const Probe::StorageRef&storage){PropertyArray r;for(const auto&p:*storage)if(p)r.push(PropertyList{{"id",rid(p)},{"item",name(p->item)},{"priority",p->priority}});else r.push(PropertyNull{});return r;}
 PropertyList state(){return {{"calls",clone(calls)},{"items",items(list.storage())},{"old",items(old)},{"box",box.getOwn("value")},{"args",value(args[1])}};}
};
Property synchronous(std::string kind,std::string action){Scenario s(kind,action);Property outcome;try{outcome=s.list.execute(s.args);}catch(...){outcome=value(a::currentReason());}auto state=s.state();state.setOwn("outcome",outcome);return state;}
Property asynchronous(std::string kind,std::string action,std::string phase){Scenario s(kind,action);Property outcome="pending";auto output=s.list.asyncExecute(s.queue,s.args);output->then([&](const auto&v){outcome=value(v);return std::any{};},[&](const auto&e){outcome=value(e);return std::any{};});PropertyArray states;auto snap=[&]{auto state=s.state();state.setOwn("outcome",clone(outcome));states.push(state);};snap();s.mutate(phase);if(kind=="promise"||kind=="second"||kind=="continuation_throw")s.input->resolve(false);for(int i=0;i<6;++i){tick(s.queue);snap();if(i==1&&kind=="second")s.second->reject(Property(PropertyList{{"reason","second"}}));}return states;}
Property construction(std::string kind){
 struct ConstructorProbe:FunctionList{
  PropertyArray calls;std::map<FunctionRef,std::string>ids;FunctionRef fa,fb,fc;FunctionDefinitionsRef defs;std::string kind;
  void addList(FunctionDefinitionsRef values)override{calls.push(PropertyArray{"list",int(values->size())});FunctionList::addList(values);}
  FunctionValue add(FunctionValue item,double priority=DEFAULTPRIORITY)override{
   auto p=std::get_if<FunctionRef>(&item);calls.push(PropertyArray{"add",p?Property(ids[*p]):Property("raw"),priority});
   if(calls.size()==2){if(kind=="live")defs->push_back(std::make_shared<FunctionTuple>(FunctionTuple{fa,-1,{}}));if(kind=="shrink")defs->resize(1);if(kind=="default")DEFAULTPRIORITY=31;}return FunctionList::add(std::move(item),priority);
  }
 };
 const double saved=FunctionList::DEFAULTPRIORITY;struct Restore{double value;~Restore(){FunctionList::DEFAULTPRIORITY=value;}}restore{saved};
 auto list=std::make_shared<ConstructorProbe>();list->kind=kind;
 list->fa=std::make_shared<Function>();list->fb=std::make_shared<Function>();list->fc=std::make_shared<Function>();list->ids={{list->fa,"a"},{list->fb,"b"},{list->fc,"c"}};
 auto defs=std::make_shared<FunctionDefinitions>();auto tuple=[&](FunctionValue f,std::optional<double>priority){return std::make_shared<FunctionTuple>(FunctionTuple{f,priority,{Property("ignored")}});};
 if(kind=="mixed")*defs={FunctionValue(list->fa),tuple(list->fb,2),tuple(list->fa,2),tuple(list->fc,{}),std::make_shared<FunctionTuple>(),FunctionValue(Property(PropertyNull{}))};
 if(kind=="numeric")*defs={tuple(list->fa,std::numeric_limits<double>::quiet_NaN()),tuple(list->fb,-std::numeric_limits<double>::infinity()),tuple(list->fc,std::numeric_limits<double>::infinity()),tuple(list->fa,-0.0),tuple(list->fb,0),tuple(list->fc,2),tuple(list->fa,2)};
 if(kind=="live"||kind=="shrink"||kind=="default")*defs={FunctionValue(list->fa),tuple(list->fb,{}),FunctionValue(list->fc)};
 list->defs=defs;list->initializeFunctions(kind=="null"?nullptr:defs);PropertyArray items;for(auto&r:*list){auto f=std::get_if<FunctionRef>(&r.item);items.push(PropertyList{{"item",f?Property(list->ids[*f]):Property("raw")},{"priority",r.priority}});}return PropertyList{{"calls",list->calls},{"items",items}};
}
Property promiseHost(std::string kind){auto q=std::make_shared<a::JobQueue>();auto first=a::Promise::create(q),second=a::Promise::create(q);PropertyArray trace,states;Property result="pending";auto output=first;auto named=[&](std::string name){return [&,name](const std::any&v){trace.push(PropertyArray{name,value(v)});return v;};};
 if(kind=="handlers"){first->then(named("a"));first->then(named("b"));first->resolve(Property(7));first->reject(Property(8));first->resolve(Property(9));first->then(named("c"));}
 if(kind=="reject"){output=first->then(named("unused"))->catchError([&](const auto&e){trace.push(PropertyArray{"catch",value(e)});return std::any(Property(11));});first->reject(Property(7));first->resolve(Property(8));}
 if(kind=="throw"){output=first->then([&](const auto&)->std::any{trace.push(PropertyArray{"throw"});throw a::ThrownValue{Property(PropertyList{{"raw",3}})};})->catchError([&](const auto&e){trace.push(PropertyArray{"catch",value(e)});return std::any(false);});first->resolve(Property(7));}
 if(kind=="adopt"){first->resolve(second);first->reject(Property(99));first->resolve(Property(19));second->resolve(Property(13));}
 if(kind=="adopt_reject"){first->resolve(second);second->reject(Property(PropertyList{{"raw",5}}));}
 if(kind=="return_promise"){output=first->then([&](const auto&v){trace.push(PropertyArray{"return",value(v)});return std::any(second);});first->resolve(Property(7));second->resolve(Property(17));}
 if(kind=="self")first->resolve(first);
 if(kind=="pending")first->then(named("never"));
 if(kind=="chain"){output=first->then(named("one"))->then(named("two"));first->resolve(Property(19));}
 if(kind=="late"){first->resolve(Property(23));output=first->catchError([](const auto&){return std::any(Property(99));})->then(named("late"));}
 output->then([&](const auto&v){result=value(v);return std::any{};},[&](const auto&e){result=value(e);return std::any{};});auto snap=[&]{states.push(PropertyList{{"trace",clone(trace)},{"result",clone(result)}});};snap();for(int i=0;i<7;++i){tick(q);snap();}return states;
}
Property priorityCapture(){struct P:PrioritizedList<Property>{StorageRef&storage(){return itemStorage();}}list;list.add("a",1);list.add("b",2);auto old=list.storage();auto it=list.begin();PropertyArray events;auto next=[&]{bool done=it==list.end();Property v=Missing{};if(!done){auto r=it.record();v=PropertyList{{"item",std::get<std::string>(r->item.value)},{"priority",r->priority}};}events.push(PropertyList{{"value",v},{"done",done}});++it;};next();list.storage()=std::make_shared<P::Storage>(P::Storage{std::make_shared<PrioritizedListItem<Property>>(PrioritizedListItem<Property>{9,"new"})});old->push_back(std::make_shared<PrioritizedListItem<Property>>(PrioritizedListItem<Property>{3,"c"}));next();old->erase(old->begin());next();return events;}
Property invalid(std::string kind,std::string mode){Scenario s(kind,"none");PropertyArray calls;auto first=std::make_shared<Function>(),last=std::make_shared<Function>();first->call=[&](auto&,auto){calls.push("before");return std::any(true);};last->call=[&](auto&,auto){calls.push("after");return std::any(true);};Probe list;list.add(first,0);list.add(value(s.result()),1);list.add(last,2);if(kind=="null_storage")list.storage().reset();Property outcome="pending";try{if(mode=="sync")outcome=list.execute();else{auto output=list.asyncExecute(s.queue);output->then([&](const auto&v){outcome=value(v);return std::any{};},[&](const auto&e){outcome=value(e);return std::any{};});for(int i=0;i<5;++i)tick(s.queue);}}catch(...){outcome=value(a::currentReason());}return PropertyList{{"calls",calls},{"outcome",outcome}};}
suite function_native_host_tests=[] {
 "math_port_function_native_lifetime"_test=[] {
  auto queue=std::make_shared<a::JobQueue>();auto pause=a::Promise::create(queue);
  auto list=std::make_unique<Probe>();auto box=std::make_shared<int>(41);
  std::weak_ptr<int>weakBox=box;std::weak_ptr<Probe::Storage>weakStorage=list->storage();
  int observed=0;auto first=std::make_shared<Function>(),second=std::make_shared<Function>();
  first->call=[pause](auto&,auto){return std::any(pause);};
  second->call=[&](auto&,auto args){observed=*std::any_cast<std::shared_ptr<int>>(args[0]);return std::any(true);};
  list->add(first);list->add(second);std::vector<std::any>args{box};
  auto output=list->asyncExecute(queue,args);list.reset();args.clear();box.reset();first.reset();second.reset();
  expect(!weakStorage.expired());expect(!weakBox.expired());expect(observed==0);
  pause->resolve(false);for(int i=0;i<6;++i)tick(queue);
  expect(observed==41);expect(output->state()==a::State::fulfilled);expect(std::any_cast<bool>(output->value()));
  expect(weakStorage.expired());expect(weakBox.expired());
 };
 "math_port_function_native_reason_identity"_test=[] {
  auto queue=std::make_shared<a::JobQueue>();Probe list;auto reason=std::make_shared<int>(19);auto f=std::make_shared<Function>();
  f->call=[reason](auto&,auto)->std::any{throw a::ThrownValue{reason};};list.add(f);
  bool sync=false;try{list.execute();}catch(const a::ThrownValue&e){sync=std::any_cast<std::shared_ptr<int>>(e.value)==reason;}expect(sync);
  auto output=list.asyncExecute(queue);expect(output->state()==a::State::rejected);expect(std::any_cast<std::shared_ptr<int>>(output->value())==reason);
  auto exception=std::make_exception_ptr(tree::FactoryError("CustomError",u"unchanged"));f->call=[exception](auto&,auto)->std::any{std::rethrow_exception(exception);};
  output=list.asyncExecute(queue);const auto captured=std::any_cast<std::exception_ptr>(output->value());bool payload=false;try{std::rethrow_exception(captured);}catch(const tree::FactoryError&e){payload=e.name=="CustomError"&&e.message==u"unchanged";}expect(payload);
  // MSVC may copy a C++ exception when current_exception captures it. Once
  // boxed, the native reason's identity must survive every propagation link.
  auto forwarded=output->then();for(int i=0;i<3;++i)tick(queue);expect(std::any_cast<std::exception_ptr>(forwarded->value())==captured);
 };
 "math_port_function_native_session_teardown"_test=[] {
  auto queue=std::make_shared<a::JobQueue>();auto first=a::Promise::create(queue),second=a::Promise::create(queue);auto token=std::make_shared<int>(3);std::weak_ptr<int>weakToken=token;
  first->then([token](const auto&){return std::any{};});first->resolve(second);second->resolve(first);token.reset();
  tick(queue);expect(!weakToken.expired());std::weak_ptr<a::JobQueue>weakQueue=queue;queue.reset();
  expect(weakQueue.expired());expect(weakToken.expired());bool expired=false;try{first->state();}catch(const tree::FactoryError&e){expired=e.name=="TypeError";}expect(expired);
 };
 "math_port_function_native_iterator_owner_lifetime"_test=[] {
  auto list=std::make_unique<Probe>();auto f=std::make_shared<Function>();list->add(f,7);auto it=list->begin(),end=list->end();std::weak_ptr<Probe::Storage>storage=list->storage();list.reset();expect(!storage.expired());expect(it!=end);expect(it.record()->priority==7);++it;expect(it==end);it={};end={};expect(storage.expired());
 };
 "math_port_function_native_independent_runs"_test=[] {
  auto queue=std::make_shared<a::JobQueue>();Probe list;auto pause=a::Promise::create(queue);auto first=std::make_shared<Function>(),second=std::make_shared<Function>();std::vector<int>calls;
  first->call=[pause](auto&,auto){return std::any(pause);};second->call=[&](auto&,auto args){calls.push_back(std::any_cast<int>(args[0]));return std::any(true);};list.add(first);list.add(second);
  std::array<std::any,1>one{1},two{2};auto a1=list.asyncExecute(queue,one),a2=list.asyncExecute(queue,two);pause->resolve(false);for(int i=0;i<6;++i)tick(queue);
  expect(calls==std::vector<int>{1,2});expect(a1!=a2);expect(a1->state()==a::State::fulfilled);expect(a2->state()==a::State::fulfilled);
 };
 "math_port_function_native_invalid_callable_handles"_test=[] {
  Probe list;list.add(FunctionRef{});bool failed=false;try{list.execute();}catch(const tree::FactoryError&e){failed=e.name=="TypeError";}expect(failed);
  list.remove(FunctionRef{});list.add(std::make_shared<Function>());failed=false;try{list.execute();}catch(const tree::FactoryError&e){failed=e.name=="TypeError";}expect(failed);
  failed=false;try{list.addList(nullptr);}catch(const tree::FactoryError&e){failed=e.name=="TypeError";}expect(failed);
 };
};
}
