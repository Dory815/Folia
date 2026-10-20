module folia.math.output.scene.builder;
import std;

namespace folia::math::output::scene {
namespace {
bool finite_rect(Rect const &bounds) noexcept {
    return std::isfinite(bounds.x) && std::isfinite(bounds.y) && std::isfinite(bounds.width) &&
           std::isfinite(bounds.height) && bounds.width >= 0 && bounds.height >= 0;
}
bool has_attribute(SemanticMetadata const &metadata, std::string_view name) noexcept {
    return std::ranges::any_of(metadata.ordered_attributes,
                               [name](auto const &attribute) { return attribute.name == name; });
}
} // namespace

struct Builder::State {
    struct Entry {
        std::variant<Primitive, NodeId> value;
    };
    struct MutableGroup {
        NodeId id{};
        Transform transform{};
        std::optional<Clip> clip;
        std::optional<Link> link;
        std::optional<Tooltip> tooltip;
        SemanticMetadata metadata;
        std::vector<Entry> entries;
        ElementRole role{ElementRole::math_wrapper};
        Paint paint{};
        std::optional<Viewport> viewport;
    };
    std::vector<MutableGroup> groups;
    Metrics metrics{};
    FontMetrics font_metrics{};
    DisplayLayout display_layout{};
    FrameLayout frame_layout{};
    NodeId next_id{1};
    State() {
        groups.push_back(MutableGroup{.id = 0});
    }
    MutableGroup *find(NodeId id) noexcept {
        for (auto &group : groups)
            if (group.id == id)
                return &group;
        return nullptr;
    }
    MutableGroup &require(NodeId id) {
        auto *value = find(id);
        if (!value)
            throw std::out_of_range("Scene group does not exist");
        return *value;
    }
    Group freeze_group(NodeId id) && {
        auto *source = find(id);
        if (!source)
            throw std::logic_error("invalid group in Scene builder");
        Group out;
        out.id = source->id;
        out.transform = source->transform;
        out.clip = std::move(source->clip);
        out.link = std::move(source->link);
        out.tooltip = std::move(source->tooltip);
        out.metadata = std::move(source->metadata);
        out.role = source->role;
        out.paint = std::move(source->paint);
        out.viewport = std::move(source->viewport);
        for (auto &entry : source->entries) {
            if (auto *primitive = std::get_if<Primitive>(&entry.value))
                out.entries.emplace_back(std::move(*primitive));
            else {
                auto child = std::make_shared<Group>(
                    std::move(*this).freeze_group(std::get<NodeId>(entry.value)));
                out.entries.emplace_back(std::shared_ptr<Group const>{std::move(child)});
            }
        }
        return out;
    }
};

Builder::Builder() : state_(std::make_unique<State>()) {}
Builder::~Builder() = default;
Builder::Builder(Builder &&) noexcept = default;
Builder &Builder::operator=(Builder &&) noexcept = default;

NodeId Builder::add_group(NodeId parent, Transform transform, std::optional<Clip> clip,
                          std::optional<Link> link, std::optional<Tooltip> tooltip,
                          SemanticMetadata metadata, ElementRole role, Paint paint,
                          std::optional<Viewport> viewport) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    auto *owner = state_->find(parent);
    if (!owner)
        throw std::out_of_range("Scene parent group does not exist");
    if (state_->next_id == 0)
        throw std::overflow_error("Scene group id space exhausted");
    auto const id = state_->next_id++;
    state_->groups.push_back(State::MutableGroup{.id = id,
                                                 .transform = transform,
                                                 .clip = std::move(clip),
                                                 .link = std::move(link),
                                                 .tooltip = std::move(tooltip),
                                                 .metadata = std::move(metadata),
                                                 .role = role,
                                                 .paint = std::move(paint),
                                                 .viewport = std::move(viewport)});
    owner = state_->find(parent); // vector growth can invalidate the previous pointer
    owner->entries.push_back(State::Entry{NodeId{id}});
    return id;
}

