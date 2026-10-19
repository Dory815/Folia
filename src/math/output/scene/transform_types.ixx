// Ordered backend-independent transforms in MathJax mathematical units.
export module folia.math.output.scene.foundation:transform_types;
import std;

export namespace folia::math::output::scene {
enum class TransformKind {Translate,Scale,Rotate,Matrix};
struct TransformOperation {
    TransformKind kind=TransformKind::Translate;
    double x=0,y=0;
    double degrees=0,centerX=0,centerY=0;
    double a=1,b=0,c=0,d=1,e=0,f=0;
    // Source serialization choices; the numeric transform is unchanged.
    bool scalarScale=false,spaceSeparated=false;
    unsigned scaleDigits=3;
    bool commaSpaceSeparated=false;
};
struct Transform {
    double translateX=0,translateY=0,scaleX=1,scaleY=1;
    // The order and full-precision operands are directly consumed by renderers.
    std::vector<TransformOperation> operations;
};
}
