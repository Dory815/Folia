#include "support/folia_test.hpp"
import folia.math.core.math_item;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.mrow;
import folia.math.output.common;
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.util.option_types;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace opts=folia::math::options;
namespace core=folia::math::core;
namespace mml=folia::math::mml;
namespace svg=folia::math::output::svg;
namespace common=folia::math::output::common;

common::FontLoadHost fontHost(){
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.warn=[](const std::any&){};
    return host;
}

suite texCombineRelations=[] {
    "tex_postfilter_combines_relations_under_inferred_mrow"_test=[] {
        svg::NativeRenderSession session(fontHost());
        auto document=svg::createNativeSvgDocument(session);
        auto& context=session.context();
        const opts::Value documentIdentity(document->sourceIdentity());
        const auto documentOptions=context.get(documentIdentity,opts::Key(u"options"));
        const auto itemConstructor=context.get(documentOptions,opts::Key(u"MathItem"));
        const auto inputJax=context.get(context.get(documentIdentity,opts::Key(u"inputJax")),opts::Key(u"0"));
        const std::array<opts::Value,3> arguments{opts::Value(u"a><b"),inputJax,opts::Value(true)};
        const auto itemIdentity=context.construct(std::get<opts::ObjectRef>(itemConstructor.data),arguments);
        const auto* payload=std::any_cast<std::shared_ptr<NativeObject>>(&context.payload(itemIdentity));
        const auto item=payload?std::dynamic_pointer_cast<core::MathItem>(*payload):nullptr;
        expect(item!=nullptr) << "document constructor creates the canonical MathItem owner";
        if(!item)return;

        const auto compile=context.get(opts::Value(itemIdentity),opts::Key(u"compile"));
        const std::array<opts::Value,1> compileArguments{opts::Value(document->sourceIdentity())};
        context.call(std::get<opts::ObjectRef>(compile.data),opts::Value(itemIdentity),compileArguments);
        const mml::NodeValue root=item->root;
        const auto top=root.requireNode("childNodes")->childNodes;
        expect(top!=nullptr) << "compiled math root has children";
        if(!top||top->size()==0)return;
        const auto row=top->get(0);
        const auto children=row.requireNode("childNodes")->childNodes;
        expect(children!=nullptr) << "inferred row has children";
        if(!children)return;

        bool combined=false;
        for(std::size_t index=0;index<children->size();++index){
            const auto child=children->get(index);
            if(!child.isNode()||!child.requireNode()->isKind("mo"))continue;
            auto* token=dynamic_cast<mml::MmlMo*>(child.nodeOrNull());
            if(token&&token->getText()==u"><")combined=true;
        }
        expect(combined) << "adjacent relation tokens inside an inferred mrow merge per Source";
    };
};
} // namespace
