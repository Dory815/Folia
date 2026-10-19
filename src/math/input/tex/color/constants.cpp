// Native Map authority for MathJax 4.1.3 input/tex/color/ColorConstants.ts.
module folia.math.input.tex.color.constants;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
options::ObjectRef ColorConstants::createMap(const std::shared_ptr<options::MapContextBindings>& maps){
    if(!maps)throw options::Exception("TypeError",u"Color constants require a Map owner");
    const auto colors=maps->create();
    for(const auto&[name,color]:ColorConstantValues)(void)maps->set(colors,options::Value(property_host::text(name)),options::Value(property_host::text(color)));
    return colors;
}
}
