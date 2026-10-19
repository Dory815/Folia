// Read-only identity diagnostics for the next-six observation executable.
// Include after its module imports; output goes only to stderr.
#pragma once

namespace g220_session_trace {
namespace o = folia::math::options;
namespace t = folia::math::tex;
using namespace folia::math;

inline std::string handle(o::ObjectRef value) {
  return std::to_string(value.context) + ":" + std::to_string(value.index);
}
inline std::string describe(const o::Value& value) {
  if (const auto* ref = std::get_if<o::ObjectRef>(&value.data)) return "object(" + handle(*ref) + ")";
  if (std::holds_alternative<o::Undefined>(value.data)) return "undefined";
  if (std::holds_alternative<o::Null>(value.data)) return "null";
  if (const auto* text = std::get_if<std::u16string>(&value.data))
    return "string(" + property_host::debugText(*text) + ")";
  return "value-index(" + std::to_string(value.data.index()) + ")";
}

inline void dump(std::string_view stage, o::Context& cx, t::ParserRuntime& runtime,
                 t::TeX& jax, o::ObjectRef expectedItem) {
  const auto actualJax = jax.instanceIdentity();
  const auto canonicalJax = runtime.texIdentities.find(runtime.texIdentities.resolve(actualJax));
  const auto parsed = jax.parseOptions();
  const auto* options = std::get_if<t::ParseOptionsRef>(&parsed);
  if (!options || !*options) {
    std::cerr << "[g220-state] " << stage << " missing parseOptions actualJax=" << handle(actualJax) << '\n';
    return;
  }
  const auto actualOptions = runtime.parseOptionsIdentities.find(*options);
  if (!actualOptions) {
    std::cerr << "[g220-state] " << stage << " unbound parseOptions actualJax=" << handle(actualJax) << '\n';
    return;
  }
  const auto* map = std::get_if<std::shared_ptr<SourceStringMap<o::Value>>>(&(*options)->packageData);
  if (!map || !*map) {
    std::cerr << "[g220-state] " << stage << " missing packageData map actualJax=" << handle(actualJax) << '\n';
    return;
  }
  const auto record = (*map)->get(u"textmacros");
  if (!record || !std::holds_alternative<o::ObjectRef>(record->data)) {
    return;
  }
  const auto recordId = std::get<o::ObjectRef>(record->data);
  const auto recordJax = cx.get(recordId, o::Key(u"jax"));
  const auto recordOptions = cx.get(recordId, o::Key(u"parseOptions"));
  const auto recordParser = cx.get(recordId, o::Key(u"texParser"));
  const auto jaxOptions = cx.get(o::Value(actualJax), o::Key(u"_parseOptions"));
  const auto root = cx.get(o::Value(*actualOptions), o::Key(u"root"));
  const auto mathItem = cx.get(o::Value(*actualOptions), o::Key(u"mathItem"));
  const auto optionsObject = cx.get(o::Value(*actualOptions), o::Key(u"options"));
  const auto internalMath = cx.get(optionsObject, o::Key(u"internalMath"));
  const bool ownsMathItem = cx.hasOwn(o::Value(*actualOptions), o::Key(u"mathItem"));
  const auto* projectedJax = std::get_if<o::ObjectRef>(&recordJax.data);
  const auto* projectedOptions = std::get_if<o::ObjectRef>(&recordOptions.data);
  const auto* projectedJaxOptions = std::get_if<o::ObjectRef>(&jaxOptions.data);
  const bool recordJaxMatches = canonicalJax && projectedJax && *projectedJax == *canonicalJax;
  const bool innerOptionsDistinct = projectedOptions && *projectedOptions != *actualOptions;
  const bool jaxOptionsMatches = actualOptions &&
      projectedJaxOptions && *projectedJaxOptions == *actualOptions;
  std::cerr << "[g220-state] " << stage
            << " actualJax=" << handle(actualJax)
            << " canonicalJax=" << (canonicalJax ? handle(*canonicalJax) : "missing")
            << " jax._parseOptions=" << describe(jaxOptions)
            << " jax._parseOptionsMatches=" << (jaxOptionsMatches ? "true" : "false")
            << " actualParseOptions=" << handle(*actualOptions)
            << " textmacrosRecord=" << handle(recordId)
            << " record.jax=" << describe(recordJax)
            << " record.jaxMatches=" << (recordJaxMatches ? "true" : "false")
            << " record.parseOptions=" << describe(recordOptions)
            << " innerOptionsDistinct=" << (innerOptionsDistinct ? "true" : "false")
            << " record.texParser=" << describe(recordParser)
            << " hasOwn(mathItem)=" << (ownsMathItem ? "true" : "false")
            << " mathItem=" << describe(mathItem)
            << " compileItem=" << handle(expectedItem)
            << " root=" << describe(root)
            << " rootNonNull=" << (!std::holds_alternative<o::Null>(root.data) &&
                                    !std::holds_alternative<o::Undefined>(root.data) ? "true" : "false")
            << " internalMath=" << describe(internalMath) << '\n';
  const auto recordedOuter=cx.get(recordJax,o::Key(u"_parseOptions"));
  std::cerr << "[g220-state] " << stage
            << " record.jax._parseOptions=" << describe(recordedOuter)
            << " record.outerMatches=" << (recordedOuter==o::Value(*actualOptions)?"true":"false")
            << " record.outerOwnMathItem=" << (cx.hasOwn(recordedOuter,o::Key(u"mathItem"))?"true":"false")
            << " record.outerMathItem=" << describe(cx.get(recordedOuter,o::Key(u"mathItem")))
            << " record.outerRoot=" << describe(cx.get(recordedOuter,o::Key(u"root"))) << '\n';
  if (const auto* parserId = std::get_if<o::ObjectRef>(&recordParser.data)) {
    try {
      const auto parser = runtime.parserIdentities.resolve(*parserId);
      const auto canonicalParser = runtime.parserIdentities.find(parser);
      const auto* config = std::get_if<t::ParseOptions*>(&parser->configuration);
      std::optional<o::ObjectRef> parserOptions;
      if (config && *config)
        parserOptions = runtime.parseOptionsIdentities.find((*config)->shared_from_this());
      std::cerr << "[g220-state] " << stage
                << " parserCanonical=" << (canonicalParser ? handle(*canonicalParser) : "missing")
                << " parser.config=" << (parserOptions ? handle(*parserOptions) : "missing")
                << " parser.configMatches=" << (parserOptions && *parserOptions == *actualOptions ? "true" : "false") << '\n';
    } catch (const o::Exception& e) {
      std::cerr << "[g220-state] " << stage << " parserResolveError=" << e.name << ':'
                << property_host::debugText(e.message) << '\n';
    }
  }
}
inline void trace(std::string_view stage,o::Context& cx,t::ParserRuntime& runtime,
                  t::TeX& jax,o::ObjectRef expectedItem){
  try{dump(stage,cx,runtime,jax,expectedItem);}
  catch(const o::Exception& error){
    std::cerr<<"[g220-state] "<<stage<<" diagnostic error="<<error.name<<':'
             <<property_host::debugText(error.message)<<'\n';
  }catch(const std::exception& error){
    std::cerr<<"[g220-state] "<<stage<<" diagnostic error="<<error.what()<<'\n';
  }
}
} // namespace g220_session_trace

