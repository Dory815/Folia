module folia.math.fonts.newcm_font_module_provider;
import folia.math.core.tree.factory_types;
import folia.math.util.property_host;
import std;

namespace folia::math::fonts {
using namespace output::common;

struct NewcmFontModuleProvider::State {
    static constexpr std::string_view jsPrefix=
        "@mathjax/mathjax-newcm-font/js/svg/dynamic/";
    static constexpr std::string_view cjsPrefix=
        "@mathjax/mathjax-newcm-font/cjs/svg/dynamic/";
    struct Module {
        std::string canonicalName;
        DynamicFileRef dynamic;
    };
    std::shared_ptr<NewcmFontDataClass> fontClass;
    async::QueueRef queue;
    mutable std::mutex mutex;
    // Both package exports resolve to the same pinned CJS filename. Keep one
    // module entry/cache identity while retaining the exact accepted request
    // spellings as resolver keys.
    std::unordered_map<std::string,std::shared_ptr<Module>> modules;
    std::unordered_map<std::string,std::shared_ptr<Module>> loaded;

    std::string sourceText(const options::Value& value)const{
        if(!fontClass->sourceProperties)
            throw tree::FactoryError("Unimplemented",u"NewCM DynamicFile conversion requires its Source Context");
        return property_host::debugText(property_host::string(fontClass->sourceProperties->from(value)));
    }
    options::Value dynamicField(const DynamicFileRef& dynamic,std::u16string_view name)const{
        if(dynamic&&dynamic->sourceIdentity&&fontClass->sourceProperties){
            auto& context=fontClass->sourceProperties->context();
            return context.get(options::Value(*dynamic->sourceIdentity),options::Key(std::u16string(name)));
        }
        if(name==u"extension")return dynamic?dynamic->extension:options::Value(options::Undefined{});
        if(name==u"file")return dynamic?dynamic->file:options::Value(options::Undefined{});
        return options::Value(options::Undefined{});
    }

    State(std::shared_ptr<NewcmFontDataClass> owner,async::QueueRef jobs)
        :fontClass(std::move(owner)),queue(std::move(jobs)){
        if(!fontClass||!queue)
            throw tree::FactoryError("TypeError",u"NewCM module provider requires its actual font class and queue");
        if(fontClass->dynamicFiles.empty()||fontClass->bundledDynamicSetups.empty())
            throw tree::FactoryError("TypeError",u"NewCM source data must be initialized before creating its module provider");
        for(const auto&[key,dynamic]:fontClass->dynamicFiles){
            if(!dynamic)continue;
            const auto file=sourceText(dynamicField(dynamic,u"file"));
            const auto module=file.size()>=3&&file.substr(file.size()-3)==".js"
                ?file:file+".js";
            const auto canonical=std::string(cjsPrefix)+module;
            auto entry=std::make_shared<Module>(Module{canonical,dynamic});
            modules.emplace(std::string(cjsPrefix)+module,entry);
            modules.emplace(std::string(jsPrefix)+module,std::move(entry));
        }
        if(modules.empty())
            throw tree::FactoryError("TypeError",u"NewCM source data has no resolvable dynamic modules");
    }

    [[noreturn]] static void missing(std::string_view request){
        throw tree::FactoryError("Error",u"Cannot find module '"+
            property_host::text(request)+u"'");
    }

    void evaluate(FontData& font,
                  const DynamicFileRef& dynamic,std::string request){
        const auto module=modules.find(request);
        if(&font.CLASS()!=fontClass.get()||!dynamic||module==modules.end())
            missing(request);
        const auto& loadedDynamic=module->second->dynamic;
        const auto key=sourceText(dynamicField(loadedDynamic,u"extension"))+"\x1f"+
            sourceText(dynamicField(loadedDynamic,u"file"));
        const auto recipe=fontClass->bundledDynamicSetups.find(key);
        if(recipe==fontClass->bundledDynamicSetups.end()||!recipe->second)
            missing(request);

        const auto canonical=module->second->canonicalName;
        {
            std::lock_guard lock(mutex);
            if(loaded.contains(canonical)){
                // Node publishes a module in its cache before evaluating it.
                // Re-entrant requires therefore see the same in-progress
                // canonical module and do not execute its body twice.
                return;
            }
            loaded.emplace(canonical,module->second);
        }

        try{
            // Requiring the resolved module evaluates its generated recipe
            // against the class's canonical DynamicFile record. FontData then
            // calls setup on the original argument, in Source order.
            fontClass->activateBundledDynamicSetup(loadedDynamic);
            if(!loadedDynamic->setup)
                throw tree::FactoryError("TypeError",u"dynamic.setup is not a function");
        }catch(...){
            // CommonJS removes a module from its cache when evaluation throws.
            std::lock_guard lock(mutex);
            loaded.erase(canonical);
            throw;
        }
    }
};

NewcmFontModuleProvider::NewcmFontModuleProvider(
    std::shared_ptr<NewcmFontDataClass> fontClass,async::QueueRef queue)
    :state_(std::make_shared<State>(std::move(fontClass),std::move(queue))){}

void NewcmFontModuleProvider::loadSync(
    FontData& font,const DynamicFileRef& dynamic,
    std::string request){
    state_->evaluate(font,dynamic,std::move(request));
}

async::PromiseRef NewcmFontModuleProvider::loadAsync(
    std::shared_ptr<FontData> font,DynamicFileRef dynamic,std::string request){
    auto result=async::Promise::create(state_->queue);
    auto state=state_;
    state_->queue->post([state,result,font=std::move(font),dynamic=std::move(dynamic),
                         request=std::move(request)]()mutable{
        try{
            if(!font)State::missing(request);
            state->evaluate(*font,dynamic,std::move(request));
            result->resolve();
        }catch(...){
            result->reject(std::current_exception());
        }
    });
    return result;
}

std::size_t NewcmFontModuleProvider::loadedModuleCount()const{
    std::lock_guard lock(state_->mutex);
    return state_->loaded.size();
}

std::optional<std::string> NewcmFontModuleProvider::resolvedModuleName(
    std::string_view request)const{
    const auto found=state_->modules.find(std::string(request));
    if(found==state_->modules.end())return std::nullopt;
    return found->second->canonicalName;
}

bool NewcmFontModuleProvider::sameCachedModuleIdentity(
    std::string_view first,std::string_view second)const{
    const auto firstModule=state_->modules.find(std::string(first));
    const auto secondModule=state_->modules.find(std::string(second));
    if(firstModule==state_->modules.end()||secondModule==state_->modules.end())return false;
    std::lock_guard lock(state_->mutex);
    const auto firstCached=state_->loaded.find(firstModule->second->canonicalName);
    const auto secondCached=state_->loaded.find(secondModule->second->canonicalName);
    return firstCached!=state_->loaded.end()&&secondCached!=state_->loaded.end()&&
        firstCached->second==secondCached->second;
}
}
