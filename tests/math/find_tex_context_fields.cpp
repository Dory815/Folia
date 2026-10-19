#include "support/folia_test.hpp"
import folia.math.input.tex.find_tex_host;
import folia.math.util.property_host;
import folia.math.util.regexp_host;

namespace {
using namespace boost::ut;
namespace o = folia::math::options;
namespace t = folia::math::tex;

struct Hooks {
  o::ObjectRef identity{};
  o::Value end{o::Undefined{}}, start{o::Undefined{}};
  bool installAccessors = false;
  bool throwEndSetter = false;
  bool captureEnd = false;
  bool installed = false;
  int endGets = 0, startGets = 0;
  std::vector<double> envValues, subValues;
  std::size_t envIndex = 0, subIndex = 0;
  o::Value observedEndPattern{o::Undefined{}}, observedEndTuple{o::Undefined{}};
  std::vector<std::string> writes;
};

struct Probe final : t::FindTeX {
  using FindTeX::FindTeX;
  Hooks& hooks() const {
    return *std::any_cast<Hooks*>(optionsContext().payload(optionsContext().intrinsics().objectPrototype));
  }
  o::Value field(std::u16string_view name) const {
    if (name == u"options") return options_;
    if (name == u"end") return end_;
    if (name == u"sub") return sub_;
    if (name == u"env") return env_;
    if (name == u"start") return start_;
    if (name == u"hasPatterns") return hasPatterns_;
    return o::Undefined{};
  }
  void refresh() { getPatterns(); }
  o::Value endPattern(o::Value end, o::Value endp) override {
    hooks().observedEndPattern = endp;
    return FindTeX::endPattern(std::move(end), std::move(endp));
  }
  folia::math::Property findEnd(o::Value text, double n, const folia::math::regexp_host::Match& start, EndItem end) override {
    if (hooks().captureEnd) {
      hooks().observedEndTuple = end;
      return folia::math::PropertyNull{};
    }
    return FindTeX::findEnd(std::move(text), n, start, std::move(end));
  }
  void getPatterns() override {
    auto& h = hooks();
    if (h.installAccessors && !h.installed) {
      auto& cx = optionsContext();
      const auto identity = instanceIdentity();
      h.identity = identity;
      const auto getEnd = cx.function(u"get end", [&h](o::Context&, o::Value, std::span<const o::Value>) {
        ++h.endGets;
        return h.end;
      });
      const auto setEnd = cx.function(u"set end", [&h](o::Context&, o::Value, std::span<const o::Value> args) {
        h.writes.push_back("end");
        if (h.throwEndSetter) throw o::Exception("TypeError", u"end setter rejected assignment");
        h.end = args.empty() ? o::Value(o::Undefined{}) : args.front();
        return o::Value(o::Undefined{});
      });
      const auto getStart = cx.function(u"get start", [&h](o::Context&, o::Value, std::span<const o::Value>) {
        ++h.startGets;
        return h.start;
      });
      const auto setStart = cx.function(u"set start", [&h](o::Context&, o::Value, std::span<const o::Value> args) {
        h.writes.push_back("start");
        h.start = args.empty() ? o::Value(o::Undefined{}) : args.front();
        return o::Value(o::Undefined{});
      });
      cx.define(identity, o::Key(u"end"), o::Descriptor::access(getEnd, setEnd, true, true));
      if (!h.throwEndSetter)
        cx.define(identity, o::Key(u"start"), o::Descriptor::access(getStart, setStart, true, true));
      h.installed = true;
    }
    FindTeX::getPatterns();
  }
};

struct Fixture {
  o::Context context;
  Hooks hooks;
  folia::math::regexp_host::RegExpBindings regexp{context};
  folia::math::core::FindMathModule findMath{context};
  t::FindTeXModule module{context, findMath, regexp};
  Fixture() { context.payload(context.intrinsics().objectPrototype) = &hooks; }
  std::shared_ptr<Probe> make() { return module.make<Probe>(o::Undefined{}); }
  o::ObjectRef strings(std::u16string value) {
    const auto array = context.array(1);
    context.set(array, o::Key(u"0"), o::Value(std::move(value)));
    return array;
  }
};

suite find_tex_context_fields = [] {
"find_tex_context_fields_follow_source_deletion_and_reinitialization_order"_test = [] {
  Fixture f;
  const auto find = f.make();
  const auto identity = find->instanceIdentity();
  const std::vector<o::Key> initial{o::Key(u"options"), o::Key(u"end"), o::Key(u"sub"),
    o::Key(u"env"), o::Key(u"start"), o::Key(u"hasPatterns")};
  expect(f.context.ownKeys(o::Value(identity)) == initial);
  for (const auto& key : initial) {
    const auto descriptor = f.context.descriptor(o::Value(identity), key);
    expect(descriptor.has_value());
    if (!descriptor) continue;
    expect(!descriptor->accessor);
    expect(descriptor->writable);
    expect(descriptor->enumerable);
    expect(descriptor->configurable);
    expect(f.context.get(o::Value(identity), key) == find->field(std::get<std::u16string>(key.data)));
  }

  for (const auto name : {u"end", u"env", u"start"}) f.context.erase(identity, o::Key(name));
  expect(!f.context.hasOwn(o::Value(identity), o::Key(u"end")));
  expect(!f.context.hasOwn(o::Value(identity), o::Key(u"env")));
  expect(!f.context.hasOwn(o::Value(identity), o::Key(u"start")));
  find->refresh();
  const std::vector<o::Key> restored{o::Key(u"options"), o::Key(u"sub"), o::Key(u"hasPatterns"),
    o::Key(u"end"), o::Key(u"env"), o::Key(u"start")};
  expect(f.context.ownKeys(o::Value(identity)) == restored);
  expect(f.context.get(o::Value(identity), o::Key(u"end")) == find->field(u"end"));
  expect(f.context.get(o::Value(identity), o::Key(u"env")) == o::Value(1.0));
  expect(f.context.get(o::Value(identity), o::Key(u"start")) == find->field(u"start"));

  f.context.define(identity, o::Key(u"hasPatterns"), o::Descriptor::data(o::Value(false), false, true, true));
  const auto noPatterns = find->findMath(o::Value(f.strings(u"\\(x\\)")));
  expect(noPatterns->empty());
  bool readOnlyRejected = false;
  try { f.context.set(o::Value(identity), o::Key(u"hasPatterns"), o::Value(true)); }
  catch (const o::Exception& error) { readOnlyRejected = error.name == "TypeError"; }
  expect(readOnlyRejected);
};

"find_tex_algorithms_use_current_end_and_start_accessors"_test = [] {
  Fixture f;
  const auto find = f.make();
  // Match the pinned Source witness: construct first, replace existing fields,
  // then invoke getPatterns again through the live accessors.
  f.hooks.installAccessors = true;
  find->refresh();
  const auto identity = find->instanceIdentity();
  expect(f.hooks.writes.size() == 2);
  if (f.hooks.writes.size() == 2) {
    expect(f.hooks.writes[0] == "end");
    expect(f.hooks.writes[1] == "start");
  }
  expect(f.hooks.endGets >= 3);
  const auto items = find->findMath(o::Value(f.strings(u"before \\(x\\) after")));
  expect(!items->empty());
  expect(f.hooks.startGets >= 3);
  expect(f.context.get(o::Value(identity), o::Key(u"end")) == f.hooks.end);
  expect(f.context.get(o::Value(identity), o::Key(u"start")) == f.hooks.start);
  const auto endDescriptor = f.context.descriptor(o::Value(identity), o::Key(u"end"));
  const auto startDescriptor = f.context.descriptor(o::Value(identity), o::Key(u"start"));
  expect(endDescriptor.has_value());
  expect(startDescriptor.has_value());
  if (endDescriptor) expect(endDescriptor->accessor);
  if (startDescriptor) expect(startDescriptor->accessor);
};

"find_tex_first_end_setter_failure_precedes_other_source_fields"_test = [] {
  Fixture f;
  f.hooks.installAccessors = true;
  f.hooks.throwEndSetter = true;
  bool rejected = false;
  try { (void)f.make(); }
  catch (const o::Exception& error) { rejected = error.name == "TypeError"; }
  expect(rejected);
  if (!rejected) return;
  const auto identity = f.hooks.identity;
  const std::vector<o::Key> own{o::Key(u"options"), o::Key(u"end")};
  expect(f.context.ownKeys(o::Value(identity)) == own);
  expect(!f.context.hasOwn(o::Value(identity), o::Key(u"sub")));
  expect(!f.context.hasOwn(o::Value(identity), o::Key(u"env")));
  expect(!f.context.hasOwn(o::Value(identity), o::Key(u"start")));
  expect(!f.context.hasOwn(o::Value(identity), o::Key(u"hasPatterns")));
  expect(f.hooks.writes == std::vector<std::string>{"end"});
};

"find_tex_env_capture_indices_are_read_in_source_order"_test = [] {
  Fixture f;
  const auto find = f.make();
  f.hooks.captureEnd = true;
  const auto identity = find->instanceIdentity();
  f.context.set(o::Value(identity), o::Key(u"start"), o::Value(f.regexp.create(u"([A-Z]+)([0-9]+)(x)(y+)", u"g")));
  f.hooks.envValues = {1, 2, 3, 4};
  const auto getter = f.context.function(u"env getter", [&h = f.hooks](o::Context&, o::Value, std::span<const o::Value>) {
    if (h.envIndex >= h.envValues.size()) return o::Value(o::Undefined{});
    return o::Value(h.envValues[h.envIndex++]);
  });
  f.context.define(identity, o::Key(u"env"), o::Descriptor::access(getter, {}, true, true));

  (void)find->findMath(o::Value(f.strings(u"ENV12xyyy")));
  expect(f.hooks.envIndex == 4);
  const auto* ending = std::get_if<std::u16string>(&f.hooks.observedEndPattern.data);
  expect(ending != nullptr);
  if (ending) expect(ending->find(u"\\{x\\}") != std::u16string::npos);
  const auto tuple = std::get_if<o::ObjectRef>(&f.hooks.observedEndTuple.data);
  expect(tuple != nullptr);
  if (tuple) expect(f.context.get(*tuple, o::Key(u"0")) == o::Value(u"{yyy}"));
};

"find_tex_sub_capture_index_is_reread_for_math"_test = [] {
  Fixture f;
  const auto find = f.make();
  const auto identity = find->instanceIdentity();
  f.context.set(o::Value(identity), o::Key(u"start"), o::Value(f.regexp.create(u"([A-Z]+)([0-9]+)(x)(y+)", u"g")));
  f.context.set(o::Value(identity), o::Key(u"env"), o::Value(0.0));
  f.hooks.subValues = {1, 2, 3, 4};
  const auto getter = f.context.function(u"sub getter", [&h = f.hooks](o::Context&, o::Value, std::span<const o::Value>) {
    if (h.subIndex >= h.subValues.size()) return o::Value(o::Undefined{});
    return o::Value(h.subValues[h.subIndex++]);
  });
  f.context.define(identity, o::Key(u"sub"), o::Descriptor::access(getter, {}, true, true));

  const auto items = find->findMath(o::Value(f.strings(u"ENV12xyyy")));
  expect(f.hooks.subIndex == 4);
  expect(items->size() == 1);
  if (items->size() != 1) return;
  const auto item = std::get_if<folia::math::Property>(&items->front());
  expect(item != nullptr);
  if (!item) return;
  const auto record = folia::math::PropertyList::record(*item);
  expect(record.has_value());
  if (record) {
    expect(folia::math::property_host::string(record->get("math")) == u"x");
    const auto end = folia::math::property_host::get(record->get("end"), "n");
    expect(std::get_if<double>(&end.value) != nullptr);
    if (const auto* endNumber = std::get_if<double>(&end.value)) expect(*endNumber == 3.0);
  }
};
"find_tex_missing_env_quote_capture_uses_context_replace_error"_test = [] {
  Fixture f;
  const auto find = f.make();
  const auto identity = find->instanceIdentity();
  f.context.set(o::Value(identity), o::Key(u"start"), o::Value(f.regexp.create(u"([A-Z]+)([0-9]+)(x)(y+)", u"g")));
  f.hooks.envValues = {1, 2, 99, 4};
  const auto getter = f.context.function(u"env getter", [&h = f.hooks](o::Context&, o::Value, std::span<const o::Value>) {
    if (h.envIndex >= h.envValues.size()) return o::Value(o::Undefined{});
    return o::Value(h.envValues[h.envIndex++]);
  });
  f.context.define(identity, o::Key(u"env"), o::Descriptor::access(getter, {}, true, true));

  bool caught = false;
  try { (void)find->findMath(o::Value(f.strings(u"ENV12xyyy"))); }
  catch (const o::Exception& error) {
    caught = true;
    expect(error.name == "TypeError");
    expect(error.message == u"Cannot read properties of undefined (reading 'replace')");
  }
  expect(caught);
  expect(f.hooks.envIndex == 3);
};

"find_tex_missing_env_close_capture_stringifies_undefined"_test = [] {
  Fixture f;
  const auto find = f.make();
  f.hooks.captureEnd = true;
  const auto identity = find->instanceIdentity();
  f.context.set(o::Value(identity), o::Key(u"start"), o::Value(f.regexp.create(u"([A-Z]+)([0-9]+)(x)(y+)", u"g")));
  f.hooks.envValues = {1, 2, 1, 99};
  const auto getter = f.context.function(u"env getter", [&h = f.hooks](o::Context&, o::Value, std::span<const o::Value>) {
    if (h.envIndex >= h.envValues.size()) return o::Value(o::Undefined{});
    return o::Value(h.envValues[h.envIndex++]);
  });
  f.context.define(identity, o::Key(u"env"), o::Descriptor::access(getter, {}, true, true));

  (void)find->findMath(o::Value(f.strings(u"ENV12xyyy")));
  expect(f.hooks.envIndex == 4);
  const auto tuple = std::get_if<o::ObjectRef>(&f.hooks.observedEndTuple.data);
  expect(tuple != nullptr);
  if (tuple) expect(f.context.get(*tuple, o::Key(u"0")) == o::Value(u"{undefined}"));
};

"find_tex_missing_sub_math_capture_uses_context_length_error_after_reread"_test = [] {
  Fixture f;
  const auto find = f.make();
  const auto identity = find->instanceIdentity();
  f.context.set(o::Value(identity), o::Key(u"start"), o::Value(f.regexp.create(u"([A-Z]+)([0-9]+)(x)(y+)", u"g")));
  f.context.set(o::Value(identity), o::Key(u"env"), o::Value(0.0));
  f.hooks.subValues = {1, 2, 99, 4};
  const auto getter = f.context.function(u"sub getter", [&h = f.hooks](o::Context&, o::Value, std::span<const o::Value>) {
    if (h.subIndex >= h.subValues.size()) return o::Value(o::Undefined{});
    return o::Value(h.subValues[h.subIndex++]);
  });
  f.context.define(identity, o::Key(u"sub"), o::Descriptor::access(getter, {}, true, true));

  bool caught = false;
  try { (void)find->findMath(o::Value(f.strings(u"ENV12xyyy"))); }
  catch (const o::Exception& error) {
    caught = true;
    expect(error.name == "TypeError");
    expect(error.message == u"Cannot read properties of undefined (reading 'length')");
  }
  expect(caught);
  expect(f.hooks.subIndex == 4);
};

"find_tex_missing_sub_end_length_uses_context_length_error"_test = [] {
  Fixture f;
  const auto find = f.make();
  const auto identity = find->instanceIdentity();
  f.context.set(o::Value(identity), o::Key(u"start"), o::Value(f.regexp.create(u"([A-Z]+)([0-9]+)(x)(y+)", u"g")));
  f.context.set(o::Value(identity), o::Key(u"env"), o::Value(0.0));
  f.hooks.subValues = {1, 2, 3, 99};
  const auto getter = f.context.function(u"sub getter", [&h = f.hooks](o::Context&, o::Value, std::span<const o::Value>) {
    if (h.subIndex >= h.subValues.size()) return o::Value(o::Undefined{});
    return o::Value(h.subValues[h.subIndex++]);
  });
  f.context.define(identity, o::Key(u"sub"), o::Descriptor::access(getter, {}, true, true));

  bool caught = false;
  try { (void)find->findMath(o::Value(f.strings(u"ENV12xyyy"))); }
  catch (const o::Exception& error) {
    caught = true;
    expect(error.name == "TypeError");
    expect(error.message == u"Cannot read properties of undefined (reading 'length')");
  }
  expect(caught);
  expect(f.hooks.subIndex == 4);
};
};
}
