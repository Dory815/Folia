module folia.math.input.tex.environment;
import std;

namespace folia::math::tex {
void overlayEnvironment(TexEnvironment& destination,
                        TexEnvironment const& source) {
#define FOLIA_COPY_PRESENT(field) \
    if (source.field.has_value()) destination.field = source.field
    FOLIA_COPY_PRESENT(display);
    FOLIA_COPY_PRESENT(in_root);
    FOLIA_COPY_PRESENT(operator_letters);
    FOLIA_COPY_PRESENT(is_global);
    FOLIA_COPY_PRESENT(bold_symbol);
    FOLIA_COPY_PRESENT(vector_star);
    FOLIA_COPY_PRESENT(script_level);
    FOLIA_COPY_PRESENT(font);
    FOLIA_COPY_PRESENT(italic_font);
    FOLIA_COPY_PRESENT(style);
    FOLIA_COPY_PRESENT(size);
    FOLIA_COPY_PRESENT(color);
    FOLIA_COPY_PRESENT(horizontal_size);
    FOLIA_COPY_PRESENT(closing_environment);
    FOLIA_COPY_PRESENT(cd_min_width);
    FOLIA_COPY_PRESENT(cd_min_height);
    FOLIA_COPY_PRESENT(vector_font);
    FOLIA_COPY_PRESENT(math_variant);
    FOLIA_COPY_PRESENT(math_size);
    FOLIA_COPY_PRESENT(math_color);
#undef FOLIA_COPY_PRESENT
}

ResolvedParserEnvironmentSeed resolveParserEnvironmentSeed(
    ParserEnvironmentSeed seed) {
    return {std::move(seed.environment), seed.is_inner.value_or(true)};
}
}

