// Portable Unicode 16 NFD host for Source String.normalize('NFD').
module folia.math.util.unicode_normalization;
import folia.math.util.unicode_normalization_data;
import std;
namespace folia::math {
std::u16string Unicode16Normalization::nfd(std::u16string_view input){
    using unicode16::CombiningClass;
    using unicode16::Decomposition;

    const auto combiningClass=[](char32_t point)->std::uint8_t{
        const auto& classes=unicode16::classes;
        const auto it=std::lower_bound(classes.begin(),classes.end(),point,
            [](const CombiningClass& entry,char32_t value){return entry.point<value;});
        return it!=classes.end()&&it->point==point?it->value:0;
    };

    std::vector<char32_t> decomposed;
    decomposed.reserve(input.size());
    const auto expand=[&](auto&& self,char32_t point)->void{
        // Hangul syllables have algorithmic canonical decompositions and are
        // intentionally not represented in the generated UnicodeData table.
        constexpr char32_t SBase=0xAC00,LBase=0x1100,VBase=0x1161,TBase=0x11A7;
        constexpr std::uint32_t LCount=19,VCount=21,TCount=28;
        constexpr std::uint32_t NCount=VCount*TCount,SCount=LCount*NCount;
        if(point>=SBase&&point<SBase+SCount){
            const auto index=static_cast<std::uint32_t>(point-SBase);
            self(self,LBase+index/NCount);
            self(self,VBase+(index%NCount)/TCount);
            const auto trailing=index%TCount;
            if(trailing)self(self,TBase+trailing);
            return;
        }

        const auto& decompositions=unicode16::decompositions;
        const auto it=std::lower_bound(decompositions.begin(),decompositions.end(),point,
            [](const Decomposition& entry,char32_t value){return entry.point<value;});
        if(it==decompositions.end()||it->point!=point){
            decomposed.push_back(point);
            return;
        }
        for(std::uint32_t i=0;i<it->length;++i)
            self(self,unicode16::values[it->offset+i]);
    };

    for(std::size_t i=0;i<input.size();++i){
        const auto first=static_cast<std::uint16_t>(input[i]);
        char32_t point=first;
        if(first>=0xD800&&first<=0xDBFF&&i+1<input.size()){
            const auto second=static_cast<std::uint16_t>(input[i+1]);
            if(second>=0xDC00&&second<=0xDFFF){
                point=0x10000+((first-0xD800)<<10)+(second-0xDC00);
                ++i;
            }
        }
        expand(expand,point);
    }

    // Canonical ordering is stable within each non-starter run. A class-zero
    // scalar terminates a run and cannot be crossed, including at the start.
    std::size_t cursor=0;
    while(cursor<decomposed.size()){
        if(combiningClass(decomposed[cursor])==0)++cursor;
        const auto first=cursor;
        while(cursor<decomposed.size()&&combiningClass(decomposed[cursor])!=0)++cursor;
        std::stable_sort(decomposed.begin()+static_cast<std::ptrdiff_t>(first),
                         decomposed.begin()+static_cast<std::ptrdiff_t>(cursor),
            [&](char32_t left,char32_t right){return combiningClass(left)<combiningClass(right);});
    }

    std::u16string output;
    output.reserve(decomposed.size());
    for(const char32_t point:decomposed){
        if(point<=0xFFFF){
            output.push_back(static_cast<char16_t>(point));
        }else{
            const auto value=static_cast<std::uint32_t>(point-0x10000);
            output.push_back(static_cast<char16_t>(0xD800+(value>>10)));
            output.push_back(static_cast<char16_t>(0xDC00+(value&0x3FF)));
        }
    }
    return output;
}
}
