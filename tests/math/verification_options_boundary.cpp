#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_factory;
import folia.math.core.mml_tree.class_binding;
import folia.math.core.mml_tree.mml_node_host;
import folia.math.core.mml_tree.mml_nodes.math;
import folia.math.core.mml_tree.mml_nodes.html_node;
import folia.math.core.mml_tree.mml_nodes.mmultiscripts;
import folia.math.core.mml_tree.mml_nodes.maction;
import folia.math.core.mml_tree.mml_nodes.mglyph;
import folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.core.mml_tree.mml_nodes.mtr;
import folia.math.core.mml_tree.mml_nodes.mtd;

namespace {
using namespace boost::ut;
using namespace folia::math;
using namespace folia::math::mml;

static_assert(std::same_as<decltype(&MmlNode::verifyTree),void(MmlNode::*)(Property)>);
static_assert(std::same_as<decltype(&MmlNode::mError),MmlNode*(MmlNode::*)(std::string,Property,bool)>);

bool same(const Property& a,const Property& b) {
    if(auto x=std::get_if<double>(&a.value)) {
        if(auto y=std::get_if<double>(&b.value))
            return std::bit_cast<std::uint64_t>(*x)==std::bit_cast<std::uint64_t>(*y);
    }
    return a==b;
}

template<class T> struct Slots:T {
    using T::T;
    Property received;
    std::string kind()const override{
        if constexpr(std::same_as<T,AbstractMmlNode>)return "options-base";
        else return T::kind();
    }
    void verifyTree(Property value=Missing{})override{received=std::move(value);}
    MmlNode* mError(std::string,Property value,bool=false)override{received=std::move(value);return nullptr;}
    void verifyAttributes(Property value)override{received=std::move(value);}
    void verifyChildren(Property value)override{received=std::move(value);}
    void attributesCall(Property value){this->verifyAttributes(std::move(value));}
    void childrenCall(Property value){this->verifyChildren(std::move(value));}
};
template<class T> struct PublicSlots:T {
    using T::T;
    Property received;
    void verifyTree(Property value=Missing{})override{received=std::move(value);}
    MmlNode* mError(std::string,Property value,bool=false)override{received=std::move(value);return nullptr;}
};

suite verification_options_boundary=[] {
    "math_port_verification_options_raw_virtual_domain"_test=[] {
        MmlFactory factory;
        PropertyList object{{"fullErrors",false}};
        PropertyArray array{1,"value"};
        const std::vector<Property> values={Missing{},PropertyNull{},false,true,-0.0,7,
            std::numeric_limits<double>::quiet_NaN(),"",object,array};
        auto check=[&]<class T>(std::string key) {
            factory.setNodeClass(key,classBinding<Slots<T>>());
            Slots<T>* probe=nullptr;
            try{probe=dynamic_cast<Slots<T>*>(factory.create(key));}
            catch(const Error& error){std::cout<<key<<" "<<error.name<<" "<<host::debugText(error.message)<<'\n';throw;}
            MmlNode* node=probe;
            for(const auto& value:values) {
                node->verifyTree(value);expect(same(probe->received,value))<<key<<"tree";
                node->mError("message",value);expect(same(probe->received,value))<<key<<"error";
                probe->attributesCall(value);expect(same(probe->received,value))<<key<<"attributes";
                probe->childrenCall(value);expect(same(probe->received,value))<<key<<"children";
            }
            node->verifyTree();expect(probe->received.missing())<<key<<"base default";
            static_cast<T*>(probe)->verifyTree();expect(probe->received.missing())<<key<<"derived default";
            object.set("observed",false);node->mError("alias",object);object.set("observed",true);
            expect(host::get(probe->received,"observed")==Property(true))<<key<<"retained object";
        };
        check.template operator()<AbstractMmlNode>("options-base");
        check.template operator()<MmlMath>("options-math");
        check.template operator()<MmlMprescripts>("options-prescripts");
        check.template operator()<MmlNone>("options-none");
        check.template operator()<MmlMmultiscripts>("options-multiscripts");
        check.template operator()<MmlMaction>("options-action");
        check.template operator()<MmlMglyph>("options-glyph");
        check.template operator()<MmlMtable>("options-table");
        check.template operator()<MmlMtr>("options-row");
        check.template operator()<MmlMtd>("options-cell");
        auto publicCheck=[&]<class T>(std::string key) {
            factory.setNodeClass(key,classBinding<PublicSlots<T>>());
            auto* empty=dynamic_cast<PublicSlots<T>*>(factory.create(key));
            MmlNode* node=empty;
            for(const auto& value:values) {
                node->verifyTree(value);expect(same(empty->received,value))<<key;
                node->mError("message",value);expect(same(empty->received,value))<<key;
            }
            node->verifyTree();expect(empty->received.missing())<<key;
            static_cast<T*>(empty)->verifyTree();expect(empty->received.missing())<<key;
            object.set("observed",false);node->mError("alias",object);object.set("observed",true);
            expect(host::get(empty->received,"observed")==Property(true))<<key;
        };
        publicCheck.template operator()<TextNode>("options-empty");
        publicCheck.template operator()<HtmlNode>("options-html");
    };
};
}
