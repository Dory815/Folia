module folia.math.output.scene;
import :xml_node;
import folia.math.output.scene.foundation;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
XmlNodePlacementData captureXmlNode(common::CommonXmlNode& wrapper){
    const auto metrics=wrapper.metrics();
    const double em=metrics.em*metrics.scale*wrapper.rscale;
    const double scale=1/em;
    // SvgXmlNode.toSVG performs these live formatter calls in this order even
    // though the completed scene keeps the full-precision numeric operands.
    // Preserve Source observable calls without storing rounded SVG strings as
    // a second geometry authority.
    const auto scaleText=wrapper.jax().sourceFixed(scale*1000,3);
    const auto bbox=wrapper.getBBox();
    const auto yText=wrapper.jax().sourceFixed(-bbox->h*em);
    const auto widthText=wrapper.jax().sourceFixed(bbox->w*em);
    const auto heightText=wrapper.jax().sourceFixed((bbox->h+bbox->d)*em);
    XmlNodePlacementData result;
    result.geometry={.x=0,.y=-bbox->h*em,.width=bbox->w*em,
        .height=(bbox->h+bbox->d)*em};
    result.geometry.transform.operations={
        {.kind=TransformKind::Scale,.x=scale,.y=scale,.scalarScale=true},
        {.kind=TransformKind::Matrix,.a=1,.d=-1}};
    std::string transform="scale("+scaleText+") matrix(1 0 0 -1 0 0)";
    result.attributes.setOwn("data-mjx-xml",Property(true));
    result.attributes.setOwn("y",Property(yText+"px"));
    result.attributes.setOwn("width",Property(widthText+"px"));
    result.attributes.setOwn("height",Property(heightText+"px"));
    result.attributes.setOwn("transform",Property(std::move(transform)));
    result.html=wrapper.getHTML();
    return result;
}

std::vector<NodeRef> XmlNodePlacement::build(common::CommonWrapper& base,
    Builder& builder,WrapperFactory& factory,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonXmlNode*>(&base);
    if(!wrapper)throw options::Exception("TypeError",
        u"XML SVG placement received a non-XML wrapper");
    if(parents.empty())throw options::Exception("TypeError",
        u"Cannot read properties of undefined (reading 'append')");

    // captureXmlNode follows the actual SvgXmlNode argument evaluation order:
    // metrics/fixed scale, getBBox/fixed dimensions, then getHTML. The scene
    // retains the resulting values and immutable HTML snapshot only.
    auto payload=captureXmlNode(*wrapper);
    // This is only a shell for the XML creator. The creator invokes the real
    // Source svg helper, whose dispatch observer records the paint operation.
    // Do not count this temporary shell as a second SVG paint creation.
    auto foreign=builder.adoptExisting(u"foreignObject",payload.attributes);
    foreign=factory.createXmlSvgNode(builder,std::move(foreign),payload.attributes,payload.html);
    foreign->geometry=std::move(payload.geometry);
    builder.append(parents.front(),foreign);
    return {std::move(foreign)};
}

void registerXmlNodePlacement(WrapperFactory& factory){
    auto placement=std::make_shared<XmlNodePlacement>();
    factory.registerKind("XML",placement);
    factory.registerKind("html",std::move(placement));
}
}
