#include "support/folia_test.hpp"
import folia.math.output.svg.scene_context_nodes;
import folia.math.output.scene;
import folia.math.util.context_property_bindings;
import folia.math.util.regexp_host;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace scene=folia::math::output::scene;
namespace svg=folia::math::output::svg;
namespace lite=folia::math::adaptors::lite;
namespace opts=folia::math::options;
using namespace folia::math::options;

suite scene_parent_projection_candidate=[] {
"lite_custom_properties_share_the_projected_context_descriptor_owner"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    auto adaptor=module.constructAdaptor();
    const auto node=adaptor->node(u"span");

    // Native properties created before projection migrate once, after the
    // built-in Lite fields, into the canonical Context object's own keys.
    adaptor->setProperty(node,"foo",Property("native-initial"));
    const auto identity=module.objectFor(node);
    expect(context.get(Value(identity),Key(u"foo"))==Value(u"native-initial"));
    auto keys=context.enumerableStringKeys(Value(identity));
    expect(keys.size()==5_u);
    if(keys.size()==5_u)expect(keys[4]==u"foo");

    // Context writes are immediately visible to the actual LiteAdaptor API.
    context.set(Value(identity),Key(u"foo"),Value(u"context-write"));
    const auto nativeValue=adaptor->getProperty(node,"foo");
    expect(std::holds_alternative<std::string>(nativeValue.value));
    if(const auto* text=std::get_if<std::string>(&nativeValue.value))
        expect(property_host::text(*text)==u"context-write");

    // Deletion and re-addition use the current Context descriptor and append
    // the property at the new insertion position; no native mirror remains.
    context.set(Value(identity),Key(u"bar"),Value(u"second"));
    context.erase(identity,Key(u"foo"));
    expect(adaptor->getProperty(node,"foo").missing());
    adaptor->setProperty(node,"foo",Property("native-readd"));
    expect(context.get(Value(identity),Key(u"foo"))==Value(u"native-readd"));
    keys=context.enumerableStringKeys(Value(identity));
    expect(keys.size()==6_u);
    if(keys.size()==6_u)expect(keys[4]==u"bar"&&keys[5]==u"foo");

    // A native append before first projection makes parent an own field only
    // after the constructor's four fields. Deep clone follows the same order.
    const auto parent=adaptor->node(u"div"),child=adaptor->node(u"i");
    adaptor->append(parent,child);
    const auto childIdentity=module.objectFor(child);
    const auto childKeys=context.enumerableStringKeys(Value(childIdentity));
    expect(childKeys.size()==5_u);
    if(childKeys.size()==5_u)expect(childKeys[0]==u"kind"&&childKeys[1]==u"attributes"&&
        childKeys[2]==u"children"&&childKeys[3]==u"styles"&&childKeys[4]==u"parent");
    const auto clonedParent=adaptor->clone(parent);
    const auto clonedChildren=lite::childArray(clonedParent);
    const auto clonedChild=lite::nodeFromProperty(clonedChildren.get(0));
    const auto clonedIdentity=module.objectFor(clonedChild);
    const auto clonedKeys=context.enumerableStringKeys(Value(clonedIdentity));
    expect(clonedKeys.size()==5_u);
    if(clonedKeys.size()==5_u)expect(clonedKeys[0]==u"kind"&&clonedKeys[1]==u"attributes"&&
        clonedKeys[2]==u"children"&&clonedKeys[3]==u"styles"&&clonedKeys[4]==u"parent");
};

"context_array_find_index_is_registered_with_source_callback_semantics"_test=[] {
    Context context;
    const auto arrayPrototype=context.intrinsics().arrayPrototype;
    const auto findIndexValue=context.get(Value(arrayPrototype),Key(u"findIndex"));
    const auto* findIndex=std::get_if<ObjectRef>(&findIndexValue.data);
    expect(findIndex!=nullptr);
    if(!findIndex)return;
    expect(context.kind(*findIndex)==ObjectKind::function);
    expect(context.get(Value(*findIndex),Key(u"name"))==Value(u"findIndex"));
    expect(context.get(Value(*findIndex),Key(u"length"))==Value(1.0));
    const auto functionLength=context.descriptor(Value(*findIndex),Key(u"length"));
    const auto methodDescriptor=context.descriptor(Value(arrayPrototype),Key(u"findIndex"));
    expect(functionLength.has_value());
    if(functionLength)expect(!functionLength->writable&&!functionLength->enumerable&&functionLength->configurable);
    expect(methodDescriptor.has_value());
    if(methodDescriptor)expect(methodDescriptor->writable&&!methodDescriptor->enumerable&&methodDescriptor->configurable);

    const auto array=context.array(2);
    context.set(Value(array),Key(u"1"),Value(u"match"));
    const auto thisArgument=context.object();
    std::vector<double> indexes;
    bool firstWasHole=false,receiverMatches=false,thisMatches=false,shapeMatches=true;
    const auto callback=context.function(u"findIndex callback",
        [&](Context& cx,Value receiver,std::span<const Value> arguments)->Value{
            shapeMatches=shapeMatches&&arguments.size()==3;
            if(arguments.size()!=3)return Value(false);
            receiverMatches=receiverMatches||receiver==Value(thisArgument);
            const auto* index=std::get_if<double>(&arguments[1].data);
            if(index)indexes.push_back(*index);
            if(index&&*index==0){
                firstWasHole=std::holds_alternative<Undefined>(arguments[0].data);
                cx.set(arguments[2],Key(u"2"),Value(u"beyond captured length"));
            }
            if(index&&*index==1){
                thisMatches=receiver==Value(thisArgument);
                return Value(arguments[0]==Value(u"match"));
            }
            return Value(false);
        });
    expect(context.callMember(*findIndex,Value(array),
        std::array<Value,2>{Value(callback),Value(thisArgument)})==Value(1.0));
    expect(firstWasHole&&receiverMatches&&thisMatches&&shapeMatches);
    expect(indexes.size()==2_u&&indexes[0]==0.0&&indexes[1]==1.0);
    expect(context.arrayLength(array)==3_u);

    const auto arrayLike=context.object();
    const auto arrayLikeThis=context.object();
    context.set(Value(arrayLike),Key(u"length"),Value(2.0));
    context.set(Value(arrayLike),Key(u"1"),Value(u"match"));
    bool genericReceiverMatches=false,genericThisMatches=false;
    const auto genericCallback=context.function(u"generic findIndex callback",
        [&](Context&,Value receiver,std::span<const Value> arguments)->Value{
            if(arguments.size()!=3)return Value(false);
            genericReceiverMatches=arguments[2]==Value(arrayLike);
            genericThisMatches=receiver==Value(arrayLikeThis);
            return Value(arguments[0]==Value(u"match"));
        });
    expect(context.callMember(*findIndex,Value(arrayLike),
        std::array<Value,2>{Value(genericCallback),Value(arrayLikeThis)})==Value(1.0));
    expect(genericReceiverMatches&&genericThisMatches);
};

