module folia.math.adaptors.lite_adaptor;
import :window;
import :document;
import std;

namespace folia::math::adaptors::lite {
LiteWindowRef LiteWindow::create() {
    auto window = LiteWindowRef(new LiteWindow());
    window->document_ = LiteDocument::create(window);
    return window;
}
}
