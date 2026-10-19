/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native C++ port of the generic Factory behavior from
 * .reference/MathJax-src/ts/core/Tree/Factory.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.core.tree.factory;
export import folia.math.core.tree.factory_types;
export import folia.math.core.tree.factory_class_map_owner;
export import folia.math.util.factory_context_field_views;
import folia.math.util.property_host;
import std;

export namespace folia::math::tree {
template<class Product>
class AbstractFactory : public FactoryServices<Product> {
public:
    inline static FactoryClassMap<Product> defaultNodes{};
    struct ProductMark { const AbstractFactory* owner=nullptr; std::size_t size=0; std::uint64_t generation=0; };
    // Source's defaultKind is an ordinary writable data property. Keep its
    // raw Value in the owner and convert to a property key only when create()
    // consumes it. Context-backed factories install the ToPropertyKey bridge.
    options::Value defaultKind = std::u16string(u"unknown");
    using DefaultKindKey = std::function<options::Key(const options::Value&)>;
    DefaultKindKey defaultKindKey_;
    struct SourceCreateHooks {
        std::function<options::Value()> readNode;
        std::function<options::Value()> readDefaultKind;
        std::function<options::Key(const options::Value&)> toKey;
        std::function<options::Value(const options::Value&,const options::Key&)> lookup;
        std::function<options::Value(const options::Value&)> readApply;
        std::function<options::Value(const std::any&)> argumentValue;
        std::function<options::Value(const options::Value&,const options::Value&,
                                     std::span<const options::Value>)> invoke;
        std::function<Product*(const options::Value&)> resolveProduct;
    };
    std::optional<SourceCreateHooks> sourceCreateHooks_;
    using SourceNodeMapReader = std::function<std::shared_ptr<FactoryClassMapAccess<Product>>() >;
    SourceNodeMapReader sourceNodeMapReader_;
    using SourceCreatorAccessReader = std::function<std::shared_ptr<FactoryCreatorAccess<Product>>() >;
    SourceCreatorAccessReader sourceCreatorAccessReader_;

    explicit AbstractFactory(
        std::optional<FactoryClassMap<Product>> nodes = std::nullopt,
        const FactoryClassMap<Product>* classDefaults = nullptr)
        : nodeMap_(std::make_shared<FactoryClassMapOwner<Product>>()),
          node_(std::make_shared<FactoryCreatorObjectAccess<Product>>(
              std::make_shared<FactoryCreatorObject<Product>>())) {
        FactoryClassMap<Product> initial;
        if (nodes) {
            initial = std::move(*nodes);
        } else if (classDefaults) {
            initial = *classDefaults;
        } else {
            initial = defaultNodes;
        }
        orderInitialMap(initial);
        for (auto& [kind, type] : initial) setNodeClass(std::move(kind), std::move(type));
    }

    AbstractFactory(const AbstractFactory&) = delete;
    AbstractFactory& operator=(const AbstractFactory&) = delete;
    AbstractFactory(AbstractFactory&&) = delete;
    AbstractFactory& operator=(AbstractFactory&&) = delete;
    ~AbstractFactory() override { factoryLifetime_->store(false,std::memory_order_release); }

    Product* create(std::string_view kind, FactoryArgs args = {}) override {
        return createWithArguments(kind,[args]{
            return std::vector<std::any>(args.begin(),args.end());
        });
    }

