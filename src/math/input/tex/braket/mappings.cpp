// Native port of MathJax 4.1.3 input/tex/braket/BraketMappings.ts.
module folia.math.input.tex.braket.mappings;
import folia.math.input.tex.braket.methods;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
void BraketMappings::initializeModule(ParserRuntime& runtime){
    const auto methods=BraketMethods::create(runtime);
    PropertyList commands;
    const auto macroTuple=[&](std::u16string source,double count){
        return Property(PropertyArray{methods.macro,property_host::debugText(source),Property(count)});
    };
    const auto bracketTuple=[&](std::u16string open,std::u16string close,bool stretchy,
                                double max,bool space=false){
        PropertyArray arguments{Property(methods.braket),property_host::debugText(open),
            property_host::debugText(close),Property(stretchy),Property(max)};
        // The Source tuple omits the optional `space` argument unless a
        // mapping explicitly supplies it (only `Set` does so).
        if(space)arguments.push(Property(true));
        return Property(std::move(arguments));
    };
    commands.setOwn("bra",macroTuple(u"{\\langle {#1} \\vert}",1));
    commands.setOwn("ket",macroTuple(u"{\\vert {#1} \\rangle}",1));
    commands.setOwn("braket",bracketTuple(u"\u27E8",u"\u27E9",false,
        std::numeric_limits<double>::infinity()));
    commands.setOwn("set",bracketTuple(u"{",u"}",false,1));
    commands.setOwn("Bra",macroTuple(u"{\\left\\langle {#1} \\right\\vert}",1));
    commands.setOwn("Ket",macroTuple(u"{\\left\\vert {#1} \\right\\rangle}",1));
    commands.setOwn("Braket",bracketTuple(u"\u27E8",u"\u27E9",true,
        std::numeric_limits<double>::infinity()));
    commands.setOwn("Set",bracketTuple(u"{",u"}",true,1,true));
    commands.setOwn("ketbra",macroTuple(u"{\\vert {#1} \\rangle\\langle {#2} \\vert}",2));
    commands.setOwn("Ketbra",macroTuple(u"{\\left\\vert {#1} \\right\\rangle\\left\\langle {#2} \\right\\vert}",2));
    commands.setOwn("|",Property(methods.bar));
    (void)TokenMapHost::create<CommandMap>(runtime.mapHandler,Property("Braket-macros"),Property(commands));
    (void)TokenMapHost::create<MacroMap>(runtime.mapHandler,Property("Braket-characters"),
        Property(PropertyList{{"|",Property(methods.bar)}}));
}
}
