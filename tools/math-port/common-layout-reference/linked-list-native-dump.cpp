// Native observations from the real LinkedList and MathList owners. Root's
// comparator consumes the same cases schema as linked-list-original.json.
import std;
import folia.math.util.linked_list;
import folia.math.util.runtime_modules;
import folia.math.core.math_list;
import folia.math.core.math_item;
import folia.math.core.tree.property;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
namespace opts=folia::math::options;
namespace util=folia::math::util;
namespace core=folia::math::core;

std::string quote(std::string_view value){
    std::string out="\"";constexpr char hex[]="0123456789abcdef";
    for(const unsigned char c:value){
        switch(c){case '\\':out+="\\\\";break;case '\"':out+="\\\"";break;
        case '\n':out+="\\n";break;case '\r':out+="\\r";break;case '\t':out+="\\t";break;
        default:if(c<0x20){out+="\\u00";out.push_back(hex[c>>4]);out.push_back(hex[c&15]);}else out.push_back(static_cast<char>(c));}
    }
    return out+'\"';
}
template<class R,class F> std::string array(const R& range,F&& value){
    std::string out="[";bool first=true;for(const auto& item:range){if(!first)out+=',';first=false;out+=value(item);}return out+']';
}
std::string ints(const auto& values){return array(values,[](const auto v){return std::to_string(v);});}
std::string strings(const auto& values){return array(values,[](const auto& v){return quote(v);});}
void field(std::string& out,bool& first,std::string_view key,std::string value){
    if(!first)out+=',';first=false;out+=quote(key);out+=':';out+=std::move(value);
}
std::string boolean(bool value){return value?"true":"false";}
struct Record{int rank;char id;};
class OrderedList:public util::LinkedList<std::shared_ptr<Record>>{
public:
    std::vector<std::array<char,3>> calls;
    const OrderedList* expectedReceiver=nullptr;
    using LinkedList::LinkedList;
    bool isBefore(const std::shared_ptr<Record>& a,const std::shared_ptr<Record>& b)override{
        calls.push_back({a->id,b->id,this==expectedReceiver?'1':'0'});return a->rank<b->rank;
    }
};
std::string recordIds(const auto& list){
    std::vector<std::string> ids;for(const auto& record:list)ids.emplace_back(1,record->id);return strings(ids);
}
Property location(double i,double n){PropertyList value;value.set("i",i);value.set("n",n);return value;}
class ProbeMathItem final:public core::AbstractMathItem{
public:
    ProbeMathItem(Property start):AbstractMathItem(Property(PropertyNull{}),core::MathInputJaxReference(Missing{}),
         Property(Missing{}),core::MathLocation(std::move(start)),core::MathLocation(Property(Missing{}))){}
};
}

