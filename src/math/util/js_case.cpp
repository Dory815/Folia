// Default Unicode lowercase host primitive, pinned to the reference runtime.
module folia.math.util.js_case;
import folia.math.util.utf16;
import std;
namespace folia::math::js_case {
namespace {
bool finalSigma(const std::vector<char32_t>& points,std::size_t index){
    bool before=false;
    for(std::size_t j=index;j>0;){
        const auto flags=detail::caseProperties(points[--j]);
        if(flags&2)continue;
        before=(flags&1)!=0;break;
    }
    if(!before)return false;
    for(std::size_t j=index+1;j<points.size();++j){
        const auto flags=detail::caseProperties(points[j]);
        if(flags&2)continue;
        return (flags&1)==0;
    }
    return true;
}
void append(std::u16string& text,char32_t point){
    if(point<=0xFFFF){text.push_back(static_cast<char16_t>(point));return;}
    point-=0x10000;
    text.push_back(static_cast<char16_t>(0xD800+(point>>10)));
    text.push_back(static_cast<char16_t>(0xDC00+(point&0x3FF)));
}
}
std::u16string lowerCase(std::u16string_view text){
    const auto points=utf16::codePoints(text);
    std::u16string result;result.reserve(text.size());
    for(std::size_t i=0;i<points.size();++i){
        const auto point=points[i];
        if(point==0x03A3&&finalSigma(points,i)){append(result,0x03C2);continue;}
        const auto mapped=detail::lowerMapping(point);
        if(mapped.empty())append(result,point);
        else for(const auto lower:mapped)append(result,lower);
    }
    return result;
}
}
