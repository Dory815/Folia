/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 input/tex/base/BaseMappings.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.input.tex.base_mappings;

import folia.math.input.tex.parser;
import folia.math.input.tex.token_map;
import folia.math.input.tex.map_handler;
import folia.math.util.property_host;
import folia.math.util.regexp_binding;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
using property_host::debugText;

Property object(std::initializer_list<std::pair<std::string,Property>> fields) {
    PropertyList result;
    for (const auto& [name,value] : fields) result.setOwn(name,value);
    return Property(result);
}

Property tablesValue(std::map<std::string,Property>& tables,std::string_view name) {
    const auto found=tables.find(std::string(name));
    if(found==tables.end())property_host::typeError("Missing generated BaseMappings table");
    return found->second;
}

ParseMethod parseMethod(const Property& table,std::u16string_view name) {
    return functions::fromProperty(property_host::get(table,debugText(name)));
}
}

#include "base_mapping_data.inc"

void BaseMappings::initializeModule(ParserRuntime& runtime,const Property& methods,
                                   const Property& parseMethods) {
    std::map<std::string,Property> tables;
    appendPinnedBaseMappingTables(tables);

    // Source construction order is observable through MapHandler registration.
    const auto letterRegexp=runtime.regexp().create(u"[a-z]",u"i");
    (void)TokenMapHost::create<RegExpMap>(runtime.mapHandler,Property("letter"),parseMethod(parseMethods,u"variable"),regexpContext(runtime,Value(letterRegexp)));
    const auto digitRegexp=runtime.regexp().create(u"[0-9.,]");
    (void)TokenMapHost::create<RegExpMap>(runtime.mapHandler,Property("digit"),parseMethod(parseMethods,u"digit"),regexpContext(runtime,Value(digitRegexp)));
    const auto commandRegexp=runtime.regexp().create(u"^\\\\");
    (void)TokenMapHost::create<RegExpMap>(runtime.mapHandler,Property("command"),parseMethod(parseMethods,u"controlSequence"),regexpContext(runtime,Value(commandRegexp)));

    const Property special=tablesValue(tables,"special");
    (void)TokenMapHost::create<MacroMap>(runtime.mapHandler,Property("special"),special,methods);
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("lcGreek"),parseMethod(parseMethods,u"lcGreek"),tablesValue(tables,"lcGreek"));
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("ucGreek"),parseMethod(parseMethods,u"ucGreek"),tablesValue(tables,"ucGreek"));
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("mathchar0mi"),parseMethod(parseMethods,u"mathchar0mi"),tablesValue(tables,"mathchar0mi"));
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("mathchar0mo"),parseMethod(parseMethods,u"mathchar0mo"),tablesValue(tables,"mathchar0mo"));
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("mathchar7"),parseMethod(parseMethods,u"mathchar7"),tablesValue(tables,"mathchar7"));
    (void)TokenMapHost::create<DelimiterMap>(runtime.mapHandler,Property("delimiter"),parseMethod(parseMethods,u"delimiter"),tablesValue(tables,"delimiter"));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("macros"),tablesValue(tables,"macros"),methods);
    (void)TokenMapHost::create<EnvironmentMap>(runtime.mapHandler,Property("environment"),parseMethod(parseMethods,u"environment"),tablesValue(tables,"environment"),methods);
    (void)TokenMapHost::create<CharacterMap>(runtime.mapHandler,Property("not_remap"),ParseMethod{Property(PropertyNull{})},tablesValue(tables,"not_remap"));
}

std::shared_ptr<RegExpContext> BaseMappings::regexpContext(ParserRuntime& runtime,const Value& value){
    return runtime.makeRegExpContext(value);
}

} // namespace folia::math::tex
