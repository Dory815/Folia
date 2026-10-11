// Root-frozen ordinary array slice binding, no parser/map algorithm.
module folia.math.core.tree.property;
import std;
namespace folia::math {
PropertyArray PropertyArray::slice(std::size_t)const{throw options::Exception("TypeError",u"P22a not implemented");}
}