"scene_children_native_array_deletion_preserves_holes_and_splice_order"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    svg::SceneContextNodes projection(context,u"SparseSceneChildren");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);
    scene::Builder builder;
    const auto parent=builder.node(u"g");
    const auto first=builder.node(u"text"),second=builder.node(u"path"),third=builder.node(u"rect");
    const auto parentHandle=projection.handle(parent);
    const auto firstHandle=projection.handle(first),secondHandle=projection.handle(second),thirdHandle=projection.handle(third);
    const auto childrenValue=context.get(Value(parentHandle),Key(u"children"));
    const auto children=std::get<ObjectRef>(childrenValue.data);
    context.arrayPush(children,std::array<Value,2>{Value(firstHandle),Value(secondHandle)});
    expect(context.get(Value(firstHandle),Key(u"parent"))==Value(Undefined{}));
    context.set(Value(firstHandle),Key(u"parent"),Value(parentHandle));

    context.erase(children,Key(u"0"));
    expect(context.arrayLength(children)==2_u);
    expect(!context.hasOwn(children,Key(u"0")));
    expect(context.get(Value(children),Key(u"1"))==Value(secondHandle));
    expect(parent->children.size()==2_u);
    expect(!parent->children.hasSlot(0));
    expect(parent->children.hasSlot(1));
    expect(!parent->children[0]);
    expect(parent->children[1]==second);
    expect(context.get(Value(firstHandle),Key(u"parent"))==Value(parentHandle));
    bool nativeHoleRejected=false;
    try{(void)parent->children.snapshot();}
    catch(const Exception& error){nativeHoleRejected=error.name=="TypeError";}
    expect(nativeHoleRejected);

    const auto splice=context.get(Value(children),Key(u"splice"));
    (void)context.callMember(splice,Value(children),std::array<Value,2>{Value(0),Value(1)});
    expect(context.arrayLength(children)==1_u);
    expect(context.hasOwn(children,Key(u"0")));
    expect(context.get(Value(children),Key(u"0"))==Value(secondHandle));
    expect(parent->children.size()==1_u&&parent->children[0]==second);
    const auto denseSnapshot=parent->children.snapshot();
    expect(denseSnapshot.size()==1_u&&denseSnapshot[0]==second);
    expect(context.get(Value(secondHandle),Key(u"parent"))==Value(Undefined{}));

    context.set(Value(children),Key(u"2"),Value(thirdHandle));
    expect(context.arrayLength(children)==3_u);
    expect(!context.hasOwn(children,Key(u"1")));
    expect(context.get(Value(children),Key(u"2"))==Value(thirdHandle));
    expect(parent->children.size()==3_u);
    expect(!parent->children.hasSlot(1));
    expect(parent->children.hasSlot(2)&&parent->children[2]==third);

    builder.addRoot(parent);
    bool nativeFreezeHoleRejected=false;
    try{(void)std::move(builder).finish();}
    catch(const Exception& error){nativeFreezeHoleRejected=error.name=="TypeError";}
    expect(nativeFreezeHoleRejected);
};

"scene_children_context_array_is_the_single_live_typed_owner"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    svg::SceneContextNodes projection(context,u"LiveRawSceneChildren");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);
    scene::Builder builder;
    const auto parent=builder.node(u"g");
    const auto first=builder.node(u"text"),second=builder.node(u"path");
    parent->children.push_back(first);
    const auto parentHandle=projection.handle(parent);
    const auto firstHandle=projection.handle(first),secondHandle=projection.handle(second);
    auto adaptor=module.bindAdaptor(module.constructAdaptor());
    auto targetScope=projection.installTargets(module,builder);
    (void)targetScope;
    const Value childNodesMethod=context.get(Value(adaptor),Key(u"childNodes"));

    const Value exposed=context.get(Value(parentHandle),Key(u"children"));
    expect(std::holds_alternative<ObjectRef>(exposed.data));
    expect(context.get(Value(parentHandle),Key(u"children"))==exposed);
    const auto array=std::get<ObjectRef>(exposed.data);
    expect(context.kind(array)==ObjectKind::array);
    expect(context.get(Value(array),Key(u"0"))==Value(firstHandle));

    context.set(Value(array),Key(u"0"),Value(Undefined{}));
    expect(parent->children.size()==1_u&&parent->children.hasSlot(0));
    expect(!parent->children[0]);
    bool undefinedRejected=false;
    try{(void)parent->children.snapshot();}
    catch(const Exception& error){undefinedRejected=error.name=="TypeError";}
    expect(undefinedRejected);

    const auto inheritedArray=context.array(1);
    const auto inheritedPrototype=context.objectWithPrototype(context.intrinsics().arrayPrototype);
    context.set(Value(inheritedPrototype),Key(u"0"),Value(firstHandle));
    context.setPrototype(inheritedArray,inheritedPrototype);
    context.set(Value(parentHandle),Key(u"children"),Value(inheritedArray));
    context.erase(inheritedArray,Key(u"0"));
    expect(!context.hasOwn(inheritedArray,Key(u"0")));
    expect(parent->children.size()==1_u&&parent->children[0]==first);
    expect(parent->children.snapshot().size()==1_u);

    const auto childNodesArray=context.array(2);
    context.set(Value(childNodesArray),Key(u"1"),Value(secondHandle));
    context.set(Value(childNodesArray),Key(context.intrinsics().symbolIterator),Value(Null{}));
    context.set(Value(parentHandle),Key(u"children"),Value(childNodesArray));
    const Value childNodesValue=context.callMember(childNodesMethod,Value(adaptor),std::array<Value,1>{Value(parentHandle)});
    const auto spread=std::get<ObjectRef>(childNodesValue.data);
    expect(spread!=childNodesArray);
    expect(context.arrayLength(spread)==2_u);
    expect(!context.hasOwn(spread,Key(u"0")));
    expect(context.hasOwn(spread,Key(u"1")));
    expect(context.get(Value(spread),Key(u"1"))==Value(secondHandle));

    const auto arrayLike=context.object();
    context.set(Value(arrayLike),Key(u"length"),Value(2.0));
    context.set(Value(arrayLike),Key(u"0"),Value(firstHandle));
    context.set(Value(parentHandle),Key(u"children"),Value(arrayLike));
    expect(context.get(Value(parentHandle),Key(u"children"))==Value(arrayLike));
    expect(parent->children.size()==2_u);
    expect(parent->children[0]==first);
    const Value arrayLikeCopy=context.callMember(childNodesMethod,Value(adaptor),std::array<Value,1>{Value(parentHandle)});
    expect(arrayLikeCopy!=Value(arrayLike));
    expect(context.get(arrayLikeCopy,Key(u"0"))==Value(firstHandle));
    expect(context.get(arrayLikeCopy,Key(u"1"))==Value(Undefined{}));
    context.set(Value(arrayLike),Key(u"1"),Value(secondHandle));
    expect(parent->children[1]==second);

    context.set(Value(parentHandle),Key(u"children"),Value(u"ab"));
    expect(parent->children.size()==2_u);
    const Value stringCopy=context.callMember(childNodesMethod,Value(adaptor),std::array<Value,1>{Value(parentHandle)});
    expect(context.arrayLength(std::get<ObjectRef>(stringCopy.data))==2_u);
    expect(context.get(stringCopy,Key(u"0"))==Value(u"a"));
    bool stringIndexRejected=false;
    try{(void)parent->children.at(0);}
    catch(const Exception& error){stringIndexRejected=error.name=="TypeError";}
    expect(stringIndexRejected);

    const auto accessorArray=context.object();
    context.set(Value(accessorArray),Key(u"length"),Value(1.0));
    context.set(Value(accessorArray),Key(u"0"),Value(secondHandle));
    const auto getter=context.function(u"children getter",
        [accessorArray](Context&,Value,std::span<const Value>){return Value(accessorArray);});
    context.define(parentHandle,Key(u"children"),Descriptor::access(getter,{}));
    expect(parent->children.size()==1_u);
    expect(parent->children[0]==second);
    context.erase(parentHandle,Key(u"children"));
    bool deletedFieldObserved=false;
    try{(void)parent->children.size();}
    catch(const Exception& error){deletedFieldObserved=error.name=="TypeError";}
    expect(deletedFieldObserved);

    const auto replacementNode=builder.node(u"g");
    replacementNode->children.push_back(first);
    const auto replacementHandle=projection.handle(replacementNode);
    const auto oldArrayValue=context.get(Value(replacementHandle),Key(u"children"));
    const auto oldArray=std::get<ObjectRef>(oldArrayValue.data);
    replacementNode->children=std::vector<scene::NodeRef>{second};
    const auto replacementArrayValue=context.get(Value(replacementHandle),Key(u"children"));
    expect(replacementArrayValue!=oldArrayValue);
    context.set(Value(oldArray),Key(u"0"),Value(Undefined{}));
    expect(replacementNode->children.size()==1_u);
    expect(replacementNode->children[0]==second);

    const auto nullSlotNode=builder.node(u"g");
    nullSlotNode->children.push_back(scene::NodeRef{});
    const auto nullSlotHandle=projection.handle(nullSlotNode);
    const auto nullSlotChildren=std::get<ObjectRef>(context.get(Value(nullSlotHandle),Key(u"children")).data);
    expect(context.hasOwn(nullSlotChildren,Key(u"0")));
    expect(context.get(Value(nullSlotChildren),Key(u"0"))==Value(Undefined{}));
};

