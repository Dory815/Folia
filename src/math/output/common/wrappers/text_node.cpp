module folia.math.output.common;
import :text_node;
import :output_jax;
import :mo;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
namespace {
std::u16string codePointText(std::uint32_t code) {
    std::u16string text;
    if(code<=0xFFFF) text.push_back(static_cast<char16_t>(code));
    else {
        code-=0x10000;
        text.push_back(static_cast<char16_t>(0xD800+(code>>10)));
        text.push_back(static_cast<char16_t>(0xDC00+(code&0x3FF)));
    }
    return text;
}
Property dataSlot(const Property& value,std::string_view key) {
    const auto record=PropertyList::object(value);
    if(!record)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'unknown')");
    return record->get(key);
}
double numberSlot(const Property& value) {
    return property_host::number(value);
}
}

std::shared_ptr<std::vector<std::uint32_t>> CommonTextNode::remappedText(
    std::u16string_view text,std::string_view name) const {
    if(!parent)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'stretch')");
    const double code=parent->stretch&&parent->stretch->c?*parent->stretch->c:0;
    if(code)return std::make_shared<std::vector<std::uint32_t>>(1,static_cast<std::uint32_t>(code));
    return parent->remapChars(unicodeChars(text,name));
}

std::u16string CommonTextNode::addUtextBBox(BBox& box,std::u16string_view text,
                                             std::string_view name) {
    if(!text.empty()) {
        const auto measured=jax().measureText(text,name);
        updateBBox(box,measured.h,measured.d,measured.w);
    }
    return {};
}

void CommonTextNode::updateBBox(BBox& box,double h,double d,double w) {
    box.w+=w;
    if(h>box.h)box.h=h;
    if(d>box.d)box.d=d;
}

void CommonTextNode::computeBBox(BBox& box,bool recompute) {
    (void)recompute;
    if(!parent)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'variant')");
    const std::string variantName=parent->variant;
    auto* textNode=dynamic_cast<mml::TextNode*>(node);
    if(!textNode)throw options::Exception("TypeError",u"TextNode wrapper has no MML TextNode");
    const auto text=textNode->getText();
    if(variantName=="-explicitFont") {
        if(!parent->styles)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'styles')");
        const auto fontData=jax().getFontData(*parent->styles);
        const auto measured=jax().measureText(text,variantName,fontData);
        box.h=measured.h;box.d=measured.d;box.w=measured.w;
        return;
    }

    const auto chars=remappedText(text,variantName);
    if(!chars)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'length')");
    std::u16string unknownText;
    box.empty();
    for(std::size_t index=0;index<chars->size();++index) {
        const auto tuple=getVariantChar(variantName,(*chars)[index]);
        const auto h=numberSlot(tuple.get(0));
        const auto d=numberSlot(tuple.get(1));
        const auto w=numberSlot(tuple.get(2));
        const auto data=tuple.get(3);
        if(property_host::truthy(dataSlot(data,"unknown"))) {
            unknownText+=codePointText((*chars)[index]);
            continue;
        }

        unknownText=addUtextBBox(box,unknownText,variantName);
        updateBBox(box,h,d,w);
        const auto ic=dataSlot(data,"ic"),sk=dataSlot(data,"sk"),dx=dataSlot(data,"dx");
        box.ic=property_host::truthy(ic)?numberSlot(ic):0;
        box.sk=property_host::truthy(sk)?numberSlot(sk):0;
        box.dx=property_host::truthy(dx)?numberSlot(dx):0;
        const auto oc=dataSlot(data,"oc");
        if(!property_host::truthy(oc)||index<chars->size()-1)continue;
        if(!parent->childNodes||!(*parent->childNodes))
            throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'length')");
        const auto parentChildren=*parent->childNodes;
        if(parentChildren->empty()||parentChildren->get(parentChildren->size()-1).requireNode("node")->node!=node)continue;

        if(!parent->parent||!parent->parent->node)
            throw options::Exception("TypeError",u"Cannot read properties of null (reading 'node')");
        auto* mmlParent=parent->parent->node;
        std::optional<mml::NodeValue> next;
        if(mmlParent->isKind("mrow")||mmlParent->isInferred()) {
            const auto childIndex=mmlParent->childIndex(mml::NodeValue(parent->node));
            if(!childIndex)throw options::Exception("TypeError",u"MML parent does not contain wrapper parent");
            const auto children=mmlParent->childArrayOwner();
            if(children&&*childIndex+1<children->size())
                next=children->get(*childIndex+1);
        }
        if(next&&next->isNode()) {
            auto* nextNode=next->nodeOrNull();
            if(auto* mo=dynamic_cast<mml::MmlMo*>(nextNode);mo&&mo->getText()==u"\u2062") {
                const auto moIndex=mmlParent->childIndex(*next);
                if(moIndex) {
                    const auto children=mmlParent->childArrayOwner();
                    next.reset();
                    if(children&&*moIndex+1<children->size())
                        next=children->get(*moIndex+1);
                }
            }
        }
        bool sameVariant=false;
        if(next&&next->isNode())sameVariant=next->nodeOrNull()->attributes()->get("mathvariant")==Property(std::string(variantName));
        if(!next||!next->isNode()||!sameVariant)box.ic=numberSlot(oc);
        else box.oc=numberSlot(oc);
    }
    unknownText=addUtextBBox(box,unknownText,variantName);
    if(chars->size()>1)box.sk=0;
    box.clean();
}
}
