/*************************************************************
 * MathJax 4.1.3 ts/output/common/FontData.ts
 * Copyright (c) 2017-2026 The MathJax Consortium; Apache-2.0.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
export module folia.math.output.common.font_data;
export import folia.math.output.common.direction;
export import folia.math.core.tree.property;
export import folia.math.util.option_types;
export import folia.math.util.promise;
export import folia.math.util.context_property_bindings;
export import folia.math.util.map_bindings;
export import folia.math.util.regexp_binding;
import std;

export namespace folia::math::output::common {
template<class Key,class Value> class OrderedMap {
    std::vector<std::pair<Key,Value>> entries_;
public:
    using iterator=typename std::vector<std::pair<Key,Value>>::iterator;
    using const_iterator=typename std::vector<std::pair<Key,Value>>::const_iterator;
    iterator begin(){return entries_.begin();} iterator end(){return entries_.end();}
    const_iterator begin()const{return entries_.begin();} const_iterator end()const{return entries_.end();}
    template<class K> iterator find(const K& key){return std::find_if(begin(),end(),[&](const auto& e){return e.first==key;});}
    template<class K> const_iterator find(const K& key)const{return std::find_if(begin(),end(),[&](const auto& e){return e.first==key;});}
    template<class K> bool contains(const K& key)const{return find(key)!=end();}
    bool empty()const{return entries_.empty();} std::size_t size()const{return entries_.size();}
    void clear(){entries_.clear();}
    Value& operator[](const Key& key){auto i=find(key);if(i==end()){entries_.emplace_back(key,Value{});return entries_.back().second;}return i->second;}
    template<class V> void insert_or_assign(Key key,V&& value){auto i=find(key);if(i==end())entries_.emplace_back(std::move(key),std::forward<V>(value));else i->second=std::forward<V>(value);}
    template<class K,class V> std::pair<iterator,bool> emplace(K&& key,V&& value){auto i=find(key);if(i!=end())return {i,false};entries_.emplace_back(std::forward<K>(key),std::forward<V>(value));return {std::prev(end()),true};}
    std::size_t erase(const Key& key){auto i=find(key);if(i==end())return 0;entries_.erase(i);return 1;}
    iterator erase(iterator i){return entries_.erase(i);}
};
inline constexpr double VFUZZ = 0.07;
inline constexpr double HFUZZ = 0.07;

struct CharOptions {
    std::optional<double> ic, oc, sk, dx;
    std::optional<bool> unknown;
    std::optional<std::uint32_t> smp;
    std::optional<std::array<double,2>> hd;
};
// MathJax character records are ordinary sparse JavaScript tuples. Keep the
// shared PropertyArray as their only storage so holes, undefined values and
// the fourth-slot options object retain their actual identity.
using CharDataArray = PropertyArray;
struct DynamicRange { std::uint32_t first=0, last=0; bool isRange=false; };
using DynamicRanges = std::vector<DynamicRange>;
using DynamicVariants = OrderedMap<std::string,DynamicRanges>;

struct DelimiterData {
    std::string dir;
    std::optional<std::vector<double>> sizes;
    std::optional<std::vector<double>> variants;
    std::optional<std::vector<double>> schar;
    std::optional<std::vector<double>> stretch;
    std::optional<std::vector<double>> stretchv;
    std::optional<std::array<double,3>> HDW;
    std::optional<std::array<double,2>> hd;
    std::optional<double> ext, min;
    std::optional<double> c;
    std::optional<std::array<double,2>> fullExt;
};
using DelimiterDataRef = std::shared_ptr<DelimiterData>;
inline const DelimiterDataRef NOSTRETCH = [] {
    auto value=std::make_shared<DelimiterData>();
    value->dir=std::string(NONE);
    return value;
}();
struct FontParameters {
    double x_height=0, quad=0, num1=0, num2=0, num3=0, denom1=0, denom2=0,
           sup1=0, sup2=0, sup3=0, sub1=0, sub2=0, sup_drop=0, sub_drop=0,
           delim1=0, delim2=0, axis_height=0, rule_thickness=0,
           big_op_spacing1=0, big_op_spacing2=0, big_op_spacing3=0,
           big_op_spacing4=0, big_op_spacing5=0, surd_height=0,
           scriptspace=0, nulldelimiterspace=0, delimiterfactor=0,
           delimitershortfall=0, rule_factor=0, min_rule_thickness=0,
           separation_factor=0, extra_ic=0, extender_factor=0;
    OrderedMap<std::string,Property> extra;
};
using FontParameterList = std::map<std::string,double,std::less<>>;
using CssFontData = PropertyArray;
using CssFontDataRef = PropertyArray;
using CssFontMap = OrderedMap<std::string,CssFontDataRef>;
using RemapMap = std::map<std::uint32_t,std::u16string>;
using RemapMapMap = OrderedMap<std::string,RemapMap>;
using SmpSource = std::variant<PropertyArray,std::string>;
using SmpRemap = std::map<std::uint32_t,std::uint32_t>;
struct CharacterMap;
struct FontVariantData {
    std::shared_ptr<CharacterMap> chars;
    std::vector<std::weak_ptr<CharacterMap>> linked;
    // SVG cache metadata is attached to the canonical variant record.
    std::optional<std::string> cacheID;
};
using FontVariantMap = OrderedMap<std::string,FontVariantData>;

struct DynamicFile;
using DynamicFileRef = std::shared_ptr<DynamicFile>;
// Glyph table slots may contain arbitrary Source values (including malformed
// object records); dynamic placeholders remain their own native identity.
using CharData = std::variant<Property,DynamicFileRef>;
using DelimiterValue = std::variant<DelimiterDataRef,DynamicFileRef,std::monostate>;
struct CharacterMap {
    std::map<std::uint32_t,CharData> own;
    std::shared_ptr<CharacterMap> prototype;
    bool hasOwn(std::uint32_t n) const { return own.contains(n); }
    std::optional<CharData> get(std::uint32_t n) const;
    void define(std::uint32_t n, CharData value);
    void erase(std::uint32_t n) { own.erase(n); }
    std::vector<std::uint32_t> ownKeys() const;
};
using CharMap = std::map<std::uint32_t,CharData>;
using CharMapMap = OrderedMap<std::string,CharMap>;
using DelimiterMap = std::map<std::uint32_t,DelimiterValue>;

struct DynamicFile {
    // Writable Source fields stay raw: Context assignment and later Source
    // reads must share one authority, including non-string replacements.
    options::Value extension=options::Undefined{}, file=options::Undefined{};
    DynamicVariants variants;
    DynamicRanges delimiters;
    async::PromiseRef promise;
    bool failed=false;
    std::function<void(class FontData&)> setup;
    std::optional<options::ObjectRef> sourceIdentity;
};
class FontDataClass;
options::ObjectRef bindDynamicFileSourceObject(FontDataClass&,const DynamicFileRef&);
using DynamicFileList = OrderedMap<std::string,DynamicFileRef>;
using BundledDynamicSetup = std::function<void(class FontDataClass&)>;
using BundledDynamicSetupMap = OrderedMap<std::string,BundledDynamicSetup>;
struct DynamicFont {
    std::string name, prefix;
    DynamicFileList files;
    std::size_t sizeN=0, stretchN=0;
    std::optional<options::ObjectRef> sourceIdentity;
};
struct DynamicFileDef { std::string file; DynamicVariants variants; DynamicRanges delimiters; };
struct ListPatch {
    // These are the actual Source arrays.  Keep their shared PropertyArray
    // storage so holes, length, and aliases survive until the merge operation.
    // `replace` records the distinct Array.isArray(source) path in Options.insert;
    // append/remove are the `[+]`/`[-]` object-patch operands.
    std::optional<PropertyArray> replace;
    std::optional<PropertyArray> append;
    std::optional<PropertyArray> remove;
};
using VariantDefinition = PropertyArray;

struct FontExtensionData {
    std::string name;
    // SVG extension metadata travels with the common Source extension record;
    // common FontData ignores it, while SvgFontDataClass consumes it after the
    // inherited static addExtension body returns.
    std::optional<std::map<std::string,std::string,std::less<>>> cacheIds;
    std::optional<options::Value> options;
    std::optional<ListPatch> variants;
    std::optional<OrderedMap<std::string,SmpSource>> variantSmp;
    std::optional<CssFontMap> cssFonts;
    std::optional<RemapMap> accentMap, moMap, mnMap;
    std::optional<FontParameterList> parameters;
    std::optional<DelimiterMap> delimiters;
    std::optional<CharMapMap> chars;
    std::optional<ListPatch> sizeVariants, stretchVariants;
    std::optional<std::vector<DynamicFileDef>> ranges;
};

struct FontLoadHost {
    async::QueueRef queue;
    // Same per-session RegExp owner used by parser and Lite; no second host.
    regexp_host::RegExpBindings* regexp=nullptr;
    // Optional native observation hook; receives the exact computed request
    // immediately before the loader callback, without recomputing the URL.
    // Observer exceptions are ignored so instrumentation cannot change Source
    // loading behavior.
    std::function<void(std::string_view)> requestObserver;
    // The loader receives the same Source receiver and DynamicFile record,
    // plus the request name already computed by dynamicFileName().
    std::function<async::PromiseRef(std::shared_ptr<class FontData>,
        DynamicFileRef,std::string)> asyncLoad;
    std::function<void(class FontData&,const DynamicFileRef&,std::string)> syncLoad;
    std::function<bool()> asyncIsSynchronous;
    // Generic Source console.warn(...args) calls retain their argument-list
    // envelope independently of font-loader failure reasons below.
    std::function<void(std::vector<std::any>)> consoleWarn;
    // Font loading reports one native reason value, not a console argument
    // list. Keep this contract separate from Source console.warn.
    std::function<void(const std::any&)> warn;
};

struct FontDataClass {
    options::Value OPTIONS;
    options::Context* optionsContext = nullptr;
    ContextPropertyBindings* sourceProperties = nullptr;
    options::MapContextBindings* sourceMaps = nullptr;
    std::optional<options::ObjectRef> sourceIdentity;
    options::Value dynamicExtensions=options::Undefined{};
    std::string JAX="common", NAME;
    Property defaultVariants=PropertyArray{};
    CssFontMap defaultCssFonts;
    std::string defaultCssFamilyPrefix;
    OrderedMap<std::string,SmpSource> VariantSmp;
    std::vector<std::array<int,3>> SmpRanges;
    SmpRemap SmpRemapMap, SmpRemapGreekU, SmpRemapGreekL;
    RemapMap defaultAccentMap, defaultMoMap, defaultMnMap;
    FontParameters defaultParams;
    DelimiterMap defaultDelimiters;
    CharMapMap defaultChars;
    Property defaultSizeVariants=PropertyArray{}, defaultStretchVariants=PropertyArray{};
    DynamicFileList dynamicFiles;
    // Native bundle recipes are not Source-visible DynamicFile.setup state.
    // They are activated only after the corresponding provider load succeeds.
    BundledDynamicSetupMap bundledDynamicSetups;

    explicit FontDataClass(options::Context&);
    void bindSourceIdentity(ContextPropertyBindings&,options::ObjectRef,
                            options::MapContextBindings&);
    static Property charOptions(CharMap& font,std::uint32_t n);
    static DynamicFileList defineDynamicFiles(const std::vector<DynamicFileDef>&,std::string extension={});
    void dynamicSetup(std::string_view extension,std::string_view file,
                      CharMapMap chars,DelimiterMap delimiters={},std::vector<std::string> fonts={});
    void registerBundledDynamicSetup(std::string extension,std::string file,BundledDynamicSetup);
    void activateBundledDynamicSetup(const DynamicFileRef&);
    static double adjustArrayIndex(double n,double N);
    static void adjustDelimiters(DelimiterMap&,const std::vector<std::uint32_t>&,std::size_t sizeN,std::size_t stretchN);
    void addExtension(const FontExtensionData&,std::string prefix={});
};

class FontData : public std::enable_shared_from_this<FontData> {
protected:
    friend struct FontDataClass;
    FontDataClass* class_;
    FontLoadHost* host_;
    options::Context* context_;
    ContextPropertyBindings* sourceProperties_ = nullptr;
    std::optional<options::ObjectRef> sourceIdentity_;
    options::Value options_;
    FontVariantMap variant_;
    DelimiterMap delimiters_;
    Property sizeVariants_=PropertyArray{}, stretchVariants_=PropertyArray{};
    CssFontMap cssFontMap_;
    RemapMapMap remapChars_;
    FontParameters params_;
    Property styles_ = Missing{};
public:
    std::string cssFamilyPrefix;
    std::string cssFontPrefix;
    double skewIcFactor=0.75;
    FontData(FontDataClass&,FontLoadHost&,options::Context&,options::Value options=options::Undefined{});
    void bindSourceIdentity(ContextPropertyBindings&,options::ObjectRef);
    virtual ~FontData()=default;
    FontData(const FontData&)=delete;
    FontData& operator=(const FontData&)=delete;
    FontDataClass& CLASS() const { return *class_; }
    options::Value options() const { return options_; }
    // Forward the actual console.warn(...args) call shape. Its argument list
    // remains distinct from FontLoadHost::warn's loader-reason contract.
    void sourceConsoleWarn(std::vector<std::any> arguments) const {
        if(host_&&host_->consoleWarn)host_->consoleWarn(std::move(arguments));
    }
    // Source constructor writes `this.options = userOptions(...)`; this is
    // intentionally replacement, distinct from the later setOptions merge.
    void replaceOptions(options::Value value) { options_ = std::move(value); }
    FontParameters& params() { return params_; }
    const FontParameters& params() const { return params_; }
    void setOptions(options::Value);
    std::vector<std::string> addExtension(const FontExtensionData&,std::string prefix={});
    // These Source-protected TypeScript methods are own enumerable runtime
    // members and are intentionally exposed to the canonical Context surface.
    void remapSmpChars(const std::shared_ptr<CharacterMap>&,std::string_view name);
    PropertyArray smpChar(std::uint32_t n) const;
    std::map<std::uint32_t,DynamicFileRef> flattenRanges(const DynamicRanges&,const DynamicFileRef&) const;
    options::Value dynamicFileName(options::Value dynamic) const;
    Property styles() const { return styles_; }
    void setStyles(Property value) { styles_=std::move(value); }
    void createVariant(std::string name,std::optional<std::string> inherit={},std::optional<std::string> link={});
    void createVariants(const PropertyArray&);
    void defineChars(std::string_view name,const CharMap&);
    void defineCssFonts(const CssFontMap&);
    void defineDelimiters(const DelimiterMap&);
    void defineRemap(std::string_view name,const RemapMap* remap);
    void defineDynamicCharacters(const DynamicFileList& files);
    async::PromiseRef loadDynamicFile(const DynamicFileRef&);
    async::PromiseRef loadDynamicFiles();
    void loadDynamicFilesSync();
    void loadDynamicFileSync(const DynamicFileRef&);
    virtual void addDynamicFontCss(const std::vector<std::string>&,std::optional<std::string> root={});
    std::optional<DelimiterValue> getDelimiter(double n);
    std::optional<std::string> getSizeVariant(double n,double i);
    std::optional<std::string> getStretchVariant(double n,double i);
    std::array<std::optional<std::string>,4> getStretchVariants(double n);
    std::optional<Property> getChar(std::string_view name,double n);
    std::optional<std::reference_wrapper<FontVariantData>> getVariant(std::string_view name);
    CssFontData getCssFont(std::string_view variant) const;
    std::string getFamily(std::string_view family) const;
    std::optional<std::u16string> getRemappedChar(std::string_view name,double c) const;
    const FontVariantMap& variants() const { return variant_; }
    const DelimiterMap& delimiters() const { return delimiters_; }
    const Property& sizeVariants() const { return sizeVariants_; }
    Property& sizeVariants() { return sizeVariants_; }
    const Property& stretchVariants() const { return stretchVariants_; }
    Property& stretchVariants() { return stretchVariants_; }
    const CssFontMap& cssFonts() const { return cssFontMap_; }
    const RemapMapMap& remapChars() const { return remapChars_; }
};
}