    // The Source create algorithm is shared by typed and Context-backed
    // callers. Field readers run at the exact Source read points, so an
    // accessor replacement can return a different node owner for fallback.
    template<class ReadNode,class ReadDefaultKind,class ToKey,class Lookup,
             class IsTruthy,class PrepareApply,class BuildArguments,class InvokePrepared>
    decltype(auto) createFromSourceFields(options::Value requested,
        ReadNode&& readNode,ReadDefaultKind&& readDefaultKind,ToKey&& toKey,
        Lookup&& lookup,IsTruthy&& isTruthy,PrepareApply&& prepareApply,
        BuildArguments&& buildArguments,InvokePrepared&& invokePrepared){
        auto node=std::invoke(readNode);
        const auto requestedKey=std::invoke(toKey,requested);
        auto selected=std::invoke(lookup,node,requestedKey);
        if(!std::invoke(isTruthy,selected)){
            node=std::invoke(readNode);
            auto fallbackKind=std::invoke(readDefaultKind);
            const auto fallbackKey=std::invoke(toKey,fallbackKind);
            selected=std::invoke(lookup,node,fallbackKey);
        }
        auto prepared=std::invoke(prepareApply,selected);
        auto arguments=std::invoke(buildArguments);
        return std::invoke(invokePrepared,prepared,
                           std::move(arguments));
    }

    // Resolve and retain the selected Source creator before evaluating any
    // argument conversion that may call back into Context. The native and
    // Context entry points share this one selection/fallback/invocation path.
    template<class BuildArguments>
    Product* createWithArguments(std::string_view kind,BuildArguments&& buildArguments) {
        const options::Value requested(property_host::text(kind));
        if(sourceCreateHooks_){
            const auto hooks=*sourceCreateHooks_;
            return hooks.resolveProduct(createSourceValue(requested,
                std::forward<BuildArguments>(buildArguments)));
        }
        return createFromNativeFields(requested,std::forward<BuildArguments>(buildArguments));
    }

    // A bound Source method returns its original Context Value. Native render
    // paths call createWithArguments, which resolves that same result to a
    // native Product only after the shared Source algorithm completes.
    template<class BuildArguments>
    options::Value createSourceValue(options::Value requested,BuildArguments&& buildArguments) {
        if(!sourceCreateHooks_)type_error("Factory has no bound Source field owner");
        const auto hooks=*sourceCreateHooks_;
        struct Prepared {options::Value selected;options::Value apply;};
        return createFromSourceFields(std::move(requested),hooks.readNode,hooks.readDefaultKind,
            hooks.toKey,
            [&](const options::Value& node,const options::Key& key){return hooks.lookup(node,key);},
            [](const options::Value& selected){return options::truthy(selected);},
            [&](const options::Value& selected){return Prepared{selected,hooks.readApply(selected)};},
            std::forward<BuildArguments>(buildArguments),
            [&](const Prepared& prepared,const auto& nativeArguments){
                std::vector<options::Value> arguments;
                arguments.reserve(nativeArguments.size());
                for(const auto& argument:nativeArguments)
                    arguments.push_back(hooks.argumentValue(argument));
                return hooks.invoke(prepared.selected,prepared.apply,arguments);
            });
    }

    void sourceCreateHooks(SourceCreateHooks hooks){
        if(!hooks.readNode||!hooks.readDefaultKind||!hooks.toKey||!hooks.lookup||
           !hooks.readApply||!hooks.argumentValue||!hooks.invoke||!hooks.resolveProduct)
            type_error("Factory source-field callbacks must all be callable");
        sourceCreateHooks_=std::move(hooks);
    }

    std::weak_ptr<std::atomic_bool> executionLease()const noexcept override{return factoryLifetime_;}

private:
    template<class BuildArguments>
    Product* createFromNativeFields(options::Value requested,BuildArguments&& buildArguments) {
        using Access=FactoryCreatorAccess<Product>;
        struct Selected {std::shared_ptr<Access> access;typename Access::Selection value;};
        struct Prepared {std::shared_ptr<Access> access;PreparedFactoryCreatorCall<Product> value;};
        return createFromSourceFields(std::move(requested),
            [this]{return currentCreatorAccess();},
            [this]{return defaultKind;},
            [this](const options::Value& value){
                if(defaultKindKey_)return defaultKindKey_(value);
                if(const auto* text=std::get_if<std::u16string>(&value.data))return options::Key(*text);
                type_error("Factory defaultKind requires a Source property-key converter");
            },
            [](const std::shared_ptr<Access>& access,const options::Key& key){
                return Selected{access,access->lookupKey(key)};
            },
            [](const Selected& selected){return selected.value.sourceTruthy();},
            [](const Selected& selected){
                return Prepared{selected.access,selected.access->prepareApply(selected.value)};
            },
            std::forward<BuildArguments>(buildArguments),
            [](const Prepared& prepared,const auto& arguments){
                return prepared.access->invokePrepared(prepared.value,FactoryArgs(arguments));
            });
    }

public:

