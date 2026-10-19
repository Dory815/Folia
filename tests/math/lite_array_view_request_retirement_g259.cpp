#include "support/folia_test.hpp"
import folia.math.adaptors.lite.lite_module;
import folia.math.adaptors.lite.node_store;
import folia.math.core.tree.property;
import folia.math.output.scene;
import folia.math.util.context_property_bindings;
import folia.math.util.regexp_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace lite=folia::math::adaptors::lite;
namespace opts=folia::math::options;
namespace scene=folia::math::output::scene;

opts::ObjectRef readArray(opts::Context& context,opts::ObjectRef node,std::u16string_view field){
    const auto value=context.get(opts::Value(node),opts::Key(std::u16string(field)));
    const auto* array=std::get_if<opts::ObjectRef>(&value.data);
    if(!array)throw std::runtime_error("Lite field did not return its Context array identity");
    return *array;
}
bool retired(opts::Context& context,opts::ObjectRef object){
    try{(void)context.kind(object);return false;}catch(const opts::Exception&){return true;}
}
}

suite lite_array_view_request_retirement_g259=[] {
    "request Lite cleanup retires only views owned by nodes created after its mark"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        auto store=lite::LiteNodeStore::create();
        lite::LiteModule module(context,functions,properties,regexp,store);

        const auto preexisting=store->create(lite::NodeKind::Element,u"div");
        const auto preexistingIdentity=module.objectFor(preexisting);
        const auto preexistingArray=readArray(context,preexistingIdentity,u"children");
        const auto lateNode=store->create(lite::NodeKind::Element,u"span");
        const auto mark=module.requestMark();

        // This node predates the request, although its Context projection is
        // first requested after the mark. Its view is therefore a persistent root.
        const auto lateIdentity=module.objectFor(lateNode);
        const auto lateArray=readArray(context,lateIdentity,u"children");
        const auto requestElement=store->create(lite::NodeKind::Element,u"g");
        const auto requestIdentity=module.objectFor(requestElement);
        const auto requestArray=readArray(context,requestIdentity,u"children");
        const auto requestList=store->create(lite::NodeKind::List,u"#list");
        const auto requestListIdentity=module.objectFor(requestList);
        const auto requestListArray=readArray(context,requestListIdentity,u"nodes");

        std::weak_ptr<PropertyStorage> weakRequestChildren;
        {
            const Property childStorageValue(store->record(requestElement).children);
            weakRequestChildren=std::get<std::shared_ptr<PropertyStorage>>(childStorageValue.value);
        }

        scene::Builder builder;
        auto path=std::make_shared<scene::Node>();
        path->kind=u"path";
        builder.addRoot(path);
        auto retainedScene=std::move(builder).finish();

        module.retireRequestNodesSince(mark);
        expect(retired(context,requestIdentity));
        expect(retired(context,requestArray));
        expect(retired(context,requestListIdentity));
        expect(retired(context,requestListArray));
        expect(weakRequestChildren.expired()) << "retiring the node and its exact view drops the request PropertyArray storage";
        expect(!retired(context,preexistingIdentity));
        expect(!retired(context,preexistingArray));
        expect(!retired(context,lateIdentity));
        expect(!retired(context,lateArray));
        expect(retainedScene.roots().size()==1_u&&retainedScene.roots()[0]->kind==u"path")
            << "Lite request cleanup leaves detached immutable scene geometry live";

        const auto nextMark=module.requestMark();
        const auto nextNode=store->create(lite::NodeKind::Element,u"math");
        const auto nextIdentity=module.objectFor(nextNode);
        const auto nextArray=readArray(context,nextIdentity,u"children");
        module.retireRequestNodesSince(nextMark);
        expect(retired(context,nextIdentity)&&retired(context,nextArray))
            << "independent request marks retire only their own Lite node projections";
    };
};

struct ForeignAttachedOwner final:opts::NativeAttachedOwner {
    bool visitContextEdges(const opts::ContextEdgeVisitor&)const override{return true;}
};

suite lite_array_view_request_retirement_retry_g259=[] {
    "Lite request mark retries the same token after earlier event detachment fails"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        auto store=lite::LiteNodeStore::create();
        lite::LiteModule module(context,functions,properties,regexp,store);
        const auto mark=module.requestMark();
        const auto node=store->create(lite::NodeKind::Element,u"g");
        const auto identity=module.objectFor(node);
        const auto children=readArray(context,identity,u"children");
        const auto callback=context.function(u"request callback",[](opts::Context&,opts::Value,
            std::span<const opts::Value>){return opts::Value(opts::Undefined{});});
        module.registerHostEvent(identity,opts::Value(u"click"),opts::Value(callback));

        const auto oldOwner=context.attachedOwner(identity);
        expect(static_cast<bool>(oldOwner));
        expect(context.clearAttachedOwner(identity,oldOwner.get()));
        auto foreignOwner=std::make_shared<ForeignAttachedOwner>();
        context.attachOwner(identity,foreignOwner);
        expect(throws<opts::Exception>([&]{module.retireRequestNodesSince(mark);}))
            << "bad event owner fails before array projections are retired";
        expect(!retired(context,identity)&&!retired(context,children));

        expect(context.clearAttachedOwner(identity,foreignOwner.get()));
        foreignOwner.reset();
        module.retireRequestNodesSince(mark);
        expect(retired(context,identity)&&retired(context,children))
            << "the same request token resumes and retires its exact view after event cleanup succeeds";
    };
};


