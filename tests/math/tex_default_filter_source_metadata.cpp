#include "support/folia_test.hpp"
import folia.math.output.common;
import folia.math.output.svg.render_session;
import folia.math.input.tex.parser;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o=folia::math::options;
namespace t=folia::math::tex;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

common::FontLoadHost fontHost(){
  common::FontLoadHost host;
  host.asyncIsSynchronous=[] {return true;};
  host.warn=[](const std::any&) {};
  return host;
}

suite tex_default_filter_source_metadata=[] {
  "tex_default_post_filters_keep_source_names_and_arity"_test=[] {
    svg::NativeRenderSession session(fontHost());
    auto& runtime=session.services().parser();
    auto& context=session.context();
    const auto jax=t::TeX::make(runtime);
    const auto identity=runtime.functionLists.bind(jax->postFilters);
    const auto items=context.get(o::Value(identity),o::Key(u"items"));
    const std::array<std::pair<std::u16string_view,double>,8> expected{{
      {u"cleanSubSup",-7},{u"setInherited",-6},{u"checkScriptlevel",-5},
      {u"filterNonscript",-4},{u"moveLimits",-4},{u"cleanStretchy",-3},
      {u"cleanAttributes",-2},{u"combineRelations",-1}}};
    expect(context.toNumber(context.get(items,o::Key(u"length")))==static_cast<double>(expected.size()));
    for(std::size_t index=0;index<expected.size();++index){
      const auto decimal=std::to_string(index);
      const std::u16string key(decimal.begin(),decimal.end());
      const auto record=context.get(items,o::Key(key));
      const auto callable=context.get(record,o::Key(u"item"));
      const auto* function=std::get_if<o::ObjectRef>(&callable.data);
      expect(function!=nullptr);
      if(!function)continue;
      expect(std::get<std::u16string>(context.get(*function,o::Key(u"name")).data)==expected[index].first);
      expect(context.toNumber(context.get(*function,o::Key(u"length")))==1.0);
      expect(context.toNumber(context.get(record,o::Key(u"priority")))==expected[index].second);
    }
  };
};
} // namespace