    void setNodeClass(std::string kind, FactoryClassRef<Product> type) override {
        const auto map=currentNodeMapAccess();
        const options::Value sourceKey(property_host::text(kind));
        map->set(sourceKey,FactoryClassMapValue<Product>::native(type));
        // Source performs a second field read after set and captures that
        // Map.get result as KIND. Keep the raw mapped value: construction of
        // KIND happens only when the generated creator is later invoked.
        const auto selectedMap=currentNodeMapAccess();
        const auto selectedValue=selectedMap->get(sourceKey);

        auto captured = std::make_shared<FactoryCreator<Product>>();
        const std::weak_ptr<std::atomic_bool> lifetime=factoryLifetime_;
        captured->callWithServices = [selectedMap,selectedValue](FactoryServices<Product>& services,FactoryArgs args) -> Product* {
            const auto selectedType=selectedValue?selectedMap->resolveValue(*selectedValue):FactoryClassRef<Product>{};
            if (!selectedType || !selectedType->construct) type_error("Invalid factory class");
            Product* const product = selectedType->construct(services, args);
            if (!product) type_error("Factory constructor returned no product");
            return product;
        };
        captured->call = [this, lifetime, selectedMap, selectedValue](FactoryArgs args) -> Product* {
            const auto live=lifetime.lock();
            if(!live||!live->load(std::memory_order_acquire))
                throw FactoryError("TypeError",u"Factory owner is no longer available");
            const auto selectedType=selectedValue?selectedMap->resolveValue(*selectedValue):FactoryClassRef<Product>{};
            if (!selectedType || !selectedType->construct) type_error("Invalid factory class");
            Product* const product = selectedType->construct(*this, args);
            if (!product) type_error("Factory constructor returned no product");
            return product;
        };

        currentCreatorAccess()->assign(std::move(kind),std::move(captured));
    }

    FactoryClassRef<Product> getNodeClass(std::string_view kind) const override {
        return currentNodeMapAccess()->resolveClass(options::Value(property_host::text(kind)));
    }

    void deleteNodeClass(std::string_view kind) override {
        currentNodeMapAccess()->erase(options::Value(property_host::text(kind)));
        currentCreatorAccess()->erase(kind);
    }

    bool nodeIsKind(const Product* node, std::string_view kind) const override {
        const auto type = getNodeClass(kind);
        if (!type || !type->isInstance) type_error("Invalid factory class");
        if (!node) return false;
        return type->isInstance(node);
    }

    // Source-facing Context API: Array.from(nodeMap.keys()) semantics over
    // the current live owner. keyAt copies only the key; mapped values are
    // never resolved or converted during enumeration.
    std::vector<options::Value> getKindsValues() const {
        return currentNodeMapAccess()->keysValues();
    }

    // The existing C++ interface has a string-kind contract. This adapter
    // performs a strict domain check and never stringifies a Map key.
    std::vector<std::string> getKinds() const override {
        std::vector<std::string> kinds;
        const auto keys=getKindsValues();
        kinds.reserve(keys.size());
        for (const auto& key : keys) {
            const auto* text=std::get_if<std::u16string>(&key.data);
            if(!text)type_error("Factory getKinds cannot represent a non-string nodeMap key");
            for(std::size_t i=0;i<text->size();++i){
                const auto unit=static_cast<std::uint16_t>((*text)[i]);
                if(unit>=0xD800&&unit<=0xDBFF){
                    if(i+1>=text->size())type_error("Factory getKinds cannot represent an unpaired UTF-16 surrogate");
                    const auto low=static_cast<std::uint16_t>((*text)[i+1]);
                    if(low<0xDC00||low>0xDFFF)type_error("Factory getKinds cannot represent an unpaired UTF-16 surrogate");
                    ++i;
                }else if(unit>=0xDC00&&unit<=0xDFFF)
                    type_error("Factory getKinds cannot represent an unpaired UTF-16 surrogate");
            }
            kinds.push_back(property_host::debugText(*text));
        }
        return kinds;
    }

