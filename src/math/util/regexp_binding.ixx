// Native reference conversion for the same Context graph; no TeX-specific
// matching behavior belongs in this owner.
export module folia.math.util.regexp_binding;
export import folia.math.util.regexp_host;
export import folia.math.util.native_identities;
import folia.math.util.js_number;
import std;
export namespace folia::math::regexp_host {
class RegExpBindings {
    options::Context& context_;
    std::shared_ptr<std::atomic_bool> active_=std::make_shared<std::atomic_bool>(true);
    options::ObjectRef constructor_;
    NativeIdentityBindings<RegExpRef> identities_;
    std::vector<std::weak_ptr<options::NativeDataField>> fields_;

    static bool nullish(const options::Value& value){
        return std::holds_alternative<options::Undefined>(value.data)||
               std::holds_alternative<options::Null>(value.data);
    }

    static std::u16string toString(options::Context& context,options::Value value){
        using namespace options;
        if(std::holds_alternative<Undefined>(value.data))return u"undefined";
        if(std::holds_alternative<Null>(value.data))return u"null";
        value=context.toPrimitive(std::move(value),PrimitiveHint::stringHint);
        if(const auto* text=std::get_if<std::u16string>(&value.data))return *text;
        if(const auto* boolean=std::get_if<bool>(&value.data))return *boolean?u"true":u"false";
        if(const auto* number=std::get_if<double>(&value.data)){
            const auto ascii=js_number::toString(*number);
            return std::u16string(ascii.begin(),ascii.end());
        }
        if(const auto* symbol=std::get_if<SymbolRef>(&value.data)){
            (void)context.symbolString(*symbol);
            throw Exception("TypeError",u"Cannot convert a Symbol value to a string");
        }
        throw Exception("TypeError",u"Cannot convert object to primitive value");
    }
    static std::uint32_t toUint32(options::Context& context,const options::Value& value){
        using namespace options;
        if(std::holds_alternative<Undefined>(value.data))return std::numeric_limits<std::uint32_t>::max();
        const double number=context.toNumber(value);
        if(!std::isfinite(number)||number==0)return 0;
        constexpr double modulus=4294967296.0;
        double integer=std::fmod(std::trunc(number),modulus);
        if(integer<0)integer+=modulus;
        return static_cast<std::uint32_t>(integer);
    }
    static std::uint64_t toLength(options::Context& context,const options::Value& value){
        const double number=context.toNumber(value);
        if(std::isnan(number)||number<=0)return 0;
        constexpr double maximum=9007199254740991.0;
        return static_cast<std::uint64_t>(std::min(std::floor(number),maximum));
    }
    static std::u16string indexKey(std::uint64_t index){
        const auto ascii=std::to_string(index);
        return std::u16string(ascii.begin(),ascii.end());
    }
    static std::u16string substring(std::u16string_view source,std::uint64_t start,std::uint64_t end){
        const auto length=static_cast<std::uint64_t>(source.size());
        start=std::min(start,length);end=std::min(end,length);
        if(end<=start)return {};
        return std::u16string(source.substr(static_cast<std::size_t>(start),
            static_cast<std::size_t>(end-start)));
    }
    static std::uint64_t advanceStringIndex(std::u16string_view input,std::uint64_t index,bool unicode){
        if(!unicode||index+1>=input.size())return index+1;
        const auto first=static_cast<std::uint32_t>(input[static_cast<std::size_t>(index)]);
        const auto second=static_cast<std::uint32_t>(input[static_cast<std::size_t>(index+1)]);
        return first>=0xD800&&first<=0xDBFF&&second>=0xDC00&&second<=0xDFFF?index+2:index+1;
    }
    static void requireActive(const std::weak_ptr<std::atomic_bool>& weak){
        const auto lease=weak.lock();
        if(!lease||!lease->load(std::memory_order_acquire))
            throw options::Exception("TypeError",u"RegExp Context binding has expired");
    }
    std::optional<RegExpRef> tryResolve(options::Value value)const{
        const auto* object=std::get_if<options::ObjectRef>(&value.data);
        if(!object)return std::nullopt;
        try{return identities_.resolve(*object);}
        catch(const NativeIdentityResolutionFailure&){return std::nullopt;}
    }
    bool isRegExp(options::Value value,const std::optional<RegExpRef>& native) {
        if(!std::holds_alternative<options::ObjectRef>(value.data))return false;
        const auto matcher=context_.get(value,options::Key(context_.intrinsics().symbolMatch));
        return std::holds_alternative<options::Undefined>(matcher.data)
            ?native.has_value():options::truthy(matcher);
    }
    options::ObjectRef constructRegExp(std::span<const options::Value> arguments,
                                       std::optional<bool> knownIsRegExp=std::nullopt){
        std::u16string pattern,flags;
        const options::Value patternValue=arguments.empty()
            ?options::Value(options::Undefined{}):arguments.front();
        const auto original=tryResolve(patternValue);
        const bool regexpPattern=knownIsRegExp.has_value()
            ?*knownIsRegExp:isRegExp(patternValue,original);
        if(original){
            // A branded native RegExp keeps its internal slots even when an
            // own Symbol.match value makes the abstract IsRegExp operation false.
            pattern=(*original)->pattern();
            if(arguments.size()<2||std::holds_alternative<options::Undefined>(arguments[1].data))
                flags=(*original)->flags();
        }else if(regexpPattern){
            const auto source=context_.get(patternValue,options::Key(u"source"));
            const bool hasFlags=arguments.size()>=2&&
                !std::holds_alternative<options::Undefined>(arguments[1].data);
            const auto sourceFlags=hasFlags?options::Value(options::Undefined{}):
                context_.get(patternValue,options::Key(u"flags"));
            if(!std::holds_alternative<options::Undefined>(source.data))
                pattern=toString(context_,source);
            if(arguments.size()<2||std::holds_alternative<options::Undefined>(arguments[1].data)){
                if(!std::holds_alternative<options::Undefined>(sourceFlags.data))
                    flags=toString(context_,sourceFlags);
            }
        }else if(!std::holds_alternative<options::Undefined>(patternValue.data)){
            pattern=toString(context_,patternValue);
        }
        if(arguments.size()>1&&!std::holds_alternative<options::Undefined>(arguments[1].data))
            flags=toString(context_,arguments[1]);
        return create(std::move(pattern),std::move(flags));
    }
    options::Value callRegExp(std::span<const options::Value> arguments){
        const options::Value pattern=arguments.empty()
            ?options::Value(options::Undefined{}):arguments.front();
        const bool noFlags=arguments.size()<2||std::holds_alternative<options::Undefined>(arguments[1].data);
        if(noFlags){
            const auto original=tryResolve(pattern);
            const bool regexpPattern=isRegExp(pattern,original);
            if(regexpPattern){
                const auto constructor=context_.get(pattern,options::Key(u"constructor"));
                if(options::sameValue(constructor,options::Value(constructor_)))return pattern;
            }
            return options::Value(constructRegExp(arguments,regexpPattern));
        }
        return options::Value(constructRegExp(arguments));
    }
    options::Value matchValue(options::Context& context,const Match& match){
        const auto result=context.array(static_cast<std::uint32_t>(match.captures.size()));
        for(std::size_t i=0;i<match.captures.size();++i){
            const auto value=match.captures[i]?options::Value(*match.captures[i]):options::Value(options::Undefined{});
            context.define(result,options::Key(indexKey(i)),options::Descriptor::data(value));
        }
        context.define(result,options::Key(u"index"),options::Descriptor::data(options::Value(static_cast<double>(match.index))));
        context.define(result,options::Key(u"input"),options::Descriptor::data(options::Value(match.input)));
        if(match.groups){
            const auto groups=context.object();
            for(const auto& [name,value]:*match.groups)
                context.define(groups,options::Key(name),options::Descriptor::data(
                    value?options::Value(*value):options::Value(options::Undefined{})));
            context.define(result,options::Key(u"groups"),options::Descriptor::data(options::Value(groups)));
        }
        return options::Value(result);
    }
    std::optional<Match> execResolved(options::Value value,RegExpRef regexp,const std::u16string& input){
        return regexp->exec(input,
            [this](const options::Value& raw){return context_.toNumber(raw);},
            [this,value]{return context_.get(value,options::Key(u"lastIndex"));},
            [this,value](const options::Value& next){context_.set(value,options::Key(u"lastIndex"),next);});
    }
    options::Value builtinExec(options::Value receiver,const std::u16string& input){
        const auto regexp=sourceResolve(receiver);
        const auto match=execResolved(std::move(receiver),regexp,input);
        return match?matchValue(context_,*match):options::Value(options::Null{});
    }
    options::Value regExpExec(options::Value splitter,const std::u16string& input){
        const auto execValue=context_.get(splitter,options::Key(u"exec"));
        if(const auto* callable=std::get_if<options::ObjectRef>(&execValue.data);
           callable&&context_.kind(*callable)==options::ObjectKind::function){
            const options::Value arguments[]{options::Value(input)};
            return context_.call(*callable,std::move(splitter),arguments);
        }
        return builtinExec(std::move(splitter),input);
    }
    options::Value stringMatchAlgorithm(options::Context& context,options::Value receiver,
                                        std::span<const options::Value> arguments){
        if(nullish(receiver))
            throw options::Exception("TypeError",u"String.prototype.match called on null or undefined");
        const options::Value regexp=arguments.empty()
            ?options::Value(options::Undefined{}):arguments.front();
        if(!nullish(regexp)){
            const auto matcher=context.get(regexp,options::Key(context.intrinsics().symbolMatch));
            if(!nullish(matcher)){
                const auto* callable=std::get_if<options::ObjectRef>(&matcher.data);
                if(!callable||context.kind(*callable)!=options::ObjectKind::function)
                    throw options::Exception("TypeError",u"String match Symbol.match is not callable");
                const options::Value callArguments[]{receiver};
                return context.call(*callable,regexp,callArguments);
            }
        }

        const std::u16string input=toString(context,receiver);
        const options::Value constructArguments[]{regexp};
        const auto matcherObject=context.construct(constructor_,constructArguments);
        const auto matcher=context.get(options::Value(matcherObject),
            options::Key(context.intrinsics().symbolMatch));
        const auto* callable=std::get_if<options::ObjectRef>(&matcher.data);
        if(!callable||context.kind(*callable)!=options::ObjectKind::function)
            throw options::Exception("TypeError",u"RegExp Symbol.match is not callable");
        const options::Value callArguments[]{options::Value(input)};
        return context.call(*callable,options::Value(matcherObject),callArguments);
    }
    options::Value regexpMatchAlgorithm(options::Context& context,options::Value receiver,
                                        std::span<const options::Value> arguments){
        if(!std::holds_alternative<options::ObjectRef>(receiver.data))
            throw options::Exception("TypeError",u"RegExp.prototype[@@match] called on a non-object");
        const std::u16string input=toString(context,arguments.empty()
            ?options::Value(options::Undefined{}):arguments.front());
        const std::u16string flags=toString(context,context.get(receiver,options::Key(u"flags")));
        const bool global=flags.find(u'g')!=flags.npos;
        const bool fullUnicode=flags.find(u'u')!=flags.npos||flags.find(u'v')!=flags.npos;
        if(!global){
            auto result=regExpExec(receiver,input);
            if(!std::holds_alternative<options::Null>(result.data)&&
               !std::holds_alternative<options::ObjectRef>(result.data))
                throw options::Exception("TypeError",u"RegExp exec result is not an object");
            return result;
        }

        context.set(receiver,options::Key(u"lastIndex"),options::Value(0));
        const auto matches=context.array(0);
        std::uint64_t count=0;
        for(;;){
            auto result=regExpExec(receiver,input);
            if(std::holds_alternative<options::Null>(result.data))
                return count==0?options::Value(options::Null{}):options::Value(matches);
            if(!std::holds_alternative<options::ObjectRef>(result.data))
                throw options::Exception("TypeError",u"RegExp exec result is not an object");
            const auto match=toString(context,context.get(result,options::Key(u"0")));
            context.define(matches,options::Key(indexKey(count++)),
                options::Descriptor::data(options::Value(match)));
            if(match.empty()){
                const auto index=toLength(context,context.get(receiver,options::Key(u"lastIndex")));
                context.set(receiver,options::Key(u"lastIndex"),
                    options::Value(static_cast<double>(advanceStringIndex(input,index,fullUnicode))));
            }
        }
    }
    options::Value splitAlgorithm(options::Context& context,options::Value receiver,
                                  std::span<const options::Value> arguments){
        if(!std::holds_alternative<options::ObjectRef>(receiver.data))
            throw options::Exception("TypeError",u"RegExp.prototype[@@split] called on a non-object");
        const options::Value inputValue=arguments.empty()
            ?options::Value(options::Undefined{}):arguments.front();
        const std::u16string input=toString(context,std::move(inputValue));

        // SpeciesConstructor's reads precede the observable flags Get.
        const options::Value constructorValue=context.get(receiver,options::Key(u"constructor"));
        options::ObjectRef speciesConstructor=constructor_;
        if(!std::holds_alternative<options::Undefined>(constructorValue.data)){
            const auto* constructorObject=std::get_if<options::ObjectRef>(&constructorValue.data);
            if(!constructorObject)throw options::Exception("TypeError",u"RegExp constructor property is not an object");
            const auto species=context.get(constructorValue,options::Key(context.intrinsics().symbolSpecies));
            if(!nullish(species)){
                const auto* speciesObject=std::get_if<options::ObjectRef>(&species.data);
                if(!speciesObject||!context.isConstructor(*speciesObject))
                    throw options::Exception("TypeError",u"RegExp species is not a constructor");
                speciesConstructor=*speciesObject;
            }
        }
        const std::u16string flags=toString(context,context.get(receiver,options::Key(u"flags")));
        const bool unicode=flags.find(u'u')!=flags.npos||flags.find(u'v')!=flags.npos;
        std::u16string splitterFlags=flags;
        if(splitterFlags.find(u'y')==splitterFlags.npos)splitterFlags+=u'y';
        const options::Value constructorArguments[]{receiver,options::Value(splitterFlags)};
        const auto splitter=context.construct(speciesConstructor,constructorArguments);

        const options::Value limitValue=arguments.size()<2
            ?options::Value(options::Undefined{}):arguments[1];
        const std::uint32_t limit=toUint32(context,limitValue);
        const auto output=context.array(0);
        std::uint32_t outputLength=0;
        const auto append=[&](options::Value value){
            context.define(output,options::Key(indexKey(outputLength)),options::Descriptor::data(std::move(value)));
            ++outputLength;
        };
        if(limit==0)return options::Value(output);

        std::uint64_t p=0,q=0;
        if(input.empty()){
            context.set(options::Value(splitter),options::Key(u"lastIndex"),options::Value(0));
            const auto result=regExpExec(options::Value(splitter),input);
            if(std::holds_alternative<options::Null>(result.data))append(options::Value(input));
            else if(!std::holds_alternative<options::ObjectRef>(result.data))
                throw options::Exception("TypeError",u"RegExp exec result is not an object");
            return options::Value(output);
        }

        while(q<input.size()){
            context.set(options::Value(splitter),options::Key(u"lastIndex"),options::Value(static_cast<double>(q)));
            const auto result=regExpExec(options::Value(splitter),input);
            if(std::holds_alternative<options::Null>(result.data)){
                q=advanceStringIndex(input,q,unicode);
                continue;
            }
            const auto* matchObject=std::get_if<options::ObjectRef>(&result.data);
            if(!matchObject)throw options::Exception("TypeError",u"RegExp exec result is not an object");
            const std::uint64_t e=toLength(context,context.get(options::Value(splitter),options::Key(u"lastIndex")));
            if(e==p){q=advanceStringIndex(input,q,unicode);continue;}

            append(options::Value(substring(input,p,q)));
            if(outputLength>=limit)return options::Value(output);
            const std::uint64_t captures=toLength(context,context.get(result,options::Key(u"length")));
            for(std::uint64_t n=1;n<captures;++n){
                append(context.get(result,options::Key(indexKey(n))));
                if(outputLength>=limit)return options::Value(output);
            }
            p=e;
            q=p;
        }
        append(options::Value(substring(input,p,input.size())));
        return options::Value(output);
    }
public:
    explicit RegExpBindings(options::Context& context)
        :context_(context),constructor_(context.nativeConstructor(u"RegExp",
            [this,active=std::weak_ptr<std::atomic_bool>(active_)](options::Context&,std::span<const options::Value> args){
                requireActive(active);return constructRegExp(args);
            },
            [this,active=std::weak_ptr<std::atomic_bool>(active_)](options::Context&,options::Value,std::span<const options::Value> args){
                requireActive(active);return callRegExp(args);
            })),identities_(context){
        context.define(constructor_,options::Key(u"length"),options::Descriptor::data(2,false,false,true));
        const auto prototypeValue=context.get(options::Value(constructor_),options::Key(u"prototype"));
        const auto* prototype=std::get_if<options::ObjectRef>(&prototypeValue.data);
        if(!prototype)throw options::Exception("TypeError",u"RegExp constructor prototype is invalid");
        const std::weak_ptr<std::atomic_bool> active=active_;
        const auto matchMethod=context.function(u"[Symbol.match]",
            [this,active](options::Context& cx,options::Value receiver,std::span<const options::Value> args){
                requireActive(active);return regexpMatchAlgorithm(cx,std::move(receiver),args);
            });
        context.define(matchMethod,options::Key(u"length"),options::Descriptor::data(options::Value(1),false,false,true));
        context.define(*prototype,options::Key(context.intrinsics().symbolMatch),
            options::Descriptor::data(options::Value(matchMethod),true,false,true));
        const auto stringMatch=context.function(u"match",
            [this,active](options::Context& cx,options::Value receiver,std::span<const options::Value> args){
                requireActive(active);return stringMatchAlgorithm(cx,std::move(receiver),args);
            });
        context.define(stringMatch,options::Key(u"length"),options::Descriptor::data(options::Value(1),false,false,true));
        context.define(context.intrinsics().stringPrototype,options::Key(u"match"),
            options::Descriptor::data(options::Value(stringMatch),true,false,true));
        const auto speciesGetter=context.function(u"get [Symbol.species]",
            [](options::Context&,options::Value receiver,std::span<const options::Value>){return receiver;});
        context.define(speciesGetter,options::Key(u"length"),options::Descriptor::data(0,false,false,true));
        context.define(constructor_,options::Key(context.intrinsics().symbolSpecies),
            options::Descriptor::access(speciesGetter,std::nullopt,false,true));
        const std::array<std::pair<std::u16string_view,char16_t>,8> flagProperties{{
            {u"hasIndices",u'd'},{u"global",u'g'},{u"ignoreCase",u'i'},
            {u"multiline",u'm'},{u"dotAll",u's'},{u"unicode",u'u'},
            {u"sticky",u'y'},{u"unicodeSets",u'v'}}};
        for(const auto [name,flag]:flagProperties){
            const auto getter=context.function(std::u16string(u"get ")+std::u16string(name),
                [this,active,flag](options::Context&,options::Value receiver,std::span<const options::Value>){
                    requireActive(active);const auto value=sourceResolve(receiver);
                    return options::Value(value->flags().find(flag)!=std::u16string::npos);
                });
            context.define(getter,options::Key(u"length"),options::Descriptor::data(0,false,false,true));
            context.define(*prototype,options::Key(std::u16string(name)),
                options::Descriptor::access(getter,std::nullopt,false,true));
        }
        const auto flagsGetter=context.function(u"get flags",
            [active,flagProperties](options::Context& cx,options::Value receiver,std::span<const options::Value>){
                const auto lease=active.lock();
                if(!lease||!lease->load(std::memory_order_acquire))
                    throw options::Exception("TypeError",u"RegExp Context binding has expired");
                std::array<bool,8> enabled{};
                for(std::size_t i=0;i<flagProperties.size();++i)
                    enabled[i]=options::truthy(cx.get(receiver,options::Key(std::u16string(flagProperties[i].first))));
                constexpr std::array<std::size_t,8> outputOrder{0,1,2,3,4,5,7,6};
                std::u16string result;
                for(const auto index:outputOrder)if(enabled[index])result.push_back(flagProperties[index].second);
                return options::Value(std::move(result));
            });
        context.define(flagsGetter,options::Key(u"length"),options::Descriptor::data(0,false,false,true));
        context.define(*prototype,options::Key(u"flags"),
            options::Descriptor::access(flagsGetter,std::nullopt,false,true));
        const auto execMethod=context.function(u"exec",
            [this,active](options::Context&,options::Value receiver,std::span<const options::Value> args){
                requireActive(active);
                const auto input=toString(context_,args.empty()
                    ?options::Value(options::Undefined{}):args.front());
                return builtinExec(std::move(receiver),input);
            });
        context.define(execMethod,options::Key(u"length"),options::Descriptor::data(1,false,false,true));
        context.define(*prototype,options::Key(u"exec"),options::Descriptor::data(options::Value(execMethod),true,false,true));
        const auto splitMethod=context.function(u"[Symbol.split]",
            [this,active](options::Context& cx,options::Value receiver,std::span<const options::Value> args){
                requireActive(active);return splitAlgorithm(cx,std::move(receiver),args);
            });
        context.define(splitMethod,options::Key(u"length"),options::Descriptor::data(2,false,false,true));
        context.define(*prototype,options::Key(context.intrinsics().symbolSplit),
            options::Descriptor::data(options::Value(splitMethod),true,false,true));
    }
    ~RegExpBindings(){
        // Context may be destroying its records while a payload releases its
        // last shared owner of this binding. Invalidate local field handles
        // only; identities_.clear() calls back into Context and is reserved
        // for explicit clear() while Context is known to be alive.
        active_->store(false,std::memory_order_release);
        for(const auto& weak:fields_)if(const auto field=weak.lock())field->releaseOwner();
        fields_.clear();
    }
    RegExpBindings(const RegExpBindings&)=delete;
    RegExpBindings&operator=(const RegExpBindings&)=delete;
    options::Context& context()const{return context_;}
    options::ObjectRef constructorIdentity()const{return constructor_;}
    options::ObjectRef bind(RegExpRef reference){
        if(const auto existing=identities_.find(reference))return *existing;
        auto owner=reference;
        auto object=identities_.instance(std::move(reference),constructor_);
        auto fieldOwner=std::static_pointer_cast<void>(owner);
        const std::weak_ptr<RegExp> weak=owner;
        auto field=std::make_shared<options::NativeDataField>(fieldOwner,[weak]{
            const auto value=weak.lock();
            if(!value)throw options::Exception("Unimplemented",u"Native RegExp owner has been released");
            return value->lastIndex.value;
        },[weak](options::Value next){
            const auto value=weak.lock();
            if(!value)throw options::Exception("Unimplemented",u"Native RegExp owner has been released");
            value->lastIndex=std::move(next);
        });
        context_.bindData(object,options::Key(u"lastIndex"),field,true,false,false);
        std::erase_if(fields_,[](const auto& stale){return stale.expired();});
        fields_.emplace_back(std::move(field));
        return object;
    }
    options::ObjectRef create(std::u16string pattern,std::u16string flags={}){
        return bind(std::make_shared<RegExp>(std::move(pattern),std::move(flags)));
    }
    RegExpRef resolve(options::Value value)const{
        const auto* object=std::get_if<options::ObjectRef>(&value.data);
        if(!object)throw options::Exception("TypeError",u"Method RegExp.prototype.exec called on incompatible receiver");
        return identities_.resolve(*object);
    }
    RegExpRef sourceResolve(options::Value value)const{
        try{return resolve(std::move(value));}
        catch(const NativeIdentityResolutionFailure&){
            throw options::Exception("TypeError",u"Method RegExp.prototype.exec called on incompatible receiver");
        }
    }
    void setLastIndex(options::Value value,options::Value cursor){
        (void)resolve(value);context_.set(value,options::Key(u"lastIndex"),std::move(cursor));
    }
    void setLastIndex(options::Value value,double cursor){setLastIndex(std::move(value),options::Value(cursor));}
    options::Value lastIndexValue(options::Value value)const{
        (void)resolve(value);return context_.get(value,options::Key(u"lastIndex"));
    }
    double lastIndex(options::Value value)const{return context_.toNumber(lastIndexValue(std::move(value)));}
    std::optional<Match> exec(options::Value value,const std::u16string& input){
        auto regexp=resolve(value);
        return execResolved(std::move(value),std::move(regexp),input);
    }
    void clear(){
        active_->store(false,std::memory_order_release);
        for(const auto& weak:fields_)if(const auto field=weak.lock())field->releaseOwner();
        fields_.clear();identities_.clear();
    }
};
}
