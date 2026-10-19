#include "support/folia_test.hpp"
import folia.math.util.option_functions;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace o = folia::math::options;

suite canonical_function_source_metadata = [] {
  "function_context_binding_retains_source_name_length_and_identity"_test = [] {
    o::Context context;
    OptionFunctionBindings bindings(context);
    auto callback = std::make_shared<Function>();
    callback->sourceMetadata = FunctionSourceMetadata{u"", 2};
    callback->call = [](const FunctionReceiver&, FunctionArgs args) -> std::any {
      if (args.size() != 2) throw std::logic_error("expected two source arguments");
      return std::any(19.0);
    };
    auto adapter = [] {
      OptionCallBinding binding;
      binding.receiver = [](o::Context&, o::Value receiver) { return FunctionReceiver(receiver); };
      binding.arguments = [](o::Context&, std::span<const o::Value> args) {
        std::vector<std::any> result;
        for (const auto& value : args) result.emplace_back(value);
        return result;
      };
      binding.result = [](o::Context&, const std::any& result) -> o::Value {
        if (const auto* number = std::any_cast<double>(&result)) return o::Value(*number);
        return o::Undefined{};
      };
      return binding;
    };

    const auto first = bindings.bind(callback, adapter(), u"native-label");
    const auto* identity = std::get_if<o::ObjectRef>(&first.data);
    expect(identity != nullptr);
    const auto name = context.get(*identity, o::Key(u"name"));
    const auto length = context.get(*identity, o::Key(u"length"));
    expect(std::get<std::u16string>(name.data).empty());
    expect(context.toNumber(length) == 2.0);
    const auto nameDescriptor = context.descriptor(first, o::Key(u"name"));
    const auto lengthDescriptor = context.descriptor(first, o::Key(u"length"));
    expect(nameDescriptor && !nameDescriptor->writable && !nameDescriptor->enumerable && nameDescriptor->configurable);
    expect(lengthDescriptor && !lengthDescriptor->writable && !lengthDescriptor->enumerable && lengthDescriptor->configurable);
    expect(bindings.bind(callback, adapter(), u"second-label") == first);
    const auto recovered = bindings.functionValue(first);
    expect(std::get<FunctionRef>(recovered) == callback);

    const std::array<o::Value, 2> args{o::Value(1), o::Value(2)};
    expect(context.call(*identity, o::Value(o::Undefined{}), args) == o::Value(19.0));
  };
};
} // namespace
