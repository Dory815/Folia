// Root-owned string primitive binding for the complete maction source methods.
export module folia.math.core.mml_tree.action_host;
export import folia.math.core.mml_tree.arithmetic_host;
import folia.math.util.js_case;
import std;
export namespace folia::math::mml::host {
inline std::u16string lowerString(const Property& value){
    if(auto s=std::get_if<std::string>(&value.value))return js_case::lowerCase(text(*s));
    typeError("toLowerCase is not callable on this receiver");
}
}
