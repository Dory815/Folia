// MathJax 4.1.3 output/common/Wrappers/TextNode.ts.
export module folia.math.output.common:text_node;
export import :wrapper;
import std;

export namespace folia::math::output::common {
class CommonTextNode : public CommonWrapper {
  public:
    using CommonWrapper::CommonWrapper;
    std::vector<std::uint32_t> remappedText(std::u16string_view, std::string_view variant) const;

  protected:
    void computeBBox(BBox &, bool recompute = false) override;

    void getStyles() override {
    }

    void getVariant() override {
    }

    void getScale() override {
    }

    void getSpace() override {
    }

  private:
    static void addMeasuredText(BBox &, std::u16string_view, std::string_view, CommonOutputJax &);
    static void updateBBox(BBox &, double height, double depth, double width);
};
} // namespace folia::math::output::common
