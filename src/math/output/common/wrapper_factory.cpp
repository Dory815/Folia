// Native Common factory ownership and source initialization.
// Source responsibilities: MathJax 4.1.3 output/common/WrapperFactory.ts
// and core/Tree/WrapperFactory.ts.
module folia.math.output.common;
import :wrapper_factory;

import :output_jax;
import std;

namespace folia::math::output::common {
namespace {

void requireRegistration(CommonWrapperRegistration const &value) {
    if (!value.creator.construct || !value.creator.isInstance) {
        throw std::invalid_argument(
            "Common wrapper registration requires constructor and type predicate");
    }
}

} // namespace

CommonWrapperClassData CommonWrapperClassData::commonDefaults() {
    CommonWrapperClassData data;

    // output/common/Wrapper.ts: CommonWrapperClassData defaults.
    // Keep the nested maps as value data; no live Source class objects or
    // callable projections are retained in the native factory.
    data.boldVariants = {{"bold",
                          {{"normal", "bold"},
                           {"italic", "bold-italic"},
                           {"fraktur", "bold-fraktur"},
                           {"script", "bold-script"},
                           {"sans-serif", "bold-sans-serif"},
                           {"sans-serif-italic", "sans-serif-bold-italic"}}},
                         {"normal",
                          {{"bold", "normal"},
                           {"bold-italic", "italic"},
                           {"bold-fraktur", "fraktur"},
                           {"bold-script", "script"},
                           {"bold-sans-serif", "sans-serif"},
                           {"sans-serif-bold-italic", "sans-serif-italic"}}}};
    data.italicVariants = {{"italic",
                            {{"normal", "italic"},
                             {"bold", "bold-italic"},
                             {"sans-serif", "sans-serif-italic"},
                             {"bold-sans-serif", "sans-serif-bold-italic"}}},
                           {"normal",
                            {{"italic", "normal"},
                             {"bold-italic", "bold"},
                             {"sans-serif-italic", "sans-serif"},
                             {"sans-serif-bold-italic", "bold-sans-serif"}}}};
    return data;
}

CommonWrapperFactory::CommonWrapperFactory(CommonOutputJax &jax, CommonWrapperCatalog catalog)
    : Base(), jax_(jax) {
    classData_.reserve(catalog.size());
    for (auto &entry : catalog) {
        setWrapperClass(std::move(entry));
    }
}

WrapperBuildArguments CommonWrapperFactory::creationArguments(mml::MmlNode &node,
                                                              CommonWrapper *parent) {
    return {*this, node, parent};
}

CommonWrapper &CommonWrapperFactory::adopt(std::unique_ptr<CommonWrapper> product) {
    if (!product) {
        throw std::invalid_argument("Cannot adopt an empty CommonWrapper");
    }
    if (&product->factory() != this) {
        throw std::invalid_argument("CommonWrapper belongs to another factory");
    }
    if (&product->node->document() != &jax_.document()) {
        throw std::invalid_argument("CommonWrapper MML node belongs to another document");
    }

    CommonWrapper *const result = product.get();
    wrappers_.push_back(std::move(product));
    // Source initialization is deliberately after ownership transfer and
    // exactly once per factory-created wrapper. Tree/index linkage remains in
    // CommonWrapper's Source-aligned wrap methods, not in factory dispatch.
    result->initializeSource();
    return *result;
}

CommonOutputJax &CommonWrapperFactory::jax() const noexcept {
    return jax_;
}

CommonWrapperCatalog CommonWrapperFactory::defaultNodes() {
    // MathJax CommonWrapperFactory intentionally has no default kind table;
    // concrete SVG/CHTML classes are supplied by their profile constructors.
    return {};
}

std::vector<mml::NodeKind> CommonWrapperFactory::getKinds() const {
    return Base::getKinds();
}

std::optional<tree::FactoryClass<CommonWrapper, WrapperBuildArguments>>
CommonWrapperFactory::getWrapperClass(mml::NodeKind kind) const {
    return Base::getNodeClass(kind);
}

void CommonWrapperFactory::setWrapperClass(CommonWrapperRegistration value) {
    requireRegistration(value);
    const auto found = std::find_if(classData_.begin(), classData_.end(),
                                    [&](auto const &entry) { return entry.first == value.kind; });
    const bool insertMetadata = found == classData_.end();
    const auto metadataIndex =
        insertMetadata ? classData_.size() : static_cast<std::size_t>(found - classData_.begin());
    if (insertMetadata) {
        classData_.reserve(classData_.size() + 1);
    }

    // The inherited Tree registry remains the only creator storage. Style
    // metadata is a separate value record, not a copied creator object.
    Base::setNodeClass(value.kind, std::move(value.creator));
    if (insertMetadata) {
        classData_.emplace_back(value.kind, std::move(value.data));
    } else {
        classData_[metadataIndex].second = std::move(value.data);
    }
}

CommonWrapperClassData const &CommonWrapperFactory::classData(mml::NodeKind kind) const {
    const auto found = std::find_if(classData_.begin(), classData_.end(),
                                    [&](auto const &entry) { return entry.first == kind; });
    if (found == classData_.end()) {
        throw std::out_of_range("No Common wrapper class data for MML kind");
    }
    return found->second;
}

std::size_t CommonWrapperFactory::wrapperCount() const noexcept {
    return wrappers_.size();
}

} // namespace folia::math::output::common
