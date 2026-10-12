// Fixed private regular expressions from MathJax 4.1.3 TexError.ts.
// Apache-2.0; UTF-16 units and ECMAScript dot semantics are preserved.
export module folia.math.input.tex.tex_error_pattern;
import std;
export namespace folia::math::tex::error_pattern {
inline bool digit(char16_t c){return c>=u'0'&&c<=u'9';}
inline bool letter(char16_t c){return c>=u'a'&&c<=u'z';}
inline bool dot(char16_t c){return c!=u'\n'&&c!=u'\r'&&c!=0x2028&&c!=0x2029;}
// Return the body start of {lowercase:%digits|...}, or npos.
inline std::size_t bodyStart(std::u16string_view s,std::size_t start){
    if(start>=s.size()||s[start++]!=u'{')return s.npos;
    const auto letters=start;while(start<s.size()&&letter(s[start]))++start;
    if(start==letters||start+1>=s.size()||s[start]!=u':'||s[start+1]!=u'%')return s.npos;
    start+=2;const auto digits=start;while(start<s.size()&&digit(s[start]))++start;
    if(start==digits||start>=s.size()||s[start]!=u'|')return s.npos;
    return start+1;
}
inline std::size_t captureEnd(std::u16string_view s,std::size_t start){
    if(start>=s.size())return s.npos;
    if(digit(s[start])){auto end=start+1;while(end<s.size()&&digit(s[end]))++end;return end;}
    if(s[start]==u'{'){
        auto end=start+1;while(end<s.size()&&digit(s[end]))++end;
        if(end>start+1&&end<s.size()&&s[end]==u'}')return end+1;
        const auto body=bodyStart(s,start);
        if(body!=s.npos){
            // Greedy repetition preserves alternative priority, not longest
            // total match. Resolve suffixes backwards in the source order:
            // %{digits}, then %dot, then [^}]. A failing suffix backtracks.
            std::vector<std::size_t> terminal(s.size()+1,s.npos);
            for(auto i=s.size();i-->body;){
                if(s[i]==u'}'){terminal[i]=i+1;continue;}
                if(s[i]==u'%'&&i+1<s.size()){
                    if(s[i+1]==u'{'){
                        auto j=i+2;while(j<s.size()&&digit(s[j]))++j;
                        if(j>i+2&&j<s.size()&&s[j]==u'}')terminal[i]=terminal[j+1];
                    }
                    if(terminal[i]==s.npos&&dot(s[i+1]))terminal[i]=terminal[i+2];
                }
                if(terminal[i]==s.npos)terminal[i]=terminal[i+1];
            }
            if(terminal[body]!=s.npos)return terminal[body];
        }
    }
    return dot(s[start])?start+1:s.npos;
}
inline std::vector<std::u16string> split(std::u16string_view s){
    std::vector<std::u16string> parts;std::size_t previous=0;
    for(std::size_t i=0;i<s.size();++i)if(s[i]==u'%'){
        const auto end=captureEnd(s,i+1);if(end==s.npos)continue;
        parts.emplace_back(s.substr(previous,i-previous));
        parts.emplace_back(s.substr(i+1,end-i-1));previous=end;i=end-1;
    }
    parts.emplace_back(s.substr(previous));return parts;
}
inline bool plural(std::u16string_view s){
    const auto body=bodyStart(s,0);
    if(body==s.npos||body>=s.size()||s.back()!=u'}')return false;
    return std::all_of(s.begin()+body,s.end()-1,dot);
}
}
