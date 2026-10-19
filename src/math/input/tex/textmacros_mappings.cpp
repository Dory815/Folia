// Native mapping registration for MathJax 4.1.3 TextMacrosMappings.ts.
module folia.math.input.tex.textmacros.mappings;
import folia.math.input.tex.token_map;
import folia.math.util.lengths;
import folia.math.util.property_host;
import folia.math.input.tex.tex_constants;
import std;
namespace folia::math::tex {
namespace {
Property method(ParserRuntime& runtime,std::string_view name){
    return TextMacrosMethods::get(runtime,property_host::text(name));
}
Property command(ParserRuntime& runtime,std::string_view name){return method(runtime,name);}
Property with(ParserRuntime& runtime,std::string_view name,Property a){return Property(PropertyArray{command(runtime,name),std::move(a)});}
Property with2(ParserRuntime& runtime,std::string_view name,Property a,Property b){return Property(PropertyArray{command(runtime,name),std::move(a),std::move(b)});}
void put(PropertyList& map,std::string name,Property value){map.set(std::move(name),std::move(value));}
}
void TextMacrosMappings::initializeModule(ParserRuntime& runtime){
    PropertyList special;
    put(special,"$",method(runtime,"Math"));put(special,"%",method(runtime,"Comment"));
    put(special,"^",method(runtime,"MathModeOnly"));put(special,"_",method(runtime,"MathModeOnly"));
    put(special,"&",method(runtime,"Misplaced"));put(special,"#",method(runtime,"Misplaced"));
    put(special,"~",method(runtime,"Tilde"));put(special," ",method(runtime,"Space"));
    put(special,"\t",method(runtime,"Space"));put(special,"\r",method(runtime,"Space"));put(special,"\n",method(runtime,"Space"));
    put(special,"\xC2\xA0",method(runtime,"Tilde"));put(special,"{",method(runtime,"OpenBrace"));put(special,"}",method(runtime,"CloseBrace"));
    put(special,"`",method(runtime,"OpenQuote"));put(special,"'",method(runtime,"CloseQuote"));
    (void)TokenMapHost::create<MacroMap>(runtime.mapHandler,Property("text-special"),Property(special));

    PropertyList map;
    put(map,"(",method(runtime,"Math"));
    for(const auto key:{"$","_","%","{","}"," ","&","#"})put(map,key,method(runtime,"SelfQuote"));
    put(map,"\\",with(runtime,"Macro",Property("$\\\\$")));
    const std::array<std::pair<const char*,const char16_t*>,12> accents{{
        {"'",u"\u00B4"},{"\u2019",u"\u00B4"},{"`",u"`"},{"\u2018",u"`"},{"^",u"^"},{"\"",u"\u00A8"},{"~",u"~"},{"=",u"\u00AF"},{".",u"\u02D9"},{"u",u"\u02D8"},{"v",u"\u02C7"},{nullptr,nullptr}}};
    for(const auto& [key,value]:accents)if(key)put(map,key,with(runtime,"Accent",Property(property_host::debugText(value))));
    put(map,"emph",method(runtime,"Emph"));
    const std::array<std::pair<const char*,const char*>,9> fonts{{{"rm","NORMAL"},{"mit","ITALIC"},{"oldstyle","OLDSTYLE"},{"cal","CALLIGRAPHIC"},{"it","MATHITALIC"},{"bf","BOLD"},{"sf","SANSSERIF"},{"tt","MONOSPACE"},{nullptr,nullptr}}};
    for(const auto& [name,variant]:fonts)if(name){const Property value=property_host::get(property_host::get(Property(TexConstant),"Variant"),variant);put(map,name,with(runtime,"SetFont",value));}
    put(map,"frak",with(runtime,"TextFont",Property("fraktur")));put(map,"Bbb",with(runtime,"TextFont",Property("double-struck")));
    const std::array<std::pair<const char*,double>,14> sizes{{{"Tiny",.5},{"tiny",.6},{"scriptsize",.7},{"SMALL",.7},{"Small",.8},{"footnotesize",.8},{"small",.9},{"normalsize",1.0},{"large",1.095},{"Large",1.2},{"LARGE",1.44},{"huge",1.73},{"Huge",2.07},{"HUGE",2.49}}};
    for(const auto& [name,size]:sizes)put(map,name,with(runtime,"SetSize",Property(size)));
    const std::array<std::pair<const char*,const char*>,7> textMacros{{{"textnormal","{\\rm #1}"},{"textup","{\\rm #1}"},{"textrm","{\\rm #1}"},{"textit","{\\it #1}"},{"textbf","{\\bf #1}"},{"textsf","{\\sf #1}"},{"texttt","{\\tt #1}"}}};
    for(const auto& [name,replacement]:textMacros)put(map,name,with2(runtime,"Macro",Property(replacement),Property(1)));
    const std::array<std::pair<const char*,const char16_t*>,6> inserts{{{"dagger",u"\u2020"},{"ddagger",u"\u2021"},{"S",u"\u00A7"},{"AA",u"\u212B"},{"ldots",u"\u2026"},{"vdots",u"\u22EE"}}};
    for(const auto& [name,value]:inserts)put(map,name,with(runtime,"Insert",Property(property_host::debugText(value))));
    const std::array<std::pair<const char*,double>,10> spaces{{{",",3.0/18},{":",4.0/18},{">",4.0/18},{";",5.0/18},{"!",-3.0/18},{"enspace",.5},{"quad",1},{"qquad",2},{"thinspace",3.0/18},{"negthinspace",-3.0/18}}};
    for(const auto& [name,width]:spaces)put(map,name,with(runtime,"Spacer",Property(width)));
    for(const auto name:{"hskip","hspace","kern","mskip","mspace","mkern"})put(map,name,method(runtime,"Hskip"));
    put(map,"rule",method(runtime,"rule"));put(map,"Rule",Property(PropertyArray{method(runtime,"Rule")}));put(map,"Space",with(runtime,"Rule",Property("blank")));
    for(const auto name:{"color","textcolor","colorbox","fcolorbox","href","style","class","data","cssId","unicode","U","char"})put(map,name,method(runtime,"CheckAutoload"));
    put(map,"ref",with(runtime,"HandleRef",Property(false)));put(map,"eqref",with(runtime,"HandleRef",Property(true)));
    put(map,"underline",with(runtime,"UnderOver",Property("2015")));
    put(map,"llap",method(runtime,"Lap"));put(map,"rlap",method(runtime,"Lap"));put(map,"phantom",method(runtime,"Phantom"));
    put(map,"vphantom",with2(runtime,"Phantom",Property(1),Property(0)));put(map,"hphantom",with2(runtime,"Phantom",Property(0),Property(1)));
    put(map,"smash",method(runtime,"Smash"));put(map,"mmlToken",method(runtime,"MmlToken"));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("text-macros"),Property(map));
}
}
