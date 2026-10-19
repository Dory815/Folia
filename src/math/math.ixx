// Application-facing native TeX rendering. Source implementation owners stay
// in input/tex, output/common, output/scene and output/svg.
export module folia.math;
export import folia.math.output.scene;
import std;

export namespace folia::math {
struct RenderOptions {
    bool display=false;
    double em=16;
    double containerWidth=1280;
    std::optional<std::chrono::steady_clock::time_point> deadline;
};

struct RenderResult {
    std::string svg;
    output::scene::Scene scene;
};

class RenderError : public std::runtime_error {
public:
    std::string name;
    RenderError(std::string kind,std::string message)
        :std::runtime_error(std::move(message)),name(std::move(kind)){}
};

// One renderer owns one native session, including the Source TeX document
// state. Calls on an instance run on its owning thread. Completed results
// own their strings and immutable geometry independently of the renderer.
class Renderer final {
    struct OwnerCounts {
        std::size_t contextObjectSlots=0,contextSymbolSlots=0,mmlFactoryProducts=0;
        std::size_t parserRegistryEntries=0,wrapperHandles=0,wrapperRetainedJobs=0;
        std::size_t sceneHandles=0,liteRecords=0,liteNodeIdentities=0;
    };
    struct State;
    std::unique_ptr<State> state_;
    friend struct RendererDiagnosticsProbe;
    OwnerCounts diagnosticsForTest()const;
public:
    Renderer();
    ~Renderer();
    Renderer(const Renderer&)=delete;
    Renderer& operator=(const Renderer&)=delete;
    RenderResult render(std::u16string_view,RenderOptions={});
    RenderResult render(std::string_view,RenderOptions={});
    RenderResult render(std::u16string_view,std::stop_token,RenderOptions);
    RenderResult render(std::string_view,std::stop_token,RenderOptions);
    // Runs the identical CommonWrapper/BBox and Source SVG-placement producer,
    // returning its owned frozen geometry without SVG DOM materialization or
    // XML serialization.
    output::scene::Scene renderScene(std::u16string_view,RenderOptions={});
    output::scene::Scene renderScene(std::string_view,RenderOptions={});
    output::scene::Scene renderScene(std::u16string_view,std::stop_token,RenderOptions);
    output::scene::Scene renderScene(std::string_view,std::stop_token,RenderOptions);
    std::string toSvg(std::u16string_view,RenderOptions={});
    std::string toSvg(std::string_view,RenderOptions={});
    std::string toSvg(std::u16string_view,std::stop_token,RenderOptions);
    std::string toSvg(std::string_view,std::stop_token,RenderOptions);
};

// Fixed diagnostic output for the opt-in renderer owner-count candidate.
// Implemented in the owning module; callers cannot provide code to execute
// against Renderer state or obtain native handles.
struct RendererDiagnosticsProbe {
    static void print(const Renderer&,std::string_view stage);
    static std::array<std::size_t,4> contextRecordCounts(const Renderer&);
    static std::array<std::size_t,9> snapshot(const Renderer&);
    static std::array<std::size_t,24> parserIdentitySnapshot(const Renderer&);
    static std::array<std::size_t,3> sceneOwnerSnapshot(const Renderer&);
    static std::vector<std::pair<std::uint64_t,int>> liteNodeIdentitySnapshot(const Renderer&);
    static std::vector<std::tuple<std::uint64_t,int,std::string>> liteNodeIdentityNamedSnapshot(const Renderer&);
};
}
