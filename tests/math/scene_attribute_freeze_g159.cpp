#include "support/folia_test.hpp"
import folia.math.output.scene;

namespace {
using namespace boost::ut;
namespace scene=folia::math::output::scene;

struct CountingAttributes final:scene::AttributeOwner {
    mutable std::size_t keyReads=0;
    folia::math::Property get(std::string_view)const override{return folia::math::Missing{};}
    folia::math::Property getOwn(std::string_view)const override{return folia::math::Missing{};}
    bool hasOwn(std::string_view)const override{return false;}
    void set(std::string_view,folia::math::Property)override{}
    void setOwn(std::string_view,folia::math::Property)override{}
    void erase(std::string_view)override{}
    std::vector<std::string> keys()const override{++keyReads;return {"x","y"};}
};
struct CapturedGeometryAttributes final:scene::GeometryOwner {
    mutable std::size_t calls=0;
    mutable std::vector<std::pair<std::string,std::string>> received;
    std::shared_ptr<const scene::GeometryOwner> cloneFor(const scene::NodeRef&)const override{
        return std::make_shared<CapturedGeometryAttributes>();
    }
    void apply(scene::Geometry&,std::optional<scene::Transform>&,
        std::span<const std::pair<std::string,std::string>> attributes)const override{
        ++calls;received.assign(attributes.begin(),attributes.end());
    }
};
}

suite scene_attribute_freeze_g159=[] {
    "freeze enumerates source attributes once and shares that capture with geometry"_test=[] {
        auto attributes=std::make_shared<CountingAttributes>();
        auto geometry=std::make_shared<CapturedGeometryAttributes>();
        auto node=std::make_shared<scene::Node>();
        node->kind=u"rect";
        node->attributes.replaceOwner(attributes);
        node->geometryOwner=geometry;
        scene::Builder builder;
        builder.addRoot(node);
        const auto frozen=std::move(builder).finish();
        expect(attributes->keyReads==1_u);
        expect(geometry->calls==1_u);
        expect(geometry->received.size()==2_u);
        expect(frozen.roots().size()==1_u);
        if(frozen.roots().size()==1_u){
            expect(frozen.roots().front()->attributes.size()==2_u);
            expect(frozen.roots().front()->attributes[0].first=="x");
            expect(frozen.roots().front()->attributes[1].first=="y");
            expect(geometry->received==frozen.roots().front()->attributes);
        }
    };
};