"scene_source_lite_parent_delete_and_redefine_share_the_current_descriptor"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    svg::SceneContextNodes projection(context,u"SceneLiteParentDescriptor");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);
    auto adaptor=module.constructAdaptor();

    const auto parentLite=adaptor->node(u"div");
    const auto childLite=adaptor->node(u"span");
    adaptor->append(parentLite,childLite);
    const auto parentIdentity=module.objectFor(parentLite);
    const auto childIdentity=module.objectFor(childLite);
    scene::Builder builder;
    const auto parent=builder.node(u"div"),child=builder.node(u"span");
    parent->semantic=scene::SourceDomPrimitive{};
    child->semantic=scene::SourceDomPrimitive{};
    projection.associateSourceNode(parent,Value(parentIdentity));
    projection.associateSourceNode(child,Value(childIdentity));
    const auto store=lite::liteStore(childLite);
    auto typed=store->object(childLite);

    expect(store->field(childLite,"parent")==nodeProperty(parentLite));
    expect(typed.hasOwn("parent"));
    context.erase(childIdentity,Key(u"parent"));
    expect(!context.hasOwn(childIdentity,Key(u"parent")));
    expect(!typed.hasOwn("parent"));
    expect(store->field(childLite,"parent").missing());

    const auto getter=context.function(u"get parent",
        [](Context&,Value,std::span<const Value>){return Value(u"redefined-parent");});
    context.define(childIdentity,Key(u"parent"),Descriptor::access(getter,{}));
    expect(context.hasOwn(childIdentity,Key(u"parent")));
    expect(typed.hasOwn("parent"));
    expect(store->field(childLite,"parent")==properties.from(Value(u"redefined-parent")));

    const auto sourceOwner=store->ownerView(childLite);
    expect(sourceOwner.status==lite::OwnerViewLookup::Status::live);
    if(sourceOwner.status==lite::OwnerViewLookup::Status::live){
        expect(sourceOwner.view->hasOwnProperty("kind"));
        context.erase(childIdentity,Key(u"kind"));
        expect(!sourceOwner.view->hasOwnProperty("kind"));
        const auto kindGetter=context.function(u"get kind",
            [](Context&,Value,std::span<const Value>){return Value(u"accessor-kind");});
        context.define(childIdentity,Key(u"kind"),Descriptor::access(kindGetter,{}));
        expect(sourceOwner.view->hasOwnProperty("kind"));
        expect(store->field(childLite,"kind")==properties.from(Value(u"accessor-kind")));
    }
};

"scene_children_source_array_owner_deletes_holes_and_splices_on_the_same_array"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    svg::SceneContextNodes projection(context,u"SourceSparseSceneChildren");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);
    auto adaptor=module.constructAdaptor();

    const auto parentLite=adaptor->node(u"div");
    const auto firstLite=adaptor->node(u"span");
    const auto secondLite=adaptor->node(u"em");
    adaptor->append(parentLite,firstLite);
    adaptor->append(parentLite,secondLite);
    const auto parentIdentity=module.objectFor(parentLite);
    const auto firstIdentity=module.objectFor(firstLite);
    const auto secondIdentity=module.objectFor(secondLite);

    scene::Builder builder;
    const auto parent=builder.node(u"div"),first=builder.node(u"span"),second=builder.node(u"em");
    for(const auto& node:{parent,first,second})node->semantic=scene::SourceDomPrimitive{};
    projection.associateSourceNode(parent,Value(parentIdentity));
    projection.associateSourceNode(first,Value(firstIdentity));
    projection.associateSourceNode(second,Value(secondIdentity));

    const auto childrenValue=context.get(Value(parentIdentity),Key(u"children"));
    const auto* childrenObject=std::get_if<ObjectRef>(&childrenValue.data);
    expect(childrenObject!=nullptr);
    if(!childrenObject)return;
    const auto children=*childrenObject;
    expect(context.arrayLength(children)==2_u);
    context.erase(children,Key(u"0"));
    expect(context.arrayLength(children)==2_u);
    expect(!context.hasOwn(children,Key(u"0")));
    expect(context.get(Value(children),Key(u"1"))==Value(secondIdentity));
    expect(context.get(Value(firstIdentity),Key(u"parent"))==Value(parentIdentity));
    expect(parent->children.size()==2_u);
    expect(!parent->children.hasSlot(0)&&parent->children.hasSlot(1));
    expect(parent->children[1]==second);
    bool sourceHoleRejected=false;
    try{(void)parent->children.snapshot();}
    catch(const Exception& error){sourceHoleRejected=error.name=="TypeError";}
    expect(sourceHoleRejected);

    const auto splice=context.get(Value(children),Key(u"splice"));
    (void)context.callMember(splice,Value(children),std::array<Value,2>{Value(0),Value(1)});
    expect(context.arrayLength(children)==1_u);
    expect(context.hasOwn(children,Key(u"0")));
    expect(context.get(Value(children),Key(u"0"))==Value(secondIdentity));
    expect(context.get(Value(secondIdentity),Key(u"parent"))==Value(parentIdentity));
    expect(parent->children.size()==1_u&&parent->children[0]==second);
};

