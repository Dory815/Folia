export module folia.math.output.common:xml_node;
export import :wrapper;
import folia.math.core.mml_tree.html_adaptor;
import folia.math.core.dom_adaptor;
import std;

export namespace folia::math::output::common {
class CommonXmlNode : public CommonWrapper {
public:
  double rscale = 1;
  CommonXmlNode(CommonWrapperFactory &, mml::MmlNode &, CommonWrapper *parent = nullptr);
  void initializeSource() override;
  std::optional<std::u16string> getHDW(DOMNodeRef, std::string_view use,
                                       std::optional<std::string_view> force = {});
  UnknownBBox splitHDW(std::u16string_view) const;
  styles::StyleList getFontStyles() const;
  DOMNodeRef getHTML();
  virtual DOMNodeRef addHDW(DOMNodeRef, styles::StyleList &) = 0;
  UnknownBBox measureXmlNode(DOMNodeRef);
  void getStyles() override;
  void getScale() override;
  void getVariant() override;

protected:
  DOMNodeRef cloneXML() const;
  void computeBBox(BBox &, bool recompute = false) override;
};
}
