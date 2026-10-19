#include "support/folia_test.hpp"
import folia.math;

namespace {
using namespace boost::ut;
using namespace folia::math;

suite mathjax_full_profile_package_bootstrap=[] {
  "renderer_initializes_the_real_38_package_tex_profile"_test=[] {
    // Renderer::State uses the production NewCM profile and its exact 38-package list.
    // Construction creates the real SVG service graph and native TeX document.
    Renderer renderer;
    const auto rendered=renderer.render("x");
    expect(!rendered.svg.empty());
  };
};
} // namespace
