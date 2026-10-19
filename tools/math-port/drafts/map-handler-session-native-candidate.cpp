// Focused native observation for per-ParserRuntime MapHandler state.
// This focused candidate uses two real NativeRenderSession ParserRuntime owners.
import std;
import folia.math.input.tex.map_handler;
import folia.math.input.tex.token_map;
import folia.math.input.tex.tex_parser_context;
import folia.math.output.common.font_data_surface;
import folia.math.output.svg.render_session;
import folia.math.util.options;
import folia.math.util.property_host;

namespace {
namespace math=folia::math;
namespace tex=folia::math::tex;
namespace property_host=folia::math::property_host;
namespace opts=folia::math::options;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

common::FontLoadHost fontHost(){
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=[](const std::any&){};
    return host;
}

struct Parser final:tex::ParserContext {
    math::Property current=math::Property("initial");
    opts::Context* context=nullptr;
    math::Property currentCS()const override{return current;}
    void setCurrentCS(math::Property value)override{current=std::move(value);}
};

struct CallbackMap final:tex::TokenMap {
    math::Property name_;
    std::string label_;
    opts::Context* context_;
    opts::ObjectRef callback_;
    std::vector<std::string>* calls_;
    CallbackMap(std::string name,std::string label,opts::Context& context,
                opts::ObjectRef callback,std::vector<std::string>& calls)
        :name_(std::move(name)),label_(std::move(label)),context_(&context),
         callback_(callback),calls_(&calls){}
    opts::ObjectRef callbackIdentity()const{return callback_;}
    math::Property name()const override{return name_;}
    tex::ParseMethod parser()const override{return math::Property(math::Missing{});}
    void parser(tex::ParseMethod)override{}
    bool contains(const math::Property&)override{return true;}
    tex::ParseMethod parserFor(const math::Property&)override{return math::Property(math::Missing{});}
    tex::ParseResult parse(const tex::ParseInput& input)override{
        const auto* parser=dynamic_cast<const Parser*>(&input.parser);
        if(!parser||parser->context!=context_)
            throw std::runtime_error("map callback entered from another Context session");
        return math::Property(sourceCallback());
    }
    std::string sourceCallback(){
        const opts::Value result=context_->call(callback_,opts::Value(opts::Undefined{}));
        const auto* text=std::get_if<std::u16string>(&result.data);
        if(!text)throw std::runtime_error("map callback returned a nonstring");
        calls_->push_back(label_);
        return property_host::debugText(*text);
    }
    tex::TokenLookup lookup(const math::Property&)override{return math::Property(label_);}
};

void addNamedMap(tex::SubHandler& handler,std::string name){
    handler.add(math::Property(math::PropertyArray{math::Property(std::move(name))}));
}

std::string invokeRegisteredMap(tex::MapHandler& registry){
    const auto map=registry.getMap(math::Property("session-map"));
    const auto callback=std::dynamic_pointer_cast<CallbackMap>(map);
    if(!callback)throw std::runtime_error("Source lookup did not resolve the expected session map");
    return callback->sourceCallback();
}

std::string parse(tex::SubHandler& handler,Parser& parser){
    const tex::ParseInput input{parser,math::Property("probe")};
    const tex::ParseResult result=handler.parse(input);
    const auto* value=std::get_if<math::Property>(&result);
    if(!value)throw std::runtime_error("SubHandler did not return a Property result");
    return property_host::debugText(property_host::string(*value));
}

void require(bool value,std::string_view message){if(!value)throw std::runtime_error(std::string(message));}
}

