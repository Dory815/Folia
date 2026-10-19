module folia.math.adaptors.lite.list;
import std;
namespace folia::math::adaptors::lite {
LiteList::LiteList(PropertyArray nodes, std::shared_ptr<LiteNodeStore> store) {
    if(!store)store=LiteNodeStore::create();
    ref_ = store->create(NodeKind::List, u"#list");
    auto& list = store->record(ref_).listNodes;
    for(std::size_t i=0;i<nodes.size();++i)list.push(nodes.get(i));
}
LiteList::LiteList(LiteNodeRef ref) : ref_(std::move(ref)) {
    if (!ref_ || recordConst(ref_).nodeKind != NodeKind::List) throw std::invalid_argument("not a LiteList");
}
PropertyArray LiteList::nodes() const { return recordConst(ref_).listNodes; }
void LiteList::nodes(PropertyArray value) { record(ref_).listNodes=std::move(value); }
void LiteList::append(Property node) { record(ref_).listNodes.push(std::move(node)); }
std::size_t LiteList::size() const { return recordConst(ref_).listNodes.size(); }
LiteNodeValue LiteList::at(std::size_t index) const {
    const auto& ids = recordConst(ref_).listNodes;
    return index < ids.size() ? propertyNode(ids.get(index)) : LiteNodeValue(Missing{});
}
[[noreturn]] void LiteList::next() const { throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'length')"); }
}
