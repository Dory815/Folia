#include "support/folia_test.hpp"
import folia.math.util.ordered_map_storage;
import folia.math.util.map_bindings;

namespace {
using namespace boost::ut;
namespace options=folia::math::options;
using folia::math::OrderedMapStorage;

struct SameValueZeroNumber {
    bool operator()(double left,double right)const{
        return left==right||(std::isnan(left)&&std::isnan(right));
    }
};

suite ordered_map_storage=[] {
    "ordered_map_storage_same_value_zero_and_reinsert"_test=[] {
        OrderedMapStorage<double,int,SameValueZeroNumber> storage;
        const double nan=std::numeric_limits<double>::quiet_NaN();
        storage.set(-0.0,1);
        storage.set(+0.0,2);
        storage.set(nan,3);
        storage.set(std::numeric_limits<double>::signaling_NaN(),4);
        expect(storage.size()==2_u);
        expect(storage.slotCount()==2_u);
        expect(*storage.find(0.0)==2);
        expect(*storage.find(nan)==4);
        expect(storage.erase(-0.0));
        expect(storage.size()==1_u);
        expect(storage.slotCount()==2_u);
        storage.set(0.0,5);
        expect(storage.size()==2_u);
        expect(storage.slotCount()==3_u);
        const auto reinserted=storage.entryAt(2);
        expect(reinserted.has_value());
        expect(reinserted->key==0.0);
        expect(reinserted->value==5);
    };

    "ordered_map_storage_live_cursor_and_owner_release"_test=[] {
        using Owner=std::shared_ptr<int>;
        OrderedMapStorage<std::string,Owner> storage;
        auto retained=std::make_shared<int>(1);
        storage.set("first",retained);
        retained.reset();
        auto removed=std::make_shared<int>(2);
        std::weak_ptr<int> released=removed;
        storage.set("removed",removed);
        removed.reset();

        std::vector<std::string> visited;
        for(auto current=storage.begin(),finish=storage.end();current!=finish;++current){
            const auto entry=*current;
            visited.push_back(entry.key);
            if(entry.key=="first"){
                expect(storage.erase("removed"));
                expect(released.expired());
                storage.set("later",std::make_shared<int>(3));
            }
        }
        expect(visited.size()==2_u);
        expect(visited[0]=="first");
        expect(visited[1]=="later");
        storage.clear();
        expect(storage.size()==0_u);
        expect(!storage.entryAt(0));
    };

    "ordered_map_storage_projected_context_roundtrip"_test=[] {
        using Native=OrderedMapStorage<std::u16string,std::u16string>;
        using View=options::ProjectedMapStorage<std::u16string,std::u16string>;
        auto native=std::make_shared<Native>();
        auto makeView=[&](){
            return std::make_shared<View>(native,
                [](const options::Value& value){
                    const auto* text=std::get_if<std::u16string>(&value.data);
                    if(!text)throw options::Exception("TypeError",u"Map key must be a string");
                    return *text;
                },
                [](const std::u16string& text){return options::Value(text);},
                [](const options::Value& value){
                    const auto* text=std::get_if<std::u16string>(&value.data);
                    if(!text)throw options::Exception("TypeError",u"Map value must be a string");
                    return *text;
                },
                [](const std::u16string& text){return options::Value(text);});
        };
        options::Context context;
        options::MapContextBindings bindings(context);
        const auto first=bindings.bind(makeView());
        const auto second=bindings.bind(makeView());
        expect(first==second);

        bindings.set(first,options::Value(u"from-context"),options::Value(u"context-value"));
        expect(native->find(u"from-context")!=nullptr);
        expect(*native->find(u"from-context")==u"context-value");

        native->set(u"from-native",u"native-value");
        expect(bindings.get(first,options::Value(u"from-native"))==options::Value(u"native-value"));
        expect(bindings.erase(first,options::Value(u"from-context")));
        expect(!native->has(u"from-context"));
        bindings.clear(first);
        expect(native->size()==0_u);
    };
};
}
