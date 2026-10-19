// Native port of MathJax 4.1.3 input/tex/physics/PhysicsConfiguration.ts.
module folia.math.input.tex.physics.configuration;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.physics.items;
import folia.math.input.tex.physics.mappings;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Value record(
    Context &cx,
    std::initializer_list<std::pair<std::u16string_view, Value>> values) {
  auto object = cx.object();
  for (const auto &[name, value] : values)
    cx.set(object, Key(std::u16string(name)), value);
  return Value(object);
}
Value array(Context &cx, std::initializer_list<Value> values) {
  auto object = cx.array(static_cast<std::uint32_t>(values.size()));
  std::size_t i = 0;
  for (const auto &value : values) {
    cx.set(object, Key(std::u16string(
                       1, static_cast<char16_t>(u'0' + i))),
           value);
    ++i;
  }
  return Value(object);
}
}
ConfigurationRef PhysicsConfiguration::initializeModule(ParserRuntime& runtime){
    const auto prior=runtime.configurationHandler.get(u"physics");if(const auto* value=std::get_if<ConfigurationRef>(&prior);value&&*value)return *value;
    auto& cx=runtime.context();const auto moduleValue=PhysicsItems::initializeModule(runtime);
    const auto* module=std::get_if<ObjectRef>(&moduleValue.data);
    if(!module)throw Exception("TypeError",u"PhysicsItems module export is not an object");
    const auto constructorValue=cx.get(Value(*module),Key(u"AutoOpen"));
    const auto* constructor=std::get_if<ObjectRef>(&constructorValue.data);
    if(!constructor)throw Exception("TypeError",u"AutoOpen constructor identity was not registered");
    PhysicsMappings::initializeModule(runtime);
    const auto handler=record(cx,{{u"macro",array(cx,{Value(u"Physics-automatic-bracing-macros"),Value(u"Physics-vector-macros"),Value(u"Physics-vector-mo"),Value(u"Physics-vector-mi"),Value(u"Physics-derivative-macros"),Value(u"Physics-expressions-macros"),Value(u"Physics-quick-quad-macros"),Value(u"Physics-bra-ket-macros"),Value(u"Physics-matrix-macros")})},
                              {u"character",array(cx,{Value(u"Physics-characters")})},
                              {u"environment",array(cx,{Value(u"Physics-aux-envs")})}});
    const auto items=record(cx,{{u"auto open",Value(*constructor)}});
    const auto physicsOptions=record(cx,{{u"italicdiff",Value(false)},{u"arrowdel",Value(false)}});
    const auto opts=record(cx,{{u"physics",physicsOptions}});
    const auto definition=record(cx,{{u"handler",handler},{u"items",items},{u"options",opts}});
    return Configuration::create(runtime,u"physics",definition);
}
}
