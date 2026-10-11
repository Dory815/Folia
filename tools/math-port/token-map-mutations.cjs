// Root semantic mutations of the complete frozen class family; Source golds stay fixed.
'use strict';
const mutants=[],file='input/tex/token_map.cpp',add=(name,from,to,f=file)=>mutants.push({name,file:f,edits:[[from,to]]});
add('undefined-false-collapse','value && value->missing()','value && (value->missing() || *value==Property(false))');
add('undefined-null-collapse','value && value->missing()','value && (value->missing() || *value==Property(PropertyNull{}))');
add('registration-missing','MapHandler::registerMap(std::static_pointer_cast<TokenMap>(shared_from_this()));','// Registration omitted.');
add('registration-name-read-twice','MapHandler::registerMap(std::static_pointer_cast<TokenMap>(shared_from_this()));','MapHandler::registerMap(std::static_pointer_cast<TokenMap>(shared_from_this()));(void)this->name();');
add('base-virtual-parser-setter','parser_ = std::move(parser);\n    MapHandler','this->parser(std::move(parser));\n    MapHandler');
add('map-before-registration','AbstractTokenMap::initializeSource(std::move(name), std::move(parser));\n    map_.emplace();','map_.emplace();\n    AbstractTokenMap::initializeSource(std::move(name), std::move(parser));');
add('regex-before-registration','AbstractTokenMap::initializeSource(std::move(name), std::move(parser));\n    regexp_ = std::move(regexp);','regexp_ = std::move(regexp);\n    AbstractTokenMap::initializeSource(std::move(name), std::move(parser));');
add('base-parser-getter-too-early','if (!contains(token)) return Property(PropertyNull{});\n    return parser();','auto method=parser();if(!contains(token))return Property(PropertyNull{});return method;');
add('base-lookup-skipped','const ParseMethod method = parserFor(input.token);\n    const TokenLookup mapped = lookup(input.token);','const ParseMethod method = parserFor(input.token);if(!truthy(method))return Property(PropertyNull{});\n    const TokenLookup mapped = lookup(input.token);');
add('base-mapped-identity-erased','const ParseAtom parseAtom = atom(mapped);','const ParseAtom parseAtom = Property("copied atom");');
add('base-bare-receiver-lost','method, FunctionReceiver{Missing{}}, input.parser, parseAtom);','method, FunctionReceiver{std::static_pointer_cast<NativeObject>(shared_from_this())}, input.parser, parseAtom);');
add('regex-second-live-test-erased','return contains(token) ? TokenLookup{token}','return true ? TokenLookup{token}');
add('map-nan-lookup-equality','if (property_host::sameValueZero(key, token)) return value;','if (key==token) return value;');
add('map-presence-tied-to-value','return property_host::sameValueZero(entry.first, token);','return property_host::sameValueZero(entry.first, token)&&truthy(entry.second);');
add('map-zero-not-canonical','token = Property(0.0);','// Negative-zero key retained.');
add('map-overwrite-old-value','value = std::move(object);\n            return;','(void)object;\n            return;');
add('map-overwrite-order-moved','value = std::move(object);\n            return;','value = std::move(object);std::rotate(map_->begin(),map_->begin()+1,map_->end());\n            return;');
add('map-remove-omitted','if (found != map_->end()) map_->erase(found);','(void)found;');
add('character-values-captured-eagerly','for (const auto& key : keys) {\n        const Property value = property_host::get(json, key);','for (const auto& [key,value] : entries(json)) {');
add('character-first-beyond-length','length > 0 ? array->get(0) : Property(Missing{})','array->get(0)');
add('character-second-beyond-length','length > 1 ? array->get(1) : Property(Missing{})','array->get(1)');
add('character-missing-attrs-null','length > 1 ? array->get(1) : Property(Missing{})','length > 1 ? array->get(1) : Property(PropertyNull{})');
add('character-attrs-erased','attributes = std::move(sourceAttributes);','attributes = Property(PropertyNull{});');
add('character-string-attrs-undefined','attributes = Property(PropertyNull{});','attributes = Property(Missing{});');
add('macro-entries-read-late','for (const auto& [key, value] : capturedEntries) {','for (const auto& [key, capturedValue] : capturedEntries) {const Property value=property_host::get(json,key);');
add('macro-explicit-null-defaulted','if (functionMap.missing()) functionMap = Property(PropertyList{});','if (property_host::nullish(functionMap)) functionMap = Property(PropertyList{});');
add('macro-string-method-not-resolved','return property_host::get(functionMap, *name);','return value;');
add('macro-args-slice-includes-method','args = Property(array->slice(1));','args = Property(array->slice(0));');
add('macro-parserfor-not-null','if (!truthy(macro)) return Property(PropertyNull{});','if (!truthy(macro)) return Property(Missing{});');
add('macro-second-lookup-erased','ParseResult MacroMap::parse(const ParseInput& input) {\n    const TokenLookup macro = lookup(input.token);\n    const ParseMethod method = parserFor(input.token);','ParseResult MacroMap::parse(const ParseInput& input) {\n    const TokenLookup macro = lookup(input.token);\n    const ParseMethod method = methodField(macro);');
add('macro-captured-object-refreshed','ParseResult MacroMap::parse(const ParseInput& input) {\n    const TokenLookup macro = lookup(input.token);\n    const ParseMethod method = parserFor(input.token);','ParseResult MacroMap::parse(const ParseInput& input) {\n    TokenLookup macro = lookup(input.token);\n    const ParseMethod method = parserFor(input.token);macro=lookup(input.token);');
add('delimiter-prefix-erased','return AbstractTokenMap::parse(modified);','return AbstractTokenMap::parse(input);');
add('command-prefix-erased','const Property macroToken = prefixedToken(u"\\\\", field(macro, "token"));','const Property macroToken = field(macro, "token");');
add('command-input-prefix-erased','input.parser.setCurrentCS(tokenValue(input.token));','input.parser.setCurrentCS(input.token);');
add('command-restoration-omitted','input.parser.setCurrentCS(savedCommand);','// Normal-return restoration omitted.');
add('command-restores-on-body-throw','const ParseResult result = invokeParseMethod(\n        method, FunctionReceiver{Missing{}}, input.parser, parseAtom, rest);\n    input.parser.setCurrentCS(savedCommand);','const ParseResult result = [&]()->ParseResult{try{return invokeParseMethod(method,FunctionReceiver{Missing{}},input.parser,parseAtom,rest);}catch(...){input.parser.setCurrentCS(savedCommand);throw;}}();\n    input.parser.setCurrentCS(savedCommand);');
add('environment-private-setter','this->parser(std::move(parser));','AbstractTokenMap::parser(std::move(parser));');
add('environment-receiver-erased','generalParser, FunctionReceiver{owner}, input.parser, parseAtom, rest);','generalParser, FunctionReceiver{Missing{}}, input.parser, parseAtom, rest);');
add('environment-method-slot-wrapped','const std::any specificParser = callableSlot(envParser);','const std::any specificParser = envParser;');
add('environment-args-spread','const std::array<std::any, 2> rest{specificParser, args};','std::vector<std::any> rest{specificParser};for(const auto&value:arguments::expand(args))rest.emplace_back(value);');
add('environment-args-cloned','const std::array<std::any, 2> rest{specificParser, args};','const std::array<std::any, 2> rest{specificParser, Property(PropertyArray::array(args)->slice(0))};');
require('./native-mutation-runner.cjs')({
 name:'math-port-token-map-mutations',fixture:'reference_token_map.cpp',pattern:'*math_port_token_map*',assertions:870,tests:13,mutants,
 modules:['util/option_types.ixx','util/object_types.ixx','util/callable_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','util/property_host.ixx','util/prioritized_list.ixx','util/promise.ixx','util/function_types.ixx','util/function_list.ixx','util/method_config.ixx','util/argument_expansion.ixx','input/tex/token.ixx','input/tex/types.ixx','input/tex/macro.ixx','input/tex/token_map_types.ixx','input/tex/map_handler.ixx','input/tex/token_map.ixx'],
 units:['util/property_host.cpp','util/promise.cpp','util/function_list.cpp','util/function_value.cpp','core/tree/property_array.cpp','util/argument_expansion.cpp','input/tex/token.cpp','input/tex/macro.cpp','input/tex/parse_callable.cpp','input/tex/map_handler.cpp','input/tex/token_map.cpp']
});
