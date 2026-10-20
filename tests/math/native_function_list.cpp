#include "support/folia_test.hpp"
import folia.math.util.function_list;

using namespace boost::ut;
using folia::math::PrioritizedList;
using folia::math::FunctionList;

namespace {
struct Trace {
    std::vector<std::string> calls;
    FunctionList<Trace&>* list{};
    bool inserted{};
};
void callA(Trace& trace) { trace.calls.push_back("a"); }
void callB(Trace& trace) { trace.calls.push_back("b"); }
void callC(Trace& trace) { trace.calls.push_back("c"); }
void append(Trace& trace) { trace.calls.push_back("append"); trace.list->add(callC, 10); }
void removeCurrent(Trace& trace) { trace.calls.push_back("remove-current"); trace.list->remove(removeCurrent); }
void removeNext(Trace& trace) { trace.calls.push_back("remove-next"); trace.list->remove(callB); }
void insertBefore(Trace& trace) {
    trace.calls.push_back("insert-before");
    if (!trace.inserted) { trace.inserted = true; trace.list->add(callA, 0); }
}
bool stop(Trace& trace) { trace.calls.push_back("stop"); return false; }
bool continueCall(Trace& trace) { trace.calls.push_back("continue"); return true; }
void fail(Trace& trace) { trace.calls.push_back("throw"); throw std::runtime_error("processor failure"); }
template<class T> std::vector<T> order(PrioritizedList<T> const& list) {
    std::vector<T> result;
    for (auto entry : list) result.push_back(entry.item);
    return result;
}
}

suite native_function_list = [] {
    "native priority insertion is stable and removes the last matching item"_test = [] {
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

    "native processor list preserves void true false and priority ordering"_test = [] {
        FunctionList<Trace&> list;
        Trace trace{.list=&list};
        list.add(callA); list.add(continueCall, 1); list.add(callC);
        expect(list.execute(trace));
        expect(trace.calls == std::vector<std::string>{"continue", "a", "c"});
        list.add(stop, 0); trace.calls.clear();
        expect(!list.execute(trace));
        expect(trace.calls == std::vector<std::string>{"stop"});
        list.remove(stop); list.remove(continueCall); trace.calls.clear();
        expect(list.execute(trace));
        expect(trace.calls == std::vector<std::string>{"a", "c"});
    };

    "native processor execution observes append and removals at Source iterator positions"_test = [] {
        FunctionList<Trace&> appended;
        Trace trace{.list=&appended};
        appended.add(append, 1); appended.add(callB, 5);
        expect(appended.execute(trace));
        expect(trace.calls == std::vector<std::string>{"append", "b", "c"});
        FunctionList<Trace&> current;
        trace = Trace{.list=&current};
        current.add(removeCurrent, 1); current.add(callB, 2); current.add(callC, 3);
        expect(current.execute(trace));
        expect(trace.calls == std::vector<std::string>{"remove-current", "c"});
        FunctionList<Trace&> next;
        trace = Trace{.list=&next};
        next.add(removeNext, 1); next.add(callB, 2); next.add(callC, 3);
        expect(next.execute(trace));
        expect(trace.calls == std::vector<std::string>{"remove-next", "c"});
    };

    "native processor insertion before current position repeats current callback in Source order"_test = [] {
        FunctionList<Trace&> list;
        Trace trace{.list=&list};
        list.add(insertBefore, 1); list.add(callB, 5);
        expect(list.execute(trace));
        expect(trace.calls == std::vector<std::string>{"insert-before", "insert-before", "b"});
    };

    "native processor exceptions propagate with existing list entries and caller data intact"_test = [] {
        FunctionList<Trace&> list;
        Trace trace{.list=&list};
        const std::array<FunctionList<Trace&>::Item, 3> entries{{{5, callA}, {1, fail}, {5, callC}}};
        list.addList(entries);
        bool failed{};
        try { list.execute(trace); }
        catch (std::runtime_error const& error) { failed = std::string_view(error.what()) == "processor failure"; }
        expect(failed);
        expect(trace.calls == std::vector<std::string>{"throw"});
        expect(list.entries().size() == 3_u);
        list.remove(fail); trace.calls.clear();
        expect(list.execute(trace));
        expect(trace.calls == std::vector<std::string>{"a", "c"});
        expect(throws<std::invalid_argument>([&] {
            list.add(static_cast<FunctionList<Trace&>::VoidCallback>(nullptr));
        }));
    };
};
