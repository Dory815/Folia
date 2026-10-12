// Portable NFD host body; Root freezes data and original observations first.
module folia.math.util.unicode_normalization;
import folia.math.util.unicode_normalization_data;
import folia.math.util.option_types;
import std;
namespace folia::math {
std::u16string Unicode16Normalization::nfd(std::u16string_view){
    throw options::Exception("Unimplemented",u"Unicode NFD host");
}
}