"scene_parent_state_uses_weak_structural_owner_without_scanning_children"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    svg::SceneContextNodes projection(context,u"ParentProjectionSceneNode");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);

    scene::Builder builder;
    const auto firstParent=builder.node(u"g");
    const auto secondParent=builder.node(u"g");
    const auto child=builder.node(u"text");
    const auto firstHandle=projection.handle(firstParent);
    const auto secondHandle=projection.handle(secondParent);
    const auto childHandle=projection.handle(child);
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(Undefined{}));

    builder.append(firstParent,child);
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(firstHandle));
    expect(firstParent->children.size()==1_u);
    builder.append(secondParent,child);
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(secondHandle));
    expect(firstParent->children.empty());
    expect(secondParent->children.size()==1_u);

    // Array writes change only that owner; they do not change the child's
    // independent parent field.
    const auto firstChildren=context.get(Value(firstHandle),Key(u"children"));
    const auto* firstArray=std::get_if<ObjectRef>(&firstChildren.data);
    expect(firstArray!=nullptr);
    if(!firstArray)return;
    context.arrayPush(*firstArray,std::array<Value,1>{Value(childHandle)});
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(secondHandle));
    expect(firstParent->children.size()==1_u);
    expect(secondParent->children.size()==1_u);

    // Replacing an entire list also leaves parent fields untouched.
    const auto replacementList=context.array();
    context.arrayPush(replacementList,std::array<Value,1>{Value(childHandle)});
    context.set(Value(firstHandle),Key(u"children"),Value(replacementList));
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(secondHandle));
    expect(firstParent->children.size()==1_u);

    // Explicit parent assignment is a field write and does not edit children.
    context.set(Value(childHandle),Key(u"parent"),Value(firstHandle));
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(firstHandle));
    expect(firstParent->children.size()==1_u);
    expect(secondParent->children.size()==1_u);
    context.set(Value(childHandle),Key(u"parent"),Value(false));
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(false));
    context.set(Value(childHandle),Key(u"parent"),Value(0.0));
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(0.0));
    context.set(Value(childHandle),Key(u"parent"),Value(u"plain value"));
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(u"plain value"));
    const auto arbitraryParent=context.object();
    context.set(Value(childHandle),Key(u"parent"),Value(arbitraryParent));
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(arbitraryParent));
    expect(firstParent->children.size()==1_u);
    expect(secondParent->children.size()==1_u);
    context.set(Value(childHandle),Key(u"parent"),Value(Null{}));
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(Null{}));
    context.set(Value(childHandle),Key(u"parent"),Value(Undefined{}));
    expect(context.get(Value(childHandle),Key(u"parent"))==Value(Undefined{}));

    scene::Builder unownedBuilder;
    auto ephemeralParent=unownedBuilder.node(u"g");
    auto retainedChild=unownedBuilder.node(u"text");
    unownedBuilder.append(ephemeralParent,retainedChild);
    std::weak_ptr<scene::Node> weakParent=ephemeralParent;
    ephemeralParent.reset();
    expect(weakParent.expired());
    expect(retainedChild->parent.expired());

    // Context handles preserve canonical parent identity without storing a
    // shared owner inside the child node. Once the Context goes away, a
    // retained child does not keep its former structural parent alive.
    auto contextReleasedParent=[] {
        Context scopedContext;
        OptionFunctionBindings scopedFunctions(scopedContext);
        ContextPropertyBindings scopedProperties(scopedContext,scopedFunctions);
        auto scopedRegexp=std::make_shared<regexp_host::RegExpBindings>(scopedContext);
        lite::LiteModule scopedModule(scopedContext,scopedFunctions,scopedProperties,scopedRegexp);
        svg::SceneContextNodes scopedProjection(scopedContext,u"ScopedParentProjection");
        auto scopedService=std::make_shared<int>(1);
        scopedProjection.bindServiceOwner(scopedService);
        scopedProjection.bindPropertyBindings(scopedProperties);
        scopedProjection.bindLiteModule(scopedModule);
        scene::Builder scopedBuilder;
        auto scopedParent=scopedBuilder.node(u"g");
        auto scopedChild=scopedBuilder.node(u"text");
        scopedBuilder.append(scopedParent,scopedChild);
        const auto scopedParentHandle=scopedProjection.handle(scopedParent);
        const auto scopedChildHandle=scopedProjection.handle(scopedChild);
        scopedContext.set(Value(scopedChildHandle),Key(u"parent"),Value(scopedParentHandle));
        expect(scopedContext.get(Value(scopedChildHandle),Key(u"parent"))==Value(scopedParentHandle));
        std::weak_ptr<scene::Node> weakScopedParent=scopedParent;
        scopedParent.reset();
        return std::pair{weakScopedParent,std::move(scopedChild)};
    }();
    expect(contextReleasedParent.first.expired());
    expect(contextReleasedParent.second->parent.expired());

    const auto operationParent=builder.node(u"g");
    const auto operationChild=builder.node(u"text");
    const auto operationReplacement=builder.node(u"path");
    const auto operationParentHandle=projection.handle(operationParent);
    const auto operationChildHandle=projection.handle(operationChild);
    const auto operationReplacementHandle=projection.handle(operationReplacement);
    const auto adaptorObject=module.bindAdaptor(module.constructAdaptor());
    auto targets=projection.installTargets(module,builder);
    const auto appendMethod=context.get(Value(adaptorObject),Key(u"append"));
    const auto parentMethod=context.get(Value(adaptorObject),Key(u"parent"));
    const auto replaceMethod=context.get(Value(adaptorObject),Key(u"replace"));
    const auto removeMethod=context.get(Value(adaptorObject),Key(u"remove"));
    const auto insertMethod=context.get(Value(adaptorObject),Key(u"insert"));
    const auto* appendFunction=std::get_if<ObjectRef>(&appendMethod.data);
    const auto* insertFunction=std::get_if<ObjectRef>(&insertMethod.data);
    const auto* parentFunction=std::get_if<ObjectRef>(&parentMethod.data);
    const auto* replaceFunction=std::get_if<ObjectRef>(&replaceMethod.data);
    const auto* removeFunction=std::get_if<ObjectRef>(&removeMethod.data);
    expect(appendFunction&&insertFunction&&parentFunction&&replaceFunction&&removeFunction);
    if(!appendFunction||!insertFunction||!parentFunction||!replaceFunction||!removeFunction)return;
    const std::array<Value,2> appendArguments{Value(operationParentHandle),Value(operationChildHandle)};
    (void)context.callMember(*appendFunction,Value(adaptorObject),appendArguments);
    expect(context.callMember(*parentFunction,Value(adaptorObject),
        std::array<Value,1>{Value(operationChildHandle)})==Value(operationParentHandle));
    expect(operationParent->children.size()==1_u);
    expect(context.callMember(*replaceFunction,Value(adaptorObject),
        std::array<Value,2>{Value(operationReplacementHandle),Value(operationChildHandle)})==Value(operationChildHandle));
    expect(context.get(Value(operationChildHandle),Key(u"parent"))==Value(Null{}));
    expect(context.get(Value(operationReplacementHandle),Key(u"parent"))==Value(operationParentHandle));
    expect(operationParent->children.size()==1_u&&operationParent->children[0]==operationReplacement);
    (void)context.callMember(*removeFunction,Value(adaptorObject),
        std::array<Value,1>{Value(operationReplacementHandle)});
    expect(context.get(Value(operationReplacementHandle),Key(u"parent"))==Value(Null{}));
    expect(operationParent->children.empty());

    // LiteAdaptor reads parent.children dynamically even when parent is an
    // ordinary Context object rather than a Scene handle.
    const auto foreignParent=context.object();
    const auto foreignChildren=context.array();
    context.arrayPush(foreignChildren,std::array<Value,1>{Value(operationChildHandle)});
    context.set(Value(foreignParent),Key(u"children"),Value(foreignChildren));
    context.set(Value(operationChildHandle),Key(u"parent"),Value(foreignParent));
    (void)context.callMember(*removeFunction,Value(adaptorObject),
        std::array<Value,1>{Value(operationChildHandle)});
    expect(context.arrayLength(foreignChildren)==0_u);
    expect(context.get(Value(operationChildHandle),Key(u"parent"))==Value(Null{}));

    // An own findIndex method is observed and called with the Source callback
    // shape (the callback itself receives only the current indexed value).
    const auto customFindIndexParent=context.object();
    const auto customFindIndexChildren=context.array();
    context.arrayPush(customFindIndexChildren,std::array<Value,1>{Value(operationChildHandle)});
    const auto customFindIndex=context.function(u"custom findIndex",
        [](Context& cx,Value receiver,std::span<const Value> arguments)->Value{
            const auto* array=std::get_if<ObjectRef>(&receiver.data);
            if(!array||arguments.empty())throw Exception("TypeError",u"Invalid custom findIndex call");
            const auto length=cx.arrayLength(*array);
            for(std::uint32_t index=0;index<length;++index){
                const std::array<Value,1> callbackArguments{cx.get(receiver,Key(property_host::text(std::to_string(index))))};
                if(opts::truthy(cx.callMember(arguments[0],Value(Undefined{}),callbackArguments)))
                    return Value(static_cast<double>(index));
            }
            return Value(-1);
        });
    context.set(Value(customFindIndexChildren),Key(u"findIndex"),Value(customFindIndex));
    context.set(Value(customFindIndexParent),Key(u"children"),Value(customFindIndexChildren));
    context.set(Value(operationChildHandle),Key(u"parent"),Value(customFindIndexParent));
    (void)context.callMember(*removeFunction,Value(adaptorObject),
        std::array<Value,1>{Value(operationChildHandle)});
    expect(context.arrayLength(customFindIndexChildren)==0_u);
    expect(context.get(Value(operationChildHandle),Key(u"parent"))==Value(Null{}));

    const auto foreignInsertParent=context.object();
    const auto foreignInsertChildren=context.array();
    const auto foreignSibling=builder.node(u"em");
    const auto foreignSiblingHandle=projection.handle(foreignSibling);
    context.arrayPush(foreignInsertChildren,std::array<Value,1>{Value(foreignSiblingHandle)});
    context.set(Value(foreignInsertParent),Key(u"children"),Value(foreignInsertChildren));
    context.set(Value(foreignSiblingHandle),Key(u"parent"),Value(foreignInsertParent));
    (void)context.callMember(*insertFunction,Value(adaptorObject),
        std::array<Value,2>{Value(operationChildHandle),Value(foreignSiblingHandle)});
    expect(context.arrayLength(foreignInsertChildren)==2_u);
    expect(context.get(Value(foreignInsertChildren),Key(u"0"))==Value(operationChildHandle));
    expect(context.get(Value(foreignInsertChildren),Key(u"1"))==Value(foreignSiblingHandle));
    expect(context.get(Value(operationChildHandle),Key(u"parent"))==Value(foreignInsertParent));

    const auto foreignReplaceParent=context.object();
    const auto foreignReplaceChildren=context.array();
    context.arrayPush(foreignReplaceChildren,std::array<Value,1>{Value(operationChildHandle)});
    context.set(Value(foreignReplaceParent),Key(u"children"),Value(foreignReplaceChildren));
    context.set(Value(operationChildHandle),Key(u"parent"),Value(foreignReplaceParent));
    (void)context.callMember(*replaceFunction,Value(adaptorObject),
        std::array<Value,2>{Value(operationReplacementHandle),Value(operationChildHandle)});
    expect(context.arrayLength(foreignReplaceChildren)==1_u);
    expect(context.get(Value(foreignReplaceChildren),Key(u"0"))==Value(operationReplacementHandle));
    expect(context.get(Value(operationReplacementHandle),Key(u"parent"))==Value(foreignReplaceParent));
    expect(context.get(Value(operationChildHandle),Key(u"parent"))==Value(Null{}));

    const auto foreignAppendParent=context.object();
    const auto foreignAppendChildren=context.array();
    const auto movedChild=builder.node(u"text");
    const auto movedChildHandle=projection.handle(movedChild);
    context.arrayPush(foreignAppendChildren,std::array<Value,1>{Value(movedChildHandle)});
    context.set(Value(foreignAppendParent),Key(u"children"),Value(foreignAppendChildren));
    context.set(Value(movedChildHandle),Key(u"parent"),Value(foreignAppendParent));
    (void)context.callMember(*appendFunction,Value(adaptorObject),
        std::array<Value,2>{Value(operationParentHandle),Value(movedChildHandle)});
    expect(context.arrayLength(foreignAppendChildren)==0_u);
    expect(operationParent->children.size()==1_u&&operationParent->children[0]==movedChild);
    expect(context.get(Value(movedChildHandle),Key(u"parent"))==Value(operationParentHandle));

    const auto invalidParent=context.object();
    context.set(Value(invalidParent),Key(u"children"),Value(false));
    context.set(Value(operationChildHandle),Key(u"parent"),Value(invalidParent));
    bool invalidChildrenRejected=false;
    try{
        (void)context.callMember(*removeFunction,Value(adaptorObject),
            std::array<Value,1>{Value(operationChildHandle)});
    }catch(const Exception& error){invalidChildrenRejected=error.name=="TypeError";}
    expect(invalidChildrenRejected);
    expect(context.get(Value(operationChildHandle),Key(u"parent"))==Value(invalidParent));

    // Native Scene insert follows Source splice(-1, 0, child) when the
    // sibling's parent field points to a parent whose children omit it.
    const auto targetParent=builder.node(u"g");
    const auto targetFirst=builder.node(u"path");
    const auto targetLast=builder.node(u"path");
    const auto targetChild=builder.node(u"text");
    const auto targetSibling=builder.node(u"text");
    builder.append(targetParent,targetFirst);
    builder.append(targetParent,targetLast);
    const auto targetParentHandle=projection.handle(targetParent);
    const auto targetFirstHandle=projection.handle(targetFirst);
    const auto targetLastHandle=projection.handle(targetLast);
    const auto targetChildHandle=projection.handle(targetChild);
    const auto targetSiblingHandle=projection.handle(targetSibling);
    context.set(Value(targetSiblingHandle),Key(u"parent"),Value(targetParentHandle));
    (void)context.callMember(*insertFunction,Value(adaptorObject),
        std::array<Value,2>{Value(targetChildHandle),Value(targetSiblingHandle)});
    expect(targetParent->children.size()==3_u);
    expect(targetParent->children[0]==targetFirst);
    expect(targetParent->children[1]==targetChild);
    expect(targetParent->children[2]==targetLast);
    expect(context.get(Value(targetChildHandle),Key(u"parent"))==Value(targetParentHandle));
    expect(context.get(Value(targetSiblingHandle),Key(u"parent"))==Value(targetParentHandle));
    (void)targetFirstHandle;
    (void)targetLastHandle;

    // Source clone assigns parent to element children but leaves text and
    // comment children unset.
    const auto cloneSource=builder.node(u"g");
    builder.append(cloneSource,builder.node(u"path"));
    builder.append(cloneSource,builder.node(u"#text"));
    builder.append(cloneSource,builder.node(u"#comment"));
    const auto cloned=scene::cloneSceneNode(cloneSource);
    expect(cloned->children.size()==3_u);
    expect(cloned->children[0]->parent.lock()==cloned);
    expect(cloned->children[1]->parent.expired());
    expect(cloned->children[2]->parent.expired());
};