    std::size_t productCountForDiagnostics() const noexcept { return products_.size(); }
    ProductMark productMark()const noexcept{return {this,products_.size(),productGeneration_};}

    // Owner-scoped cleanup hooks. The caller must detach every persistent
    // reference and retire Context identities before destroying products.
    // Marks are valid only for the same live factory and are not GC proofs.
    template<class Visitor>void visitProductsFrom(ProductMark mark,Visitor&& visitor)const{
        validateProductMark(mark);
        for(std::size_t i=mark.size;i<products_.size();++i)visitor(*products_[i]);
    }
    void retireProductsFrom(ProductMark mark){
        validateProductMark(mark);
        products_.resize(mark.size);
        ++productGeneration_;
    }

    Product* adopt(std::unique_ptr<Product> product) override {
        if (!product) type_error("Cannot adopt a null factory product");
        Product* const result = product.get();
        products_.push_back(std::move(product));
        return result;
    }

    FactoryCreatorView<Product> creatorView() const {
        const auto owner=node_->nativeOwner();
        if(!owner)type_error("Current factory node is Context-backed; request its Source object from the Context binder");
        return owner;
    }
    std::shared_ptr<FactoryCreatorAccess<Product>> creatorAccess()const noexcept{return node_;}
    void creatorAccess(std::shared_ptr<FactoryCreatorAccess<Product>> replacement){
        if(!replacement)type_error("Factory node requires an object owner");
        node_=std::move(replacement);
    }
    std::optional<options::ObjectRef> creatorSourceObject()const{return node_->sourceObject();}

    // The Context field binder uses this same access owner for live projection
    // and replacement by any same-Context Map. No entries are copied.
    std::shared_ptr<FactoryClassMapAccess<Product>> nodeMapAccess() const noexcept {
        return nodeMap_;
    }
    std::optional<options::Value> nodeMapSourceValue()const{return nodeMap_->sourceValue();}
    void defaultKindValue(options::Value value) { defaultKind=std::move(value); }
    void defaultKindKey(DefaultKindKey convert) { defaultKindKey_=std::move(convert); }
    void nodeMapAccess(std::shared_ptr<FactoryClassMapAccess<Product>> replacement) {
        if(!replacement)type_error("Factory nodeMap requires a Map owner");
        nodeMap_=std::move(replacement);
    }
    void sourceNodeMapReader(SourceNodeMapReader reader){sourceNodeMapReader_=std::move(reader);}
    void sourceCreatorAccessReader(SourceCreatorAccessReader reader){sourceCreatorAccessReader_=std::move(reader);}

protected:
    Product* createDirect(std::string_view kind, FactoryArgs args) {
        return createDirectWithArguments(kind,[args]{
            return std::vector<std::any>(args.begin(),args.end());
        });
    }

    // Resolve the Source creator once, then build typed arguments and invoke
    // that captured creator. Context bridges use this same selection path so
    // iterable/property reads cannot change which class Source selected.
    template<class BuildArguments>
    Product* createDirectWithArguments(std::string_view kind,BuildArguments&& buildArguments) {
        return createDirectWithArgumentPhases(kind,
            std::forward<BuildArguments>(buildArguments),
            [](auto&& rawArguments)->decltype(auto){return std::forward<decltype(rawArguments)>(rawArguments);});
    }