std::optional<Rect> Builder::background_bounds(NodeId id) const {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    auto const &entries = state_->require(id).entries;
    if (entries.empty())
        return {};
    auto const *primitive = std::get_if<Primitive>(&entries.front().value);
    auto const *rule = primitive ? std::get_if<Rule>(primitive) : nullptr;
    if (!rule || rule->role != ElementRole::background_box ||
        !has_attribute(rule->metadata, "data-bgcolor"))
        return {};
    return rule->bounds;
}
bool Builder::set_background_bounds(NodeId id, Rect bounds) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    if (!finite_rect(bounds))
        throw std::invalid_argument("Scene background bounds are invalid");
    auto &entries = state_->require(id).entries;
    if (entries.empty())
        return false;
    auto *primitive = std::get_if<Primitive>(&entries.front().value);
    auto *rule = primitive ? std::get_if<Rule>(primitive) : nullptr;
    if (!rule || rule->role != ElementRole::background_box ||
        !has_attribute(rule->metadata, "data-bgcolor"))
        return false;
    rule->bounds = bounds;
    return true;
}
bool Builder::set_background_width(NodeId id, Unit width) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    if (!std::isfinite(width) || width < 0)
        throw std::invalid_argument("Scene background width is invalid");
    auto &entries = state_->require(id).entries;
    if (entries.empty())
        return false;
    auto *primitive = std::get_if<Primitive>(&entries.front().value);
    if (!primitive)
        return false;
    if (auto *rule = std::get_if<Rule>(primitive)) {
        if (rule->role != ElementRole::background_box)
            return false;
        rule->bounds.width = width;
        return true;
    }
    if (auto *path = std::get_if<Path>(primitive)) {
        if (path->role != ElementRole::background_box)
            return false;
        path->layout_width = width;
        return true;
    }
    return false;
}
void Builder::reparent_group(NodeId child_id, NodeId parent_id) {
    static_assert(std::is_nothrow_move_constructible_v<State::Entry>);
    static_assert(std::is_nothrow_move_assignable_v<State::Entry>);
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    if (child_id == 0)
        throw std::invalid_argument("Scene root group cannot be reparented");
    auto const *child_group = state_->find(child_id);
    auto *new_parent = state_->find(parent_id);
    if (!child_group || !new_parent)
        throw std::out_of_range("Scene reparent group does not exist");

    std::unordered_map<NodeId, NodeId> parents;
    parents.reserve(state_->groups.size());
    for (auto const &owner : state_->groups) {
        for (auto const &entry : owner.entries) {
            auto const *descendant = std::get_if<NodeId>(&entry.value);
            if (!descendant)
                continue;
            if (!state_->find(*descendant))
                throw std::logic_error("Scene group has a dangling child entry");
            if (!parents.emplace(*descendant, owner.id).second)
                throw std::invalid_argument("Scene group has multiple parent entries");
        }
    }
    auto const owner_it = parents.find(child_id);
    if (owner_it == parents.end())
        throw std::invalid_argument("Scene group has no parent entry");
    auto const old_parent_id = owner_it->second;
    if (old_parent_id == parent_id)
        return;
    std::unordered_set<NodeId> ancestors;
    ancestors.reserve(state_->groups.size());
    auto ancestor = parent_id;
    while (ancestor != 0) {
        if (!ancestors.insert(ancestor).second)
            throw std::invalid_argument("Scene group hierarchy contains a cycle");
        if (ancestor == child_id)
            throw std::invalid_argument("Scene reparent would create a cycle");
        auto const found = parents.find(ancestor);
        if (found == parents.end())
            break;
        ancestor = found->second;
    }
    auto &old_entries = state_->require(old_parent_id).entries;
    auto const old_entry = std::ranges::find_if(old_entries, [child_id](auto const &entry) {
        auto const *id = std::get_if<NodeId>(&entry.value);
        return id && *id == child_id;
    });
    if (old_entry == old_entries.end())
        throw std::logic_error("Scene group parent entry is inconsistent");
    auto const old_index = static_cast<std::size_t>(old_entry - old_entries.begin());
    if (new_parent->entries.size() == new_parent->entries.max_size())
        throw std::length_error("Scene parent cannot accept another group entry");
    new_parent->entries.reserve(new_parent->entries.size() + 1);
    auto &stable_old_entries = state_->require(old_parent_id).entries;
    State::Entry moving = std::move(stable_old_entries[old_index]);
    stable_old_entries.erase(stable_old_entries.begin() + static_cast<std::ptrdiff_t>(old_index));
    state_->require(parent_id).entries.push_back(std::move(moving));
}
void Builder::set_viewport_bounds(ElementRole role, Rect bounds) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    if (!finite_rect(bounds))
        throw std::invalid_argument("Scene viewport bounds are invalid");
    for (auto &group : state_->groups)
        if (group.role == role && group.viewport)
            group.viewport->bounds = bounds;
}

