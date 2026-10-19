/*************************************************************
 * Source call adapters for the CommonMaction constructor and action call.
 * Kept separate from the registered Common wrapper until the complete SVG
 * action Map and its instance methods are installed as one packet.
 *************************************************************/
export module folia.math.output.common.maction_source;
export import folia.math.output.common;
import std;

export namespace folia::math::output::common {
void initializeCommonMactionSource(CommonMaction& wrapper);
options::Value invokeCommonMactionAction(CommonMaction& wrapper);
}
