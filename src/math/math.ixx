// Application-facing TeX renderer contract composed by the original math owner.
export module folia.math;
export import folia.math.output.scene;
import std;

export namespace folia::math {
struct RenderOptions {
    bool display = false;
    double em = 16;
    double containerWidth = 1280;
    std::optional<std::chrono::steady_clock::time_point> deadline;
};

struct RenderResult {
    std::string svg;
    output::scene::Scene scene;
};

class RenderError : public std::runtime_error {
  public:
    std::string name;
    RenderError(std::string kind, std::string message)
        : std::runtime_error(std::move(message)), name(std::move(kind)) {}
};

// One renderer owns one configured TeX session. Calls must use its creating
// thread. Results own their strings and immutable Scene values independently.
class Renderer final {
    struct State;
    std::unique_ptr<State> state_;

  public:
    Renderer();
    ~Renderer();
    Renderer(Renderer const &) = delete;
    Renderer &operator=(Renderer const &) = delete;
    RenderResult render(std::u16string_view, RenderOptions = {});
    RenderResult render(std::string_view, RenderOptions = {});
    RenderResult render(std::u16string_view, std::stop_token, RenderOptions);
    RenderResult render(std::string_view, std::stop_token, RenderOptions);
    output::scene::Scene renderScene(std::u16string_view, RenderOptions = {});
    output::scene::Scene renderScene(std::string_view, RenderOptions = {});
    output::scene::Scene renderScene(std::u16string_view, std::stop_token, RenderOptions);
    output::scene::Scene renderScene(std::string_view, std::stop_token, RenderOptions);
    std::string toSvg(std::u16string_view, RenderOptions = {});
    std::string toSvg(std::string_view, RenderOptions = {});
    std::string toSvg(std::u16string_view, std::stop_token, RenderOptions);
    std::string toSvg(std::string_view, std::stop_token, RenderOptions);
};
} // namespace folia::math