"source_lite_parent_updates_follow_adaptor_operations_not_array_mutations"_test=[] {
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto regexp=std::make_shared<regexp_host::RegExpBindings>(context);
    lite::LiteModule module(context,functions,properties,regexp);
    svg::SceneContextNodes projection(context,u"ParentProjectionSourceNode");
    auto serviceOwner=std::make_shared<int>(1);
    projection.bindServiceOwner(serviceOwner);
    projection.bindPropertyBindings(properties);
    projection.bindLiteModule(module);
    auto adaptor=module.constructAdaptor();

    // LiteAdaptor.clone assigns an own parent property to cloned element
    // children, but leaves cloned text/comment parent fields unset. The
    // Context identity must expose the same own-field distinction that the
    // typed LiteAdaptor accessors observe.
    const auto cloneSource=adaptor->node(u"div");
    const auto cloneElement=adaptor->node(u"span");
    const auto cloneText=adaptor->text(u"text");
    const auto cloneComment=adaptor->comment(u"comment");
    adaptor->append(cloneSource,cloneElement);
    adaptor->append(cloneSource,cloneText);
    adaptor->append(cloneSource,cloneComment);
    const auto cloneCopy=adaptor->clone(cloneSource);
    const auto cloneChildren=adaptor->childNodes(cloneCopy).slots;
    const auto clonedElement=std::get<lite::LiteNodeRef>(propertyNode(cloneChildren.get(0)));
    const auto clonedText=std::get<lite::LiteNodeRef>(propertyNode(cloneChildren.get(1)));
    const auto clonedComment=std::get<lite::LiteNodeRef>(propertyNode(cloneChildren.get(2)));
    const auto clonedElementIdentity=Value(module.objectFor(clonedElement));
    const auto clonedTextIdentity=Value(module.objectFor(clonedText));
    const auto clonedCommentIdentity=Value(module.objectFor(clonedComment));
    expect(context.hasOwn(clonedElementIdentity,Key(u"parent")));
    expect(context.get(clonedElementIdentity,Key(u"parent"))==Value(module.objectFor(cloneCopy)));
    expect(!context.hasOwn(clonedTextIdentity,Key(u"parent")));
    expect(!context.hasOwn(clonedCommentIdentity,Key(u"parent")));

    const auto parentLite=adaptor->node(u"div");
    const auto otherParentLite=adaptor->node(u"section");
    const auto childLite=adaptor->node(u"span");
    const auto siblingLite=adaptor->node(u"em");
    const auto lastLite=adaptor->node(u"b");
    const auto replacementLite=adaptor->node(u"strong");
    const auto parentIdentity=module.objectFor(parentLite);
    const auto otherParentIdentity=module.objectFor(otherParentLite);
    const auto childIdentity=module.objectFor(childLite);
    const auto siblingIdentity=module.objectFor(siblingLite);
    const auto lastIdentity=module.objectFor(lastLite);
    const auto replacementIdentity=module.objectFor(replacementLite);

    scene::Builder builder;
    const auto parentNode=builder.node(u"div");
    const auto otherParentNode=builder.node(u"section");
    const auto childNode=builder.node(u"span");
    const auto replacementNode=builder.node(u"strong");
    for(const auto& node:{parentNode,otherParentNode,childNode,replacementNode})
        node->semantic=scene::SourceDomPrimitive{};
    projection.associateSourceNode(parentNode,Value(parentIdentity));
    projection.associateSourceNode(otherParentNode,Value(otherParentIdentity));
    projection.associateSourceNode(childNode,Value(childIdentity));
    projection.associateSourceNode(replacementNode,Value(replacementIdentity));
    auto targets=projection.installTargets(module,builder);
    const auto adaptorObject=module.bindAdaptor(adaptor);

    // Native append uses the current Context descriptors for both fields.
    // Accessor overrides remain the visible Source behavior while the Scene
    // owner stays the backing storage for ordinary data descriptors.
    const auto accessorParentLite=adaptor->node(u"div");
    const auto accessorChildLite=adaptor->node(u"span");
    const auto accessorParentNode=builder.node(u"div");
    const auto accessorChildNode=builder.node(u"span");
    accessorParentNode->semantic=scene::SourceDomPrimitive{};
    accessorChildNode->semantic=scene::SourceDomPrimitive{};
    const auto accessorParent=module.objectFor(accessorParentLite);
    const auto accessorChild=module.objectFor(accessorChildLite);
    projection.associateSourceNode(accessorParentNode,Value(accessorParent));
    projection.associateSourceNode(accessorChildNode,Value(accessorChild));
    const auto accessorChildren=context.array();
    const auto parentSetterCalls=context.array();
    const auto parentSlot=std::make_shared<Value>(Undefined{});
    const auto childrenGetter=context.function(u"get children",[accessorChildren](Context&,Value,std::span<const Value>){return Value(accessorChildren);});
    const auto childrenSetter=context.function(u"set children",[](Context&,Value,std::span<const Value>){return Undefined{};});
    context.define(accessorParent,Key(u"children"),Descriptor::access(childrenGetter,childrenSetter,true,true));
    const auto parentGetter=context.function(u"get parent",[parentSlot](Context&,Value,std::span<const Value>){return *parentSlot;});
    const auto parentSetter=context.function(u"set parent",[parentSetterCalls,parentSlot](Context& cx,Value,std::span<const Value> args){
        *parentSlot=args.empty()?Value(Undefined{}):args.front();
        cx.arrayPush(parentSetterCalls,std::array<Value,1>{*parentSlot});return Undefined{};
    });
    context.define(accessorChild,Key(u"parent"),Descriptor::access(parentGetter,parentSetter,true,true));
    adaptor->append(accessorParentLite,accessorChildLite);
    expect(context.arrayLength(accessorChildren)==1_u);
    expect(context.get(Value(accessorChildren),Key(u"0"))==Value(accessorChild));
    expect(accessorParentNode->children.size()==1_u);
    expect(accessorParentNode->children[0]==accessorChildNode);
    expect(context.get(Value(accessorChild),Key(u"parent"))==Value(accessorParent));
    expect(context.arrayLength(parentSetterCalls)==1_u);
    expect(context.get(Value(parentSetterCalls),Key(u"0"))==Value(accessorParent));
    context.erase(accessorParent,Key(u"children"));
    expect(!context.hasOwn(Value(accessorParent),Key(u"children")));
    bool deletedChildrenObserved=false;
    try{(void)accessorParentNode->children.size();}
    catch(const Exception& error){deletedChildrenObserved=error.name=="TypeError";}
    expect(deletedChildrenObserved);
    context.erase(accessorChild,Key(u"parent"));
    expect(!context.hasOwn(Value(accessorChild),Key(u"parent")));
    expect(lite::liteStore(accessorChildLite)->field(accessorChildLite,"parent").missing());

    const auto orderedParentLite=adaptor->node(u"div");
    const auto orderedChildLite=adaptor->node(u"i");
    const auto orderedNextLite=adaptor->node(u"b");
    (void)module.objectFor(orderedParentLite);
    (void)module.objectFor(orderedChildLite);
    (void)module.objectFor(orderedNextLite);
    const auto orderedChildRecord=lite::liteStore(orderedChildLite)->object(orderedChildLite);
    const std::vector<std::string> baseFieldOrder{"kind","attributes","children","styles"};
    expect(orderedChildRecord.keys()==baseFieldOrder);
    adaptor->append(orderedParentLite,orderedChildLite);
    const auto parentValueAfterAppend=adaptor->parent(orderedChildLite);
    const auto* canonicalParent=std::get_if<lite::LiteNodeRef>(&parentValueAfterAppend);
    expect(canonicalParent&&*canonicalParent==orderedParentLite);
    adaptor->append(orderedParentLite,orderedNextLite);
    const auto nextValue=adaptor->next(orderedChildLite);
    const auto* canonicalNext=std::get_if<lite::LiteNodeRef>(&nextValue);
    expect(canonicalNext&&*canonicalNext==orderedNextLite);
    auto afterAppendOrder=baseFieldOrder;afterAppendOrder.push_back("parent");
    expect(orderedChildRecord.keys()==afterAppendOrder);
    (void)adaptor->remove(orderedChildLite);
    expect(std::holds_alternative<PropertyNull>(adaptor->parent(orderedChildLite)));
    context.erase(module.objectFor(orderedChildLite),Key(u"parent"));
    expect(orderedChildRecord.keys()==baseFieldOrder);
    adaptor->append(orderedParentLite,orderedChildLite);
    expect(orderedChildRecord.keys()==afterAppendOrder);

    // Child-read operations on a source-mapped Scene node use the canonical
    // Context array. This keeps raw indexed reads distinct from childNodes'
    // Source iterator/spread behavior, including holes.
    const auto readChildren=context.array();
    context.arrayPush(readChildren,std::array<Value,2>{Value(siblingIdentity),Value(childIdentity)});
    context.set(Value(readChildren),Key(u"length"),Value(4.0));
    context.set(Value(readChildren),Key(u"3"),Value(replacementIdentity));
    context.set(Value(parentIdentity),Key(u"children"),Value(readChildren));
    context.set(Value(childIdentity),Key(u"parent"),Value(parentIdentity));
    auto callAdaptor=[&](std::u16string_view name,std::span<const Value> arguments){
        const Value method=context.get(Value(adaptorObject),Key(std::u16string(name)));
        const auto* function=std::get_if<ObjectRef>(&method.data);
        expect(function!=nullptr);
        return function?context.call(*function,Value(adaptorObject),arguments):Value(Undefined{});
    };
    expect(callAdaptor(u"firstChild",std::array<Value,1>{Value(parentIdentity)})==Value(siblingIdentity));
    expect(callAdaptor(u"lastChild",std::array<Value,1>{Value(parentIdentity)})==Value(replacementIdentity));
    expect(callAdaptor(u"childNode",std::array<Value,2>{Value(parentIdentity),Value(2.0)})==Value(Undefined{}));
    expect(callAdaptor(u"next",std::array<Value,1>{Value(childIdentity)})==Value(Undefined{}));
    expect(callAdaptor(u"previous",std::array<Value,1>{Value(childIdentity)})==Value(siblingIdentity));
    const Value spreadValue=callAdaptor(u"childNodes",std::array<Value,1>{Value(parentIdentity)});
    const auto* spreadArray=std::get_if<ObjectRef>(&spreadValue.data);
    expect(spreadArray!=nullptr);
    if(spreadArray){
        expect(context.arrayLength(*spreadArray)==4_u);
        expect(context.hasOwn(Value(*spreadArray),Key(u"2")));
        expect(context.get(Value(*spreadArray),Key(u"2"))==Value(Undefined{}));
    }
    bool yieldedCustomChild=false;
    const auto iterator=context.object();
    const auto next=context.function(u"next",[&](Context& cx,Value,std::span<const Value>){
        const auto step=cx.object();
        cx.set(Value(step),Key(u"value"),yieldedCustomChild?Value(Undefined{}):Value(replacementIdentity));
        cx.set(Value(step),Key(u"done"),Value(yieldedCustomChild));
        yieldedCustomChild=true;
        return Value(step);
    });
    context.set(Value(iterator),Key(u"next"),Value(next));
    const auto iteratorMethod=context.function(u"iterator",[iterator](Context&,Value,std::span<const Value>){return Value(iterator);});
    context.set(Value(readChildren),Key(context.intrinsics().symbolIterator),Value(iteratorMethod));
    const Value customSpreadValue=callAdaptor(u"childNodes",std::array<Value,1>{Value(parentIdentity)});
    const auto* customSpread=std::get_if<ObjectRef>(&customSpreadValue.data);
    expect(customSpread!=nullptr);
    if(customSpread){
        expect(context.arrayLength(*customSpread)==1_u);
        expect(context.get(Value(*customSpread),Key(u"0"))==Value(replacementIdentity));
    }
    expect(callAdaptor(u"firstChild",std::array<Value,1>{Value(parentIdentity)})==Value(siblingIdentity));
    context.set(Value(childIdentity),Key(u"parent"),Value(Undefined{}));
    context.set(Value(parentIdentity),Key(u"children"),Value(context.array()));

    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(Undefined{}));
    const auto childrenValue=context.get(Value(parentIdentity),Key(u"children"));
    const auto* children=std::get_if<ObjectRef>(&childrenValue.data);
    expect(children!=nullptr);
    if(!children)return;
    context.arrayPush(*children,std::array<Value,1>{Value(childIdentity)});
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(Undefined{}));
    context.set(Value(parentIdentity),Key(u"children"),Value(context.array()));
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(Undefined{}));
    context.set(Value(childIdentity),Key(u"parent"),Value(otherParentIdentity));
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(otherParentIdentity));
    expect(context.arrayLength(std::get<ObjectRef>(context.get(Value(otherParentIdentity),Key(u"children")).data))==0_u);

    context.set(Value(childIdentity),Key(u"parent"),Value(Undefined{}));
    // Source remove uses childIndex() == -1 for an explicitly assigned parent
    // whose children do not contain the child. It still clears parent.
    context.set(Value(childIdentity),Key(u"parent"),Value(parentIdentity));
    const auto beforeInconsistentRemove=context.get(Value(parentIdentity),Key(u"children"));
    (void)adaptor->remove(childLite);
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(Null{}));
    expect(context.get(Value(parentIdentity),Key(u"children"))==beforeInconsistentRemove);

    // Source insert passes childIndex() == -1 to Array.splice. With a
    // two-entry list, splice(-1, 0, child) inserts before the last entry.
    context.set(Value(parentIdentity),Key(u"children"),Value(context.array()));
    context.arrayPush(std::get<ObjectRef>(context.get(Value(parentIdentity),Key(u"children")).data),
        std::array<Value,1>{Value(replacementIdentity)});
    context.arrayPush(std::get<ObjectRef>(context.get(Value(parentIdentity),Key(u"children")).data),
        std::array<Value,1>{Value(lastIdentity)});
    context.set(Value(siblingIdentity),Key(u"parent"),Value(parentIdentity));
    context.set(Value(childIdentity),Key(u"parent"),Value(Undefined{}));
    adaptor->insert(childLite,siblingLite);
    const auto insertedChildrenValue=context.get(Value(parentIdentity),Key(u"children"));
    const auto* insertedChildren=std::get_if<ObjectRef>(&insertedChildrenValue.data);
    expect(insertedChildren!=nullptr);
    if(insertedChildren){
        expect(context.arrayLength(*insertedChildren)==3_u);
        expect(context.get(Value(*insertedChildren),Key(u"1"))==Value(childIdentity));
        expect(context.get(Value(*insertedChildren),Key(u"2"))==Value(lastIdentity));
    }
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(parentIdentity));

    // The Scene operation target must preserve the raw Context owner for a
    // source-mapped Lite identity, including an ordinary parent object.
    const auto mappedForeignParent=context.object();
    const auto mappedForeignChildren=context.array();
    context.arrayPush(mappedForeignChildren,std::array<Value,1>{Value(childIdentity)});
    context.set(Value(mappedForeignParent),Key(u"children"),Value(mappedForeignChildren));
    context.set(Value(childIdentity),Key(u"parent"),Value(mappedForeignParent));
    const auto mappedRemove=context.get(Value(adaptorObject),Key(u"remove"));
    const auto* mappedRemoveFunction=std::get_if<ObjectRef>(&mappedRemove.data);
    expect(mappedRemoveFunction!=nullptr);
    if(!mappedRemoveFunction)return;
    (void)context.callMember(*mappedRemoveFunction,Value(adaptorObject),
        std::array<Value,1>{Value(childIdentity)});
    expect(context.arrayLength(mappedForeignChildren)==0_u);
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(Null{}));

    // Each Source childIndex call creates a distinct predicate closure that
    // retains its own node identity after the custom findIndex returns.
    const auto retainedParent=context.object();
    const auto retainedChildren=context.array();
    const auto retainedPredicates=context.array();
    const auto retainFindIndex=context.function(u"retaining findIndex",
        [retainedPredicates](Context& cx,Value,std::span<const Value> arguments)->Value{
            if(arguments.empty())throw Exception("TypeError",u"Missing findIndex predicate");
            cx.arrayPush(retainedPredicates,std::array<Value,1>{arguments.front()});
            return Value(-1.0);
        });
    context.set(Value(retainedChildren),Key(u"findIndex"),Value(retainFindIndex));
    context.set(Value(retainedParent),Key(u"children"),Value(retainedChildren));
    context.set(Value(childIdentity),Key(u"parent"),Value(retainedParent));
    context.set(Value(replacementIdentity),Key(u"parent"),Value(retainedParent));
    (void)context.callMember(*mappedRemoveFunction,Value(adaptorObject),
        std::array<Value,1>{Value(childIdentity)});
    (void)context.callMember(*mappedRemoveFunction,Value(adaptorObject),
        std::array<Value,1>{Value(replacementIdentity)});
    expect(context.arrayLength(retainedPredicates)==2_u);
    const auto firstPredicateValue=context.get(Value(retainedPredicates),Key(u"0"));
    const auto secondPredicateValue=context.get(Value(retainedPredicates),Key(u"1"));
    const auto* firstPredicate=std::get_if<ObjectRef>(&firstPredicateValue.data);
    const auto* secondPredicate=std::get_if<ObjectRef>(&secondPredicateValue.data);
    expect(firstPredicate!=nullptr&&secondPredicate!=nullptr);
    if(firstPredicate&&secondPredicate){
        expect(*firstPredicate!=*secondPredicate);
        expect(context.get(Value(*firstPredicate),Key(u"name"))==Value(u""));
        expect(context.get(Value(*firstPredicate),Key(u"length"))==Value(1.0));
        const auto predicateLength=context.descriptor(Value(*firstPredicate),Key(u"length"));
        expect(predicateLength.has_value());
        if(predicateLength)expect(!predicateLength->writable&&!predicateLength->enumerable&&predicateLength->configurable);
        expect(context.hasOwn(Value(*firstPredicate),Key(u"prototype")));
        const auto predicatePrototypeDescriptor=context.descriptor(Value(*firstPredicate),Key(u"prototype"));
        expect(predicatePrototypeDescriptor.has_value());
        if(predicatePrototypeDescriptor)expect(predicatePrototypeDescriptor->writable&&!predicatePrototypeDescriptor->enumerable&&!predicatePrototypeDescriptor->configurable);
        const auto predicatePrototypeValue=context.get(Value(*firstPredicate),Key(u"prototype"));
        const auto* predicatePrototype=std::get_if<ObjectRef>(&predicatePrototypeValue.data);
        expect(predicatePrototype!=nullptr);
        if(predicatePrototype){
            const auto predicateInstance=context.construct(*firstPredicate);
            const auto instancePrototype=context.prototype(predicateInstance);
            expect(instancePrototype.has_value());
            if(instancePrototype)expect(*instancePrototype==*predicatePrototype);
        }
        expect(context.call(*firstPredicate,Value(Undefined{}),
            std::array<Value,1>{Value(childIdentity)})==Value(true));
        expect(context.call(*firstPredicate,Value(Undefined{}),
            std::array<Value,1>{Value(replacementIdentity)})==Value(false));
        expect(context.call(*secondPredicate,Value(Undefined{}),
            std::array<Value,1>{Value(childIdentity)})==Value(false));
        expect(context.call(*secondPredicate,Value(Undefined{}),
            std::array<Value,1>{Value(replacementIdentity)})==Value(true));
    }

    // findIndex's result remains a Value: replace uses ToPropertyKey for its
    // assignment, while splice receives the unmodified numeric index.
    const auto fractionalParent=context.object();
    const auto fractionalChildren=context.array();
    context.arrayPush(fractionalChildren,std::array<Value,1>{Value(childIdentity)});
    const auto fractionalFindIndex=context.function(u"fractional findIndex",
        [](Context&,Value,std::span<const Value>)->Value{return Value(0.5);});
    context.set(Value(fractionalChildren),Key(u"findIndex"),Value(fractionalFindIndex));
    context.set(Value(fractionalParent),Key(u"children"),Value(fractionalChildren));
    context.set(Value(childIdentity),Key(u"parent"),Value(fractionalParent));
    const auto mappedReplace=context.get(Value(adaptorObject),Key(u"replace"));
    const auto* mappedReplaceFunction=std::get_if<ObjectRef>(&mappedReplace.data);
    expect(mappedReplaceFunction!=nullptr);
    if(!mappedReplaceFunction)return;
    (void)context.callMember(*mappedReplaceFunction,Value(adaptorObject),
        std::array<Value,2>{Value(replacementIdentity),Value(childIdentity)});
    expect(context.get(Value(fractionalChildren),Key(u"0.5"))==Value(replacementIdentity));
    expect(context.get(Value(fractionalChildren),Key(u"0"))==Value(childIdentity));
    expect(context.get(Value(replacementIdentity),Key(u"parent"))==Value(fractionalParent));
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(Null{}));

    context.set(Value(childIdentity),Key(u"parent"),Value(fractionalParent));
    const auto negativeFindIndex=context.function(u"negative fractional findIndex",
        [](Context&,Value,std::span<const Value>)->Value{return Value(-0.5);});
    context.set(Value(fractionalChildren),Key(u"findIndex"),Value(negativeFindIndex));
    (void)context.callMember(*mappedReplaceFunction,Value(adaptorObject),
        std::array<Value,2>{Value(replacementIdentity),Value(childIdentity)});
    expect(context.get(Value(fractionalChildren),Key(u"0.5"))==Value(replacementIdentity));
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(fractionalParent));

    // Deleting the intrinsic method is observed; operations do not substitute
    // a local native search when the live property lookup returns undefined.
    const auto beforeMissingMethodParent=context.object();
    const auto beforeMissingMethodChildren=context.array();
    context.arrayPush(beforeMissingMethodChildren,std::array<Value,1>{Value(childIdentity)});
    context.set(Value(beforeMissingMethodParent),Key(u"children"),Value(beforeMissingMethodChildren));
    context.set(Value(childIdentity),Key(u"parent"),Value(beforeMissingMethodParent));
    const auto savedFindIndex=context.descriptor(Value(context.intrinsics().arrayPrototype),Key(u"findIndex"));
    expect(savedFindIndex.has_value());
    if(!savedFindIndex)return;
    context.erase(context.intrinsics().arrayPrototype,Key(u"findIndex"));
    bool missingFindIndexThrew=false;
    try{(void)context.callMember(*mappedRemoveFunction,Value(adaptorObject),
        std::array<Value,1>{Value(childIdentity)});}catch(const Exception&){missingFindIndexThrew=true;}
    expect(missingFindIndexThrew);
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(beforeMissingMethodParent));
    context.define(context.intrinsics().arrayPrototype,Key(u"findIndex"),*savedFindIndex);
    context.set(Value(childIdentity),Key(u"parent"),Value(Undefined{}));

    // Continue with clean membership for append/remove/replace assertions.
    context.set(Value(siblingIdentity),Key(u"parent"),Value(Undefined{}));
    context.set(Value(childIdentity),Key(u"parent"),Value(Undefined{}));
    context.set(Value(parentIdentity),Key(u"children"),Value(context.array()));
    (void)adaptor->append(parentLite,childLite);
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(parentIdentity));
    (void)adaptor->remove(childLite);
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(Null{}));
    (void)adaptor->append(parentLite,childLite);
    expect(adaptor->replace(replacementLite,childLite)==childLite);
    expect(context.get(Value(childIdentity),Key(u"parent"))==Value(Null{}));
    expect(context.get(Value(replacementIdentity),Key(u"parent"))==Value(parentIdentity));
};
};
}
