// One Source-ordered MML constructor initializer for native and Context views.
// This module depends only on the primitive receiver protocol; it owns no
// node fields and does not select or cache a factory, class, or attribute.
export module folia.math.core.mml_tree.mml_node_initialization;
export import folia.math.core.mml_tree.mml_node_algorithms;
import std;

export namespace folia::math::mml {
template<class Product,class Dynamic>
void initializeMmlNode(MmlNodeReceiver<Product,Dynamic>& receiver,
                       Dynamic rawFactory,
                       Dynamic rawAttributes,
                       Dynamic rawChildren){
    using Child=typename MmlNodeReceiver<Product,Dynamic>::Child;
    const Child self=receiver.self();
    const Dynamic rawSelf=receiver.toDynamic(self);

    receiver.set(rawSelf,u"prevClass",receiver.nullValue());
    receiver.set(rawSelf,u"prevLevel",receiver.nullValue());
    receiver.set(rawSelf,u"texclass",receiver.nullValue());

    if(receiver.number(receiver.get(rawSelf,u"arity"))<0){
        // The constructor uses its lexical `factory` argument, even if a
        // receiver accessor makes this.factory return another value.
        const Dynamic create=receiver.get(rawFactory,u"create");
        const std::array<Dynamic,1> createArgs{receiver.string(u"inferredMrow")};
        const Dynamic inferred=receiver.invoke(create,rawFactory,createArgs);
        receiver.set(rawSelf,u"childNodes",receiver.arrayOfOne(inferred));
        const Dynamic currentChildren=receiver.get(rawSelf,u"childNodes");
        const Dynamic currentChild=receiver.getIndex(currentChildren,0);
        receiver.set(currentChild,u"parent",rawSelf);
    }

    const Dynamic setChildren=receiver.get(rawSelf,u"setChildren");
    const std::array<Dynamic,1> childrenArgs{rawChildren};
    (void)receiver.invoke(setChildren,rawSelf,childrenArgs);

    const Dynamic getNodeClass=receiver.get(rawFactory,u"getNodeClass");
    const Dynamic kind=receiver.get(rawSelf,u"kind");
    const std::array<Dynamic,1> kindArgs{kind};
    const Dynamic nodeClass=receiver.invoke(getNodeClass,rawFactory,kindArgs);
    const Dynamic defaults=receiver.get(nodeClass,u"defaults");

    const Dynamic getMathClass=receiver.get(rawFactory,u"getNodeClass");
    const std::array<Dynamic,1> mathArgs{receiver.string(u"math")};
    const Dynamic mathClass=receiver.invoke(getMathClass,rawFactory,mathArgs);
    const Dynamic mathDefaults=receiver.get(mathClass,u"defaults");

    const Dynamic attributes=receiver.constructAttributes(defaults,mathDefaults);
    receiver.set(rawSelf,u"attributes",attributes);
    const Dynamic currentAttributes=receiver.get(rawSelf,u"attributes");
    const Dynamic setList=receiver.get(currentAttributes,u"setList");
    const std::array<Dynamic,1> attributesArgs{rawAttributes};
    (void)receiver.invoke(setList,currentAttributes,attributesArgs);
}
}
