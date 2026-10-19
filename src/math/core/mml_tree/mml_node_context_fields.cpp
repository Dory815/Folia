// Live Context field projection for the actual MathJax MML node owners.
// MathJax 4.1.3 source is Apache-2.0; see LICENSE-MathJax.txt.
module folia.math.core.mml_tree.mml_node_context_fields;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.util.native_data_fields;
import folia.math.util.property_host;
import std;

namespace folia::math::mml {
using namespace options;

struct MmlNodeContextFields::State:std::enable_shared_from_this<State>{
    Context& context;
    ContextPropertyBindings& properties;
    AttributesContextBindings& attributes;
    NativeIdentityBindings<MmlNode*>& identities;
    tree::TreeNodeContextBindings<MmlNode>& treeNodes;
    std::weak_ptr<std::atomic_bool> executionLifetime;
    BindChildren bindChildren;
    ChildrenFromValue childrenFromValue;
    FactoryIdentity factoryIdentity;
    ResolveFactory resolveFactory;

    std::optional<Descriptor> descriptorInChain(ObjectRef identity,const Key& key)const{
        if(auto own=context.descriptor(Value(identity),key))return own;
        auto prototype=context.prototype(identity);
        while(prototype){
            if(auto inherited=context.descriptor(Value(*prototype),key))return inherited;
            prototype=context.prototype(*prototype);
        }
        return std::nullopt;
    }

    void install(ObjectRef identity,const Key& key,NativeDataFieldRef cell){
        const auto descriptor=descriptorInChain(identity,key);
        if(descriptor){
            if(descriptor->accessor)return;
            if(!context.hasOwn(Value(identity),key)){
                if(!descriptor->writable)return;
                return;
            }
            if(!descriptor->writable)return;
            (void)context.assignNativeData(identity,key,std::move(cell),descriptor->value);
            return;
        }
        context.bindData(identity,key,std::move(cell));
    }

