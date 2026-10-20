/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/msubsup.ts.
 *************************************************************/
export module folia.math.output.common:msubsup;
export import :scriptbase;
import std;

export namespace folia::math::output::common {
class CommonMsub : public CommonScriptbase {
  public:
    inline static constexpr bool useIC = false;
    CommonMsub(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr,
               std::optional<std::size_t> scriptChildIndex = {});
    CommonWrapper *scriptChild() const override;
    std::array<double, 2> getOffset() const override;
};

class CommonMsup : public CommonScriptbase {
  public:
    inline static constexpr bool useIC = true;
    CommonMsup(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr,
               std::optional<std::size_t> scriptChildIndex = {});
    CommonWrapper *scriptChild() const override;
    std::array<double, 2> getOffset() const override;
};

class CommonMsubsup : public CommonScriptbase {
  public:
    inline static constexpr bool useIC = false;
    std::optional<std::array<double, 3>> UVQ;
    CommonMsubsup(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr,
                  std::optional<std::size_t> scriptChildIndex = {});
    SceneNodes emitScene(scene::Builder &, std::span<scene::NodeId const> parents) override;
    virtual CommonWrapper *subChild() const;
    virtual CommonWrapper *supChild() const;
    CommonWrapper *scriptChild() const override;
    virtual std::array<double, 3> getUVQ(const BBox *subbox = nullptr,
                                         const BBox *supbox = nullptr);
    BBox appendScripts(BBox &) const override;
};
} // namespace folia::math::output::common