Transform Builder::transform(NodeId id) const {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    return state_->require(id).transform;
}
void Builder::set_transform(NodeId id, Transform value) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    state_->require(id).transform = std::move(value);
}
SemanticMetadata Builder::metadata(NodeId id) const {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    return state_->require(id).metadata;
}
void Builder::set_metadata(NodeId id, SemanticMetadata value) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    state_->require(id).metadata = std::move(value);
}
Paint Builder::paint(NodeId id) const {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    return state_->require(id).paint;
}
void Builder::set_paint(NodeId id, Paint value) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    state_->require(id).paint = std::move(value);
}
std::size_t Builder::entry_count(NodeId id) const {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    return state_->require(id).entries.size();
}
std::vector<NodeId> Builder::child_groups(NodeId id) const {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    auto const &group = state_->require(id);
    std::vector<NodeId> result;
    for (auto const &entry : group.entries)
        if (auto child = std::get_if<NodeId>(&entry.value))
            result.push_back(*child);
    return result;
}
void Builder::discard_group(NodeId id) {
    static_assert(std::is_nothrow_move_assignable_v<State::Entry>);
    static_assert(std::is_nothrow_move_assignable_v<State::MutableGroup>);
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    if (id == 0)
        throw std::invalid_argument("Scene root group cannot be discarded");
    if (!state_->find(id))
        throw std::out_of_range("Scene group does not exist");

    // Validate the parent links and collect the complete subtree before the
    // first mutation. Orphan groups are permitted during builder assembly,
    // but the requested group itself must have exactly one owning entry.
    std::unordered_map<NodeId, NodeId> parents;
    parents.reserve(state_->groups.size());
    for (auto const &owner : state_->groups) {
        for (auto const &entry : owner.entries) {
            if (auto child = std::get_if<NodeId>(&entry.value)) {
                if (!parents.emplace(*child, owner.id).second)
                    throw std::logic_error("Scene group has multiple parent entries");
            }
        }
    }
    auto const parent = parents.find(id);
    if (parent == parents.end())
        throw std::invalid_argument("Scene group has no parent entry");
    auto const parent_id = parent->second;
    auto const &parent_group = state_->require(parent_id);
    auto const parent_entry = std::ranges::find_if(parent_group.entries, [id](auto const &entry) {
        auto child = std::get_if<NodeId>(&entry.value);
        return child && *child == id;
    });
    if (parent_entry == parent_group.entries.end())
        throw std::logic_error("Scene group parent entry is inconsistent");
    auto const parent_index = static_cast<std::size_t>(parent_entry - parent_group.entries.begin());

    std::vector<NodeId> pending;
    std::unordered_set<NodeId> visited;
    pending.reserve(state_->groups.size());
    visited.reserve(state_->groups.size());
    pending.push_back(id);
    while (!pending.empty()) {
        auto const current_id = pending.back();
        pending.pop_back();
        if (!visited.insert(current_id).second)
            throw std::logic_error("Scene group hierarchy contains a cycle");
        auto const &current = state_->require(current_id);
        for (auto const &entry : current.entries) {
            auto child = std::get_if<NodeId>(&entry.value);
            if (!child)
                continue;
            if (!state_->find(*child))
                throw std::logic_error("Scene group has a dangling child entry");
            auto const child_parent = parents.find(*child);
            if (child_parent == parents.end() || child_parent->second != current_id)
                throw std::logic_error("Scene group child entry is inconsistent");
            pending.push_back(*child);
        }
    }
    if (visited.contains(parent_id))
        throw std::logic_error("Scene group hierarchy contains a cycle");

    // All traversals and allocations are complete. Erasing vector entries
    // releases the discarded values and group-owned primitives normally.
    auto &mutable_parent = state_->require(parent_id);
    mutable_parent.entries.erase(mutable_parent.entries.begin() +
                                 static_cast<std::ptrdiff_t>(parent_index));
    state_->groups.erase(
        std::remove_if(state_->groups.begin(), state_->groups.end(),
                       [&](auto const &group) { return visited.contains(group.id); }),
        state_->groups.end());
}
void Builder::insert_primitive(NodeId id, std::size_t index, Primitive value) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    auto &entries = state_->require(id).entries;
    if (index > entries.size())
        throw std::out_of_range("Scene primitive insertion index");
    entries.insert(entries.begin() + static_cast<std::ptrdiff_t>(index),
                   State::Entry{std::move(value)});
}
NodeId Builder::wrap_contents(NodeId id, Transform transform_value, SemanticMetadata metadata_value,
                              ElementRole role_value) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    auto &parent = state_->require(id);
    if (state_->next_id == 0)
        throw std::overflow_error("Scene group id space exhausted");
    auto const wrapped_id = state_->next_id++;
    State::MutableGroup wrapped{.id = wrapped_id,
                                .transform = std::move(transform_value),
                                .metadata = std::move(metadata_value),
                                .role = role_value};
    wrapped.entries = std::move(parent.entries);
    parent.entries.clear();
    state_->groups.push_back(std::move(wrapped));
    state_->require(id).entries.push_back(State::Entry{wrapped_id});
    return wrapped_id;
}
std::vector<NodeId> Builder::split_group_contents(NodeId shell_id,
                                                  std::span<NodeId const> content_sources) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    if (content_sources.empty())
        throw std::invalid_argument("Scene split needs at least one content source");

    // Prepare a complete parent map and all replacement storage before touching
    // any existing entries. This makes every validation/allocation failure
    // leave the Builder's observable tree unchanged.
    std::unordered_map<NodeId, NodeId> parents;
    parents.reserve(state_->groups.size());
    for (auto const &group : state_->groups) {
        for (auto const &entry : group.entries) {
            if (auto child = std::get_if<NodeId>(&entry.value)) {
                if (!parents.emplace(*child, group.id).second)
                    throw std::invalid_argument("Scene group has multiple parents");
            }
        }
    }
    auto *shell = state_->find(shell_id);
    if (!shell)
        throw std::out_of_range("Scene split shell does not exist");
    auto const root_child =
        std::ranges::find_if(state_->require(0).entries, [shell_id](auto const &entry) {
            auto child = std::get_if<NodeId>(&entry.value);
            return child && *child == shell_id;
        });
    if (root_child == state_->require(0).entries.end())
        throw std::invalid_argument("Scene split shell must be a direct child of root");

    auto is_descendant = [&](NodeId candidate, NodeId ancestor) {
        std::unordered_set<NodeId> visited;
        auto current = candidate;
        while (current != 0 && current != ancestor) {
            if (!visited.insert(current).second)
                throw std::invalid_argument("Scene group hierarchy contains a cycle");
            auto found = parents.find(current);
            if (found == parents.end())
                return false;
            current = found->second;
        }
        return current == ancestor;
    };
    std::unordered_set<NodeId> unique;
    unique.reserve(content_sources.size());
    for (std::size_t i = 0; i < content_sources.size(); ++i) {
        const auto id = content_sources[i];
        if (!state_->find(id))
            throw std::out_of_range("Scene split content source does not exist");
        if (id == shell_id || !is_descendant(id, shell_id))
            throw std::invalid_argument("Scene split content source must descend from its shell");
        if (!unique.insert(id).second)
            throw std::invalid_argument("Scene split content sources must be distinct");
        for (std::size_t j = 0; j < i; ++j) {
            if (is_descendant(id, content_sources[j]) || is_descendant(content_sources[j], id))
                throw std::invalid_argument("Scene split content sources cannot overlap");
        }
    }
    auto const first_id = state_->next_id;
    if (first_id == 0 || content_sources.size() - 1 > std::numeric_limits<NodeId>::max() - first_id)
        throw std::overflow_error("Scene group id space exhausted");

    std::vector<State::MutableGroup> prepared;
    prepared.reserve(content_sources.size());
    std::vector<NodeId> result;
    result.reserve(content_sources.size());
    for (std::size_t i = 0; i < content_sources.size(); ++i) {
        auto const id = i == 0 ? shell_id : first_id + i - 1;
        auto const &source = state_->require(content_sources[i]);
        State::MutableGroup copy{.id = id,
                                 .transform = shell->transform,
                                 .clip = shell->clip,
                                 .link = shell->link,
                                 .tooltip = shell->tooltip,
                                 .metadata = shell->metadata,
                                 .role = shell->role,
                                 .paint = shell->paint,
                                 .viewport = shell->viewport};
        copy.entries.reserve(source.entries.size());
        prepared.push_back(std::move(copy));
        result.push_back(id);
    }
    const auto root_index =
        static_cast<std::size_t>(root_child - state_->require(0).entries.begin());
    auto const appended_groups = content_sources.size() - 1;
    if (appended_groups > state_->groups.max_size() - state_->groups.size() ||
        appended_groups > state_->require(0).entries.max_size() - state_->require(0).entries.size())
        throw std::length_error("Scene split exceeds builder capacity");
    state_->groups.reserve(state_->groups.size() + content_sources.size() - 1);
    auto &root_entries = state_->require(0).entries;
    root_entries.reserve(root_entries.size() + content_sources.size() - 1);

    for (std::size_t i = 0; i < content_sources.size(); ++i) {
        auto &source = state_->require(content_sources[i]);
        prepared[i].entries = std::move(source.entries);
        source.entries.clear();
    }
    shell = state_->find(shell_id);
    shell->entries = std::move(prepared.front().entries);
    for (std::size_t i = 1; i < prepared.size(); ++i)
        state_->groups.push_back(std::move(prepared[i]));

    root_entries.erase(root_entries.begin() + static_cast<std::ptrdiff_t>(root_index));
    for (std::size_t i = 0; i < result.size(); ++i)
        root_entries.insert(root_entries.begin() + static_cast<std::ptrdiff_t>(root_index + i),
                            State::Entry{result[i]});
    state_->next_id = first_id + content_sources.size() - 1;
    return result;
}
void Builder::set_display_layout(DisplayLayout value) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    state_->display_layout = std::move(value);
}
DisplayLayout Builder::display_layout() const {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    return state_->display_layout;
}
void Builder::set_frame_layout(FrameLayout value) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    state_->frame_layout = std::move(value);
}
FrameLayout Builder::frame_layout() const {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    return state_->frame_layout;
}

void Builder::add_primitive(NodeId group, Primitive primitive) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    auto *owner = state_->find(group);
    if (!owner)
        throw std::out_of_range("Scene target group does not exist");
    owner->entries.push_back(State::Entry{std::move(primitive)});
}

void Builder::set_metrics(Metrics metrics) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    state_->metrics = metrics;
}

void Builder::set_font_metrics(FontMetrics metrics) {
    if (!state_)
        throw std::logic_error("Scene builder is moved from");
    state_->font_metrics = metrics;
}

Scene Builder::freeze() && {
    if (!state_)
        throw std::logic_error("Scene builder is already moved from");
    auto root = std::move(*state_).freeze_group(0);
    auto metrics = state_->metrics;
    auto font_metrics = state_->font_metrics;
    auto display_layout = state_->display_layout;
    auto frame_layout = state_->frame_layout;
    state_.reset();
    return Scene{std::move(root), metrics, font_metrics, std::move(display_layout),
                 std::move(frame_layout)};
}
} // namespace folia::math::output::scene
