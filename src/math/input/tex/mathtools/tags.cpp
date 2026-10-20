// MathJax 4.1.3 input/tex/mathtools/MathtoolsTags.ts; Apache-2.0.
module folia.math.input.tex.mathtools.tags;
import std;
namespace folia::math::tex {
MathtoolsTags::MathtoolsTags(Formats forms) : mtFormats(std::move(forms)) {}
std::optional<TagFormatValue> MathtoolsTags::formatTag(std::u16string_view tag) const {
    if (!mtCurrent)
        return std::nullopt;
    auto const &[left, right, format] = *mtCurrent;
    auto text = format.empty() ? std::u16string(tag) : format + u"{" + std::u16string(tag) + u"}";
    return TagFormatValue(std::vector<std::u16string>{left, std::move(text), right});
}
void MathtoolsTagFormat(ParserConfiguration &configuration, TeX &jax) {
    const auto selected = jax.options.tags.value_or(u"none");
    if (selected == u"mathtoolsTags")
        return;
    auto parent = TagsFactory::creator(selected, configuration.defaultTags, configuration.tags);
    TagsFactory::add(configuration,
                     {u"mathtoolsTags",
                      [parent = std::move(parent)](ParseOptions &owner) -> std::unique_ptr<Tags> {
                          auto product = parent(owner);
                          if (!product)
                              throw std::runtime_error("Tags creator returned no product");
                          const auto &options = owner.options.mathtools;
                          auto forms = options && options->tagforms ? *options->tagforms
                                                                    : MathtoolsTags::Formats{};
                          product->setTagForm(std::make_unique<MathtoolsTags>(std::move(forms)));
                          return product;
                      }});
    jax.options.tags = u"mathtoolsTags";
}
} // namespace folia::math::tex
