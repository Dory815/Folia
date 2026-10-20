// Native lifetime/binding owner for one loaded Source module environment.
// This owner provides no TeX, configuration, tag or layout algorithms.
export module folia.math.input.tex.parser:runtime;
export import :configuration;
export import :tags;
export import folia.math.input.tex.node_factory;
export import folia.math.input.tex.token_map_context_bindings;
export import folia.math.input.tex.handler_context_bindings;
export import folia.math.core.mml_tree.attributes_bindings;
export import folia.math.core.mml_tree.mml_node_context_fields;
export import folia.math.core.tree.node_context_bindings;
export import folia.math.core.tree.node_context_classes;
export import folia.math.core.mml_tree.mml_factory_context_fields;
export import folia.math.core.mml_tree.mml_node_context_methods;
export import folia.math.core.mml_tree.mml_node_context_module;
export import folia.math.core.mml_tree.mml_context_bindings;
export import folia.math.core.mml_tree.mml_factory_context_methods;
export import folia.math.input.tex.stack_property;
export import folia.math.input.tex.stack_context_bindings;
export import folia.math.core.input_jax_host;
export import folia.math.input.tex.find_tex_host;
export import folia.math.util.context_property_bindings;
export import folia.math.util.function_list_bindings;
export import folia.math.util.promise;
export import folia.math.util.runtime_modules;
import std;
export namespace folia::math::tex {
struct ColumnState;
class ParserRuntime {
    options::Context& context_;
    OptionFunctionBindings& functions_;
    regexp_host::RegExpBindings& regexp_;
    UnicodeNormalization& unicode_;
    core::InputJaxModule& inputJaxModule_;
    FindTeXModule& findTeXModule_;
    util::RuntimeModules ownedRuntimeModules_;
    std::optional<util::RuntimeModules> sharedRuntimeModulesOwner_;
    util::RuntimeModules* runtimeModules_;
    std::shared_ptr<mml::MmlNodeContextModule> mmlNodeContextModuleOwner_;
    std::shared_ptr<options::MapContextBindings> mapsOwner_;
    std::shared_ptr<mml::MmlContextBindings> mmlContextBindingsOwner_;
    async::QueueRef queue_=std::make_shared<async::JobQueue>();
    std::shared_ptr<std::atomic_bool> executionLifetime_=std::make_shared<std::atomic_bool>(true);
    // Converts the existing native symbol identity into this Context's actual
    // primitive symbol. Names/descriptions never determine symbol equality.
    std::vector<std::pair<ParseSymbolRef,options::SymbolRef>> parseSymbols_;
    std::unordered_map<SubHandlers*,options::ObjectRef> subHandlersIdentities_;
public:
    ContextPropertyBindings properties;
    FunctionListContextBindings functionLists;
    // One Context Map host shared by the SourceStringMap<Value> owners. The
    // actual MapStorage lives on each map and is also used by typed methods.
    options::MapContextBindings& maps;
    std::shared_ptr<tree::FactoryContextBaseModule> factoryContextBaseModule;
    AttributesContextBindings& attributeBindings;
    // Source MapHandler.ts has one module-owned registry. Keep that registry
    // with this runtime so separate parser environments never share entries.
    MapHandler mapHandler;
    TokenMapContextBindings tokenMaps;
    SubHandlerContextBindings handlerBindings;
    // Bind buckets precede all registries/factories. Their Context objects
    // must exist before module class constructors can register native classes.
    NativeIdentityBindings<ConfigurationRef> configurationIdentities;
    NativeIdentityBindings<ParserConfigurationRef> parserConfigurationIdentities;
    NativeIdentityBindings<ParseOptionsRef> parseOptionsIdentities;
    NativeIdentityBindings<NodeFactory*> nodeFactoryIdentities;
    NativeIdentityBindings<TexParserRef> parserIdentities;
    NativeIdentityBindings<TagsRef> tagIdentities;
    NativeIdentityBindings<TagsClassRef> tagClassIdentities;
    NativeIdentityBindings<TeXRef> texIdentities;
    NativeIdentityBindings<KeyValueType> keyValueIdentities;
    NativeIdentityBindings<LabelRef> labelIdentities;
    NativeIdentityBindings<TagInfoRef> tagInfoIdentities;
    NativeIdentityBindings<mml::Children>& nodeArrayIdentities;
    NativeIdentityBindings<GlobalDefinitionScopeRef> globalDefinitionScopeIdentities;
    NativeIdentityBindings<StackItemClassRef> itemClassIdentities;
    NativeIdentityBindings<StackItemFactory*> stackItemFactoryIdentities;
    NativeIdentityBindings<StackItemRef> stackItemIdentities;
    NativeIdentityBindings<StackPropertyStorage*> stackPropertyIdentities;
    NativeIdentityBindings<mml::MmlNode*>& mmlNodeIdentities;
    tree::TreeNodeContextBindings<mml::MmlNode>& treeNodeBindings;
    tree::TreeNodeContextClasses<mml::MmlNode>& treeNodeClasses;
    NativeIdentityBindings<StackRef> stackIdentities;
    StackContextBindings stackBindings;
    NativeIdentityBindings<mml::MmlFactory*>& mmlFactoryIdentities;
    mml::MmlFactoryContextMethods& mmlFactoryContextMethods;
    mml::MmlFactoryContextFields& mmlFactoryContextFields;
    mml::MmlNodeContextFields& mmlNodeContextFields;
    mml::MmlNodeContextMethods& mmlNodeContextMethods;
    NativeIdentityBindings<std::shared_ptr<ColumnState>> columnStateIdentities;
    NativeIdentityBindings<ColumnParser*> columnParserIdentities;
    ConfigurationHandler configurationHandler;
    options::Value configurationHandlerObject=options::Undefined{};
    options::Value stackItemModule=options::Undefined{};
    options::Value baseItemsModule=options::Undefined{};
    options::Value amsItemsModule=options::Undefined{};
    options::Value newcommandItemsModule=options::Undefined{};
    options::Value stackItemFactoryClass=options::Undefined{};
    // Canonical function tables belong to this loaded module environment.
    // Packages share these refs; reinitializing a table changes Source identity.
    Property baseMethods=Missing{},parseMethods=Missing{},textMacrosMethods=Missing{};
    Property bboldxMethods=Missing{};
    Property newcommandMethods=Missing{},amsMethods=Missing{},mathtoolsMethods=Missing{};
    // Source module method tables keep their identity across direct imports.
    Property htmlMethods=Missing{},texHtmlMethods=Missing{};
    Property textcompInsert=Missing{};
    bool textcompMappingsInitialized=false;
    // AmsCdMethods is one default-export object in the Source module instance.
    Property amsCdMethods=Missing{};
    // UnitsConfiguration.ts exports this actual methods record; its command
    // map retains the exact same FunctionRefs for this runtime.
    Property unitsMethods=Missing{};
    // CenternotConfiguration.ts exposes one named filter export per loaded
    // runtime; its postprocessor and Context callable share this FunctionRef.
    Property centernotMethods=Missing{};
    Property begingroupMethods=Missing{};
    TagsFactory tagsFactory;
    SourceStringMap<options::Value> variantMap; // actual ParseOptions class Map
    options::Value KeyValueTypes=options::Undefined{};
    options::Value texClass=options::Undefined{};
    options::Value configurationClass=options::Undefined{},parserConfigurationClass=options::Undefined{};
    options::Value parseOptionsClass=options::Undefined{},texParserClass=options::Undefined{};
    options::Value nodeFactoryClass=options::Undefined{};
    options::Value stackClass=options::Undefined{};
    options::Value textParserClass=options::Undefined{},baseTagsClass=options::Undefined{};
    options::Value labelClass=options::Undefined{},tagInfoClass=options::Undefined{};
    options::Value abstractTagsClass=options::Undefined{},noTagsClass=options::Undefined{},allTagsClass=options::Undefined{};
    options::Value columnParserClass=options::Undefined{},keyValueClass=options::Undefined{};
    options::Value begingroupStackClass=options::Undefined{};
    // A runtime may initialize its true Configuration/Tags metadata while
    // parser-family peer constructors are still being integrated.
    bool moduleInitialized=false;
    // Presence check for peer class constructors only; never a Source parity gate.
    bool moduleMetadataComplete=false;
    std::vector<std::u16string> missingClassMetadata;
private:
    bool requestContextRecordCaptureActive_=false;
    std::vector<options::ObjectRef> requestContextRecords_;
    std::vector<std::pair<options::Key,options::Descriptor>> objectPrototypeRootSnapshot_;
    std::optional<options::ObjectRef> objectPrototypeParentSnapshot_;
    bool objectPrototypeRootSnapshotValid_=false;
public:
    ParserRuntime(options::Context&,OptionFunctionBindings&,regexp_host::RegExpBindings&,UnicodeNormalization&,
        core::InputJaxModule&,FindTeXModule&,util::RuntimeModules* sharedModules=nullptr);
    ~ParserRuntime();
    // One-time loaded-module phase. All real constructor metadata and native
    // identity buckets are installed before TagsFactory registers its classes.
    void initializeParserRuntime();
    ParserRuntime(const ParserRuntime&)=delete;
    ParserRuntime&operator=(const ParserRuntime&)=delete;
    options::Context& context()const;
    OptionFunctionBindings& functions()const;
    regexp_host::RegExpBindings& regexp()const;
    UnicodeNormalization& unicode()const;
    core::InputJaxModule& inputJaxModule()const;
    FindTeXModule& findTeXModule()const;
    util::RuntimeModules& modules() noexcept { return *runtimeModules_; }
    const async::QueueRef& queue()const;
    std::weak_ptr<std::atomic_bool> executionLifetime()const{return executionLifetime_;}
    void beginRequestContextRecordCapture(){
        if(requestContextRecordCaptureActive_)
            throw options::Exception("TypeError",u"ParserRuntime request Context capture is already active");
        requestContextRecords_.clear();requestContextRecordCaptureActive_=true;
    }
    void trackRequestContextRecord(options::ObjectRef object){
        if(!requestContextRecordCaptureActive_)return;
        (void)context_.kind(object);
        if(std::find(requestContextRecords_.begin(),requestContextRecords_.end(),object)==requestContextRecords_.end()){
            try{requestContextRecords_.push_back(object);}
            catch(...){context_.retireObjectRecord(object);throw;}
        }
    }
    void cancelRequestContextRecordCapture()noexcept{
        requestContextRecords_.clear();requestContextRecordCaptureActive_=false;
    }
    void retireRequestContextRecords(){
        if(!requestContextRecordCaptureActive_)
            throw options::Exception("TypeError",u"ParserRuntime request Context capture is not active");
        while(!requestContextRecords_.empty()){
            context_.retireObjectRecord(requestContextRecords_.back());
            requestContextRecords_.pop_back();
        }
        requestContextRecordCaptureActive_=false;
    }
    options::SymbolRef bindParseSymbol(ParseSymbolRef);
    options::ObjectRef bindSubHandlers(const std::shared_ptr<SubHandlers>&,options::ObjectRef);
    std::optional<options::ObjectRef> findSubHandlers(const SubHandlers*)const;
    std::shared_ptr<SubHandlers> resolveSubHandlers(options::ObjectRef)const;
    options::ObjectRef bindMmlFactory(std::shared_ptr<mml::MmlFactory>);
    std::shared_ptr<mml::MmlFactory> resolveMmlFactory(options::ObjectRef)const;
    options::ObjectRef bindMmlChildren(mml::Children);
    // Private Renderer only: retire stale TagInfo identities after SVG/Scene
    // results have been copied. Keep TagInfo objects still present in the live
    // Tags currentTag/history/stack raw data fields. NativeRenderSession callers
    // must not use this because their Source handles may retain old TagInfo.
    std::size_t retireTagInfoOwnersOutsidePrivateRendererRoots();
    // Private Renderer only: preserve Label owners reachable from the live
    // Tags labels/allLabels maps and retire replaced placeholders or unresolved
    // reference temporaries after the result has been copied.
    std::size_t retireLabelOwnersOutsidePrivateRendererRoots();
    std::shared_ptr<RegExpContext> makeRegExpContext(options::Value);
    options::Value regexpContextValue(const std::shared_ptr<RegExpContext>&)const;
    std::shared_ptr<StackItemFactory> resolveStackItemFactory(options::ObjectRef)const;
    options::ObjectRef bindStack(const StackRef&);
    StackRef resolveStack(options::ObjectRef)const;
    mml::MmlNode& resolveMmlNode(options::ObjectRef)const;
    std::array<std::size_t,24> identityRegistryBreakdownForDiagnostics()const noexcept {
        return {parseSymbols_.size(),subHandlersIdentities_.size(),
            configurationIdentities.canonicalHandleCount(),parserConfigurationIdentities.canonicalHandleCount(),
            parseOptionsIdentities.canonicalHandleCount(),nodeFactoryIdentities.canonicalHandleCount(),
            parserIdentities.canonicalHandleCount(),tagIdentities.canonicalHandleCount(),
            tagClassIdentities.canonicalHandleCount(),texIdentities.canonicalHandleCount(),
            keyValueIdentities.canonicalHandleCount(),labelIdentities.canonicalHandleCount(),
            tagInfoIdentities.canonicalHandleCount(),nodeArrayIdentities.canonicalHandleCount(),
            globalDefinitionScopeIdentities.canonicalHandleCount(),itemClassIdentities.canonicalHandleCount(),
            stackItemFactoryIdentities.canonicalHandleCount(),stackItemIdentities.canonicalHandleCount(),
            stackPropertyIdentities.canonicalHandleCount(),mmlNodeIdentities.canonicalHandleCount(),
            stackIdentities.canonicalHandleCount(),mmlFactoryIdentities.canonicalHandleCount(),
            columnStateIdentities.canonicalHandleCount(),columnParserIdentities.canonicalHandleCount()};
    }
    std::size_t canonicalIdentityHandleCountForDiagnostics()const noexcept {
        return parseSymbols_.size()+subHandlersIdentities_.size()+
            configurationIdentities.canonicalHandleCount()+parserConfigurationIdentities.canonicalHandleCount()+
            parseOptionsIdentities.canonicalHandleCount()+nodeFactoryIdentities.canonicalHandleCount()+
            parserIdentities.canonicalHandleCount()+tagIdentities.canonicalHandleCount()+
            tagClassIdentities.canonicalHandleCount()+texIdentities.canonicalHandleCount()+
            keyValueIdentities.canonicalHandleCount()+labelIdentities.canonicalHandleCount()+
            tagInfoIdentities.canonicalHandleCount()+nodeArrayIdentities.canonicalHandleCount()+
            globalDefinitionScopeIdentities.canonicalHandleCount()+itemClassIdentities.canonicalHandleCount()+
            stackItemFactoryIdentities.canonicalHandleCount()+stackItemIdentities.canonicalHandleCount()+
            stackPropertyIdentities.canonicalHandleCount()+mmlNodeIdentities.canonicalHandleCount()+
            stackIdentities.canonicalHandleCount()+mmlFactoryIdentities.canonicalHandleCount()+
            columnStateIdentities.canonicalHandleCount()+columnParserIdentities.canonicalHandleCount();
    }
};
}
