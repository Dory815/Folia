export module folia.math.input.tex.tex_error;
import std;

export namespace folia::math::tex {
class TexError final : public std::exception {
    std::string id_;
    std::u16string message_;
    std::string what_;
public:
    TexError(std::string id, std::u16string format,
             std::span<std::u16string const> arguments = {});
    std::string_view id() const noexcept { return id_; }
    std::u16string_view message() const noexcept { return message_; }
    char const* what() const noexcept override { return what_.c_str(); }
};
}