// Draft additions for tests/math/lite_array_view_request_retirement_g259.cpp.
// Append inside the existing anonymous namespace test translation unit.
// It already imports folia_test.hpp and the Lite/Context modules; no additional imports are needed.

suite lite_array_view_source_replacement_and_mark_contract_g261=[] {
    "Source replacement arrays and their aliases are not in the Lite native-view retirement set"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        auto store=lite::LiteNodeStore::create();
        lite::LiteModule module(context,functions,properties,regexp,store);

        const auto mark=module.requestMark();
        const auto sourceNode=store->create(lite::NodeKind::Element,u"span");
        const auto sourceIdentity=module.objectFor(sourceNode);
        const auto initialChildren=readArray(context,sourceIdentity,u"children");

        // Model a Source assignment through the node's live data field. The
        // replacement is an ordinary Context Array and the alias is persistent
        // on a separate ordinary object.
        const auto replacement=context.array();
        context.arrayPush(replacement,std::array<opts::Value,1>{opts::Value(u"replacement child")});
        const auto aliasHolder=context.object();
        context.set(opts::Value(aliasHolder),opts::Key(u"savedChildren"),opts::Value(replacement));
        context.set(opts::Value(sourceIdentity),opts::Key(u"children"),opts::Value(replacement));
        const auto currentChildren=readArray(context,sourceIdentity,u"children");
        expect(static_cast<bool>(currentChildren==replacement));

        module.retireRequestNodesSince(mark);
        expect(retired(context,sourceIdentity));
        expect(retired(context,initialChildren));
        expect(!retired(context,replacement));
        expect(static_cast<bool>(context.get(opts::Value(aliasHolder),opts::Key(u"savedChildren"))==opts::Value(replacement)));
        expect(context.arrayLength(replacement)==1u);
    };

    "Lite array-view retirement preserves partial progress and retries the same mark"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        auto store=lite::LiteNodeStore::create();
        lite::LiteModule module(context,functions,properties,regexp,store);

        const auto mark=module.requestMark();
        const auto firstNode=store->create(lite::NodeKind::Element,u"first");
        const auto firstIdentity=module.objectFor(firstNode);
        const auto firstArray=readArray(context,firstIdentity,u"children");
        const auto secondNode=store->create(lite::NodeKind::Element,u"second");
        const auto secondIdentity=module.objectFor(secondNode);
        const auto secondArray=readArray(context,secondIdentity,u"children");

        // Retirement walks the registry in creation order. The second record
        // is made stale so the first succeeds before cleanup reaches the error.
        context.retireObjectRecord(secondArray);
        std::optional<std::string> firstName,secondName;
        std::optional<std::u16string> firstMessage,secondMessage;
        try{module.retireRequestNodesSince(mark);}
        catch(const opts::Exception& error){firstName=error.name;firstMessage=error.message;}
        expect(static_cast<bool>(firstName&&*firstName=="TypeError"));
        expect(firstMessage.has_value());
        expect(retired(context,firstArray));
        expect(retired(context,secondArray));
        // Node records were not retired because array cleanup failed first.
        expect(!retired(context,firstIdentity));

        try{module.retireRequestNodesSince(mark);}
        catch(const opts::Exception& error){secondName=error.name;secondMessage=error.message;}
        expect(static_cast<bool>(secondName&&firstName&&*secondName==*firstName));
        expect(static_cast<bool>(secondMessage&&firstMessage&&*secondMessage==*firstMessage));
        expect(!retired(context,firstIdentity));
    };

    "Lite node request marks reject an outer mark after an inner mark retires"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
        auto store=lite::LiteNodeStore::create();
        lite::LiteModule module(context,functions,properties,regexp,store);

        const auto outer=module.requestMark();
        (void)module.objectFor(store->create(lite::NodeKind::Element,u"outer"));
        const auto inner=module.requestMark();
        (void)module.objectFor(store->create(lite::NodeKind::Element,u"inner"));
        module.retireRequestNodesSince(inner);
        expect(throws<opts::Exception>([&]{module.retireRequestNodesSince(outer);}))
            << "retiring an inner store generation makes the older outer mark invalid";
    };
};
