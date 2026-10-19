#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.core.mml_tree.mml_nodes.mrow;

namespace {
using namespace boost::ut;
namespace o=folia::math::options;
namespace m=folia::math::mml;
namespace t=folia::math::tex;
namespace tree=folia::math::tree;
suite mmlSourceChildOwner = [] {
  "mml_child_nodes_field_and_native_owner_share_the_raw_context_array"_test = [] {
    o::Context context;
    folia::math::OptionFunctionBindings functions(context);
    folia::math::regexp_host::RegExpBindings regexp(context);
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input(context);
    folia::math::core::FindMathModule findMath(context);
    t::FindTeXModule findTeX(context,findMath,regexp);
    t::ParserRuntime runtime(context,functions,regexp,unicode,input,findTeX);
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=runtime.bindMmlFactory(factory);
    auto* row=factory->create("mrow");
    const auto nodeIdentity=runtime.mmlNodeIdentities.find(row);
    expect(nodeIdentity.has_value());
    if(!nodeIdentity)return;

    const auto sourceArray=context.array(1);
    const auto foreignChild=context.object();
    context.set(o::Value(sourceArray),o::Key(u"0"),o::Value(foreignChild));
    context.set(o::Value(*nodeIdentity),o::Key(u"childNodes"),o::Value(sourceArray));

    const auto owner=row->childArrayOwner();
    expect(owner!=nullptr);
    expect(owner&&owner->sourceValue()==std::optional<o::Value>{o::Value(sourceArray)});
    expect(context.get(o::Value(*nodeIdentity),o::Key(u"childNodes"))==o::Value(sourceArray));
    expect(owner&&owner->get(0).sourceValue()==o::Value(foreignChild));

    const auto imported=runtime.treeNodeBindings.childValue(o::Value(foreignChild));
    expect(row->appendChild(imported)==imported);
    expect(context.get(o::Value(*nodeIdentity),o::Key(u"childNodes"))==o::Value(sourceArray));
    expect(context.get(o::Value(sourceArray),o::Key(u"length"))==o::Value(2));
    expect(context.get(o::Value(foreignChild),o::Key(u"parent"))==o::Value(*nodeIdentity));
    expect(owner&&owner->indexOf(imported)==std::optional<std::size_t>{0});

    auto* copySource=factory->create("mrow");
    const auto copySourceIdentity=runtime.mmlNodeIdentities.find(copySource);
    expect(copySourceIdentity.has_value());
    const auto properties=context.object();
    const auto marker=context.symbol(u"copy-key");
    context.set(o::Value(properties),o::Key(marker),o::Value(u"symbol-value"));
    context.set(o::Value(*copySourceIdentity),o::Key(u"properties"),o::Value(properties));
    auto* copied=copySource->copy();
    const auto copiedIdentity=runtime.mmlNodeIdentities.find(copied);
    expect(copiedIdentity.has_value());
    if(copiedIdentity){
      const auto copiedProperties=context.get(o::Value(*copiedIdentity),o::Key(u"properties"));
      expect(context.get(copiedProperties,o::Key(marker))==o::Value(u"symbol-value"));
    }
    (void)factoryIdentity;
  };

  "source_mml_table_copy_and_setChildren_retain_top_row_for_phantom_bbox"_test = [] {
    o::Context context;
    folia::math::OptionFunctionBindings functions(context);
    folia::math::regexp_host::RegExpBindings regexp(context);
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input(context);
    folia::math::core::FindMathModule findMath(context);
    t::FindTeXModule findTeX(context,findMath,regexp);
    t::ParserRuntime runtime(context,functions,regexp,unicode,input,findTeX);
    auto factory=std::make_shared<m::MmlFactory>();
    (void)runtime.bindMmlFactory(factory);

    auto* table=factory->create("mtable",tree::FactoryArgs{});
    auto* row=factory->create("mtr",tree::FactoryArgs{});
    auto* cell=factory->create("mtd",tree::FactoryArgs{});
    expect(table!=nullptr&&row!=nullptr&&cell!=nullptr);
    if(!table||!row||!cell)return;
    row->appendChild(m::NodeValue(cell));
    table->appendChild(m::NodeValue(row));
    expect(table->childArrayOwner()->size()==1u);
    expect(row->childArrayOwner()->size()==1u);

    // Empheq's topRowTable() sequence is copy(), then setChildren(slice(0,1)),
    // then add align="baseline 1". Keep the two checkpoints separate so a
    // failure identifies which Source operation loses the copied row.
    auto* copied=table->copy();
    auto copiedChildren=copied->childArrayOwner();
    expect(copiedChildren&&copiedChildren->size()==1u)
        << "AbstractMmlNode.copy must retain the copied mtr before top-row slicing";
    if(!copiedChildren||copiedChildren->size()!=1u)return;
    const auto copiedRowValue=copiedChildren->get(0);
    auto* copiedRow=copiedRowValue.nodeOrNull();
    expect(copiedRow!=nullptr);
    if(!copiedRow)return;
    expect(copiedRow->childArrayOwner()&&copiedRow->childArrayOwner()->size()==1u)
        << "the copied mtr must retain its mtd child";

    copied->setChildren(copiedChildren->slice(0,1));
    const auto topRowChildren=copied->childArrayOwner();
    expect(topRowChildren&&topRowChildren->size()==1u)
        << "setChildren(childNodes.slice(0,1)) must keep the source top row";
    if(topRowChildren&&topRowChildren->size()==1u){
      auto* selected=topRowChildren->get(0).nodeOrNull();
      expect(selected==copiedRow)
          << "top-row slicing must preserve the row selected from the copied table";
      expect(selected&&selected->childArrayOwner()&&selected->childArrayOwner()->size()==1u)
          << "the selected row must retain its cell for mtable BBox measurement";
    }

    const auto sourceChildren=copied->childArrayOwner();
    const auto rawChildren=sourceChildren->sourceValue();
    expect(rawChildren.has_value());
    if(!rawChildren)return;
    const auto sliceResult=context.array(0);
    bool sliced=false;
    const auto customSlice=context.function(u"slice",
      [&](o::Context&,o::Value receiver,std::span<const o::Value> arguments){
        expect(receiver==*rawChildren);
        expect(arguments.size()==2u);
        expect(arguments[0]==o::Value(0)&&arguments[1]==o::Value(1));
        sliced=true;return o::Value(sliceResult);
      });
    context.set(*rawChildren,o::Key(u"slice"),o::Value(customSlice));
    const auto customResult=sourceChildren->slice(0,1);
    expect(sliced&&customResult->sourceValue()==std::optional<o::Value>(o::Value(sliceResult)));

    bool numericSlice=false;
    const auto changedSlice=context.function(u"slice",
      [&](o::Context&,o::Value receiver,std::span<const o::Value> arguments){
        expect(receiver==*rawChildren);
        expect(arguments.size()==1u&&std::isnan(std::get<double>(arguments[0].data)));
        numericSlice=true;return o::Value(sliceResult);
      });
    context.set(*rawChildren,o::Key(u"slice"),o::Value(changedSlice));
    const auto numericResult=sourceChildren->sliceNumber(std::numeric_limits<double>::quiet_NaN());
    expect(numericSlice&&numericResult->sourceValue()==std::optional<o::Value>(o::Value(sliceResult)));
  };

  "source_owned_mml_base_node_slice_visits_hidden_script_texclass_tree"_test = [] {
    o::Context context;
    folia::math::OptionFunctionBindings functions(context);
    folia::math::regexp_host::RegExpBindings regexp(context);
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input(context);
    folia::math::core::FindMathModule findMath(context);
    t::FindTeXModule findTeX(context,findMath,regexp);
    t::ParserRuntime runtime(context,functions,regexp,unicode,input,findTeX);
    auto factory=std::make_shared<m::MmlFactory>();
    (void)runtime.bindMmlFactory(factory);

    auto* x=factory->create("mi");
    auto* plus=factory->create("mo");
    auto* y=factory->create("mi");
    auto* copy=factory->create("inferredMrow");
    auto* phantom=factory->create("mphantom");
    auto* phantomRow=factory->create("inferredMrow");
    auto* padded=factory->create("mpadded");
    auto* visibleX=factory->create("mi");
    auto* visiblePlus=factory->create("mo");
    auto* visibleY=factory->create("mi");
    auto* visible=factory->create("mrow");
    auto* under=factory->create("munder");
    expect(x&&plus&&y&&copy&&phantom&&phantomRow&&padded&&visibleX&&visiblePlus&&visibleY&&visible&&under);
    if(!x||!plus||!y||!copy||!phantom||!phantomRow||!padded||!visibleX||!visiblePlus||!visibleY||!visible||!under)return;

    const auto append=[](m::MmlNode* parent,m::MmlNode* child){parent->appendChild(m::NodeValue(child));};
    append(copy,x);append(copy,plus);append(copy,y);
    append(phantom,copy);append(phantomRow,phantom);append(padded,phantomRow);
    append(visible,visibleX);append(visible,visiblePlus);append(visible,visibleY);
    append(under,visible);append(under,padded);

    // Rebind each array in this hand-built graph to the actual Context owner.
    // This makes AbstractMmlBaseNode's SourceChildArrayView::slice(1) observable.
    const auto bindChildren=[&](m::MmlNode* parent){
      const auto identity=runtime.mmlNodeIdentities.find(parent);
      expect(identity.has_value());
      if(!identity)return;
      const auto children=parent->childArrayOwner();
      expect(children!=nullptr);
      if(!children)return;
      const auto sourceArray=context.array(children->size());
      for(std::size_t i=0;i<children->size();++i){
        auto* child=children->get(i).nodeOrNull();
        expect(child!=nullptr);
        if(!child)continue;
        const auto childIdentity=runtime.mmlNodeIdentities.find(child);
        expect(childIdentity.has_value());
        if(childIdentity)context.set(o::Value(sourceArray),o::Key(folia::math::property_host::text(std::to_string(i))),o::Value(*childIdentity));
      }
      context.set(o::Value(*identity),o::Key(u"childNodes"),o::Value(sourceArray));
    };
    for(auto* node:{x,plus,y,copy,phantom,phantomRow,padded,visibleX,visiblePlus,visibleY,visible,under})bindChildren(node);

    x->setTexClass(static_cast<double>(m::TEXCLASS.ORD));
    plus->setTexClass(static_cast<double>(m::TEXCLASS.BIN));
    y->setTexClass(static_cast<double>(m::TEXCLASS.ORD));
    under->setTeXclass(m::NodeValue(folia::math::PropertyNull{}));

    expect(plus->prevClass()==folia::math::Property(static_cast<double>(m::TEXCLASS.ORD)))
        << "the copied operator must receive the preceding ORD class through the source-backed script slice";
    expect(y->prevClass()==folia::math::Property(static_cast<double>(m::TEXCLASS.BIN)))
        << "the following copied identifier must receive BIN as its previous class";
    expect(plus->texSpacing()==folia::math::Property("mediummathspace"));
    expect(y->texSpacing()==folia::math::Property("mediummathspace"));
  };

  "mml_constructor_defers_arraylike_iteration_until_after_the_length_branch"_test = [] {
    o::Context context;
    folia::math::OptionFunctionBindings functions(context);
    folia::math::regexp_host::RegExpBindings regexp(context);
    folia::math::Unicode16Normalization unicode;
    folia::math::core::InputJaxModule input(context);
    folia::math::core::FindMathModule findMath(context);
    t::FindTeXModule findTeX(context,findMath,regexp);
    t::ParserRuntime runtime(context,functions,regexp,unicode,input,findTeX);
    auto factory=std::make_shared<m::MmlFactory>();
    const auto factoryIdentity=runtime.bindMmlFactory(factory);
    const auto map=std::get<o::ObjectRef>(context.get(factoryIdentity,o::Key(u"nodeMap")).data);
    const auto constructor=std::get<o::ObjectRef>(runtime.maps.resolve(map)->get(o::Value(u"mrow")).data);

    std::vector<std::string> events;
    const auto inputChildren=context.object();
    const auto lengthGetter=context.function(u"get",[&](o::Context&,o::Value receiver,
        std::span<const o::Value>){expect(receiver==o::Value(inputChildren));events.push_back("length");return o::Value(1);});
    context.define(inputChildren,o::Key(u"length"),o::Descriptor::access(lengthGetter,std::nullopt,true,true));
    const auto child=context.object();
    const auto iterator=context.object();
    int nextCount=0;
    const auto next=context.function(u"next",[&](o::Context& cx,o::Value receiver,
        std::span<const o::Value>){
          expect(receiver==o::Value(iterator));events.push_back("next");
          const auto result=cx.object();
          if(nextCount++==0){cx.set(o::Value(result),o::Key(u"value"),o::Value(child));cx.set(o::Value(result),o::Key(u"done"),o::Value(false));}
          else{cx.set(o::Value(result),o::Key(u"done"),o::Value(true));}
          return o::Value(result);
        });
    context.set(o::Value(iterator),o::Key(u"next"),o::Value(next));
    const auto iteratorMethod=context.function(u"[Symbol.iterator]",[&](o::Context&,o::Value receiver,
        std::span<const o::Value>){expect(receiver==o::Value(inputChildren));events.push_back("iterator");return o::Value(iterator);});
    context.set(o::Value(inputChildren),o::Key(context.intrinsics().symbolIterator),o::Value(iteratorMethod));
    const std::array<o::Value,3> args{o::Value(factoryIdentity),o::Value(context.object()),o::Value(inputChildren)};
    const auto created=context.construct(constructor,args);
    expect(events==std::vector<std::string>{"iterator","next","next"})
        << "MML construction iterates children after the AbstractNode base constructor";
    expect(context.get(o::Value(child),o::Key(u"parent"))==o::Value(created))
        << "MML appendChild must assign the constructed node as the child's parent";
  };
};
}

