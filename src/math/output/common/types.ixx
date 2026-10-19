/*************************************************************
 * Common output class graph forward declarations.
 * MathJax 4.1.3 ts/output/common.
 * This partition exists only to break the Source class dependency cycle;
 * it stores no runtime state and owns no algorithms.
 *************************************************************/
export module folia.math.output.common:types;
export import folia.math.core.tree.factory_types;
export import folia.math.core.mml_tree.mml_node;
import std;

export namespace folia::math::output::common {
class CommonOutputJax;
class CommonWrapper;
class CommonWrapperFactory;
class Linebreaks;
class LayoutJob;

struct ExtendedMetrics {
    double em = 16;
    double ex = 8;
    double containerWidth = 1280;
    double scale = 1;
    // Source MathItem.metrics.family is an arbitrary JS value. Keep it raw
    // until the font-style boundary applies Source truthiness and coercion.
    Property family = Missing{};
    bool display = false;
};

using CommonWrapperClassRef = tree::FactoryClassRef<CommonWrapper>;
using CommonWrapperClassMap = tree::FactoryClassMap<CommonWrapper>;
}
