module folia.math.output.common;
import :mn;
import folia.math.output.common.font_data;
import folia.math.util.property_host;
import std;
namespace folia::math::output::common {
std::shared_ptr<std::vector<std::uint32_t>> CommonMn::remapChars(
    std::shared_ptr<std::vector<std::uint32_t>> chars)const{
    if(chars&&!chars->empty()){
        if(!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getRemappedChar')");
        if(const auto remapped=font->getRemappedChar("mn",static_cast<double>(chars->front()))){
            const auto replacement=unicodeChars(*remapped,variant);
            if(replacement->size()==1){(*chars)[0]=replacement->front();}
            else{
                auto expanded=std::make_shared<std::vector<std::uint32_t>>(*replacement);
                expanded->insert(expanded->end(),chars->begin()+1,chars->end());
                chars=std::move(expanded);
            }
        }
    }
    return chars;
}
}
