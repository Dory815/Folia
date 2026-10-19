#include "support/folia_test.hpp"
import folia.math.core.output_jax;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace opts = folia::math::options;
namespace core = folia::math::core;

struct OutputProbe final : core::AbstractOutputJax {
    OutputProbe(OptionFunctionBindings& bindings, opts::ObjectRef identity,
                FunctionListContextBindings& filters)
        : AbstractOutputJax(bindings, identity, filters) {}
    core::MathOutput typeset(core::MathItem&, core::MathDocumentReference) override {
        return PropertyNull{};
    }
    core::MathOutput escaped(core::MathItem&, core::MathDocumentReference) override {
        return PropertyNull{};
    }
    using core::AbstractOutputJax::executeFilters;
};

struct OutputFixture {
    opts::Context context;
    OptionFunctionBindings bindings{context};
    core::OutputJaxModule module{context, bindings};
    opts::ObjectRef constructor = context.constructor(u"OutputProbe");

    OutputFixture() {
        context.setPrototype(constructor, module.abstractClass());
        const auto instancePrototype = std::get<opts::ObjectRef>(
            context.get(opts::Value(constructor), opts::Key(u"prototype")).data);
        const auto basePrototype = std::get<opts::ObjectRef>(
            context.get(opts::Value(module.abstractClass()), opts::Key(u"prototype")).data);
        context.setPrototype(instancePrototype, basePrototype);

        const auto defaults = context.object();
        context.set(defaults, opts::Key(u"preFilters"), context.array());
        context.set(defaults, opts::Key(u"postFilters"), context.array());
        context.set(defaults, opts::Key(u"scale"), opts::Value(1));
        context.set(constructor, opts::Key(u"NAME"), opts::Value(u"probe"));
        context.set(constructor, opts::Key(u"OPTIONS"), opts::Value(defaults));
    }

    std::shared_ptr<OutputProbe> make(opts::Value supplied = opts::Undefined{}) {
        return core::makeOutputJax<OutputProbe>(bindings, module.filterBindings(),
                                                constructor, std::move(supplied));
    }
};

suite outputJaxBaseMembers = [] {
    "output_jax_base_initialization_name_adaptor_and_default_hooks"_test = [] {
        OutputFixture fixture;
        const auto supplied = fixture.context.object();
        fixture.context.set(supplied, opts::Key(u"scale"), opts::Value(2));
        auto jax = fixture.make(opts::Value(supplied));

        expect(fixture.context.get(jax->options, opts::Key(u"scale")) == opts::Value(2));
        expect(jax->preFilters != nullptr && jax->postFilters != nullptr);
        expect(jax->preFilters != jax->postFilters);
        expect(jax->name() == opts::Value(u"probe"));

        // The Source name getter reads this.constructor.NAME at each access.
        fixture.context.set(fixture.constructor, opts::Key(u"NAME"), opts::Value(u"renamed"));
        expect(jax->name() == opts::Value(u"renamed"));

        const opts::Value adaptor(fixture.context.object());
        jax->setAdaptor(core::OutputAdaptorReference(adaptor));
        expect(std::get<opts::Value>(jax->adaptor) == adaptor);

        std::array<std::any, 1> resetArgs{std::any(17)};
        jax->initialize();
        jax->reset(FunctionArgs(resetArgs));
        jax->getMetrics(core::MathDocumentReference{Missing{}});
        expect(std::holds_alternative<PropertyNull>(
            jax->styleSheet(core::MathDocumentReference{Missing{}})));
        expect(std::holds_alternative<PropertyNull>(
            jax->pageElements(core::MathDocumentReference{Missing{}})));
    };

    "output_jax_execute_filters_returns_the_mutated_live_data_field"_test = [] {
        OutputFixture fixture;
        auto jax = fixture.make();
        auto math = fixture.context.object();
        FunctionList filters;
        bool sawMath = false;
        bool sawUndefinedDocument = false;
        bool sawInitialData = false;
        auto callback = std::make_shared<Function>();
        callback->call = [&](const FunctionReceiver&, FunctionArgs arguments) -> std::any {
            expect(arguments.size() == 1);
            const auto record = std::any_cast<core::OutputJaxFilterDataRef>(arguments[0]);
            sawMath = record->field(u"math") == opts::Value(math);
            sawUndefinedDocument = std::holds_alternative<opts::Undefined>(
                record->field(u"document").data);
            sawInitialData = record->dataValue() == opts::Value(3);
            record->setField(u"data", opts::Value(9));
            return std::any{};
        };
        filters.add(FunctionValue{FunctionRef(callback)}, 0);

        const auto result = jax->executeFilters(
            filters,
            core::OutputMathItemReference{opts::Value(math)},
            core::MathDocumentReference{Missing{}},
            std::any(opts::Value(3)));
        const auto* value = std::any_cast<opts::Value>(&result);
        expect(sawMath && sawUndefinedDocument && sawInitialData);
        expect(value != nullptr);
        if (value) expect(*value == opts::Value(9));
    };
};
}
