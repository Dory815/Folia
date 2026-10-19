module folia.math.output.svg.native_document;
import folia.math.input.tex.parser;
import folia.math.core.mml_tree.mml_node;
import folia.math.core.dom_node;
import folia.math.util.context_property_bindings;
import folia.math.util.option_types;
import std;

namespace folia::math::output::svg {
namespace {
using namespace options;

struct PublicDocumentLease {
    std::shared_ptr<Context> context;
    std::shared_ptr<SvgRenderServiceGraph> services;
    std::shared_ptr<handlers::html::HTMLMathDocument> document;
};

Value sourceMmlValue(tex::ParserRuntime& parser,const tree::ChildValue<mml::MmlNode>& node){
    if(node.kind()==tree::ChildKind::Undefined)return Value(Undefined{});
    if(node.kind()==tree::ChildKind::Null)return Value(Null{});
    auto* actual=node.requireNode("MathItem.root");
    const auto identity=parser.mmlNodeIdentities.find(actual);
    if(!identity)
        throw Exception("TypeError",u"MathItem root has no canonical MML Context identity");
    return Value(*identity);
}
}

std::shared_ptr<handlers::html::HTMLMathDocument> createNativeSvgDocument(
    NativeRenderSession& session,Value options,Value inputJaxOptions){
    auto services=session.services().shared_from_this();
    auto jax=session.outputJax();
    if(!jax)jax=session.construct();
    jax=services->svgJax().resolve(jax->instanceIdentity());
    const auto adaptorObject=session.adaptorObject();
    auto adaptor=services->lite().resolveAdaptor(adaptorObject);
    // The parser/TeX objects are owned by this graph and retain their MML
    // factory. Give them the graph's actual factory owner, not an aliasing
    // handle that would create Graph -> Parser -> TeX -> Factory(alias Graph).
    auto factory=services->mmlFactoryOwner();
    auto input=tex::TeX::make(services->parser(),std::move(inputJaxOptions));
    if(!adaptor||!factory||!input)
        throw Exception("TypeError",u"Native SVG session did not create its actual providers");
    const auto documentNode=adaptor->createDocument();
    std::vector<core::InputJaxBinding> inputs;
    inputs.push_back({input->instanceIdentity(),input});
    auto actual=handlers::html::HTMLMathDocument::create(
        session.context(),services->parser().properties,services->modules(),
        services->parser().regexp(),
        std::move(inputs),std::static_pointer_cast<core::OutputJax>(jax),
        jax->instanceIdentity(),std::static_pointer_cast<AbstractDOMAdaptor>(adaptor),
        adaptorObject,
        std::move(factory),documentNode,
        [services](const tree::ChildValue<mml::MmlNode>& value){return sourceMmlValue(services->parser(),value);},
        [services](ObjectRef identity)->mml::MmlNode&{return services->parser().resolveMmlNode(identity);},
        std::move(options),services);

    auto lease=std::make_shared<PublicDocumentLease>();
    lease->context=session.contextOwner();
    lease->services=std::move(services);
    lease->document=std::move(actual);
    auto* documentOwner=lease->document.get();
    return std::shared_ptr<handlers::html::HTMLMathDocument>(std::move(lease),documentOwner);
}
}
