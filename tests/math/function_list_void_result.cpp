#include "support/folia_test.hpp"
import folia.math.util.function_list_bindings;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o=folia::math::options;
suite functionListVoidResult=[] {
    "function_list_native_void_result_is_undefined_and_false_still_stops"_test=[] {
        o::Context context;
        OptionFunctionBindings functions(context);
        FunctionListContextBindings lists(context,functions);
        auto list=std::make_shared<FunctionList>();
        int voidCalls=0,falseCalls=0,afterCalls=0;
        auto noResult=std::make_shared<Function>();
        noResult->call=[&](const FunctionReceiver&,FunctionArgs)->std::any{
            ++voidCalls;return std::monostate{};
        };
        auto stop=std::make_shared<Function>();
        stop->call=[&](const FunctionReceiver&,FunctionArgs)->std::any{
            ++falseCalls;return false;
        };
        auto after=std::make_shared<Function>();
        after->call=[&](const FunctionReceiver&,FunctionArgs)->std::any{
            ++afterCalls;return true;
        };
        list->add(noResult,0);list->add(stop,1);list->add(after,2);
        const o::Value identity(lists.bind(list));
        const auto result=context.callMember(context.get(identity,u"execute"),identity,{});
        expect(result==o::Value(false));
        expect(voidCalls==1&&falseCalls==1&&afterCalls==0);
        const auto callable=functions.find(noResult);
        expect(callable.has_value());
        if(callable)expect(context.call(*callable,o::Undefined{})==o::Value(o::Undefined{}));
        expect(voidCalls==2);
    };
};
}
