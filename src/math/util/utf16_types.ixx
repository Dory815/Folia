// Root-frozen UTF-8 API / UTF-16 source indexing boundary.
export module folia.math.util.utf16_types;
import std;
export namespace folia::math::utf16 {
struct Input {
    std::u16string text;
    std::vector<std::size_t> byteOffsets;
};
struct DecodeError : std::runtime_error {
    std::size_t byteOffset;
    explicit DecodeError(std::size_t offset)
        : std::runtime_error("Invalid UTF-8 input"), byteOffset(offset) {}
};
}
