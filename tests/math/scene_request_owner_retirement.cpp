#include "support/folia_test.hpp"
import folia.math.adaptors.lite.lite_module;
import folia.math.output.scene;
import folia.math.output.svg.scene_context_nodes;
import folia.math.util.context_property_bindings;
import folia.math.util.regexp_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace svg=folia::math::output::svg;
namespace scene=folia::math::output::scene;
namespace lite=folia::math::adaptors::lite;
namespace opts=folia::math::options;
}

suite scene_request_owner_retirement=[] {
    "scene request retirement drops mutable source roots after freeze"_test=[] {
        opts::Context context;
        OptionFunctionBindings functions(context);
        ContextPropertyBindings properties(context,functions);
        auto regexp=std::make_shared<folia::math::regexp_host::RegExpBindings>(context);
        lite::LiteModule liteModule(context,functions,properties,regexp);
        svg::SceneContextNodes projection(context,u"RequestRetirementSceneNode");
        projection.bindPropertyBindings(properties);
        projection.bindLiteModule(liteModule);
        auto provider=std::make_shared<int>(1);
        projection.bindServiceOwner(provider);

        const auto sceneMark=projection.beginRequestScope();
        const auto liteMark=liteModule.requestMark();
        const auto liteNode=liteModule.store()->create(lite::NodeKind::Element,u"g");
        const auto liteIdentity=liteModule.objectFor(liteNode);
        auto mutableNode=std::make_shared<scene::Node>();
        mutableNode->kind=u"g";
        mutableNode->semantic=scene::SourceDomPrimitive{};
        std::weak_ptr<scene::Node> mutableLifetime=mutableNode;
        projection.associateSourceNode(mutableNode,opts::Value(liteIdentity));
        const auto sourceHandle=projection.handle(mutableNode);

        auto generatedNode=std::make_shared<scene::Node>();
        generatedNode->kind=u"path";
        std::weak_ptr<scene::Node> generatedLifetime=generatedNode;
        const auto generatedHandle=projection.handle(generatedNode);
        scene::Scene frozen;
        {
            scene::Builder builder;
            builder.addRoot(mutableNode);
            builder.addRoot(generatedNode);
            frozen=std::move(builder).finish();
        }

        const auto populated=projection.requestOwnerCountsForDiagnostics();
        expect(populated.handles==2_ul);
        expect(populated.sourceIdentities==1_ul);
        expect(populated.sourceViews==1_ul);
        projection.retireRequestScope(sceneMark);
        liteModule.retireRequestNodesSince(liteMark);
        const auto retired=projection.requestOwnerCountsForDiagnostics();
        expect(retired.handles==0_ul);
        expect(retired.sourceIdentities==0_ul);
        expect(retired.sourceViews==0_ul);
        expect(!liteModule.store()->owns(liteNode));
        mutableNode.reset();generatedNode.reset();
        expect(mutableLifetime.expired());
        expect(generatedLifetime.expired());
        expect(frozen.roots().size()==2_ul);
        if(frozen.roots().size()==2_ul){
            expect(frozen.roots()[0]->kind==u"g");
            expect(frozen.roots()[1]->kind==u"path");
        }
        const auto staleHandle=[&](opts::ObjectRef object){
            try{(void)context.kind(object);return false;}catch(const opts::Exception&){return true;}
        };
        expect(staleHandle(sourceHandle));
        expect(staleHandle(generatedHandle));
        expect(staleHandle(liteIdentity));
    };
};
