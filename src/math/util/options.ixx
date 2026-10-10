// Ported from MathJax 4.1.3 ts/util/Options.ts (Apache-2.0).
// Copyright (c) 2017-2026 The MathJax Consortium; see ../LICENSE-MathJax.txt.
// Preserve source operation order and object-property semantics in the native graph.
export module folia.math.util.options;
export import folia.math.util.option_types;
import std;

namespace folia::math::options {
namespace {
std::string to_utf8(std::u16string_view text);
}

export inline constexpr std::u16string_view APPEND = u"[+]";
export inline constexpr std::u16string_view REMOVE = u"[-]";

export enum class InvalidOption { warn, fatal };
export struct Diagnostics {
    InvalidOption invalidOption = InvalidOption::warn;
    std::function<void(std::u16string, std::u16string)> optionError;
    std::function<void(std::u16string)> warningSink;
};

export Diagnostics OPTIONS = [] {
    Diagnostics diagnostics;
    diagnostics.warningSink = [](std::u16string message) {
        std::cerr << to_utf8(message) << '\n';
    };
    diagnostics.optionError = [](std::u16string message, std::u16string) {
        if (OPTIONS.invalidOption == InvalidOption::fatal) {
            throw Exception("Error", std::move(message));
        }
        if (OPTIONS.warningSink) {
            OPTIONS.warningSink(std::u16string(u"MathJax: ") + message);
        }
    };
    return diagnostics;
}();

export std::vector<Key> keys(Context&, Value);
export bool isObject(Context&, const Value&);

namespace {

constexpr std::size_t max_option_depth = 512;

[[noreturn]] void depth_error() {
    throw Exception("RangeError", u"Maximum call stack size exceeded");
}

void check_depth(std::size_t depth) {
    if (depth >= max_option_depth) depth_error();
}

void append_utf8(std::string& out, std::uint32_t cp) {
    if (cp <= 0x7f) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7ff) {
        out.push_back(static_cast<char>(0xc0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else if (cp <= 0xffff) {
        out.push_back(static_cast<char>(0xe0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else {
        out.push_back(static_cast<char>(0xf0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    }
}

std::string to_utf8(std::u16string_view text) {
    std::string out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        std::uint32_t cp = text[i];
        if (cp >= 0xd800 && cp <= 0xdbff) {
            const auto low = i + 1 < text.size()
                ? static_cast<std::uint32_t>(text[i + 1]) : 0;
            if (low >= 0xdc00 && low <= 0xdfff) {
                cp = 0x10000 + ((cp - 0xd800) << 10) + (low - 0xdc00);
                ++i;
            } else {
                cp = 0xfffd;
            }
        } else if (cp >= 0xdc00 && cp <= 0xdfff) {
            cp = 0xfffd;
        }
        append_utf8(out, cp);
    }
    return out;
}

bool is_array(Context& context, const Value& value) {
    const auto* object = std::get_if<ObjectRef>(&value.data);
    return object && context.kind(*object) == ObjectKind::array;
}

ObjectRef require_object(Context& context, const Value& value) {
    const auto* object = std::get_if<ObjectRef>(&value.data);
    if (!object) throw Exception("TypeError", u"Value is not an object");
    return *object;
}

std::u16string string_key(const Key& key) {
    if (const auto* text = std::get_if<std::u16string>(&key.data)) return *text;
    throw Exception("TypeError", u"Cannot convert a Symbol value to a string");
}

bool is_function_or_object(Context& context, const Value& value) {
    if (std::holds_alternative<Null>(value.data) ||
        std::holds_alternative<Undefined>(value.data)) return false;
    if (const auto* object = std::get_if<ObjectRef>(&value.data)) {
        const auto kind = context.kind(*object);
        return kind == ObjectKind::object || kind == ObjectKind::array ||
               kind == ObjectKind::instance || kind == ObjectKind::function;
    }
    return false;
}

bool has_valid_sentinel_shape(Context& context, const Value& source_value,
                              const std::vector<Key>& ids) {
    if (ids.size() == 1) {
        const bool named_sentinel = ids.front() == Key(std::u16string(APPEND)) ||
                                    ids.front() == Key(std::u16string(REMOVE));
        return named_sentinel && is_array(context, context.get(source_value, ids.front()));
    }
    if (ids.size() != 2) return false;

    // Array.sort() without a comparator stringifies elements. A Symbol throws
    // during ToString; preserve that behavior instead of comparing variants.
    std::vector<std::u16string> names;
    names.reserve(ids.size());
    for (const Key& id : ids) names.push_back(string_key(id));
    std::sort(names.begin(), names.end());
    const bool exact_names = names[0] == APPEND && names[1] == REMOVE;
    if (!exact_names) return false;
    return is_array(context, context.get(source_value, Key(std::u16string(APPEND)))) &&
           is_array(context, context.get(source_value, Key(std::u16string(REMOVE))));
}

ObjectRef spread_arrays(Context& context, ObjectRef first,
                        const std::function<Value()>& get_second) {
    auto left = context.arrayValues(first);
    const Value second_value = get_second();
    const ObjectRef second = require_object(context, second_value);
    auto right = context.arrayValues(second);
    const auto max_length = std::numeric_limits<std::uint32_t>::max();
    if (right.size() > max_length || left.size() > max_length - right.size()) {
        throw Exception("RangeError", u"Invalid array length");
    }
    const auto length = static_cast<std::uint32_t>(left.size() + right.size());
    const ObjectRef result = context.array(length);
    // Use decimal UTF-16 property names for indices; this helper avoids depending
    // on locale or narrow-character conversion.
    auto set_at = [&](std::uint32_t i, Value value) {
        std::u16string name;
        do {
            name.push_back(static_cast<char16_t>(u'0' + i % 10));
            i /= 10;
        } while (i);
        std::reverse(name.begin(), name.end());
        context.set(Value(result), Key(std::move(name)), std::move(value));
    };
    std::uint32_t index = 0;
    for (Value& value : left) set_at(index++, std::move(value));
    for (Value& value : right) set_at(index++, std::move(value));
    return result;
}

enum class TraversalKind { copy, insert };
enum class TraversalStage {
    copy_visit, copy_wait_child, copy_define_read, copy_define_apply,
    insert_visit, insert_wait_child
};

struct TraversalFrame {
    TraversalKind kind = TraversalKind::copy;
    TraversalStage stage = TraversalStage::copy_visit;
    std::size_t depth = 0;
    bool warn = false;
    Value destination;
    Value source;
    std::vector<Key> source_keys;
    std::size_t cursor = 0;
    ObjectRef properties;
    ObjectRef result;
    std::vector<Key> define_keys;
    std::size_t define_cursor = 0;
    std::vector<std::pair<Key, Descriptor>> definitions;
    std::optional<Key> current_key;
    Descriptor current_descriptor;
    bool current_value_is_child_copy = false;
    Value child_result;
    bool child_ready = false;
};

TraversalFrame make_copy_frame(Context& context, Value definition, std::size_t depth) {
    check_depth(depth);
    TraversalFrame frame;
    frame.kind = TraversalKind::copy;
    frame.stage = TraversalStage::copy_visit;
    frame.depth = depth;
    frame.source = std::move(definition);
    frame.source_keys = keys(context, frame.source);
    frame.properties = context.object();
    return frame;
}

TraversalFrame make_insert_frame(Context& context, Value destination, Value source,
                                 bool warn, std::size_t depth) {
    check_depth(depth);
    TraversalFrame frame;
    frame.kind = TraversalKind::insert;
    frame.stage = TraversalStage::insert_visit;
    frame.depth = depth;
    frame.warn = warn;
    frame.destination = std::move(destination);
    frame.source = std::move(source);
    frame.source_keys = keys(context, frame.source);
    return frame;
}

Value run_traversal(Context& context, TraversalFrame root) {
    std::vector<TraversalFrame> stack;
    stack.reserve(32);
    stack.push_back(std::move(root));

    auto save_descriptor = [&](TraversalFrame& frame) {
        if (frame.current_descriptor.enumerable) {
            const ObjectRef descriptor_value = context.descriptorObject(frame.current_descriptor);
            context.set(Value(frame.properties), *frame.current_key, Value(descriptor_value));
        }
        frame.current_key.reset();
        frame.stage = TraversalStage::copy_visit;
    };

    for (;;) {
        TraversalFrame& frame = stack.back();
        if (frame.child_ready) {
            frame.child_ready = false;
            if (frame.kind == TraversalKind::copy) {
                if (frame.current_value_is_child_copy) {
                    frame.current_descriptor.value = frame.child_result;
                }
                save_descriptor(frame);
            } else if (frame.current_value_is_child_copy) {
                context.set(frame.destination, *frame.current_key, frame.child_result);
                frame.current_key.reset();
                frame.stage = TraversalStage::insert_visit;
            } else {
                frame.current_key.reset();
                frame.stage = TraversalStage::insert_visit;
            }
            continue;
        }

        if (frame.kind == TraversalKind::copy) {
            if (frame.stage == TraversalStage::copy_visit) {
                if (frame.cursor == frame.source_keys.size()) {
                    const bool keep_expandable =
                        context.get(frame.source, Key(u"constructor")) ==
                        Value(context.intrinsics().expandableConstructor);
                    frame.result = keep_expandable
                        ? context.objectWithPrototype(context.intrinsics().expandablePrototype)
                        : context.object();
                    frame.define_keys = context.ownKeys(Value(frame.properties));
                    frame.define_cursor = 0;
                    frame.stage = TraversalStage::copy_define_read;
                    continue;
                }

                const Key key = frame.source_keys[frame.cursor++];
                const auto descriptor = context.descriptor(frame.source, key);
                if (!descriptor) {
                    throw Exception("TypeError", u"Cannot read properties of undefined (reading 'value')");
                }
                frame.current_key = key;
                frame.current_descriptor = *descriptor;
                if (is_array(context, descriptor->value)) {
                    const ObjectRef replacement = context.array();
                    frame.current_descriptor.value = Value(replacement);
                    frame.current_value_is_child_copy = false;
                    frame.stage = TraversalStage::copy_wait_child;
                    stack.push_back(make_insert_frame(context, Value(replacement),
                                                      descriptor->value, false, frame.depth + 1));
                } else if (isObject(context, descriptor->value)) {
                    frame.current_value_is_child_copy = true;
                    frame.stage = TraversalStage::copy_wait_child;
                    stack.push_back(make_copy_frame(context, descriptor->value, frame.depth + 1));
                } else {
                    save_descriptor(frame);
                }
                continue;
            }

            if (frame.stage == TraversalStage::copy_define_read) {
                if (frame.define_cursor == frame.define_keys.size()) {
                    frame.define_cursor = 0;
                    frame.stage = TraversalStage::copy_define_apply;
                    continue;
                }
                const Key key = frame.define_keys[frame.define_cursor++];
                const auto entry = context.descriptor(Value(frame.properties), key);
                if (!entry || !entry->enumerable) continue;
                const Value descriptor_value = context.get(Value(frame.properties), key);
                frame.definitions.emplace_back(key, context.descriptorFromObject(descriptor_value));
                continue;
            }

            if (frame.stage == TraversalStage::copy_define_apply) {
                if (frame.define_cursor == frame.definitions.size()) {
                    const Value result(frame.result);
                    stack.pop_back();
                    if (stack.empty()) return result;
                    stack.back().child_result = result;
                    stack.back().child_ready = true;
                    continue;
                }
                const auto& [key, descriptor] = frame.definitions[frame.define_cursor++];
                context.define(frame.result, key, descriptor);
                continue;
            }

            // copy_wait_child resumes only through child_ready above.
            continue;
        }

        if (frame.stage == TraversalStage::insert_visit) {
            if (frame.cursor == frame.source_keys.size()) {
                const Value result = frame.destination;
                stack.pop_back();
                if (stack.empty()) return result;
                stack.back().child_result = result;
                stack.back().child_ready = true;
                continue;
            }

            const Key key = frame.source_keys[frame.cursor++];
            if (frame.warn && context.get(frame.destination, key) == Value(Undefined{}) &&
                context.get(frame.destination, Key(u"constructor")) !=
                    Value(context.intrinsics().expandableConstructor)) {
                const std::u16string invalid_key = context.keyString(key);
                const std::u16string message = u"Invalid option \"" + invalid_key +
                                               u"\" (no default value).";
                if (!OPTIONS.optionError) {
                    throw Exception("TypeError", u"OPTIONS.optionError is not callable");
                }
                OPTIONS.optionError(message, invalid_key);
                continue;
            }

            const Value source_value = context.get(frame.source, key);
            const Value destination_value = context.get(frame.destination, key);
            if (isObject(context, source_value) &&
                !std::holds_alternative<Null>(destination_value.data) &&
                is_function_or_object(context, destination_value)) {
                const std::vector<Key> ids = keys(context, source_value);
                if (is_array(context, destination_value) &&
                    has_valid_sentinel_shape(context, source_value, ids)) {
                    ObjectRef current = require_object(context, destination_value);
                    const Value remove_value = context.get(source_value, Key(std::u16string(REMOVE)));
                    if (truthy(remove_value)) {
                        current = context.arrayFilter(current, [&](Value candidate) {
                            const Value current_remove = context.get(source_value, Key(std::u16string(REMOVE)));
                            const ObjectRef remove_array = require_object(context, current_remove);
                            return context.arrayIndexOf(remove_array, candidate) < 0;
                        });
                        context.set(frame.destination, key, Value(current));
                    }
                    const Value append_value = context.get(source_value, Key(std::u16string(APPEND)));
                    if (truthy(append_value)) {
                        current = spread_arrays(context, current, [&] {
                            return context.get(source_value, Key(std::u16string(APPEND)));
                        });
                        context.set(frame.destination, key, Value(current));
                    }
                } else {
                    frame.current_key = key;
                    frame.current_value_is_child_copy = false;
                    frame.stage = TraversalStage::insert_wait_child;
                    stack.push_back(make_insert_frame(context, destination_value, source_value,
                                                      frame.warn, frame.depth + 1));
                }
            } else if (is_array(context, source_value)) {
                context.set(frame.destination, key, Value(context.array()));
                const Value new_array = context.get(frame.destination, key);
                frame.current_key.reset();
                frame.current_value_is_child_copy = false;
                frame.stage = TraversalStage::insert_wait_child;
                stack.push_back(make_insert_frame(context, new_array, source_value,
                                                  false, frame.depth + 1));
            } else if (isObject(context, source_value)) {
                frame.current_key = key;
                frame.current_value_is_child_copy = true;
                frame.stage = TraversalStage::insert_wait_child;
                stack.push_back(make_copy_frame(context, source_value, frame.depth + 1));
            } else {
                context.set(frame.destination, key, source_value);
            }
            continue;
        }

        // insert_wait_child resumes only through child_ready above.
    }
}

Value run_copy(Context& context, Value definition, std::size_t depth) {
    return run_traversal(context, make_copy_frame(context, std::move(definition), depth));
}

Value run_insert(Context& context, Value destination, Value source,
                 bool warn, std::size_t depth) {
    return run_traversal(context, make_insert_frame(context, std::move(destination),
                                                     std::move(source), warn, depth));
}

} // namespace

bool isObject(Context& context, const Value& value) {
    if (!std::holds_alternative<ObjectRef>(value.data)) return false;
    const ObjectRef object = std::get<ObjectRef>(value.data);
    const auto kind = context.kind(object);
    if (kind == ObjectKind::function) return false;
    const Key constructor_key(u"constructor");
    if (context.get(value, constructor_key) == Value(context.intrinsics().objectConstructor)) {
        return true;
    }
    return context.get(value, constructor_key) == Value(context.intrinsics().expandableConstructor);
}

export Value expandable(Context& context, Value definition) {
    const ObjectRef result = context.objectWithPrototype(context.intrinsics().expandablePrototype);
    for (const Key& key : keys(context, definition)) {
        const auto descriptor = context.descriptor(definition, key);
        if (descriptor && descriptor->enumerable) {
            context.set(Value(result), key, context.get(definition, key));
        }
    }
    return Value(result);
}

export Value makeArray(Context& context, Value value) {
    if (is_array(context, value)) return value;
    const ObjectRef result = context.array(1);
    context.set(Value(result), Key(u"0"), std::move(value));
    return Value(result);
}

std::vector<Key> keys(Context& context, Value definition) {
    if (!truthy(definition)) return {};
    std::vector<Key> result;
    for (auto& name : context.enumerableStringKeys(definition)) {
        result.emplace_back(std::move(name));
    }
    for (SymbolRef symbol : context.ownSymbols(definition)) {
        result.emplace_back(symbol);
    }
    return result;
}

export Value copy(Context& context, Value definition) {
    return run_copy(context, std::move(definition), 0);
}

export Value insert(Context& context, Value destination, Value source, bool warn = true) {
    return run_insert(context, std::move(destination), std::move(source), warn, 0);
}

export Value defaultOptions(Context& context, Value destination, std::span<const Value> definitions) {
    for (const Value& definition : definitions) run_insert(context, destination, definition, false, 0);
    return destination;
}

export Value userOptions(Context& context, Value destination, std::span<const Value> definitions) {
    for (const Value& definition : definitions) run_insert(context, destination, definition, true, 0);
    return destination;
}

export Value selectOptions(Context& context, Value options, std::span<const std::u16string> names) {
    const ObjectRef result = context.object();
    for (const std::u16string& name : names) {
        const Key key(name);
        if (context.hasOwn(options, key)) {
            context.set(Value(result), key, context.get(options, key));
        }
    }
    return Value(result);
}

export Value selectOptionsFromKeys(Context& context, Value options, Value key_source) {
    const auto names = context.enumerableStringKeys(key_source);
    return selectOptions(context, std::move(options), names);
}

export std::vector<Value> separateOptions(Context& context, Value options,
                                   std::span<const Value> selectors) {
    std::vector<Value> results;
    results.reserve(selectors.size() + 1);
    for (const Value& selector : selectors) {
        const ObjectRef exists = context.object();
        const ObjectRef missing = context.object();
        const auto names = truthy(options)
            ? context.enumerableStringKeys(options)
            : std::vector<std::u16string>{};
        for (const std::u16string& name : names) {
            const Key key(name);
            const Value selector_value = context.get(selector, key);
            const ObjectRef target = std::holds_alternative<Undefined>(selector_value.data)
                ? missing : exists;
            context.set(Value(target), key, context.get(options, key));
        }
        results.emplace_back(exists);
        options = Value(missing);
    }
    results.insert(results.begin(), std::move(options));
    return results;
}

export Value lookup(Context& context, std::u16string_view name, Value options,
             Value default_value = Null{}) {
    if (std::holds_alternative<Undefined>(default_value.data)) default_value = Null{};
    const Key key{std::u16string(name)};
    return context.hasOwn(options, key) ? context.get(options, key) : default_value;
}

} // namespace folia::math::options
