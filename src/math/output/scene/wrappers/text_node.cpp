/*************************************************************
 * MathJax 4.1.3 output/svg/Wrappers/TextNode.ts placement.
 *************************************************************/
module folia.math.output.scene;
import :text_node;
import folia.math.core.mml_tree.mml_node;
import folia.math.util.property_host;
import std;

namespace folia::math::output::scene {
std::vector<NodeRef> TextNodePlacement::build(common::CommonWrapper& base,Builder& builder,
    WrapperFactory& placements,std::span<const NodeRef> parents){
    auto* wrapper=dynamic_cast<common::CommonTextNode*>(&base);
    if(!wrapper)throw options::Exception("TypeError",u"TextNode placement received a non-TextNode wrapper");
    auto* node=dynamic_cast<mml::TextNode*>(wrapper->node);
    if(!node)throw options::Exception("TypeError",u"TextNode wrapper has an incompatible MathML node");
    if(!wrapper->parent)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'variant')");
    if(parents.empty())throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'append')");
    const auto text=node->getText();
    if(text.empty())return {};
    const auto variant=wrapper->parent->variant;
    if(variant=="-explicitFont"){
        const auto node=emitTextRun(*wrapper,builder,parents.front(),text,variant,0,0,false);
        return {node};
    }
    NodeRef target=parents.front();
    const bool grouped=wrapper->parent->childNodes&&*wrapper->parent->childNodes&&
        (*wrapper->parent->childNodes)->size()>1;
    if(grouped){
        target=builder.node(u"g",PropertyList{{"data-mml-node",Property("text")}});
        builder.append(parents.front(),target);
    }
    const auto chars=wrapper->remappedText(text,variant);
    if(!chars)throw options::Exception("TypeError",u"TextNode.remappedText returned no character sequence");
    placeGlyphText(*wrapper,builder,placements,target,*chars,variant,true);
    if(grouped)return {target};
    return std::vector<NodeRef>(parents.begin(),parents.end());
}

void registerTextNodePlacement(WrapperFactory& factory){
    factory.registerKind("text",std::make_shared<TextNodePlacement>());
}
}
