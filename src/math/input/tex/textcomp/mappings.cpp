// Native mapping table for MathJax 4.1.3 TextcompMappings.ts.
module folia.math.input.tex.textcomp.mappings;
import folia.math.input.tex.parser;
import folia.math.input.tex.textcomp.methods;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
struct Entry { const char* name; std::u16string_view character; const char* variant; };
#include "mapping_data.inc"
using namespace options;
}
void TextcompMappings::initializeModule(ParserRuntime& runtime){
    if(runtime.textcompMappingsInitialized)return;
    const auto method=TextcompMethods::initializeModule(runtime);
    const auto* function=std::get_if<FunctionRef>(&method.value);
    if(!function||!*function)throw Exception("TypeError",u"Textcomp Insert method is unavailable");
    PropertyList commands;
    for(const auto& entry:entries){
        const std::u16string glyph(entry.character);
        Property mapping(PropertyArray{Property(*function),Property(property_host::debugText(glyph))});
        if(entry.variant&&*entry.variant){
            const auto variant=std::string_view(entry.variant)=="oldstyle"?u"-tex-oldstyle":u"-tex-bold-oldstyle";
            mapping=Property(PropertyArray{Property(*function),Property(property_host::debugText(glyph)),Property(property_host::debugText(std::u16string_view(variant)))});
        }
        commands.set(entry.name,std::move(mapping));
    }
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("textcomp-macros"),Property(std::move(commands)));
    runtime.textcompMappingsInitialized=true;
}
}
