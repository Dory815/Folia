// Root P22a acceptance. Semantic mutants of native bindings, not source fixtures.
'use strict';
const mutants=[],add=(name,file,from,to)=>mutants.push({name,file,edits:[[from,to]]});
const values='util/function_value.cpp',slice='core/tree/property_array.cpp';
add('raw-reference-erased',values,'return *function;\n    return {};','return {};\n    return {};');
add('direct-reference-erased',values,'std::get_if<FunctionRef>(&value)) return *function;','std::get_if<FunctionRef>(&value)) return {};');
add('raw-to-typed-erased',values,'return FunctionValue{*function};','return FunctionValue{Property(Missing{})};');
add('raw-to-typed-cloned',values,'return FunctionValue{*function};','return FunctionValue{std::make_shared<Function>(**function)};');
add('typed-to-raw-erased',values,'return Property(std::get<FunctionRef>(value));','return Property(Missing{});');
add('typed-to-raw-cloned',values,'return Property(std::get<FunctionRef>(value));','const auto f=std::get<FunctionRef>(value);return Property(f?std::make_shared<Function>(*f):f);');
add('container-sensitive-equality',values,'const FunctionRef* leftFunction = functionValue(left);','if(left.index()!=right.index())return false;\n    const FunctionRef* leftFunction = functionValue(left);');
add('all-functions-equal',values,'*leftFunction == *rightFunction','true');
add('nan-equal',values,'return leftProperty && rightProperty && *leftProperty == *rightProperty;','if(leftProperty&&rightProperty){auto a=std::get_if<double>(&leftProperty->value),b=std::get_if<double>(&rightProperty->value);if(a&&b&&std::isnan(*a)&&std::isnan(*b))return true;}\n    return leftProperty && rightProperty && *leftProperty == *rightProperty;');
add('slice-ignores-first',slice,'std::min(first, length)','std::size_t{0}');
add('slice-length-collapsed',slice,'result.resize(resultLength);','result.resize(0);');
add('slice-spread-fills-holes',slice,'if (!source.has(key)) continue;','// Spread materializes absent slots.');
add('slice-own-only',slice,'source.has(key)','source.hasOwn(key)');
add('slice-undefined-dropped',slice,'result.set(sourceIndex - start, value);','if(!value.missing())result.set(sourceIndex - start, value);');
add('slice-aliases-input',slice,'PropertyArray result;','if(first==0)return *this;\n    PropertyArray result;');
add('slice-functions-cloned',slice,'result.set(sourceIndex - start, value);','if(const auto* f=std::get_if<FunctionRef>(&value.value))result.set(sourceIndex-start,Property(std::make_shared<Function>(**f)));else result.set(sourceIndex - start, value);');
add('function-list-rejects-raw','util/function_types.ixx','const auto function=reference(receiver->item);','const auto* direct=std::get_if<FunctionRef>(&receiver->item);const FunctionRef function=direct?*direct:nullptr;');
add('tex-call-rejects-raw','input/tex/parse_callable.cpp','const ParseCallableRef callable = functions::reference(method);','const auto* direct=std::get_if<FunctionRef>(&method);const ParseCallableRef callable = direct?*direct:nullptr;');
add('property-clones-function','core/tree/property.ixx','Property(FunctionRef v) : value(v) {','Property(FunctionRef v) : value(v?std::make_shared<Function>(*v):v) {');
add('property-null-accepted','core/tree/property.ixx','if(!v)throw options::Exception("TypeError",u"Native callable property is null");','// Invalid null function accepted.');
add('unbound-metadata-fabricated','util/property_host.ixx','typeError("Native callable property evaluation is unbound");','return Property(Missing{});');
add('closure-state-copied','util/callable_types.ixx','const auto active=callback_;','const auto active=callback_?std::make_shared<Callback>(*callback_):nullptr;');
require('./native-mutation-runner.cjs')({
 name:'math-port-function-property-mutations',fixture:'reference_function_property.cpp',pattern:'*callable_property*',assertions:4040,tests:5,mutants,
 modules:['util/option_types.ixx','util/object_types.ixx','util/callable_types.ixx','core/tree/property.ixx','core/tree/factory_types.ixx','util/js_number.ixx','util/js_numeric_input.ixx','util/utf16_types.ixx','util/utf16.ixx','util/property_host.ixx','util/prioritized_list.ixx','util/promise.ixx','util/function_types.ixx','util/function_list.ixx','input/tex/token.ixx','input/tex/types.ixx','input/tex/macro.ixx'],
 units:['util/property_host.cpp','util/promise.cpp','util/function_list.cpp','util/function_value.cpp','core/tree/property_array.cpp','input/tex/token.cpp','input/tex/macro.cpp','input/tex/parse_callable.cpp']
});