    State(Context& context,ContextPropertyBindings& properties,
          AttributesContextBindings& attributes,NativeIdentityBindings<MmlNode*>& identities,
          tree::TreeNodeContextBindings<MmlNode>& treeNodes,
          std::weak_ptr<std::atomic_bool> executionLifetime,BindChildren bindChildren,
          ChildrenFromValue childrenFromValue,
          FactoryIdentity factoryIdentity,ResolveFactory resolveFactory)
        :context(context),properties(properties),attributes(attributes),identities(identities),
         treeNodes(treeNodes),executionLifetime(std::move(executionLifetime)),bindChildren(std::move(bindChildren)),
         childrenFromValue(std::move(childrenFromValue)),
         factoryIdentity(std::move(factoryIdentity)),resolveFactory(std::move(resolveFactory)){
        if(&properties.context()!=&context||&attributes.context()!=&context||&identities.context()!=&context)
            throw Exception("TypeError",u"MML node fields require one canonical Context graph");
        if(!this->bindChildren||!this->childrenFromValue||!this->factoryIdentity||!this->resolveFactory)
            throw Exception("TypeError",u"MML node fields require canonical child-array and factory owners");
    }
    void requireActive()const{
        const auto lease=executionLifetime.lock();
        if(!lease||!lease->load(std::memory_order_acquire))
            throw Exception("TypeError",u"MML node Context fields execution has expired");
    }
    MmlNode& receiver(ObjectRef identity,MmlNode& node)const{
        requireActive();
        if(identities.resolve(identity)!=&node)
            throw Exception("Unimplemented",u"MML Context field identity does not match its actual node owner");
        return node;
    }
    void bindParent(ObjectRef identity,MmlNode& node,const std::shared_ptr<MmlFactory>& owner){
        const auto self=shared_from_this();
        auto* const nodeOwner=&node;
        node.sourceParentAccessors(
            [self,nodeOwner,identity]{self->receiver(identity,*nodeOwner);return self->treeNodes.childValue(self->context.get(Value(identity),Key(u"parent")));},
            [self,nodeOwner,identity](tree::ChildValue<MmlNode> value){self->receiver(identity,*nodeOwner);self->context.set(Value(identity),Key(u"parent"),self->treeNodes.sourceValue(value));});
        auto cell=std::make_shared<NativeDataField>(std::static_pointer_cast<void>(owner),
            [self,nodeOwner,identity]()->Value{
                self->receiver(identity,*nodeOwner);
                const auto parent=nodeOwner->sourceParentCell();
                return self->treeNodes.sourceValue(parent);
            },
            [self,nodeOwner,identity](Value value){
                self->receiver(identity,*nodeOwner);
                nodeOwner->setSourceParentCell(self->treeNodes.childValue(std::move(value)));
            },
            [self,nodeOwner,identity]{
                self->receiver(identity,*nodeOwner);
                nodeOwner->setSourceParentCell(self->treeNodes.childValue(Value(Undefined{})));
            });
        install(identity,Key(u"parent"),std::move(cell));
    }
    void bindProperties(ObjectRef identity,MmlNode& node,const std::shared_ptr<MmlFactory>& owner){
        auto* storage=dynamic_cast<tree::AbstractNode<MmlNode,MmlNode>*>(&node);
        if(!storage)throw Exception("TypeError",u"MML node has no canonical PropertyList owner");
        const auto self=shared_from_this();auto* const nodeOwner=storage;auto* const actualNode=&node;
        nodeOwner->sourcePropertyAccessors(
            [self,actualNode,identity]{
                self->receiver(identity,*actualNode);
                const auto raw=self->properties.from(self->context.get(Value(identity),Key(u"properties")));
                const auto record=PropertyList::record(raw);
                if(!record)throw Exception("TypeError",u"MML node properties getter must return an object");
                return *record;
            },
            [self,actualNode,identity](PropertyList value){
                self->receiver(identity,*actualNode);
                self->context.set(Value(identity),Key(u"properties"),self->properties.to(Property(std::move(value))));
            });
        auto cell=std::make_shared<NativeDataField>(std::static_pointer_cast<void>(owner),
            [self,nodeOwner,actualNode,identity]{
                self->receiver(identity,*actualNode);
                return self->properties.to(nodeOwner->sourcePropertiesCell());
            },
            [self,nodeOwner,actualNode,identity](Value value){
                self->receiver(identity,*actualNode);
                nodeOwner->setSourcePropertiesRawField(self->properties.from(std::move(value)));
            },
            [self,nodeOwner,actualNode,identity]{self->receiver(identity,*actualNode);nodeOwner->setSourcePropertiesRawField(Property(PropertyNull{}));});
        install(identity,Key(u"properties"),std::move(cell));
    }
    void bindChildrenField(ObjectRef identity,MmlNode& node,const std::shared_ptr<MmlFactory>& owner){
        const auto self=shared_from_this();auto* const nodeOwner=&node;
        node.sourceChildArrayAccessors(
            [self,nodeOwner,identity]{
                self->receiver(identity,*nodeOwner);
                return self->childrenFromValue(self->context.get(Value(identity),Key(u"childNodes")));
            },
            [self,nodeOwner,identity](Children value){
                self->receiver(identity,*nodeOwner);
                self->context.set(Value(identity),Key(u"childNodes"),Value(self->bindChildren(std::move(value))));
            });
        auto cell=std::make_shared<NativeDataField>(std::static_pointer_cast<void>(owner),
            [self,nodeOwner,identity]{
                self->receiver(identity,*nodeOwner);
                const auto current=nodeOwner->childNodes;
                if(!current)return Value(Null{});
                if(const auto raw=current->sourceValue())return *raw;
                return Value(self->bindChildren(current));
            },
            [self,nodeOwner,identity](Value value){
                self->receiver(identity,*nodeOwner);
                nodeOwner->childNodes=self->childrenFromValue(std::move(value));
            },
            [self,nodeOwner,identity]{self->receiver(identity,*nodeOwner);nodeOwner->childNodes=self->childrenFromValue(Value(Undefined{}));});
        install(identity,Key(u"childNodes"),std::move(cell));
    }
    void bindFactory(ObjectRef identity,MmlNode& node,const std::shared_ptr<MmlFactory>& owner){
        const auto self=shared_from_this();auto* const nodeOwner=&node;
        const auto weakSelf=weak_from_this();
        const auto weakLease=executionLifetime;
        const auto resolver=[weakSelf,weakLease](const Value& raw)->tree::FactoryServices<MmlNode>*{
            const auto lease=weakLease.lock();
            if(!lease||!lease->load(std::memory_order_acquire))
                throw Exception("TypeError",u"MML node factory Context runtime has been released");
            const auto state=weakSelf.lock();
            if(!state)throw Exception("TypeError",u"MML node factory field owner has been released");
            const auto* object=std::get_if<ObjectRef>(&raw.data);
            if(!object)throw Exception("TypeError",u"MML node factory is not usable by native Tree operations");
            const auto replacement=state->resolveFactory(*object);
            if(!replacement)throw Exception("TypeError",u"MML node factory is not usable by native Tree operations");
            return replacement.get();
        };
        const auto currentDescriptor=descriptorInChain(identity,Key(u"factory"));
        if(currentDescriptor&&currentDescriptor->accessor){
            nodeOwner->sourceFactoryReader([weakSelf,weakLease,identity]()->Value{
                const auto lease=weakLease.lock();
                if(!lease||!lease->load(std::memory_order_acquire))
                    throw Exception("TypeError",u"MML node factory Context runtime has been released");
                const auto state=weakSelf.lock();
                if(!state)throw Exception("TypeError",u"MML node factory field owner has been released");
                return state->context.get(Value(identity),Key(u"factory"));
            },resolver);
        }
        else if(context.hasOwn(Value(identity),Key(u"factory"))){
            nodeOwner->sourceFactoryOwner(context.get(Value(identity),Key(u"factory")),resolver);
        }
        auto cell=std::make_shared<NativeDataField>(std::static_pointer_cast<void>(owner),
            [self,nodeOwner,identity]{
                self->receiver(identity,*nodeOwner);
                if(const auto& raw=nodeOwner->sourceFactoryValue())return *raw;
                auto* current=dynamic_cast<MmlFactory*>(&nodeOwner->factoryOwner());
                if(!current)throw Exception("TypeError",u"MML node factory owner is incompatible");
                auto retained=current->weak_from_this().lock();
                if(!retained)throw Exception("TypeError",u"MML node factory owner has been released");
                return Value(self->factoryIdentity(retained));
            },
            [self,nodeOwner,identity,resolver](Value value){
                self->receiver(identity,*nodeOwner);
                nodeOwner->sourceFactoryOwner(std::move(value),resolver);
            },
            [self,nodeOwner,identity]{self->receiver(identity,*nodeOwner);throw Exception("Unimplemented",u"MML node factory field cannot be deleted from its C++ reference owner");});
        install(identity,Key(u"factory"),std::move(cell));
    }
    void bindAbstractMmlFields(ObjectRef identity,AbstractMmlNode& node,const std::shared_ptr<MmlFactory>& owner){
        const auto self=shared_from_this();auto* const nodeOwner=&node;
        nodeOwner->sourceAttributeAssignment([self,nodeOwner,identity](std::shared_ptr<Attributes> value){
            self->receiver(identity,*nodeOwner);
            self->context.set(Value(identity),Key(u"attributes"),
                              Value(self->attributes.bind(value)));
            return value;
        });
        const auto attributesKey=Key(u"attributes");
        if(const auto descriptor=descriptorInChain(identity,attributesKey);descriptor&&descriptor->accessor){
            nodeOwner->sourceAttributeReader([self,nodeOwner,identity]{
                self->receiver(identity,*nodeOwner);
                const Value current=self->context.get(Value(identity),Key(u"attributes"));
                if(std::holds_alternative<Undefined>(current.data)||std::holds_alternative<Null>(current.data))
                    return std::shared_ptr<Attributes>{};
                const auto* object=std::get_if<ObjectRef>(&current.data);
                if(!object)return std::shared_ptr<Attributes>{};
                return self->attributes.resolve(*object);
            });
        }
        const auto bindProperty=[&](std::u16string_view name,
                                    std::function<Property()> read,
                                    std::function<void(Property)> write){
            auto cell=std::make_shared<NativeDataField>(std::static_pointer_cast<void>(owner),
                [self,nodeOwner,identity,read]{self->receiver(identity,*nodeOwner);return self->properties.to(read());},
                [self,nodeOwner,identity,write](Value value){self->receiver(identity,*nodeOwner);write(self->properties.from(std::move(value)));},
                [self,nodeOwner,identity,write]{self->receiver(identity,*nodeOwner);write(Missing{});});
            install(identity,Key(std::u16string(name)),std::move(cell));
        };
        bindProperty(u"prevClass",[nodeOwner]{return nodeOwner->sourcePrevClassField();},[nodeOwner](Property v){nodeOwner->setSourcePrevClassField(std::move(v));});
        bindProperty(u"prevLevel",[nodeOwner]{return nodeOwner->sourcePrevLevelField();},[nodeOwner](Property v){nodeOwner->setSourcePrevLevelField(std::move(v));});
        bindProperty(u"texclass",[nodeOwner]{return nodeOwner->sourceTexclassField();},[nodeOwner](Property v){nodeOwner->setSourceTexclassField(std::move(v));});
        auto attributesCell=std::make_shared<NativeDataField>(std::static_pointer_cast<void>(owner),
            [self,nodeOwner,identity]{
                self->receiver(identity,*nodeOwner);
                if(const auto raw=nodeOwner->sourceAttributesRawField())return raw->value;
                const auto current=nodeOwner->sourceAttributesField();
                return current?Value(self->attributes.bind(current)):Value(Null{});
            },
            [self,nodeOwner,identity](Value value){
                self->receiver(identity,*nodeOwner);
                if(std::holds_alternative<Undefined>(value.data)||std::holds_alternative<Null>(value.data)){
                    nodeOwner->setSourceAttributesRawField(std::move(value),self->treeNodes.nodeAccess(Value(identity)));return;
                }
                if(const auto* object=std::get_if<ObjectRef>(&value.data)){
                    try{nodeOwner->setSourceAttributesField(self->attributes.resolve(*object));return;}
                    catch(const Exception&){/* preserve foreign Source values at the field boundary */}
                }
                nodeOwner->setSourceAttributesRawField(std::move(value),self->treeNodes.nodeAccess(Value(identity)));
            },
            [self,nodeOwner,identity]{self->receiver(identity,*nodeOwner);nodeOwner->setSourceAttributesRawField(Value(Undefined{}),self->treeNodes.nodeAccess(Value(identity)));});
        // The Source constructor publishes attributes only after setChildren
        // and class-default lookup. Reserve the one native cell now so that
        // its actual assignment updates the native owner before setList runs.
        // A dormant cell contributes no own key or value before that Set.
        if(!context.hasOwn(Value(identity),attributesKey))
            context.declareNativeData(identity,attributesKey,std::move(attributesCell));
        else install(identity,attributesKey,std::move(attributesCell));
    }
    void bindRowFields(ObjectRef identity,MmlMrow& node,const std::shared_ptr<MmlFactory>& owner){
        const auto self=shared_from_this();auto* const nodeOwner=&node;
        auto cell=std::make_shared<NativeDataField>(std::static_pointer_cast<void>(owner),
            [self,nodeOwner,identity]{self->receiver(identity,*nodeOwner);return self->properties.to(nodeOwner->sourceCoreField());},
            [self,nodeOwner,identity](Value value){self->receiver(identity,*nodeOwner);nodeOwner->setSourceCoreField(self->properties.from(std::move(value)));},
            [self,nodeOwner,identity]{self->receiver(identity,*nodeOwner);nodeOwner->setSourceCoreField(Missing{});});
        install(identity,Key(u"_core"),std::move(cell));
    }
    void bindTextNodeFields(ObjectRef identity,TextNode& node,const std::shared_ptr<MmlFactory>& owner,bool late){
        const auto self=shared_from_this();auto* const nodeOwner=&node;
        nodeOwner->sourceTextAccessors(
            [self,nodeOwner,identity](const std::u16string& value){
                self->receiver(identity,*nodeOwner);
                self->context.set(Value(identity),Key(u"text"),Value(value));
            },{});
        const auto textKey=Key(u"text");
        if(const auto descriptor=descriptorInChain(identity,textKey);descriptor&&descriptor->accessor){
            nodeOwner->sourceTextReader([self,nodeOwner,identity]{
                self->receiver(identity,*nodeOwner);
                const Value value=self->context.get(Value(identity),Key(u"text"));
                if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
                throw Exception("TypeError",u"TextNode.text is not a string");
            });
        }
        if(!late)return;
        auto cell=std::make_shared<NativeDataField>(std::static_pointer_cast<void>(owner),
            [self,nodeOwner,identity]{self->receiver(identity,*nodeOwner);return Value(nodeOwner->sourceTextField());},
            [self,nodeOwner,identity](Value value){
                self->receiver(identity,*nodeOwner);
                const auto* text=std::get_if<std::u16string>(&value.data);
                if(!text)throw Exception("TypeError",u"TextNode.text must be a string");
                nodeOwner->setSourceTextField(*text);
            },
            [self,nodeOwner,identity]{
                self->receiver(identity,*nodeOwner);
                throw Exception("Unimplemented",u"TextNode.text deletion cannot be represented by its string owner");
            });
        install(identity,Key(u"text"),std::move(cell));
    }
    void bindLate(ObjectRef identity,MmlNode& node,const std::shared_ptr<MmlFactory>& owner){
        if(auto* row=dynamic_cast<MmlMrow*>(&node))bindRowFields(identity,*row,owner);
        if(auto* text=dynamic_cast<TextNode*>(&node))bindTextNodeFields(identity,*text,owner,true);
    }
};

MmlNodeContextFields::MmlNodeContextFields(Context& context,ContextPropertyBindings& properties,
    AttributesContextBindings& attributes,NativeIdentityBindings<MmlNode*>& identities,
    tree::TreeNodeContextBindings<MmlNode>& treeNodes,
    std::weak_ptr<std::atomic_bool> executionLifetime,BindChildren bindChildren,
    ChildrenFromValue childrenFromValue,
    FactoryIdentity factoryIdentity,ResolveFactory resolveFactory)
    :state_(std::make_shared<State>(context,properties,attributes,identities,treeNodes,
        std::move(executionLifetime),std::move(bindChildren),std::move(childrenFromValue),
        std::move(factoryIdentity),std::move(resolveFactory))){}

void MmlNodeContextFields::bind(ObjectRef identity,MmlNode& node,const std::shared_ptr<MmlFactory>& owner){
    auto& state=*state_;state.requireActive();state.receiver(identity,node);
    if(!owner)throw Exception("TypeError",u"MML node fields require the actual factory owner");
    const std::weak_ptr<State> weakState=state_;
    node.sourceArgumentValues({
        [weakState](Factory* factory)->Value{
            const auto state=weakState.lock();
            if(!state)throw Exception("TypeError",u"MML constructor Context owner has expired");
            state->requireActive();
            auto* mmlFactory=dynamic_cast<MmlFactory*>(factory);
            if(!mmlFactory)throw Exception("TypeError",u"MML constructor factory has no Context identity");
            auto retained=mmlFactory->weak_from_this().lock();
            if(!retained)throw Exception("TypeError",u"MML constructor factory owner has expired");
            return Value(state->factoryIdentity(retained));
        },
        [weakState](Property value)->Value{
            const auto state=weakState.lock();
            if(!state)throw Exception("TypeError",u"MML constructor Context owner has expired");
            state->requireActive();
            return state->properties.to(value);
        },
        [weakState](Children value)->Value{
            const auto state=weakState.lock();
            if(!state)throw Exception("TypeError",u"MML constructor Context owner has expired");
            state->requireActive();
            if(!value)return Null{};
            return Value(state->bindChildren(std::move(value)));
        }});
    // Pinned Node.js own-key order: the constructor parameter property is
    // published before the inherited Node fields.
    state.bindFactory(identity,node,owner);
    state.bindParent(identity,node,owner);
    state.bindProperties(identity,node,owner);
    state.bindChildrenField(identity,node,owner);
    if(auto* abstract=dynamic_cast<AbstractMmlNode*>(&node))state.bindAbstractMmlFields(identity,*abstract,owner);
    if(auto* text=dynamic_cast<TextNode*>(&node))state.bindTextNodeFields(identity,*text,owner,false);
    const std::weak_ptr<MmlFactory> weakOwner=owner;
    auto* const nodeOwner=&node;
    node.sourceInitializationComplete([weakState,weakOwner,identity,nodeOwner]{
        const auto state=weakState.lock();
        const auto factory=weakOwner.lock();
        if(!state||!factory)throw Exception("TypeError",u"MML node initialization owner has expired");
        state->bindLate(identity,*nodeOwner,factory);
    });
}
}
