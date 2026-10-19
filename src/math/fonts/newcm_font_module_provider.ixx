// Per-session loader for the generated NewCM dynamic font modules.
export module folia.math.fonts.newcm_font_module_provider;
export import folia.math.fonts.newcm_font_data;
export import folia.math.util.promise;
import std;

export namespace folia::math::fonts {
class NewcmFontModuleProvider {
    struct State;
    std::shared_ptr<State> state_;
public:
    explicit NewcmFontModuleProvider(
        std::shared_ptr<NewcmFontDataClass> fontClass,
        async::QueueRef queue);
    NewcmFontModuleProvider(const NewcmFontModuleProvider&) = delete;
    NewcmFontModuleProvider& operator=(const NewcmFontModuleProvider&) = delete;

    // Resolve one request made by the actual FontData load operation. The
    // request string is computed once by FontData::dynamicFileName and passed
    // through; the provider never reconstructs or normalizes it.
    void loadSync(output::common::FontData& font,
                  const output::common::DynamicFileRef& dynamic,
                  std::string request);
    async::PromiseRef loadAsync(
        std::shared_ptr<output::common::FontData> font,
        output::common::DynamicFileRef dynamic,
        std::string request);

    // The provider is an owner-scoped module cache. It retains the actual
    // loaded DynamicFile record, not a copy of its setup/data.
    std::size_t loadedModuleCount() const;
    std::optional<std::string> resolvedModuleName(std::string_view request) const;
    // Report whether both requests resolve to the same actual cached module
    // record, rather than inferring identity from equal filename strings.
    bool sameCachedModuleIdentity(std::string_view first,
                                  std::string_view second) const;
};
}
