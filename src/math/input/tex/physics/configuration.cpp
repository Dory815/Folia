// MathJax 4.1.3 input/tex/physics/PhysicsConfiguration.ts; Apache-2.0.
module folia.math.input.tex.physics.configuration;
import folia.math.input.tex.physics.mappings;
import std;
namespace folia::math::tex {
ConfigurationRef PhysicsConfiguration::initializeModule(ConfigurationHandler &registry,
                                                        MapHandler &maps) {
    PhysicsMappings::initializeModule(maps);
    ConfigurationDefinition definition;
    definition.handler = {
        {HandlerKind::macro,
         {u"Physics-automatic-bracing-macros", u"Physics-vector-macros", u"Physics-vector-mo",
          u"Physics-vector-mi", u"Physics-derivative-macros", u"Physics-expressions-macros",
          u"Physics-quick-quad-macros", u"Physics-bra-ket-macros", u"Physics-matrix-macros"}},
        {HandlerKind::character, {u"Physics-characters"}},
        {HandlerKind::environment, {u"Physics-aux-envs"}}};
    definition.items = {
        {StackKind::auto_open,
         StackItemClass{[](StackBuildArguments const &a) -> std::unique_ptr<BaseItem> {
                            return std::make_unique<AutoOpen>(a.factory);
                        },
                        [](BaseItem const &item) {
                            return dynamic_cast<AutoOpen const *>(&item) != nullptr;
                        }}}};
    PhysicsOptions options;
    options.italicdiff = false;
    options.arrowdel = false;
    definition.options.physics = options;
    return Configuration::create(registry, u"physics", std::move(definition));
}
} // namespace folia::math::tex
