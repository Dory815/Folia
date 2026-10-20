export module folia.math.output.common:menclose;
export import :wrapper;
export import :msqrt;
export import :notation;
export import folia.math.core.mml_tree.mml_nodes.menclose;
import std;

export namespace folia::math::output::common {
class CommonMenclose : public CommonWrapper {
    std::span<NotationSpec const> catalog_;
    std::vector<NotationSpec const *> notations_;
    std::optional<NotationKind> renderChild_;
    CommonMsqrt *msqrt_{};
    PaddingData trbl_{};
    [[nodiscard]] BBox const *childBoxFor(NotationSpec const &) const;
    void removeNotation(std::string_view source_name);

  public:
    static constexpr double defaultPadding = .2;
    static constexpr double defaultThickness = .067;
    static constexpr Arrowhead defaultArrowhead{4, 2, 1};

    double padding{defaultPadding};
    double thickness{defaultThickness};
    Arrowhead arrowhead{defaultArrowhead};

    CommonMenclose(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *,
                   std::span<NotationSpec const> profileCatalog);
    void initializeSource() override;
    SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;
    void getParameters();
    void getNotations();
    void removeRedundantNotations();
    void initializeNotations();
    [[nodiscard]] PaddingData getBBoxExtenders() const;
    [[nodiscard]] PaddingData getPadding() const;
    static void maximizeEntries(PaddingData &, PaddingData const &) noexcept;
    [[nodiscard]] double getOffset(std::string_view direction) const noexcept;
    [[nodiscard]] std::pair<double, double> getArgMod(double width, double height) const noexcept;
    [[nodiscard]] ArrowGeometry arrow(double width, double angle, bool doubleHeaded,
                                      std::string_view offset = {}, double distance = 0);
    [[nodiscard]] ArrowMetrics arrowData() const;
    [[nodiscard]] std::pair<double, double> arrowAW() const;
    [[nodiscard]] CommonMsqrt &createMsqrt(CommonWrapper &child);
    [[nodiscard]] PaddingData sqrtTRBL() const;

    [[nodiscard]] std::span<NotationSpec const *const> notations() const noexcept {
        return notations_;
    }

    [[nodiscard]] std::optional<NotationKind> renderChild() const noexcept {
        return renderChild_;
    }

    [[nodiscard]] CommonMsqrt *msqrt() const noexcept {
        return msqrt_;
    }

    // Common returns backend-neutral Scene path geometry in the common
    // wrapper's completed local box; render-child placement remains traversal.
    [[nodiscard]] std::vector<NotationGeometry> sceneGeometry();

  protected:
    void computeBBox(BBox &, bool recompute = false) override;
};
} // namespace folia::math::output::common
