// Root-owned source static records, copied at family initialization.
export module folia.math.core.mml_tree.script_defaults;
export import folia.math.core.mml_tree.mml_node;
import std;
namespace folia::math::mml {namespace {
struct State {
    PropertyList msubsup,msub,msup,munderover,munder,mover;
    std::shared_ptr<std::vector<Property>> underAccents=std::make_shared<std::vector<Property>>(std::initializer_list<Property>{"","accentunder","accent"});
    std::shared_ptr<std::vector<Property>> overAccents=std::make_shared<std::vector<Property>>(std::initializer_list<Property>{"","accent","accentunder"});
    State(){
        msubsup.assign(AbstractMmlBaseNode::defaults());
        msubsup.set("subscriptshift","");msubsup.set("superscriptshift","");
        msub.assign(msubsup);msup.assign(msubsup);
        munderover.assign(AbstractMmlBaseNode::defaults());
        munderover.set("accent",false);munderover.set("accentunder",false);munderover.set("align","center");
        munder.assign(munderover);mover.assign(munderover);
    }
};
State& state(){static State value;return value;}
}}
export namespace folia::math::mml {
PropertyList& msubsupDefaults(){return state().msubsup;}
PropertyList& msubDefaults(){return state().msub;}
PropertyList& msupDefaults(){return state().msup;}
PropertyList& munderoverDefaults(){return state().munderover;}
PropertyList& munderDefaults(){return state().munder;}
PropertyList& moverDefaults(){return state().mover;}
std::shared_ptr<std::vector<Property>>& underAccents(){return state().underAccents;}
std::shared_ptr<std::vector<Property>>& overAccents(){return state().overAccents;}
}
