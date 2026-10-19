/*************************************************************
 * Source port of MathJax 4.1.3 ts/output/common/FontData.ts.
 * Copyright (c) 2017-2026 The MathJax Consortium; Apache-2.0.
 * See src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.output.common.font_data;
import folia.math.util.options;
import folia.math.util.retries;
import folia.math.util.property_host;
import folia.math.core.tree.factory_types;
import folia.math.util.context_property_bindings;
import folia.math.output.common.font_data_options;
import std;

namespace folia::math::output::common {
namespace {
[[noreturn]] void typeError(std::u16string message){throw tree::FactoryError("TypeError",std::move(message));}
[[noreturn]] void rangeError(std::u16string message){throw tree::FactoryError("RangeError",std::move(message));}
std::string utf8(std::u16string_view text){return property_host::debugText(text);}
std::u16string u16(std::string_view text){return property_host::text(text);}
std::string sourceString(ContextPropertyBindings* properties,const options::Value& value){
    if(!properties)throw options::Exception("Unimplemented",u"DynamicFile value conversion requires its Source Context");
    return utf8(property_host::string(properties->from(value)));
}
options::Value dynamicFileFieldValue(ContextPropertyBindings* properties,
    const DynamicFileRef& file,std::u16string_view field){
    if(!file)typeError(u"Cannot read properties of null DynamicFile");
    if(properties&&file->sourceIdentity){
        return properties->context().get(options::Value(*file->sourceIdentity),
            options::Key(std::u16string(field)));
    }
    if(field==u"extension")return file->extension;
    if(field==u"file")return file->file;
    return options::Value(options::Undefined{});
}
std::u16string upperHex(std::uint32_t n){
    constexpr char16_t digits[]=u"0123456789ABCDEF";std::u16string result;
    do{result.push_back(digits[n&15]);n>>=4;}while(n);std::reverse(result.begin(),result.end());return result;
}
std::string lowerHex(std::uint32_t n){
    constexpr char digits[]="0123456789abcdef";std::string result;
    do{result.push_back(digits[n&15]);n>>=4;}while(n);std::reverse(result.begin(),result.end());return result;
}
options::Value callMapMethod(options::Context& context,options::Value map,
    std::u16string_view method,std::span<const options::Value> arguments){
    const auto callable=context.get(map,options::Key(std::u16string(method)));
    const auto* function=std::get_if<options::ObjectRef>(&callable.data);
    if(!function||context.kind(*function)!=options::ObjectKind::function)
        typeError(u"Map."+std::u16string(method)+u" is not a function");
    return context.call(*function,std::move(map),arguments);
}
options::Value classDynamicExtensions(FontDataClass& owner){
    if(owner.sourceProperties&&owner.sourceIdentity)
        return owner.sourceProperties->context().get(options::Value(*owner.sourceIdentity),options::Key(u"dynamicExtensions"));
    return owner.dynamicExtensions;
}
std::shared_ptr<DynamicFont> dynamicFontOwner(options::Context& context,
    const options::Value& value){
    const auto* object=std::get_if<options::ObjectRef>(&value.data);
    if(!object)typeError(u"Cannot read properties of undefined (reading 'files')");
    (void)context.kind(*object);
    const auto* payload=std::any_cast<std::shared_ptr<DynamicFont>>(&context.payload(*object));
    if(!payload||!*payload)throw options::Exception("Unimplemented",u"Dynamic extension has no canonical native owner");
    return *payload;
}
std::shared_ptr<DynamicFile> dynamicFileOwner(options::Context& context,
    const options::Value& value){
    const auto* object=std::get_if<options::ObjectRef>(&value.data);
    if(!object)typeError(u"Cannot read properties of undefined (reading 'setup')");
    (void)context.kind(*object);
    const auto* payload=std::any_cast<std::shared_ptr<DynamicFile>>(&context.payload(*object));
    if(!payload||!*payload)throw options::Exception("Unimplemented",u"Dynamic file has no canonical native owner");
    return *payload;
}
options::ObjectRef makeDynamicFontObject(FontDataClass& owner,
    const std::shared_ptr<DynamicFont>& dynamic){
    if(!owner.sourceProperties)throw options::Exception("Unimplemented",u"Dynamic extension requires its Source Context owner");
    auto& context=owner.sourceProperties->context();
    const auto identity=context.object();
    dynamic->sourceIdentity=identity;
    context.payload(identity)=dynamic;
    const auto lease=std::static_pointer_cast<void>(dynamic);
    auto* properties=owner.sourceProperties;
    context.bindData(identity,options::Key(u"name"),std::make_shared<options::NativeDataField>(lease,
        [dynamic]{return options::Value(property_host::text(dynamic->name));},
        [dynamic,properties](options::Value value){dynamic->name=property_host::debugText(
            property_host::string(properties->from(std::move(value))));}));
    context.bindData(identity,options::Key(u"prefix"),std::make_shared<options::NativeDataField>(lease,
        [dynamic]{return options::Value(property_host::text(dynamic->prefix));},
        [dynamic,properties](options::Value value){dynamic->prefix=property_host::debugText(
            property_host::string(properties->from(std::move(value))));}));
    const auto files=context.object();
    for(const auto& [name,file]:dynamic->files){
        const auto fileIdentity=bindDynamicFileSourceObject(owner,file);
        context.set(options::Value(files),options::Key(property_host::text(name)),options::Value(fileIdentity));
    }
    context.set(options::Value(identity),options::Key(u"files"),options::Value(files));
    // The Source object literal inserts files before the two variant counts;
    // preserve that own-key order for reflection and later user mutation.
    context.bindData(identity,options::Key(u"sizeN"),std::make_shared<options::NativeDataField>(lease,
        [dynamic]{return options::Value(static_cast<double>(dynamic->sizeN));},
        [dynamic,&context](options::Value value){dynamic->sizeN=static_cast<std::size_t>(context.toNumber(value));}));
    context.bindData(identity,options::Key(u"stretchN"),std::make_shared<options::NativeDataField>(lease,
        [dynamic]{return options::Value(static_cast<double>(dynamic->stretchN));},
        [dynamic,&context](options::Value value){dynamic->stretchN=static_cast<std::size_t>(context.toNumber(value));}));
    return identity;
}
options::ObjectRef requireSourceReceiver(ContextPropertyBindings* bindings,
    const std::optional<options::ObjectRef>& receiver,std::u16string_view owner){
    if(!bindings||!receiver)
        throw options::Exception("Unimplemented",std::u16string(owner)+
            u".addExtension requires its canonical Source receiver");
    (void)bindings->context().kind(*receiver);
    return *receiver;
}
Property optionSource(ContextPropertyBindings& bindings,
    const std::optional<options::Value>& value){
    if(!value)return Property(Missing{});
    return bindings.from(*value);
}
Property patchSource(const std::optional<ListPatch>& patch){
    return patch?listPatchSource(*patch):Property(Missing{});
}
void setParameter(FontParameters& p,std::string_view key,double value){
    using Pair=std::pair<std::string_view,double FontParameters::*>;
    static constexpr Pair fields[]={
        {"x_height",&FontParameters::x_height},{"quad",&FontParameters::quad},{"num1",&FontParameters::num1},
        {"num2",&FontParameters::num2},{"num3",&FontParameters::num3},{"denom1",&FontParameters::denom1},
        {"denom2",&FontParameters::denom2},{"sup1",&FontParameters::sup1},{"sup2",&FontParameters::sup2},
        {"sup3",&FontParameters::sup3},{"sub1",&FontParameters::sub1},{"sub2",&FontParameters::sub2},
        {"sup_drop",&FontParameters::sup_drop},{"sub_drop",&FontParameters::sub_drop},{"delim1",&FontParameters::delim1},
        {"delim2",&FontParameters::delim2},{"axis_height",&FontParameters::axis_height},{"rule_thickness",&FontParameters::rule_thickness},
        {"big_op_spacing1",&FontParameters::big_op_spacing1},{"big_op_spacing2",&FontParameters::big_op_spacing2},
        {"big_op_spacing3",&FontParameters::big_op_spacing3},{"big_op_spacing4",&FontParameters::big_op_spacing4},
        {"big_op_spacing5",&FontParameters::big_op_spacing5},{"surd_height",&FontParameters::surd_height},
        {"scriptspace",&FontParameters::scriptspace},{"nulldelimiterspace",&FontParameters::nulldelimiterspace},
        {"delimiterfactor",&FontParameters::delimiterfactor},{"delimitershortfall",&FontParameters::delimitershortfall},
        {"rule_factor",&FontParameters::rule_factor},{"min_rule_thickness",&FontParameters::min_rule_thickness},
        {"separation_factor",&FontParameters::separation_factor},{"extra_ic",&FontParameters::extra_ic},
        {"extender_factor",&FontParameters::extender_factor}};
    for(const auto&[name,member]:fields)if(name==key){p.*member=value;return;}
    p.extra.insert_or_assign(std::string(key),Property(value));
}
void mergeParameters(FontParameters& target,const FontParameterList& source){for(const auto&[key,value]:source)setParameter(target,key,value);}
std::shared_ptr<DynamicFont> findDynamicFont(FontDataClass& owner,std::string_view name){
    if(!owner.sourceProperties)return {};
    auto& context=owner.sourceProperties->context();
    const auto map=classDynamicExtensions(owner);
    const std::array<options::Value,1> arguments{options::Value(property_host::text(name))};
    const auto value=callMapMethod(context,map,u"get",arguments);
    if(std::holds_alternative<options::Undefined>(value.data)||std::holds_alternative<options::Null>(value.data))return {};
    return dynamicFontOwner(context,value);
}
void forEachDynamicExtension(FontDataClass& owner,
    const std::function<void(options::Value)>& body){
    if(!owner.sourceProperties)return;
    auto& context=owner.sourceProperties->context();
    const auto map=classDynamicExtensions(owner);
    const auto callback=context.function(u"",[&body](options::Context&,options::Value,
        std::span<const options::Value> arguments)->options::Value{
        body(arguments.empty()?options::Value(options::Undefined{}):arguments[0]);
        return options::Undefined{};
    });
    const std::array<options::Value,1> arguments{options::Value(callback)};
    (void)callMapMethod(context,map,u"forEach",arguments);
}
void forEachDynamicExtensionValue(FontDataClass& owner,
    const std::function<void(options::Value)>& body){
    if(!owner.sourceProperties)return;
    auto& context=owner.sourceProperties->context();
    const auto map=classDynamicExtensions(owner);
    const auto values=callMapMethod(context,map,u"values",std::span<const options::Value>{});
    context.forEachIterable(values,[&](options::Value extension){body(std::move(extension));});
}
std::vector<DynamicFileRef> extensionFiles(options::Context& context,
    const options::Value& extension){
    const auto files=context.get(extension,options::Key(u"files"));
    const auto keys=context.enumerableStringKeys(files);
    std::vector<DynamicFileRef> result;result.reserve(keys.size());
    for(const auto& key:keys){
        const auto value=context.get(files,options::Key(key));
        result.push_back(dynamicFileOwner(context,value));
    }
    return result;
}
options::Value addFontExtensionToMap(FontDataClass& owner,
    const std::shared_ptr<DynamicFont>& dynamic){
    const auto map=classDynamicExtensions(owner);
    const auto object=makeDynamicFontObject(owner,dynamic);
    const std::array<options::Value,2> arguments{
        options::Value(property_host::text(dynamic->name)),options::Value(object)};
    return callMapMethod(owner.sourceProperties->context(),map,u"set",arguments);
}
std::optional<std::size_t> findVariant(const std::vector<std::string>& values,std::string_view value){
    for(std::size_t i=0;i<values.size();++i)if(values[i]==value)return i;return {};
}
std::optional<std::size_t> arrayIndex(double value,std::size_t size){
    if(!std::isfinite(value)||value<0||std::trunc(value)!=value||value>=static_cast<double>(size))return {};
    return static_cast<std::size_t>(value);
}
PropertyArray requireArrayField(const Property& value,std::u16string_view field){
    if(const auto array=PropertyArray::array(value))return *array;
    const auto name=std::u16string(field);
    if(value.missing())typeError(u"Cannot read properties of undefined (reading 'length')");
    if(std::holds_alternative<PropertyNull>(value.value))typeError(u"Cannot read properties of null (reading 'length')");
    typeError(u"Source font field is not an array: "+name);
}
std::size_t sourceArrayLength(ContextPropertyBindings* bindings,
    const std::optional<options::ObjectRef>& identity,std::u16string_view key,
    const Property& field){
    if(bindings&&identity){
        auto& context=bindings->context();
        const auto value=context.get(options::Value(*identity),options::Key(std::u16string(key)));
        if(std::holds_alternative<options::Null>(value.data))
            typeError(u"Cannot read properties of null (reading 'length')");
        if(std::holds_alternative<options::Undefined>(value.data))
            typeError(u"Cannot read properties of undefined (reading 'length')");
        const auto length=context.get(value,options::Key(u"length"));
        const auto number=context.toNumber(length);
        if(!std::isfinite(number)||number<0||number>static_cast<double>(std::numeric_limits<std::size_t>::max()))
            rangeError(u"Invalid font variant array length");
        return static_cast<std::size_t>(number);
    }
    return requireArrayField(field,key).size();
}
std::optional<std::string> optionString(options::Context& context,const options::Value& object,std::u16string_view key){
    auto value=context.get(object,options::Key(std::u16string(key)));
    if(const auto* text=std::get_if<std::u16string>(&value.data))return utf8(*text);
    return {};
}
async::PromiseRef rejected(const async::QueueRef& queue,std::string name,std::u16string message){
    auto promise=async::Promise::create(queue);
    promise->reject(std::make_exception_ptr(tree::FactoryError(std::move(name),std::move(message))));
    return promise;
}
VariantDefinition variantDef(std::string name,std::optional<std::string> inherit={},std::optional<std::string> link={}){
    VariantDefinition result;result.push(Property(std::move(name)));
    if(inherit||link)result.push(inherit?Property(std::move(*inherit)):Property(Missing{}));
    if(link)result.push(Property(std::move(*link)));
    return result;
}
std::optional<std::string> optionalVariantName(const Property& tuple,std::size_t index){
    auto value=property_host::get(tuple,std::to_string(index));if(!property_host::truthy(value))return {};
    return utf8(property_host::string(value));
}
std::string requiredVariantName(const Property& tuple,std::size_t index){
    return utf8(property_host::string(property_host::get(tuple,std::to_string(index))));
}
}

options::ObjectRef bindDynamicFileSourceObject(FontDataClass& owner,const DynamicFileRef& file){
    if(!file)throw options::Exception("TypeError",u"Dynamic file owner is null");
    if(!owner.sourceProperties)
        throw options::Exception("Unimplemented",u"Dynamic file requires its Source FontData class");
    auto& context=owner.sourceProperties->context();
    if(file->sourceIdentity){
        (void)context.kind(*file->sourceIdentity);
        return *file->sourceIdentity;
    }
    const auto identity=context.object();
    file->sourceIdentity=identity;
    context.payload(identity)=file;
    const auto lease=std::static_pointer_cast<void>(file);
    auto* properties=owner.sourceProperties;
    context.bindData(identity,options::Key(u"extension"),std::make_shared<options::NativeDataField>(lease,
        [file]{return file->extension;},
        [file](options::Value value){file->extension=std::move(value);},
        [file]{file->extension=options::Undefined{};}));
    context.bindData(identity,options::Key(u"file"),std::make_shared<options::NativeDataField>(lease,
        [file]{return file->file;},
        [file](options::Value value){file->file=std::move(value);},
        [file]{file->file=options::Undefined{};}));
    context.bindData(identity,options::Key(u"failed"),std::make_shared<options::NativeDataField>(lease,
        [file]{return options::Value(file->failed);},[file,properties](options::Value value){
            file->failed=property_host::truthy(properties->from(std::move(value)));}));
    return identity;
}

std::optional<CharData> CharacterMap::get(std::uint32_t n) const{
    if(auto it=own.find(n);it!=own.end())return it->second;
    return prototype?prototype->get(n):std::optional<CharData>{};
}
void CharacterMap::define(std::uint32_t n,CharData value){own.insert_or_assign(n,std::move(value));}
std::vector<std::uint32_t> CharacterMap::ownKeys()const{
    std::vector<std::uint32_t> result;result.reserve(own.size());for(const auto&[n,_]:own)result.push_back(n);return result;
}

FontDataClass::FontDataClass(options::Context& context):optionsContext(&context){
    const auto object=context.object();OPTIONS=options::Value(object);
    context.set(OPTIONS,options::Key(u"unknownFamily"),options::Value(u"serif"));
    context.set(OPTIONS,options::Key(u"dynamicPrefix"),options::Value(u"."));
    defaultVariants=PropertyArray{
        Property(variantDef("normal")),Property(variantDef("bold","normal")),Property(variantDef("italic","normal")),
        Property(variantDef("bold-italic","italic","bold")),Property(variantDef("double-struck","bold")),
        Property(variantDef("fraktur","normal")),Property(variantDef("bold-fraktur","bold","fraktur")),
        Property(variantDef("script","italic")),Property(variantDef("bold-script","bold-italic","script")),
        Property(variantDef("sans-serif","normal")),Property(variantDef("bold-sans-serif","bold","sans-serif")),
        Property(variantDef("sans-serif-italic","italic","sans-serif")),
        Property(variantDef("sans-serif-bold-italic","bold-italic","bold-sans-serif")),
        Property(variantDef("monospace","normal")),Property(variantDef("-smallop","normal")),
        Property(variantDef("-largeop","normal")),Property(variantDef("-tex-calligraphic","italic")),
        Property(variantDef("-tex-bold-calligraphic","bold-italic")),Property(variantDef("-tex-oldstyle","normal")),
        Property(variantDef("-tex-bold-oldstyle","bold")),Property(variantDef("-tex-mathit","italic")),
        Property(variantDef("-tex-variant","normal"))};
    defaultSizeVariants=PropertyArray{};
    defaultStretchVariants=PropertyArray{};
    auto css=[&](std::string name,std::string family,bool italic,bool bold){defaultCssFonts.emplace(std::move(name),CssFontData{Property(std::move(family)),Property(italic),Property(bold)});};
    css("normal","unknown",false,false);css("bold","unknown",false,true);css("italic","unknown",true,false);
    css("bold-italic","unknown",true,true);css("double-struck","unknown",false,true);css("fraktur","unknown",false,false);
    css("bold-fraktur","unknown",false,true);css("script","cursive",false,false);css("bold-script","cursive",false,true);
    css("sans-serif","sans-serif",false,false);css("bold-sans-serif","sans-serif",false,true);
    css("sans-serif-italic","sans-serif",true,false);css("sans-serif-bold-italic","sans-serif",true,true);
    css("monospace","monospace",false,false);css("-smallop","unknown",false,false);css("-largeop","unknown",false,false);
    css("-tex-calligraphic","cursive",true,false);css("-tex-bold-calligraphic","cursive",true,true);
    css("-tex-oldstyle","unknown",false,false);css("-tex-bold-oldstyle","unknown",false,true);
    css("-tex-mathit","unknown",true,false);css("-tex-variant","unknown",false,false);
    auto smp=[&](std::string name,std::initializer_list<std::optional<std::uint32_t>> starts){
        PropertyArray data;data.resize(starts.size());std::size_t i=0;
        for(auto value:starts){if(value)data.set(i,Property(static_cast<double>(*value)));++i;}
        VariantSmp.emplace(std::move(name),std::move(data));
    };
    smp("bold",{0x1d400,0x1d41a,0x1d6a8,0x1d6c2,0x1d7ce});
    std::get<PropertyArray>(VariantSmp["bold"]).set(5,Property(PropertyList{{"988",Property(0x1d7ca)},{"989",Property(0x1d7cb)}}));
    smp("italic",{0x1d434,0x1d44e,0x1d6e2,0x1d6fc});
    smp("bold-italic",{0x1d468,0x1d482,0x1d71c,0x1d736});smp("script",{0x1d49c,0x1d4b6});
    smp("bold-script",{0x1d4d0,0x1d4ea});smp("fraktur",{0x1d504,0x1d51e});
    smp("double-struck",{0x1d538,0x1d552,std::nullopt,std::nullopt,0x1d7d8});
    smp("bold-fraktur",{0x1d56c,0x1d586});smp("sans-serif",{0x1d5a0,0x1d5ba,std::nullopt,std::nullopt,0x1d7e2});
    smp("bold-sans-serif",{0x1d5d4,0x1d5ee,0x1d756,0x1d770,0x1d7ec});
    smp("sans-serif-italic",{0x1d608,0x1d622});smp("sans-serif-bold-italic",{0x1d63c,0x1d656,0x1d790,0x1d7aa});
    smp("monospace",{0x1d670,0x1d68a,std::nullopt,std::nullopt,0x1d7f6});
    SmpRanges={{0,0x41,0x5a},{1,0x61,0x7a},{2,0x391,0x3a9},{3,0x3b1,0x3c9},{4,0x30,0x39}};
    SmpRemapMap={{0x1d455,0x210e},{0x1d49d,0x212c},{0x1d4a0,0x2130},{0x1d4a1,0x2131},{0x1d4a3,0x210b},{0x1d4a4,0x2110},{0x1d4a7,0x2112},{0x1d4a8,0x2133},{0x1d4ad,0x211b},{0x1d4ba,0x212f},{0x1d4bc,0x210a},{0x1d4c4,0x2134},{0x1d506,0x212d},{0x1d50b,0x210c},{0x1d50c,0x2111},{0x1d515,0x211c},{0x1d51d,0x2128},{0x1d53a,0x2102},{0x1d53f,0x210d},{0x1d545,0x2115},{0x1d547,0x2119},{0x1d548,0x211a},{0x1d549,0x211d},{0x1d551,0x2124}};
    SmpRemapGreekU={{0x2207,0x19},{0x3f4,0x11}};
    SmpRemapGreekL={{0x3d1,0x1b},{0x3d5,0x1d},{0x3d6,0x1f},{0x3f0,0x1c},{0x3f1,0x1e},{0x3f5,0x1a},{0x2202,0x19}};
    defaultAccentMap={{0x5e,u"\u02c6"},{0x7e,u"\u02dc"},{0x300,u"\u02cb"},{0x301,u"\u02ca"},{0x302,u"\u02c6"},{0x303,u"\u02dc"},{0x304,u"\u02c9"},{0x306,u"\u02d8"},{0x307,u"\u02d9"},{0x308,u"\u00a8"},{0x30a,u"\u02da"},{0x30c,u"\u02c7"},{0x2192,u"\u20d7"}};
    defaultMoMap={{0x2d,u"\u2212"}};defaultMnMap=defaultMoMap;
    defaultParams={.x_height=.442,.quad=1,.num1=.676,.num2=.394,.num3=.444,.denom1=.686,.denom2=.345,
      .sup1=.413,.sup2=.363,.sup3=.289,.sub1=.15,.sub2=.247,.sup_drop=.386,.sub_drop=.05,
      .delim1=2.39,.delim2=1,.axis_height=.25,.rule_thickness=.06,.big_op_spacing1=.111,
      .big_op_spacing2=.167,.big_op_spacing3=.2,.big_op_spacing4=.6,.big_op_spacing5=.1,
      .surd_height=.06,.scriptspace=.05,.nulldelimiterspace=.12,.delimiterfactor=901,
      .delimitershortfall=.3,.rule_factor=1.25,.min_rule_thickness=1.25,.separation_factor=1.75,
      .extra_ic=.033,.extender_factor=.333};
}

Property FontDataClass::charOptions(CharMap& font,std::uint32_t n){
    auto it=font.find(n);
    const auto errorMessage=u"Character data hasn't been loaded for 0x"+upperHex(n);
    auto invalid=[&]()->void{throw tree::FactoryError("Error",errorMessage);};
    if(it==font.end())invalid();
    auto* record=std::get_if<Property>(&it->second);
    if(!record)invalid();
    auto array=PropertyArray::array(*record);
    if(!array)invalid();
    const auto length=property_host::get(*record,"length");
    if(const auto* number=std::get_if<double>(&length.value);number&&*number==3){
        array->push(Property(PropertyList{}));
        return array->get(3);
    }
    return property_host::get(*record,"3");
}

DynamicFileList FontDataClass::defineDynamicFiles(const std::vector<DynamicFileDef>& definitions,std::string extension){
    DynamicFileList result;
    for(const auto& definition:definitions){
        auto file=std::make_shared<DynamicFile>();
        file->extension=options::Value(property_host::text(extension));
        file->file=options::Value(property_host::text(definition.file));
        file->variants=definition.variants;file->delimiters=definition.delimiters;
        // Match the Source placeholder installed by defineDynamicFiles. A
        // successfully evaluated dynamic module replaces this callback.
        std::weak_ptr<DynamicFile> fileLease=file;
        file->setup=[fileLease](FontData&){if(const auto current=fileLease.lock())current->failed=true;};
        result.insert_or_assign(definition.file,std::move(file));
    }
    return result;
}

void FontDataClass::registerBundledDynamicSetup(std::string extension,std::string file,BundledDynamicSetup setup){
    bundledDynamicSetups.insert_or_assign(extension+"\x1f"+file,std::move(setup));
}

void FontDataClass::activateBundledDynamicSetup(const DynamicFileRef& file){
    if(!file)return;
    // Module evaluation executes Source dynamicSetup and replaces the
    // placeholder callback installed by defineDynamicFiles.
    const auto key=sourceString(sourceProperties,dynamicFileFieldValue(sourceProperties,file,u"extension"))+"\x1f"+
                   sourceString(sourceProperties,dynamicFileFieldValue(sourceProperties,file,u"file"));
    if(auto it=bundledDynamicSetups.find(key);it!=bundledDynamicSetups.end()&&it->second)it->second(*this);
}

void FontDataClass::dynamicSetup(std::string_view extension,std::string_view file,
                                CharMapMap chars,DelimiterMap delimiters,std::vector<std::string> fonts){
    DynamicFileRef target;
    std::shared_ptr<DynamicFont> dynamicData;
    if(extension.empty()){
        if(auto it=dynamicFiles.find(file);it!=dynamicFiles.end())target=it->second;
    }else if(auto data=findDynamicFont(*this,extension)){
        dynamicData=data;
        if(data->sourceIdentity&&sourceProperties){
            auto& context=sourceProperties->context();
            const auto files=context.get(options::Value(*data->sourceIdentity),options::Key(u"files"));
            const auto fileValue=context.get(files,options::Key(property_host::text(file)));
            if(!std::holds_alternative<options::Undefined>(fileValue.data))target=dynamicFileOwner(context,fileValue);
        }
    }
    if(!target)typeError(u"Cannot set up unknown dynamic font file '"+u16(file)+u"'");
    // The Source callback re-reads `dynamicExtensions.get(extension)` when it
    // runs. Keep only a weak lease to avoid the C++ cycle
    // class -> extension -> file -> setup -> extension.
    std::weak_ptr<DynamicFont> dynamicLease=dynamicData;
    target->setup=[chars=std::move(chars),delimiters=std::move(delimiters),fonts=std::move(fonts),dynamicLease,hasExtension=!extension.empty()](FontData& font){
        for(const auto&[name,map]:chars)font.defineChars(name,map);
        font.defineDelimiters(delimiters);
        if(hasExtension){
            const auto dynamicData=dynamicLease.lock();
            if(!dynamicData)typeError(u"Cannot read properties of undefined (reading 'sizeN')");
            std::vector<std::uint32_t> ids;for(const auto&[cp,_]:delimiters)ids.push_back(cp);
            FontDataClass::adjustDelimiters(font.delimiters_,ids,dynamicData->sizeN,dynamicData->stretchN);
        }
        if(!fonts.empty())font.addDynamicFontCss(fonts);
    };
}

double FontDataClass::adjustArrayIndex(double n,double N){return n<0?N-1-n:n;}

void FontDataClass::adjustDelimiters(DelimiterMap& delimiters,const std::vector<std::uint32_t>& keys,
                                    std::size_t sizeN,std::size_t stretchN){
    for(const auto id:keys){auto it=delimiters.find(id);if(it==delimiters.end())continue;
        if(std::holds_alternative<DynamicFileRef>(it->second))continue;
        auto* record=std::get_if<DelimiterDataRef>(&it->second);
        if(!record||!*record)typeError(u"Cannot use 'in' operator to search for 'dir' in null");
        auto* data=record->get();
        if(data->variants){for(auto& n:*data->variants)n=adjustArrayIndex(n,static_cast<double>(sizeN));}
        if(data->stretchv){for(auto& n:*data->stretchv)n=adjustArrayIndex(n,static_cast<double>(stretchN));}
    }
}

void FontDataClass::addExtension(const FontExtensionData& data,std::string prefix){
    auto dynamic=std::make_shared<DynamicFont>();dynamic->name=data.name;
    std::string jax=JAX;std::transform(jax.begin(),jax.end(),jax.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    dynamic->prefix=prefix.empty()?"["+data.name+"-extension]/"+jax+"/dynamic":prefix;
    dynamic->files=defineDynamicFiles(data.ranges.value_or(std::vector<DynamicFileDef>{}),data.name);
    dynamic->sizeN=sourceArrayLength(sourceProperties,sourceIdentity,u"defaultSizeVariants",defaultSizeVariants);
    dynamic->stretchN=sourceArrayLength(sourceProperties,sourceIdentity,u"defaultStretchVariants",defaultStretchVariants);
    (void)addFontExtensionToMap(*this,dynamic);
    const auto receiver=requireSourceReceiver(sourceProperties,sourceIdentity,u"FontDataClass");
    (void)mergeSourceOption(*sourceProperties,receiver,u"OPTIONS",optionSource(*sourceProperties,data.options));
    (void)mergeSourceOption(*sourceProperties,receiver,u"defaultVariants",patchSource(data.variants));
    if(data.variantSmp)for(const auto&[key,value]:*data.variantSmp)VariantSmp.insert_or_assign(key,value);
    if(data.cssFonts)for(const auto&[key,value]:*data.cssFonts)defaultCssFonts.insert_or_assign(key,value);
    if(data.accentMap)for(const auto&[key,value]:*data.accentMap)defaultAccentMap.insert_or_assign(key,value);
    if(data.moMap)for(const auto&[key,value]:*data.moMap)defaultMoMap.insert_or_assign(key,value);
    if(data.mnMap)for(const auto&[key,value]:*data.mnMap)defaultMnMap.insert_or_assign(key,value);
    if(data.parameters)mergeParameters(defaultParams,*data.parameters);
    if(data.chars)for(const auto&[variant,map]:*data.chars){auto& target=defaultChars[variant];for(const auto&[cp,ch]:map)target.insert_or_assign(cp,ch);}
    (void)mergeSourceOption(*sourceProperties,receiver,u"defaultSizeVariants",patchSource(data.sizeVariants));
    (void)mergeSourceOption(*sourceProperties,receiver,u"defaultStretchVariants",patchSource(data.stretchVariants));
    if(data.delimiters){std::vector<std::uint32_t> ids;for(const auto&[cp,delimiter]:*data.delimiters){defaultDelimiters.insert_or_assign(cp,delimiter);ids.push_back(cp);}adjustDelimiters(defaultDelimiters,ids,dynamic->sizeN,dynamic->stretchN);}
}

void FontDataClass::bindSourceIdentity(ContextPropertyBindings& bindings,options::ObjectRef receiver,
                                       options::MapContextBindings& maps){
    (void)bindings.context().kind(receiver);
    if(&bindings.context()!=&maps.context())
        throw options::Exception("TypeError",u"FontData Map bindings must share its Context");
    sourceProperties=&bindings;
    sourceMaps=&maps;
    sourceIdentity=receiver;
}

FontData::FontData(FontDataClass& fontClass,FontLoadHost& host,options::Context& context,options::Value supplied)
    :class_(&fontClass),host_(&host),context_(&context),cssFamilyPrefix(fontClass.defaultCssFamilyPrefix){
    const auto destination=options::Value(context.object());
    const std::array<options::Value,1> defaults{fontClass.OPTIONS};
    const auto defaulted=options::defaultOptions(context,destination,defaults);
    const std::array<options::Value,1> users{supplied};
    options_=options::userOptions(context,defaulted,users);
    params_=fontClass.defaultParams;
    sizeVariants_=requireArrayField(fontClass.defaultSizeVariants,u"defaultSizeVariants").spread();
    stretchVariants_=requireArrayField(fontClass.defaultStretchVariants,u"defaultStretchVariants").spread();
    defineCssFonts(fontClass.defaultCssFonts);
    createVariants(requireArrayField(fontClass.defaultVariants,u"defaultVariants"));
    defineDelimiters(fontClass.defaultDelimiters);
    for(const auto&[name,chars]:fontClass.defaultChars)defineChars(name,chars);
    defineRemap("accent",&fontClass.defaultAccentMap);defineRemap("mo",&fontClass.defaultMoMap);defineRemap("mn",&fontClass.defaultMnMap);
    defineDynamicCharacters(fontClass.dynamicFiles);
    forEachDynamicExtension(fontClass,[&](options::Value extension){
        for(const auto& file:extensionFiles(*context_,extension)){
            DynamicFileList one;one.insert_or_assign(sourceString(class_->sourceProperties,
                dynamicFileFieldValue(class_->sourceProperties,file,u"file")),file);defineDynamicCharacters(one);
        }
    });
}

void FontData::setOptions(options::Value definitions){const std::array<options::Value,1> items{std::move(definitions)};options::defaultOptions(*context_,options_,items);}

std::vector<std::string> FontData::addExtension(const FontExtensionData& data,std::string prefix){
    const auto jax=class_->JAX;
    auto dynamic=std::make_shared<DynamicFont>();dynamic->name=data.name;
    std::string lower=jax;std::transform(lower.begin(),lower.end(),lower.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    dynamic->prefix=prefix.empty()?"["+data.name+"-extension]/"+lower+"/dynamic":prefix;
    // Source FontData.addExtension passes the caller's prefix to
    // CLASS.defineDynamicFiles (the class-level method uses data.name).
    dynamic->files=FontDataClass::defineDynamicFiles(data.ranges.value_or(std::vector<DynamicFileDef>{}),prefix);
    dynamic->sizeN=sourceArrayLength(sourceProperties_,sourceIdentity_,u"sizeVariants",sizeVariants_);
    dynamic->stretchN=sourceArrayLength(sourceProperties_,sourceIdentity_,u"stretchVariants",stretchVariants_);
    (void)addFontExtensionToMap(*class_,dynamic);
    const auto receiver=requireSourceReceiver(sourceProperties_,sourceIdentity_,u"FontData");
    Property optionDefinitions=optionSource(*sourceProperties_,data.options);
    if(!property_host::truthy(optionDefinitions))optionDefinitions=Property(PropertyList{});
    (void)mergeSourceOption(*sourceProperties_,receiver,u"options",optionDefinitions);
    if(data.parameters)mergeParameters(params_,*data.parameters);
    (void)mergeSourceOption(*sourceProperties_,receiver,u"sizeVariants",patchSource(data.sizeVariants));
    (void)mergeSourceOption(*sourceProperties_,receiver,u"stretchVariants",patchSource(data.stretchVariants));
    if(data.variantSmp)for(const auto&[key,value]:*data.variantSmp)class_->VariantSmp.insert_or_assign(key,value);
    if(data.cssFonts)defineCssFonts(*data.cssFonts);
    if(data.variants){
        auto& context=sourceProperties_->context();
        const auto temporary=context.object();
        context.set(options::Value(temporary),options::Key(u"variants"),
            sourceProperties_->to(Property(PropertyArray{})));
        const auto definitions=mergeSourceOption(*sourceProperties_,temporary,u"variants",
            listPatchSource(*data.variants));
        const auto array=PropertyArray::array(definitions);
        if(!array)typeError(u"FontData extension variants are not an array");
        createVariants(*array);
    }
    if(data.delimiters){std::vector<std::uint32_t> ids;for(const auto&[cp,d]:*data.delimiters){delimiters_.insert_or_assign(cp,d);ids.push_back(cp);}FontDataClass::adjustDelimiters(delimiters_,ids,dynamic->sizeN,dynamic->stretchN);}
    if(data.chars)for(const auto&[name,map]:*data.chars)defineChars(name,map);
    if(data.accentMap)defineRemap("accent",&*data.accentMap);
    if(data.moMap)defineRemap("mo",&*data.moMap);
    if(data.mnMap)defineRemap("mn",&*data.mnMap);
    if(data.ranges)defineDynamicCharacters(dynamic->files);
    return {};
}

void FontData::bindSourceIdentity(ContextPropertyBindings& bindings,options::ObjectRef receiver){
    (void)bindings.context().kind(receiver);
    sourceProperties_=&bindings;
    sourceIdentity_=receiver;
}

void FontData::createVariant(std::string name,std::optional<std::string> inherit,std::optional<std::string> link){
    auto data=FontVariantData{};
    if(inherit){auto it=variant_.find(*inherit);if(it==variant_.end())typeError(u"Cannot read properties of undefined (reading 'chars')");data.chars=std::make_shared<CharacterMap>();data.chars->prototype=it->second.chars;}
    else data.chars=std::make_shared<CharacterMap>();
    if(link){auto found=variant_.find(*link);if(found!=variant_.end()){
        auto copy=std::make_shared<CharacterMap>();
        copy->prototype=data.chars;
        for(const auto&[cp,value]:found->second.chars->own)copy->own.insert_or_assign(cp,value);
        found->second.linked.emplace_back(copy);
        auto top=std::make_shared<CharacterMap>();top->prototype=copy;data.chars=std::move(top);
    }}
    remapSmpChars(data.chars,name);
    variant_.insert_or_assign(std::move(name),std::move(data));
}
void FontData::createVariants(const PropertyArray& definitions){
    // Source uses for-of, so read the live length/slot at each iteration and
    // let tuple field access preserve the original array value.
    for(std::size_t i=0;i<definitions.size();++i){
        const auto value=definitions.get(i);
        if(value.missing())typeError(u"Cannot read properties of undefined (reading '0')");
        if(std::holds_alternative<PropertyNull>(value.value))typeError(u"Cannot read properties of null (reading '0')");
        createVariant(requiredVariantName(value,0),optionalVariantName(value,1),optionalVariantName(value,2));
    }
}
PropertyArray FontData::smpChar(std::uint32_t n)const{
    PropertyArray glyph;glyph.resize(4);
    glyph.set(3,Property(PropertyList{{"smp",Property(static_cast<double>(n))}}));
    return glyph;
}
void FontData::remapSmpChars(const std::shared_ptr<CharacterMap>& chars,std::string_view name){
    auto source=class_->VariantSmp.find(name);if(source==class_->VariantSmp.end())return;
    PropertyArray remap;
    if(auto* array=std::get_if<PropertyArray>(&source->second))remap=*array;
    else {const auto& alias=std::get<std::string>(source->second);auto found=class_->VariantSmp.find(alias);if(found==class_->VariantSmp.end())return;
        auto* aliasedArray=std::get_if<PropertyArray>(&found->second);if(!aliasedArray)return;remap=*aliasedArray;}
    for(const auto& range:class_->SmpRanges){const int index=range[0];if(index<0)continue;
        const auto baseValue=remap.get(static_cast<std::size_t>(index));if(!property_host::truthy(baseValue))continue;
        const auto base=property_host::number(baseValue);
        for(std::uint32_t n=static_cast<std::uint32_t>(range[1]);n<=static_cast<std::uint32_t>(range[2]);++n){
            if(n==0x3a2)continue;const auto smp=static_cast<std::uint32_t>(base+static_cast<double>(n)-static_cast<double>(range[1]));
            auto mapped=class_->SmpRemapMap.find(smp);
            chars->define(n,Property(smpChar(mapped==class_->SmpRemapMap.end()?smp:mapped->second)));
        }
        if(index==2||index==3){const auto& greek=index==2?class_->SmpRemapGreekU:class_->SmpRemapGreekL;
            for(const auto&[n,offset]:greek)chars->define(n,Property(smpChar(static_cast<std::uint32_t>(base+offset))));}
    }
    const auto extra=remap.get(5);if(!property_host::truthy(extra))return;
    for(const auto& key:property_host::ownKeys(extra)){
        std::uint32_t cp{};const auto [end,error]=std::from_chars(key.data(),key.data()+key.size(),cp);
        if(error!=std::errc{}||end!=key.data()+key.size())continue;
        const auto smp=property_host::number(property_host::get(extra,key));
        chars->define(cp,Property(smpChar(static_cast<std::uint32_t>(smp))));
    }
}
void FontData::defineChars(std::string_view name,const CharMap& chars){
    auto found=variant_.find(name);if(found==variant_.end())typeError(u"Cannot read properties of undefined (reading 'chars')");
    for(const auto&[cp,value]:chars){found->second.chars->define(cp,value);for(auto it=found->second.linked.begin();it!=found->second.linked.end();){if(auto map=it->lock()){map->define(cp,value);++it;}else it=found->second.linked.erase(it);}}
}
void FontData::defineCssFonts(const CssFontMap& fonts){
    const auto unknown=optionString(*context_,options_,u"unknownFamily").value_or("serif");
    for(const auto&[key,value]:fonts){cssFontMap_.insert_or_assign(key,value);auto mutableValue=value;const auto family=mutableValue.get(0);if(family==Property("unknown"))mutableValue.set(0,Property(unknown));}
}
void FontData::defineDelimiters(const DelimiterMap& delims){for(const auto&[cp,value]:delims)delimiters_.insert_or_assign(cp,value);}
void FontData::defineRemap(std::string_view name,const RemapMap* remap){if(!remap)return;auto& target=remapChars_[std::string(name)];for(const auto&[key,value]:*remap)target.insert_or_assign(key,value);}
void FontData::defineDynamicCharacters(const DynamicFileList& files){
    for(const auto&[_,file]:files){if(!file)continue;
        for(const auto&[variant,ranges]:file->variants){auto found=this->variant_.find(variant);if(found==this->variant_.end())typeError(u"Cannot read properties of undefined (reading 'chars')");auto& target=found->second.chars;for(const auto& range:ranges){const auto last=range.isRange?range.last:range.first;for(std::uint32_t cp=range.first;;++cp){target->define(cp,file);if(cp==last)break;}}}
        for(const auto& range:file->delimiters){const auto last=range.isRange?range.last:range.first;for(std::uint32_t cp=range.first;;++cp){delimiters_.insert_or_assign(cp,file);if(cp==last)break;}}
    }
}
std::map<std::uint32_t,DynamicFileRef> FontData::flattenRanges(const DynamicRanges& ranges,const DynamicFileRef& dynamic)const{
    std::map<std::uint32_t,DynamicFileRef> result;for(const auto& range:ranges){const auto last=range.isRange?range.last:range.first;for(std::uint32_t cp=range.first;;++cp){result.emplace(cp,dynamic);if(cp==last)break;}}return result;
}
options::Value FontData::dynamicFileName(options::Value dynamic)const{
    if(!class_->sourceProperties||!sourceIdentity_)
        throw options::Exception("Unimplemented",u"dynamicFileName requires its Source Context");
    auto& context=class_->sourceProperties->context();
    const options::Value receiver(*sourceIdentity_);
    if(!host_->regexp)
        throw options::Exception("Unimplemented",u"dynamicFileName requires the session RegExp owner");
    // Preserve the pinned emitted CJS read order. In particular, file is read
    // again for the successful absolute-path return and again before replace.
    const auto extension=context.get(dynamic,options::Key(u"extension"));
    options::Value prefix;
    if(!options::truthy(extension)){
        const auto ownOptions=context.get(receiver,options::Key(u"options"));
        prefix=context.get(ownOptions,options::Key(u"dynamicPrefix"));
    }else{
        const auto sourceClass=context.get(receiver,options::Key(u"CLASS"));
        const auto map=context.get(sourceClass,options::Key(u"dynamicExtensions"));
        // The emitted Source expression reads dynamic.extension once for the
        // condition, then evaluates dynamicExtensions.get before evaluating
        // dynamic.extension again as the call argument.
        const auto getValue=context.get(map,options::Key(u"get"));
        const auto lookupKey=context.get(dynamic,options::Key(u"extension"));
        const auto* getFunction=std::get_if<options::ObjectRef>(&getValue.data);
        if(!getFunction||context.kind(*getFunction)!=options::ObjectKind::function)
            typeError(u"this.CLASS.dynamicExtensions.get is not a function");
        const std::array<options::Value,1> args{lookupKey};
        const auto dynamicExtension=context.call(*getFunction,map,args);
        prefix=context.get(dynamicExtension,options::Key(u"prefix"));
    }

    const auto absolutePattern=host_->regexp->bind(std::make_shared<regexp_host::RegExp>(
        u"^(?:[/[]|[a-z]+://|[a-z]:)",u"i"));
    const auto absoluteRegex=options::Value(absolutePattern);
    const auto fileForMatch=context.get(dynamic,options::Key(u"file"));
    bool absolute=false;
    if(const auto* text=std::get_if<std::u16string>(&fileForMatch.data)){
        absolute=host_->regexp->exec(absoluteRegex,*text).has_value();
    }else{
        const auto match=context.get(fileForMatch,options::Key(u"match"));
        const auto* function=std::get_if<options::ObjectRef>(&match.data);
        if(!function||context.kind(*function)!=options::ObjectKind::function)
            throw options::Exception("TypeError",u"dynamic.file.match is not a function");
        const std::array<options::Value,1> args{absoluteRegex};
        absolute=options::truthy(context.call(*function,fileForMatch,args));
    }
    if(absolute){
        const auto returned=context.get(dynamic,options::Key(u"file"));
        return returned;
    }

    const auto fileForReplace=context.get(dynamic,options::Key(u"file"));
    options::Value replaced;
    if(const auto* text=std::get_if<std::u16string>(&fileForReplace.data)){
        const auto suffixPattern=host_->regexp->bind(std::make_shared<regexp_host::RegExp>(u"(\\.js)?$"));
        const auto match=host_->regexp->exec(options::Value(suffixPattern),*text);
        if(!match)replaced=fileForReplace;
        else{
            const auto full=match->at(0).value_or(u"");
            const auto index=std::min(match->index,text->size());
            std::u16string result=text->substr(0,index);
            result+=u".js";
            const auto end=std::min(text->size(),index+full.size());
            result+=text->substr(end);
            replaced=std::move(result);
        }
    }else{
        const auto replace=context.get(fileForReplace,options::Key(u"replace"));
        const auto* function=std::get_if<options::ObjectRef>(&replace.data);
        if(!function||context.kind(*function)!=options::ObjectKind::function)
            throw options::Exception("TypeError",u"dynamic.file.replace is not a function");
        const auto suffixPattern=host_->regexp->bind(std::make_shared<regexp_host::RegExp>(u"(\\.js)?$"));
        const std::array<options::Value,2> args{options::Value(suffixPattern),options::Value(u".js")};
        replaced=context.call(*function,fileForReplace,args);
    }
    auto path=property_host::add(context,prefix,options::Value(u"/"));
    const auto request=property_host::add(context,path,replaced);
    return request;
}
void FontData::addDynamicFontCss(const std::vector<std::string>&,std::optional<std::string>){}

async::PromiseRef FontData::loadDynamicFile(const DynamicFileRef& dynamic){
    if(!dynamic)return rejected(host_->queue,"TypeError",u"Dynamic font file is null");
    if(dynamic->failed)return rejected(host_->queue,"Error",u"dynamic file '"+
        property_host::text(sourceString(class_->sourceProperties,
            dynamicFileFieldValue(class_->sourceProperties,dynamic,u"file")))+u"' failed to load");
    if(!dynamic->promise){
        std::string name;
        try{
            const auto sourceObject=dynamic->sourceIdentity
                ?options::Value(*dynamic->sourceIdentity)
                :options::Value(bindDynamicFileSourceObject(*class_,dynamic));
            name=sourceString(class_->sourceProperties,dynamicFileName(sourceObject));
        }
        catch(...){auto failure=async::Promise::create(host_->queue);failure->reject(async::currentReason());return failure;}
        if(host_->requestObserver){try{host_->requestObserver(name);}catch(...){}}
        async::PromiseRef loaded;
        if(!host_->asyncLoad){
            loaded=async::Promise::create(host_->queue);
            loaded->reject(std::string("Can't load '"+name+"': No mathjax.asyncLoad method specified"));
        }else{
            try{
                loaded=host_->asyncLoad(shared_from_this(),dynamic,name);
            }
            catch(...){loaded=async::Promise::create(host_->queue);loaded->reject(async::currentReason());}
        }
        if(!loaded){loaded=async::Promise::create(host_->queue);loaded->resolve();}
        auto self=shared_from_this();
        dynamic->promise=loaded->catchError([self,dynamic](const std::any& error)->std::any{
            dynamic->failed=true;if(self->host_->warn)self->host_->warn(error);return {};
        });
    }
    auto self=shared_from_this();
    return dynamic->promise->then([self,dynamic](const std::any&)->std::any{
        if(!dynamic->setup)typeError(u"dynamic.setup is not a function");
        dynamic->setup(*self);return {};
    });
}
async::PromiseRef FontData::loadDynamicFiles(){
    std::vector<async::PromiseRef> all;for(const auto&[_,file]:class_->dynamicFiles)all.push_back(loadDynamicFile(file));
    forEachDynamicExtensionValue(*class_,[&](options::Value extension){
        for(const auto& file:extensionFiles(*context_,extension))all.push_back(loadDynamicFile(file));
    });
    auto result=async::Promise::create(host_->queue);if(all.empty()){result->resolve(std::vector<std::any>{});return result;}
    struct WaitState{std::mutex mutex;std::size_t left=0;bool rejected=false;std::vector<std::any> values;};
    auto state=std::make_shared<WaitState>();state->left=all.size();state->values.resize(all.size());
    for(std::size_t i=0;i<all.size();++i)all[i]->then([state,result,i](const std::any& value)->std::any{
        std::lock_guard lock(state->mutex);state->values[i]=value;if(--state->left==0&&!state->rejected)result->resolve(state->values);return {};
    },[state,result](const std::any& error)->std::any{
        std::lock_guard lock(state->mutex);if(!state->rejected){state->rejected=true;result->reject(error);}return {};
    });
    return result;
}
void FontData::loadDynamicFilesSync(){
    if(!host_->asyncIsSynchronous||!host_->asyncIsSynchronous())throw tree::FactoryError("Error",u"MathJax(loadDynamicFilesSync): mathjax.asyncLoad must be specified and synchronous\n    Try importing #js/../components/require.mjs and #js/util/asyncLoad/node.js");
    for(const auto&[_,file]:class_->dynamicFiles)loadDynamicFileSync(file);
    forEachDynamicExtensionValue(*class_,[&](options::Value extension){
        for(const auto& file:extensionFiles(*context_,extension))loadDynamicFileSync(file);
    });
}
void FontData::loadDynamicFileSync(const DynamicFileRef& dynamic){
    if(!dynamic)return;
    if(!dynamic->promise){
        dynamic->promise=async::Promise::create(host_->queue);dynamic->promise->resolve();
        try{
            const auto sourceObject=dynamic->sourceIdentity
                ?options::Value(*dynamic->sourceIdentity)
                :options::Value(bindDynamicFileSourceObject(*class_,dynamic));
            const auto name=sourceString(class_->sourceProperties,dynamicFileName(sourceObject));
            if(host_->requestObserver){try{host_->requestObserver(name);}catch(...){}}
            if(!host_->syncLoad)typeError(u"mathjax_js_1.mathjax.asyncLoad is not a function");
            host_->syncLoad(*this,dynamic,name);
        }catch(...){dynamic->failed=true;if(host_->warn)host_->warn(async::currentReason());}
        if(!dynamic->setup)typeError(u"dynamic.setup is not a function");
        dynamic->setup(*this);
    }
}

std::optional<DelimiterValue> FontData::getDelimiter(double n){
    if(!std::isfinite(n)||n<0||std::trunc(n)!=n||n>std::numeric_limits<std::uint32_t>::max())return {};
    auto key=static_cast<std::uint32_t>(n);auto it=delimiters_.find(key);if(it==delimiters_.end())return {};
    if(auto* file=std::get_if<DynamicFileRef>(&it->second);file&&*file){
        delimiters_[key]=std::monostate{};
        if(host_->asyncIsSynchronous&&host_->asyncIsSynchronous()){loadDynamicFileSync(*file);return getDelimiter(n);}
        retries::retryAfter(loadDynamicFile(*file));
        return DelimiterValue{std::monostate{}};
    }
    return it->second;
}
std::optional<std::string> FontData::getSizeVariant(double n,double index){
    auto delim=getDelimiter(n);if(delim){if(auto* ref=std::get_if<DelimiterDataRef>(&*delim);ref&&*ref&&(*ref)->variants){auto& variants=*(*ref)->variants;auto slot=arrayIndex(index,variants.size());index=slot?variants[*slot]:std::numeric_limits<double>::quiet_NaN();}}
    const auto variants=requireArrayField(sizeVariants_,u"sizeVariants");
    auto slot=arrayIndex(index,variants.size());if(!slot)return std::nullopt;
    const auto value=variants.get(*slot);if(value.missing())return std::nullopt;
    return utf8(property_host::string(value));
}
std::optional<std::string> FontData::getStretchVariant(double n,double index){
    auto delim=getDelimiter(n);if(!delim)typeError(u"Cannot read properties of undefined (reading 'stretchv')");
    auto* ref=std::get_if<DelimiterDataRef>(&*delim);if(!ref||!*ref)typeError(u"Cannot read properties of null (reading 'stretchv')");
    auto* data=ref->get();
    if(data->stretchv){auto slot=arrayIndex(index,data->stretchv->size());index=slot?(*data->stretchv)[*slot]:std::numeric_limits<double>::quiet_NaN();}else index=0;
    const auto variants=requireArrayField(stretchVariants_,u"stretchVariants");
    auto slot=arrayIndex(index,variants.size());if(!slot)return std::nullopt;
    const auto value=variants.get(*slot);if(value.missing())return std::nullopt;
    return utf8(property_host::string(value));
}
std::array<std::optional<std::string>,4> FontData::getStretchVariants(double n){return {getStretchVariant(n,0),getStretchVariant(n,1),getStretchVariant(n,2),getStretchVariant(n,3)};}
std::optional<Property> FontData::getChar(std::string_view name,double n){
    auto variant=variant_.find(name);if(variant==variant_.end())typeError(u"Cannot read properties of undefined (reading 'chars')");
    if(!std::isfinite(n)||n<0||std::trunc(n)!=n||n>std::numeric_limits<std::uint32_t>::max())return {};
    const auto key=static_cast<std::uint32_t>(n);auto value=variant->second.chars->get(key);
    if(!value)return {};
    if(auto* file=std::get_if<DynamicFileRef>(&*value);file&&*file){
        variant->second.chars->erase(key);for(auto it=variant->second.linked.begin();it!=variant->second.linked.end();){if(auto link=it->lock()){link->erase(key);++it;}else it=variant->second.linked.erase(it);}
        if(host_->asyncIsSynchronous&&host_->asyncIsSynchronous()){loadDynamicFileSync(*file);return getChar(name,n);}
        retries::retryAfter(loadDynamicFile(*file));return {};
    }
    if(auto* data=std::get_if<Property>(&*value))return *data;
    return {};
}
std::optional<std::reference_wrapper<FontVariantData>> FontData::getVariant(std::string_view name){auto it=variant_.find(name);if(it==variant_.end())return {};return it->second;}
CssFontData FontData::getCssFont(std::string_view variant)const{if(auto it=cssFontMap_.find(variant);it!=cssFontMap_.end())return it->second;return CssFontData{Property("serif"),Property(false),Property(false)};}
std::string FontData::getFamily(std::string_view family)const{return cssFamilyPrefix.empty()?std::string(family):cssFamilyPrefix+", "+std::string(family);}
std::optional<std::u16string> FontData::getRemappedChar(std::string_view name,double cp)const{auto map=remapChars_.find(name);if(map==remapChars_.end()||!std::isfinite(cp)||cp<0||std::trunc(cp)!=cp||cp>std::numeric_limits<std::uint32_t>::max())return {};auto it=map->second.find(static_cast<std::uint32_t>(cp));return it==map->second.end()?std::optional<std::u16string>{}:it->second;}
}