    // One selection/evaluation/conversion/call sequence serves both ordinary
    // Source-style arguments and adapters that convert already-evaluated
    // Context values into native constructor arguments.
    template<class EvaluateRawArguments,class ConvertArguments>
    Product* createDirectWithArgumentPhases(std::string_view kind,
        EvaluateRawArguments&& evaluateRawArguments,ConvertArguments&& convertArguments) {
        const auto access=currentCreatorAccess();
        const auto requested = access->lookup(kind);
        auto rawArguments=std::invoke(std::forward<EvaluateRawArguments>(evaluateRawArguments));
        // Raw JS arguments are already evaluated at this point. Conversion
        // to native values may read Context properties or iterate children;
        // it belongs only to an actually selected callable.
        if(!access->directCallable(requested))
            return access->invokeDirect(requested,FactoryArgs{});
        auto arguments=std::invoke(std::forward<ConvertArguments>(convertArguments),rawArguments);
        return access->invokeDirect(requested,FactoryArgs(arguments));
    }

    std::shared_ptr<FactoryClassMapAccess<Product>> nodeMap_;
    std::shared_ptr<FactoryCreatorAccess<Product>> node_;
    std::shared_ptr<std::atomic_bool> factoryLifetime_=std::make_shared<std::atomic_bool>(true);

private:
    std::shared_ptr<FactoryClassMapAccess<Product>> currentNodeMapAccess()const{
        auto current=sourceNodeMapReader_?sourceNodeMapReader_():nodeMap_;
        if(!current)type_error("Factory nodeMap has no current owner");
        return current;
    }
    std::shared_ptr<FactoryCreatorAccess<Product>> currentCreatorAccess()const{
        auto current=sourceCreatorAccessReader_?sourceCreatorAccessReader_():node_;
        if(!current)type_error("Factory node has no current owner");
        return current;
    }

    using CreatorLookupKind=FactoryCreatorSelectionKind;
    using CreatorLookup=FactoryCreatorSelection<Product>;

    std::vector<std::unique_ptr<Product>> products_;
    std::uint64_t productGeneration_=0;

    void validateProductMark(ProductMark mark)const{
        if(mark.owner!=this||mark.generation!=productGeneration_||mark.size>products_.size())
            throw FactoryError("TypeError",u"Factory product mark does not belong to the current product sequence");
    }

    [[noreturn]] static void type_error(std::string_view message) {
        throw FactoryError("TypeError", std::u16string(message.begin(), message.end()));
    }

    static std::optional<std::uint32_t> array_index(std::string_view key) {
        if (key.empty() || (key.size() > 1 && key.front() == '0')) return std::nullopt;
        std::uint64_t value = 0;
        const auto [end, error] = std::from_chars(key.data(), key.data() + key.size(), value);
        if (error != std::errc{} || end != key.data() + key.size() || value >= std::numeric_limits<std::uint32_t>::max()) return std::nullopt;
        return static_cast<std::uint32_t>(value);
    }

    static void orderInitialMap(FactoryClassMap<Product>& entries) {
        FactoryClassMap<Product> unique;
        unique.reserve(entries.size());
        for (auto& [kind, type] : entries) {
            const auto found = std::find_if(unique.begin(), unique.end(), [&](const auto& entry) { return entry.first == kind; });
            if (found == unique.end()) unique.emplace_back(std::move(kind), std::move(type));
            else found->second = std::move(type);
        }
        std::stable_sort(unique.begin(), unique.end(), [](const auto& left, const auto& right) {
            const auto leftIndex = array_index(left.first);
            const auto rightIndex = array_index(right.first);
            if (leftIndex && rightIndex) return *leftIndex < *rightIndex;
            return leftIndex.has_value() && !rightIndex.has_value();
        });
        entries = std::move(unique);
    }

    CreatorLookup creator(std::string_view kind) const {
        return currentCreatorAccess()->lookup(kind);
    }
};
}
