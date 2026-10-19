#include "support/folia_test.hpp"
import folia.math.core.math_list;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace core = folia::math::core;
namespace opts = folia::math::options;

struct SequencedLocation final : PropertyRecordBinding {
    std::string label;
    std::vector<Property> iReads;
    Property nValue;
    std::size_t nextI = 0;
    std::vector<std::string>* trace = nullptr;

    Property read(std::string_view key) {
        if (trace) trace->push_back(label + "." + std::string(key));
        if (key == "i") {
            if (iReads.empty()) return Missing{};
            const auto index = (std::min)(nextI, iReads.size() - 1);
            ++nextI;
            return iReads[index];
        }
        if (key == "n") return nValue;
        return Missing{};
    }
    Property get(std::string_view key) override { return read(key); }
    Property getOwn(std::string_view key) override { return read(key); }
    bool has(std::string_view key) const override { return key == "i" || key == "n"; }
    bool hasOwn(std::string_view key) const override { return has(key); }
    void set(std::string_view key, Property value) override { setOwn(key, std::move(value)); }
    void setOwn(std::string_view key, Property value) override {
        if (key == "n") nValue = std::move(value);
        else if (key == "i") iReads = {std::move(value)};
    }
    void erase(std::string_view key) override {
        if (key == "i") iReads.clear();
        else if (key == "n") nValue = Missing{};
    }
    std::vector<std::string> keys() const override { return {"i", "n"}; }
    Property prototype() const override { return Missing{}; }
    Property primitive(opts::PrimitiveHint) override { return Property("location"); }
};

struct MathProbe final : core::AbstractMathItem {
    explicit MathProbe(Property location)
        : AbstractMathItem(Property("x"), Missing{}, Missing{}, core::MathLocation{std::move(location)}) {}
    bool isEscaped() const override { return false; }
};

Property location(std::string label, std::vector<Property> iValues, Property nValue,
                  std::vector<std::string>& trace) {
    auto binding = std::make_shared<SequencedLocation>();
    binding->label = std::move(label);
    binding->iReads = std::move(iValues);
    binding->nValue = std::move(nValue);
    binding->trace = &trace;
    return Property(PropertyList::live(std::move(binding)));
}

std::string joined(const std::vector<std::string>& values) {
    std::string result;
    for (const auto& value : values) {
        if (!result.empty()) result += ',';
        result += value;
    }
    return result;
}

suite mathListIsBefore = [] {
    "math_list_is_before_preserves_source_short_circuit_and_live_read_order"_test = [] {
        opts::Context context;
        util::LinkedListModule sourceModule(context);
        core::AbstractMathList list(sourceModule);
        std::vector<std::string> trace;
        MathProbe left(location("L", {Property(1), Property(9)}, Property(2), trace));
        MathProbe right(location("R", {Property(1), Property(9)}, Property(3), trace));

        const bool result = list.isBefore(&left, &right);
        expect(result);
        expect(joined(trace) == "L.i,R.i,L.i,R.i,L.n,R.n");
    };

    "math_list_is_before_skips_repeated_fields_after_first_less_than"_test = [] {
        opts::Context context;
        util::LinkedListModule sourceModule(context);
        core::AbstractMathList list(sourceModule);
        std::vector<std::string> trace;
        MathProbe left(location("L", {Property(1)}, Property(99), trace));
        MathProbe right(location("R", {Property(2)}, Property(-99), trace));

        expect(list.isBefore(&left, &right));
        expect(joined(trace) == "L.i,R.i");
    };

    "math_list_is_before_uses_abstract_less_than_but_strict_equal_for_index"_test = [] {
        opts::Context context;
        util::LinkedListModule sourceModule(context);
        core::AbstractMathList list(sourceModule);
        std::vector<std::string> trace;
        MathProbe numeric(location("L", {Property(1)}, Property(1), trace));
        MathProbe textual(location("R", {Property("1")}, Property(0), trace));

        // Number(1) < String("1") is false, but 1 !== "1", so the node
        // positions are not compared. Source strict equality is intentional.
        expect(!list.isBefore(&numeric, &textual));
        expect(joined(trace) == "L.i,R.i,L.i,R.i");
    };

    "math_list_is_before_compares_two_string_indices_lexically"_test = [] {
        opts::Context context;
        util::LinkedListModule sourceModule(context);
        core::AbstractMathList list(sourceModule);
        std::vector<std::string> trace;
        MathProbe ten(location("L", {Property("10")}, Property(0), trace));
        MathProbe two(location("R", {Property("2")}, Property(0), trace));

        expect(list.isBefore(&ten, &two));
        expect(joined(trace) == "L.i,R.i");
    };
};
}
