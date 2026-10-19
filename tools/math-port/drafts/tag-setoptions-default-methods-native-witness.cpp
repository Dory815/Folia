// Independent native witness paired with
// tag-setoptions-reference/default-methods-source-witness.cjs. Keep this
// separate from the locked 23-case observer until the callback/prototype
// checkpoint is accepted.
#define main tag_setoptions_locked_candidate_main
#include "tag-setoptions-native-candidate.cpp"
#undef main

namespace {
std::string attemptValue(const std::function<opts::Value()>& body,Serializer& serializer){
    try{return "{\"value\":"+serializer.value(body())+"}";}
    catch(const opts::Exception& error){return "{\"error\":{\"name\":"+q(error.name)+",\"message\":"+q(text(error.message))+"}}";}
    catch(const std::exception& error){return "{\"error\":{\"name\":\"Error\",\"message\":"+q(error.what())+"}}";}
}
std::string methodEvent(std::string_view name,std::string_view receiver,std::string_view tail=""){
    return "{\"name\":"+q(name)+",\"receiver\":"+q(receiver)+std::string(tail)+"}";
}
}

int main(){
    try{
        Fixture fixture;
        const auto jax=fixture.jax(baseAmsTag,"tagformat-default-methods");
        const auto shared=jax->parseOptions();
        const auto* parsed=std::get_if<tex::ParseOptionsRef>(&shared);
        if(!parsed||!*parsed)throw std::runtime_error("default-method witness has no ParseOptions");
        Serializer serializer;serializer.context=&fixture.cx;serializer.functions=&fixture.functions;serializer.regexp=&fixture.runtime.regexp();
        const opts::Value format=fixture.cx.get((*parsed)->options,opts::Key(u"tagformat"));
        const auto callback=[&](std::u16string_view name){
            const opts::Value value=fixture.cx.get(format,opts::Key(std::u16string(name)));
            const auto* function=std::get_if<opts::ObjectRef>(&value.data);
            if(!function||fixture.cx.kind(*function)!=opts::ObjectKind::function)throw std::runtime_error("default callback is missing");
            return *function;
        };
        const auto call=[&](std::u16string_view name,std::vector<opts::Value> args){return fixture.cx.call(callback(name),format,args);};
        std::vector<std::string> events;

        const opts::ObjectRef numberObject=fixture.cx.object();
        const opts::ObjectRef numberToString=fixture.cx.function(u"toString",[&](opts::Context&,opts::Value receiver,std::span<const opts::Value> args){
            events.push_back(methodEvent("number.toString",std::get_if<opts::ObjectRef>(&receiver.data)&&*std::get_if<opts::ObjectRef>(&receiver.data)==numberObject?"argument":"other",
                ",\"arity\":"+std::to_string(args.size())));
            return opts::Value(u"custom-number");
        });
        fixture.cx.set(numberObject,opts::Key(u"toString"),opts::Value(numberToString));
        const std::string number="{\"ordinary\":"+attemptValue([&]{return call(u"number",{opts::Value(12.0)});},serializer)+
            ",\"custom\":"+attemptValue([&]{return call(u"number",{opts::Value(numberObject)});},serializer)+
            ",\"null\":"+attemptValue([&]{return call(u"number",{opts::Value(opts::Null{})});},serializer)+
            ",\"undefined\":"+attemptValue([&]{return call(u"number",{opts::Value(opts::Undefined{})});},serializer)+"}";

        const auto& intrinsics=fixture.cx.intrinsics();
        const opts::Value stringPrototype(intrinsics.stringPrototype);
        const opts::Value previousReplace=fixture.cx.get(stringPrototype,opts::Key(u"replace"));
        const opts::ObjectRef replacement=fixture.cx.function(u"replace",[&](opts::Context& context,opts::Value receiver,std::span<const opts::Value> args){
            const auto* string=std::get_if<std::u16string>(&receiver.data);
            const auto* regex=args.empty()?nullptr:std::get_if<opts::ObjectRef>(&args[0].data);
            std::string pattern="null",flags="";
            if(regex){const auto expression=fixture.runtime.regexp().resolve(args[0]);pattern=text(expression->source());flags=text(expression->flags());}
            events.push_back("{\"name\":\"String.prototype.replace\",\"receiver\":"+q(string&&*string==u"a b"?"argument":"other")+
                ",\"args\":[{\"$\":\"regexp\",\"source\":"+q(pattern)+",\"flags\":"+q(flags)+"},"+
                (args.size()>1?serializer.value(args[1]):"{\"$\":\"undefined\"}")+ "]}");
            (void)context;return opts::Value(u"patched-id");
        });
        fixture.cx.set(intrinsics.stringPrototype,opts::Key(u"replace"),opts::Value(replacement));
        const auto overridden=attemptValue([&]{return call(u"id",{opts::Value(u"a b")});},serializer);
        fixture.cx.set(intrinsics.stringPrototype,opts::Key(u"replace"),previousReplace);

        const opts::ObjectRef idObject=fixture.cx.object();
        const opts::ObjectRef primitiveResult=fixture.cx.object();
        const opts::ObjectRef primitive=fixture.cx.function(u"[Symbol.toPrimitive]",[&](opts::Context&,opts::Value,std::span<const opts::Value> args){
            const auto* hint=args.empty()?nullptr:std::get_if<std::u16string>(&args[0].data);
            events.push_back("{\"name\":\"id.result.toPrimitive\",\"hint\":"+q(hint?text(*hint):"undefined")+"}");
            return opts::Value(u"custom-id");
        });
        fixture.cx.set(primitiveResult,opts::Key(fixture.cx.intrinsics().symbolToPrimitive),opts::Value(primitive));
        const opts::ObjectRef replaceMethod=fixture.cx.function(u"replace",[&](opts::Context& context,opts::Value receiver,std::span<const opts::Value> args){
            const auto* target=std::get_if<opts::ObjectRef>(&receiver.data);const auto* regexp=args.empty()?nullptr:std::get_if<opts::ObjectRef>(&args[0].data);
            std::string pattern="null",flags="";
            if(regexp){const auto expression=fixture.runtime.regexp().resolve(args[0]);pattern=text(expression->source());flags=text(expression->flags());}
            events.push_back("{\"name\":\"id.replace\",\"receiver\":"+q(target&&*target==idObject?"argument":"other")+
                ",\"arity\":"+std::to_string(args.size())+",\"pattern\":{\"$\":\"regexp\",\"source\":"+q(pattern)+",\"flags\":"+q(flags)+"},\"replacement\":"+
                (args.size()>1?serializer.value(args[1]):"{\"$\":\"undefined\"}")+"}");
            (void)context;return opts::Value(primitiveResult);
        });
        fixture.cx.set(idObject,opts::Key(u"replace"),opts::Value(replaceMethod));
        const std::string id="{\"ordinary\":"+attemptValue([&]{return call(u"id",{opts::Value(u"alpha beta")});},serializer)+
            ",\"customReplace\":"+attemptValue([&]{return call(u"id",{opts::Value(idObject)});},serializer)+
            ",\"null\":"+attemptValue([&]{return call(u"id",{opts::Value(opts::Null{})});},serializer)+
            ",\"undefined\":"+attemptValue([&]{return call(u"id",{opts::Value(opts::Undefined{})});},serializer)+
            ",\"nonString\":"+attemptValue([&]{return call(u"id",{opts::Value(7.0)});},serializer)+
            ",\"stringReplaceOverride\":"+overridden+"}";

        const auto tags=std::get_if<std::shared_ptr<tex::Tags>>(&(*parsed)->tags);
        if(!tags||!*tags)throw std::runtime_error("default-method witness has no Tags owner");
        const auto prototype=fixture.cx.prototype((*tags)->sourceIdentity());
        if(!prototype)throw std::runtime_error("TagFormat prototype is absent");
        std::string ownKeys="[";const auto keys=fixture.cx.ownKeys(opts::Value(*prototype));
        for(std::size_t i=0;i<keys.size();++i){if(i)ownKeys+=',';ownKeys+=q(text(fixture.cx.keyString(keys[i])));}ownKeys+=']';
        constexpr std::u16string_view methodNames[]{u"formatNumber",u"formatTag",u"formatRef",u"formatId",u"formatUrl"};
        std::string methodSurface="[";
        for(std::size_t i=0;i<std::size(methodNames);++i){if(i)methodSurface+=',';const auto name=methodNames[i];const auto descriptor=fixture.cx.descriptor(opts::Value(*prototype),opts::Key(std::u16string(name)));
            if(!descriptor){methodSurface+="{\"name\":"+q(text(name))+",\"descriptor\":null}";continue;}
            methodSurface+="{\"name\":"+q(text(name))+",\"descriptor\":{\"value\":"+serializer.value(descriptor->value)+
                ",\"writable\":"+(descriptor->writable?"true":"false")+",\"enumerable\":"+(descriptor->enumerable?"true":"false")+
                ",\"configurable\":"+(descriptor->configurable?"true":"false")+"}}";
        }methodSurface+=']';
        std::string eventJson="[";for(std::size_t i=0;i<events.size();++i){if(i)eventJson+=',';eventJson+=events[i];}eventJson+=']';
        const std::string output="{\"schema\":1,\"sourceLockSha256\":\"adff29b700cdc2e84f2451e84f9424f69679a94bd5f3448048bb6a4d3e6ea58d\",\"number\":"+number+
            ",\"id\":"+id+",\"prototypeSurface\":{\"ownKeys\":"+ownKeys+",\"methods\":"+methodSurface+"},\"callbackEvents\":"+eventJson+"}\n";
        std::cout<<output;
        return 0;
    }catch(const opts::Exception& error){std::cerr<<error.name<<": "<<text(error.message)<<'\n';}
     catch(const std::exception& error){std::cerr<<"NativeException: "<<error.what()<<'\n';}
    return 2;
}