int main(){
    try{
        svg::NativeRenderSession sessionA(fontHost()),sessionB(fontHost());
        opts::Context& contextA=sessionA.context();
        opts::Context& contextB=sessionB.context();
        tex::ParserRuntime& runtimeA=sessionA.services().parser();
        tex::ParserRuntime& runtimeB=sessionB.services().parser();
        std::vector<std::string> callbacks;
        const auto callbackA=contextA.function(u"callbackA",[&](opts::Context&,opts::Value receiver,std::span<const opts::Value>){
            if(!std::holds_alternative<opts::Undefined>(receiver.data))throw std::runtime_error("A callback receiver changed");
            return opts::Value(u"A");
        });
        const auto callbackB=contextB.function(u"callbackB",[&](opts::Context&,opts::Value receiver,std::span<const opts::Value>){
            if(!std::holds_alternative<opts::Undefined>(receiver.data))throw std::runtime_error("B callback receiver changed");
            return opts::Value(u"B");
        });
        require(callbackA!=callbackB,"callback identities unexpectedly match across Contexts");
        tex::MapHandler& registryA=runtimeA.mapHandler;
        tex::MapHandler& registryB=runtimeB.mapHandler;
        auto mapA=std::make_shared<CallbackMap>("session-map","A",contextA,callbackA,callbacks);
        auto mapB=std::make_shared<CallbackMap>("session-map","B",contextB,callbackB,callbacks);
        require(mapA->callbackIdentity()==callbackA&&mapB->callbackIdentity()==callbackB,
                "map did not retain its original Context callback identity");
        registryA.registerMap(mapA);
        const auto sameRuntimeA=registryA.getMap(math::Property("session-map"));
        registryB.registerMap(mapB);

        const std::array<std::string,4> expected{"A","B","A","B"};
        const std::array<std::string,4> observed{
            invokeRegisteredMap(registryA),invokeRegisteredMap(registryB),
            invokeRegisteredMap(registryA),invokeRegisteredMap(registryB)};
        const bool sameModuleOwnersShareMap=sameRuntimeA==mapA;
        const bool moduleAStillOwnsOriginalMapAfterModuleBRegistersSameName=
            registryA.getMap(math::Property("session-map"))==mapA;
        const bool moduleBOwnsItsMap=registryB.getMap(math::Property("session-map"))==mapB;
        const bool distinctModuleMaps=mapA!=mapB;
        const bool interleavedCallbacks=std::ranges::equal(observed,expected);
        require(sameModuleOwnersShareMap,"same-runtime registry lookup changed map identity");
        require(moduleAStillOwnsOriginalMapAfterModuleBRegistersSameName,"A registry resolved another runtime's map");
        require(moduleBOwnsItsMap,"B registry resolved another runtime's map");
        require(distinctModuleMaps,"same-name maps unexpectedly share identity");
        require(interleavedCallbacks,"interleaved map callbacks crossed registry owners");

        // SubHandler lookup, sharing, and callback checks are native-only
        // coverage after the Source-shaped registration and invocation path.
        tex::SubHandler handlerA(registryA),handlerA2(registryA),handlerB(registryB);
        addNamedMap(handlerA,"session-map");
        addNamedMap(handlerA2,"session-map");
        addNamedMap(handlerB,"session-map");
        const bool subHandlersShareMap=
            handlerA.retrieve(math::Property("session-map"))==mapA&&
            handlerA2.retrieve(math::Property("session-map"))==mapA;
        require(subHandlersShareMap,"handlers in one runtime did not share the map");
        Parser parserA,parserB;parserA.context=&contextA;parserB.context=&contextB;
        const std::array<std::string,4> subHandlerCallbacks{
            parse(handlerA,parserA),parse(handlerB,parserB),
            parse(handlerA,parserA),parse(handlerB,parserB)};
        const bool callbackIdentitiesPreserved=std::ranges::equal(subHandlerCallbacks,expected);
        require(callbackIdentitiesPreserved,"wrong SubHandler callback identities ran");

        // Additional native-only coverage follows the complete Source witness.
        // Replacing a registry entry affects future adds only; already
        // configured handlers retain the concrete Source map they added.
        const auto callbackA2=contextA.function(u"callbackA2",[&](opts::Context&,opts::Value receiver,std::span<const opts::Value>){
            if(!std::holds_alternative<opts::Undefined>(receiver.data))throw std::runtime_error("replacement callback receiver changed");
            return opts::Value(u"A2");
        });
        auto replacementA=std::make_shared<CallbackMap>("session-map","A2",contextA,callbackA2,callbacks);
        registryA.registerMap(replacementA);
        tex::SubHandler futureA(registryA);
        addNamedMap(futureA,"session-map");
        const bool replacementVisibleToFutureAdds=futureA.retrieve(math::Property("session-map"))==replacementA;
        const bool replacementPreservesExistingHandler=handlerA.retrieve(math::Property("session-map"))==mapA;
        require(replacementVisibleToFutureAdds,"replacement was not visible to future adds");
        require(replacementPreservesExistingHandler,"replacement changed an existing handler list");

        // TokenMapHost's constructor path must use the same explicit owner.
        auto initializedA=tex::TokenMapHost::create<tex::CommandMap>(
            registryA,math::Property("host-owned"),math::Property(math::PropertyList{}));
        const bool tokenMapHostUsesSelectedOwner=
            registryA.getMap(math::Property("host-owned"))==initializedA&&
            !registryB.getMap(math::Property("host-owned"));
        require(tokenMapHostUsesSelectedOwner,"TokenMapHost registered outside its owner");

        std::unique_ptr<tex::SubHandler> orphan;
        std::weak_ptr<tex::TokenMap> releasedMap;
        {
            auto registry=std::make_unique<tex::MapHandler>();
            auto map=std::make_shared<CallbackMap>("lifetime-map","L",contextA,callbackA,callbacks);
            releasedMap=map;
            registry->registerMap(map);
            orphan=std::make_unique<tex::SubHandler>(*registry);
            addNamedMap(*orphan,"lifetime-map");
        }
        bool deadOwnerRejected=false;
        try{(void)parse(*orphan,parserA);}
        catch(const opts::Exception&){deadOwnerRejected=true;}
        require(deadOwnerRejected,"SubHandler used a destroyed registry owner");
        orphan.reset();
        const bool registryRelease=releasedMap.expired();
        require(registryRelease,"map callback outlived its registry and handlers");

        std::cout<<"{\"cases\":[{\"name\":\"map-handler-two-module-instances-interleaved\",\"values\":{"
                 <<"\"sameModuleOwnersShareMap\":"<<(sameModuleOwnersShareMap?"true":"false")<<','
                 <<"\"moduleAStillOwnsOriginalMapAfterModuleBRegistersSameName\":"
                 <<(moduleAStillOwnsOriginalMapAfterModuleBRegistersSameName?"true":"false")<<','
                 <<"\"moduleBOwnsItsMap\":"<<(moduleBOwnsItsMap?"true":"false")<<','
                 <<"\"distinctModuleMaps\":"<<(distinctModuleMaps?"true":"false")<<','
                 <<"\"interleavedCallbacks\":[";
        for(std::size_t i=0;i<observed.size();++i){if(i)std::cout<<',';std::cout<<'"'<<observed[i]<<'"';}
        std::cout<<"]}}],\"nativeChecks\":{"
                 <<"\"callbackIdentitiesPreserved\":"<<(callbackIdentitiesPreserved?"true":"false")<<','
                 <<"\"subHandlersShareMap\":"<<(subHandlersShareMap?"true":"false")<<','
                 <<"\"replacementVisibleToFutureAdds\":"<<(replacementVisibleToFutureAdds?"true":"false")<<','
                 <<"\"replacementPreservesExistingHandler\":"<<(replacementPreservesExistingHandler?"true":"false")<<','
                 <<"\"tokenMapHostUsesSelectedOwner\":"<<(tokenMapHostUsesSelectedOwner?"true":"false")<<','
                 <<"\"deadOwnerRejected\":"<<(deadOwnerRejected?"true":"false")<<','
                 <<"\"registryRelease\":"<<(registryRelease?"true":"false")<<"}}\n";
        return 0;
    }catch(const opts::Exception& error){
        std::cerr<<error.name<<": "<<property_host::debugText(error.message)<<'\n';return 1;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
