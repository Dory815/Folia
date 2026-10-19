#include "support/folia_test.hpp"
import folia.math.util.linked_list;
import folia.math.util.runtime_modules;

namespace {using namespace boost::ut;
namespace options=folia::math::options;
using folia::math::util::LinkedList;
using folia::math::util::LinkedListModule;
using folia::math::util::RuntimeModules;
suite linked_list_lifetime=[] {

"linked_list_reclaims_arena_after_all_list_node_and_iterator_leases_drop"_test = [] {
    options::Context context;LinkedListModule module(context);
    std::weak_ptr<void> arena;
    std::weak_ptr<LinkedList<int>::Item> sentinel;
    std::optional<LinkedList<int>::Iterator> iterator;
    {
        LinkedList<int> list(module.end(),1,2,3);
        auto sentinelLease=list.sentinel();
        arena=sentinelLease.weakArena();
        sentinel=sentinelLease.weakItem();
        iterator.emplace(list.begin());
        list.clear();
        expect(arena.expired()==false);
    }
    expect(arena.expired()==false);
    expect(sentinel.expired()==false);
    iterator.reset();
    expect(arena.expired()==true);
    expect(sentinel.expired()==true);
};

"runtime_modules_cache_linked_list_end_once_per_context_and_source_identity"_test = [] {
    options::Context context,otherContext;
    RuntimeModules modules(context),otherModules(otherContext);
    const auto sourceModule=modules.getOrCreate<LinkedListModule>(u"@mathjax/src/ts/util/LinkedList",[&]{
        return std::make_shared<LinkedListModule>(context);
    });
    const auto sameModule=modules.getOrCreate<LinkedListModule>(u"@mathjax/src/ts/util/LinkedList",[&]{
        return std::make_shared<LinkedListModule>(context);
    });
    const auto otherModule=otherModules.getOrCreate<LinkedListModule>(u"@mathjax/src/ts/util/LinkedList",[&]{
        return std::make_shared<LinkedListModule>(otherContext);
    });
    expect(sourceModule==sameModule);
    expect(sourceModule->end()==sameModule->end());
    expect(sourceModule->end()!=otherModule->end());
    LinkedList<int> first(sourceModule->end(),1),second(sameModule->end(),2);
    first.merge(second,[](int a,int b){return a<b;});
    expect(std::ranges::equal(first,std::array{1,2}));
    modules.clear();
    expect(modules.find<LinkedListModule>(u"@mathjax/src/ts/util/LinkedList")==nullptr);
};

"runtime_modules_do_not_cache_failed_source_initializer"_test = [] {
    options::Context context;RuntimeModules modules(context);int calls=0;
    expect(throws<options::Exception>([&]{modules.getOrCreate<int>(u"retry",[&]() -> std::shared_ptr<int>{++calls;throw options::Exception("Error",u"first failure");});}));
    auto owner=modules.getOrCreate<int>(u"retry",[&]{++calls;return std::make_shared<int>(4);});
    expect(calls==2&&*owner==4&&modules.size()==1);
};

"linked_list_iterator_stops_at_any_same_source_end_after_merge"_test = [] {
    options::Context context;LinkedListModule module(context);
    LinkedList<int> target(module.end(),1,4),source(module.end(),2,3,5);
    auto iterator=source.begin();
    expect(*iterator==2);
    target.merge(source,[](int a,int b){return a<b;});
    ++iterator;
    std::vector<int> rest;
    for(auto current=iterator;current!=source.end();++current)rest.push_back(*current);
    expect(std::ranges::equal(rest,std::array{3,4,5}));
};

"linked_list_sort_releases_replaced_sentinel_graph"_test = [] {
    options::Context context;LinkedListModule module(context);
    LinkedList<int> list(module.end(),3,1,2);
    auto oldSentinel=list.sentinel();
    auto oldArena=oldSentinel.weakArena();
    std::optional<LinkedList<int>::Iterator> oldNode{list.begin()};
    list.sort();
    oldSentinel={};
    expect(oldArena.expired()==false);
    expect(**oldNode==3);
    oldNode.reset();
    expect(oldArena.expired()==true);
    expect(std::ranges::equal(list,std::array{1,2,3}));
};

"linked_list_merge_leases_keep_moved_items_without_owner_cycles"_test = [] {
    options::Context context;LinkedListModule module(context);
    std::weak_ptr<void> arena;
    std::weak_ptr<LinkedList<int>::Item> sourceSentinel;
    std::optional<LinkedList<int>::Node> lease;
    {
        LinkedList<int> left(module.end(),1,3),right(module.end(),2,4);
        auto source=right.sentinel();
        arena=source.weakArena();
        sourceSentinel=source.weakItem();
        auto item=right.begin();
        lease.emplace(item == right.end() ? LinkedList<int>::Node{} : right.sentinel());
        left.merge(right);
        expect(std::ranges::equal(left,std::array{1,2,3,4}));
        expect(right.empty());
    }
    expect(arena.expired()==false);
    expect(sourceSentinel.expired()==false);
    lease.reset();
    expect(arena.expired()==true);
    expect(sourceSentinel.expired()==true);
};

"linked_list_mutating_sort_comparator_uses_live_original_receiver"_test = [] {
    options::Context context;LinkedListModule module(context);
    LinkedList<int> list(module.end(),3,1,2);
    std::vector<std::array<int,2>> calls;
    bool changed=false;
    list.sort([&](const int& left,const int& right){
        calls.push_back({left,right});
        if(!changed){changed=true;list.push(9);}
        return left<right;
    });
    expect(changed);
    expect(std::ranges::equal(list,std::array{1,2,3}));
    expect(!calls.empty());
};
};}
