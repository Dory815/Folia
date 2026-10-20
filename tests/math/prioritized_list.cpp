#include "support/folia_test.hpp"
import folia.math.util.prioritized_list;

using namespace boost::ut;
using folia::math::PrioritizedList;

namespace {
template<class T> std::vector<T> order(PrioritizedList<T> const& list) {
    std::vector<T> result;
    for (auto entry : list) result.push_back(entry.item);
    return result;
}
}

suite prioritized_list = [] {
    "priority insertion is stable and removes the last matching item"_test = [] {
        PrioritizedList<std::string> list;
        expect(list.add("a") == "a");
        list.add("b", 1); list.add("c", 5); list.add("d", -2); list.add("e", 1);
        expect(order(list) == std::vector<std::string>{"d", "b", "e", "a", "c"});
        list.add("a", 8);
        expect(&list.remove("a") == &list);
        expect(order(list) == std::vector<std::string>{"d", "b", "e", "a", "c"});
        expect(&list.remove("missing") == &list);
        PrioritizedList<std::string> numeric;
        numeric.add("a", 5); numeric.add("n", std::numeric_limits<double>::quiet_NaN());
        numeric.add("l", -10); numeric.add("i", std::numeric_limits<double>::infinity());
        numeric.add("m", -std::numeric_limits<double>::infinity());
        expect(order(numeric) == std::vector<std::string>{"a", "n", "m", "l", "i"});
    };

};
