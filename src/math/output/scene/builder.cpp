module folia.math.output.scene.builder;
import std;

namespace folia::math::output::scene {
struct Builder::State {
    struct Entry { std::variant<Primitive, NodeId> value; };
    struct MutableGroup {
        NodeId id{};
        Transform transform{};
        std::optional<Clip> clip;
        std::optional<Link> link;
        std::optional<Tooltip> tooltip;
        SemanticMetadata metadata;
        std::vector<Entry> entries;
    };
    std::vector<MutableGroup> groups;
    Metrics metrics{};
    FontMetrics font_metrics{};
    NodeId next_id{1};
    State() { groups.push_back(MutableGroup{.id=0}); }
    MutableGroup* find(NodeId id) noexcept {
        for (auto& group : groups) if (group.id == id) return &group;
        return nullptr;
    }
    Group freeze_group(NodeId id) && {
        auto* source = find(id);
        if (!source) throw std::logic_error("invalid group in Scene builder");
        Group out;
        out.id = source->id;
        out.transform = source->transform;
        out.clip = std::move(source->clip);
        out.link = std::move(source->link);
        out.tooltip = std::move(source->tooltip);
        out.metadata = std::move(source->metadata);
        for (auto& entry : source->entries) {
            if (auto* primitive = std::get_if<Primitive>(&entry.value))
                out.entries.emplace_back(std::move(*primitive));
            else {
                auto child = std::make_shared<Group>(std::move(*this).freeze_group(std::get<NodeId>(entry.value)));
                out.entries.emplace_back(std::shared_ptr<Group const>{std::move(child)});
            }
        }
        return out;
    }
};

Builder::Builder() : state_(std::make_unique<State>()) {}
Builder::~Builder() = default;
Builder::Builder(Builder&&) noexcept = default;
Builder& Builder::operator=(Builder&&) noexcept = default;

NodeId Builder::add_group(NodeId parent, Transform transform, std::optional<Clip> clip,
    std::optional<Link> link, std::optional<Tooltip> tooltip, SemanticMetadata metadata) {
    if (!state_) throw std::logic_error("Scene builder is moved from");
    auto* owner = state_->find(parent);
    if (!owner) throw std::out_of_range("Scene parent group does not exist");
    if (state_->next_id == 0) throw std::overflow_error("Scene group id space exhausted");
    auto const id = state_->next_id++;
    state_->groups.push_back(State::MutableGroup{.id=id, .transform=transform,
        .clip=std::move(clip), .link=std::move(link), .tooltip=std::move(tooltip),
        .metadata=std::move(metadata)});
    owner = state_->find(parent); // vector growth can invalidate the previous pointer
    owner->entries.push_back(State::Entry{NodeId{id}});
    return id;
}

void Builder::add_primitive(NodeId group, Primitive primitive) {
    if (!state_) throw std::logic_error("Scene builder is moved from");
    auto* owner = state_->find(group);
    if (!owner) throw std::out_of_range("Scene target group does not exist");
    owner->entries.push_back(State::Entry{std::move(primitive)});
}

void Builder::set_metrics(Metrics metrics) {
    if (!state_) throw std::logic_error("Scene builder is moved from");
    state_->metrics = metrics;
}

void Builder::set_font_metrics(FontMetrics metrics) {
    if (!state_) throw std::logic_error("Scene builder is moved from");
    state_->font_metrics = metrics;
}

Scene Builder::freeze() && {
    if (!state_) throw std::logic_error("Scene builder is already moved from");
    auto root = std::move(*state_).freeze_group(0);
    auto metrics = state_->metrics;
    auto font_metrics = state_->font_metrics;
    state_.reset();
    return Scene{std::move(root), metrics, font_metrics};
}
}
