// Root compiling behavioral mutants; observations remain unchanged.
'use strict';
const file='util/argument_expansion.cpp';
module.exports=[
 ['nullish-empty','property_host::typeError("Cannot convert null or undefined to object");','return {};'],
 ['nullish-error-kind','property_host::typeError("Cannot convert null or undefined to object");','invalid_array_length();'],
 ['string-utf16-units','const auto points = utf16::codePoints(units);','std::vector<char32_t> points;for(auto unit:units)points.push_back(unit);'],
 ['string-reversed','for (const auto point : points) result.emplace_back(iterator_string(point));','for (auto i=points.rbegin();i!=points.rend();++i)result.emplace_back(iterator_string(*i));'],
 ['lone-surrogate-replaced','units.push_back(static_cast<char16_t>(point));','units.push_back(static_cast<char16_t>((point>=0xD800&&point<=0xDFFF)?0xFFFD:point));'],
 ['supplementary-low-lost','units.push_back(static_cast<char16_t>(0xDC00 + (scalar & 0x3FF)));','/* missing low unit */'],
 ['array-last-lost','const auto length = static_cast<std::uint64_t>(array->size());','const auto length = static_cast<std::uint64_t>(array->size()?array->size()-1:0);'],
 ['array-prototype-lost','result.push_back(array->get(static_cast<std::size_t>(index)));','result.push_back(array->record().getOwn(std::to_string(index)));'],
 ['array-hole-skipped','result.push_back(array->get(static_cast<std::size_t>(index)));','if(array->has(static_cast<std::size_t>(index)))result.push_back(array->get(static_cast<std::size_t>(index)));'],
 ['array-reversed','result.push_back(array->get(static_cast<std::size_t>(index)));','result.push_back(array->get(static_cast<std::size_t>(length-1-index)));'],
 ['array-nested-cloned','result.push_back(array->get(static_cast<std::size_t>(index)));','auto value=array->get(static_cast<std::size_t>(index));if(auto handle=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);handle&&*handle)*handle=std::make_shared<PropertyStorage>(**handle);result.push_back(value);'],
 ['generic-length-ignored','to_length(property_host::get(input, "length"))','0'],
 ['length-rounded-up','std::trunc(number)','std::ceil(number)'],
 ['length-string-ignored','const double number = folia::math::property_host::number(value);','const double number = std::holds_alternative<double>(value.value)?std::get<double>(value.value):0.0;'],
 ['length-infinity-zero','if (number >= max_safe_integer) return 9007199254740991ULL;','if (number >= max_safe_integer) return 0;'],
 ['arraycreate-wrong-error','throw folia::math::options::Exception("RangeError", u"Invalid array length");','throw folia::math::options::Exception("TypeError", u"Invalid array length");'],
 ['generic-inherited-lost','result.push_back(property_host::get(input, std::to_string(index)));','result.push_back(PropertyList::record(input)?PropertyList::record(input)->getOwn(std::to_string(index)):property_host::get(input,std::to_string(index)));'],
 ['generic-reversed','result.push_back(property_host::get(input, std::to_string(index)));','result.push_back(property_host::get(input,std::to_string(length-1-index)));'],
].map(([name,from,to])=>({name,file,edits:[[from,to]]}));