int main(int argc,char** argv){
    if(argc<2)return 2;
    opts::Context context;util::RuntimeModules modules(context);
    auto sourceModule=modules.getOrCreate<util::LinkedListModule>(u"@mathjax/src/ts/util/LinkedList",[&]{return std::make_shared<util::LinkedListModule>(context);});
    const auto END=sourceModule->end();
    std::vector<std::string> rows;
    auto add=[&](std::string name,std::string fields){rows.push_back("{\"name\":"+quote(name)+","+fields+"}");};

    {
        util::LinkedList<int> list(END);auto sentinel=list.sentinel();
        add("end-symbol-and-circular-sentinel","\"endIsSymbol\":"+
            boolean(std::holds_alternative<opts::SymbolRef>(sentinel->data))+",\"sentinelDataIsEnd\":"+
            boolean(std::holds_alternative<opts::SymbolRef>(sentinel->data)&&std::get<opts::SymbolRef>(sentinel->data)==END)+
            ",\"emptyLinksSelf\":"+boolean(sentinel->next==&*sentinel&&sentinel->prev==&*sentinel)+
            ",\"listHasEndSentinel\":"+boolean(list.endSymbol()==END));
    }
    {
        util::LinkedList<int> list(END);auto* sentinel=&*list.sentinel();
        auto* push=&list.push(2,3);auto* unshift=&list.unshift(-1,0);
        const auto values=ints(list);
        add("push-unshift-and-chain-identity","\"values\":"+values+",\"pushReturnsSelf\":"+boolean(push==&list)+
            ",\"unshiftReturnsSelf\":"+boolean(unshift==&list)+
            ",\"headPrevIsSentinel\":"+boolean(sentinel->next->prev==sentinel)+
            ",\"tailNextIsSentinel\":"+boolean(sentinel->prev->next==sentinel));
    }
    {
        util::LinkedList<std::string> list(END,"a","b","c");auto* sentinel=&*list.sentinel();
        auto* tail=sentinel->prev;auto* head=sentinel->next;
        const auto popped=list.pop().value();const auto shifted=list.shift().value();const auto last=list.pop().value();
        const auto emptyPop=list.pop();const auto emptyShift=list.shift();
        add("pop-shift-detach-and-empty-null","\"popped\":"+quote(popped)+",\"shifted\":"+quote(shifted)+
            ",\"last\":"+quote(last)+",\"emptyPop\":null,\"emptyShift\":null,\"tailDetached\":"+
            boolean(!tail->next&&!tail->prev)+",\"headDetached\":"+boolean(!head->next&&!head->prev)+
            ",\"empty\":"+boolean(!emptyPop&&!emptyShift&&list.empty()));
    }
    {
        auto a=std::make_shared<char>('a'),b=std::make_shared<char>('b'),c=std::make_shared<char>('c');
        util::LinkedList<std::shared_ptr<char>> list(END,a,b,a,c);auto* removed=&*list.sentinel()->next;
        auto* receiver=&list;auto* result=&list.remove(a,a);
        std::vector<std::string> values;for(const auto& value:list)values.emplace_back(1,*value);
        add("remove-map-membership-and-detach","\"values\":"+strings(values)+",\"returnsSelf\":"+boolean(result==receiver)+
            ",\"removedDetached\":"+boolean(!removed->next&&!removed->prev));
    }
    {
        util::LinkedList<int> list(END,1,2,3);auto* sentinel=&*list.sentinel();
        auto* first=sentinel->next;auto* last=sentinel->prev;auto* interior=first->next;
        auto* receiver=&list;auto* result=&list.clear();
        add("clear-detaches-sentinel-endpoints-only","\"returnsSelf\":"+boolean(result==receiver)+
            ",\"empty\":"+boolean(list.empty())+",\"firstDetached\":"+boolean(first->prev==nullptr)+
            ",\"lastDetached\":"+boolean(last->next==nullptr)+",\"interiorStillLinked\":"+
            boolean(interior->prev==first&&interior->next==last));
    }
    {
        util::LinkedList<int> list(END,1,2);auto forward=list.begin();const auto first=*forward;
        list.push(3);++forward;std::vector<int> rest;for(;forward!=list.end();++forward)rest.push_back(*forward);
        std::vector<int> reverse;for(auto i=list.rbegin();i!=list.rend();++i)reverse.push_back(*i);
        add("forward-and-reverse-iterators-see-live-tail","\"first\":"+std::to_string(first)+
            ",\"rest\":"+ints(rest)+",\"reverse\":"+ints(reverse));
    }
    {
        OrderedList list(END);list.expectedReceiver=&list;auto a=std::make_shared<Record>(Record{2,'a'}),b=std::make_shared<Record>(Record{1,'b'}),c=std::make_shared<Record>(Record{2,'c'});
        auto* result=&list.insert(a);list.insert(b).insert(c);
        std::vector<std::string> calls;for(auto call:list.calls)calls.push_back("["+quote(std::string(1,call[0]))+","+quote(std::string(1,call[1]))+",true]");
        add("default-insert-dispatches-virtual-isBefore","\"ids\":"+recordIds(list)+",\"returnsSelf\":"+boolean(result==&list)+",\"calls\":"+array(calls,[](const auto& v){return v;}));
    }
    {
        util::LinkedList<std::shared_ptr<Record>> list(END);std::vector<std::shared_ptr<Record>> items;
        for(auto [rank,id]:std::array{std::pair{2,'a'},std::pair{1,'b'},std::pair{2,'c'},std::pair{1,'d'}})items.push_back(std::make_shared<Record>(Record{rank,id}));
        list.push(items);auto old=list.sentinel();const auto* oldRaw=old.operator->();auto* result=&list.sort([](const auto& a,const auto& b){return a->rank<b->rank;});
        add("sort-is-stable-and-replaces-sentinel","\"ids\":"+recordIds(list)+",\"returnsSelf\":"+boolean(result==&list)+",\"sentinelChanged\":"+
            boolean(list.sentinel().operator->()!=oldRaw)+",\"oldSentinelStillEnd\":"+
            boolean(std::holds_alternative<opts::SymbolRef>(old->data)&&std::get<opts::SymbolRef>(old->data)==END));
    }
    {
        util::LinkedList<int> a(END,1,3,5),b(END,2,4,6);auto other=b.sentinel();auto* receiver=&a;auto* result=&a.merge(b,[](int x,int y){return x<y;});
        add("merge-mutates-both-lists-as-source","\"values\":"+ints(a)+",\"returnsReceiver\":"+boolean(result==receiver)+
            ",\"otherEmptied\":"+boolean(b.empty())+",\"otherSentinelRetained\":"+boolean(b.sentinel()==other));
    }
    {
        util::LinkedList<std::string> list(END,"a","b","c");auto iterator=list.begin();const auto first=*iterator;auto* firstNode=list.sentinel()->next;
        list.remove("a");std::string errorName,errorMessage;
        try{++iterator;(void)*iterator;}catch(const opts::Exception& error){errorName=error.name;errorMessage=property_host::debugText(error.message);}
        add("iterator-retains-current-node-after-remove","\"first\":"+quote(first)+",\"thrown\":{\"name\":"+quote(errorName)+
            ",\"message\":"+quote(errorMessage)+"},\"removedLinksAreNull\":"+boolean(!firstNode->next&&!firstNode->prev));
    }
    {
        util::LinkedList<int> target(END,1,4),source(END,2,3,5);auto iterator=source.begin();const auto first=*iterator;
        target.merge(source,[](int a,int b){return a<b;});std::vector<int> rest;std::string errorName,errorMessage;
        try{++iterator;for(;iterator!=source.end();++iterator)rest.push_back(*iterator);}
        catch(const opts::Exception& error){errorName=error.name;errorMessage=property_host::debugText(error.message);}
        catch(const std::exception& error){errorName="std::exception";errorMessage=error.what();}
        std::string thrown="null";
        if(!errorName.empty())thrown="{\"name\":"+quote(errorName)+",\"message\":"+quote(errorMessage)+"}";
        add("source-iterator-survives-merge-relinks","\"first\":"+std::to_string(first)+",\"rest\":"+ints(rest)+
            ",\"thrown\":"+thrown+",\"target\":"+ints(target)+",\"source\":"+ints(source));
    }
    {
        util::LinkedList<int> list(END,3,1,2);std::vector<std::array<int,2>> calls;bool changed=false;
        auto* receiver=&list;auto* result=&list.sort([&](int a,int b){calls.push_back({a,b});if(!changed){changed=true;list.push(9);}return a<b;});
        std::vector<std::string> callText;for(const auto& call:calls)callText.push_back("["+std::to_string(call[0])+","+std::to_string(call[1])+"]");
        add("sort-comparator-mutates-original-during-singleton-merge","\"returnsSelf\":"+boolean(result==receiver)+
            ",\"values\":"+ints(list)+",\"calls\":"+array(callText,[](const auto& v){return v;})+",\"changed\":"+boolean(changed));
    }
    {
        util::LinkedListModule& module=*sourceModule;core::AbstractMathList list(module);
        std::vector<std::unique_ptr<ProbeMathItem>> items;
        for(const auto [i,n]:std::array{std::pair{2.0,0.0},std::pair{1.0,4.0},std::pair{1.0,2.0},std::pair{1.0,2.0}})
            items.push_back(std::make_unique<ProbeMathItem>(location(i,n)));
        for(const auto& item:items)list.push(item.get());
        auto* result=&list.sort();std::vector<int> order;for(auto* item:list){const auto found=std::find_if(items.begin(),items.end(),[&](const auto& candidate){return candidate.get()==item;});order.push_back(static_cast<int>(found-items.begin()));}
        add("abstract-math-list-start-i-then-n","\"order\":"+ints(order)+",\"returnsSelf\":"+boolean(result==&list));
    }

    std::string output="{\"source\":[{\"file\":\"ts/util/LinkedList.ts\",\"sha256\":\"61cd7d5d45af4026e09aca4ef086fae3d462978164995008a024e76522e8c0af\"},{\"file\":\"ts/core/MathList.ts\",\"sha256\":\"c7b250b9d2053f9d99ea6aaccc1b2805ffd056eca29d4e6a170372347aebdae9\"}],\"cjs\":{\"cjs/util/LinkedList.js\":\"18eb268740bbbc5ae1ff2b2a05fcd7756eb332769a706f2046c2a6e38a395f6e\",\"cjs/core/MathList.js\":\"0d5b387e640a0d52afb8aa3509122fb75f8641623301d7e8a63d39aa656933bd\"},\"cases\":[";
    for(std::size_t i=0;i<rows.size();++i){if(i)output+=',';output+=rows[i];}
    output+="]}\n";
    std::ofstream file(argv[1],std::ios::binary);file.write(output.data(),static_cast<std::streamsize>(output.size()));
    return file?0:3;
}
