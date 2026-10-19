export module folia.math.adaptors.lite.parser;
export import folia.math.core.dom_adaptor;
export import folia.math.adaptors.lite.node_store;
import folia.math.util.options;
import folia.math.util.entities;
import std;
export namespace folia::math::adaptors::lite {
class LiteParser {
    // Reads the live static value through this parser instance's actual
    // `constructor` property.  The Context owner supplies the lookup so
    // writable/replaced Source statics remain authoritative.
    std::function<Property(std::u16string_view)> staticProperty_;
    std::function<Property(const Property&, std::u16string_view)> staticElement_;
    std::function<bool(const Property&, std::u16string_view)> staticHasOwn_;
    std::shared_ptr<entities::EntityModule> entities_;
    std::function<options::Value(std::u16string_view,std::span<const options::Value>)> methodCall_;
    std::function<options::Value(std::u16string_view)> methodValue_;
    std::function<options::Value(options::Value,std::u16string_view)> propertyGet_;
    std::function<void(options::Value,std::u16string_view,options::Value)> propertySet_;
    std::function<options::Value(options::Value,std::span<const options::Value>)> parserCall_;
    std::function<options::Value(AbstractDOMAdaptor&,std::u16string_view)> adaptorMethod_;
    std::function<options::Value(options::Value,options::Value,std::span<const options::Value>)> functionCall_;
    std::function<options::Value(std::function<options::Value(options::Value)>)> callback_;
    std::function<options::Value(NodeKind,std::span<const options::Value>)> constructNode_;
    std::function<options::Value(const Property&)> valueFromProperty_;
    std::function<std::u16string(const options::Value&)> stringValue_;
    std::function<options::Value(AbstractDOMAdaptor&)> adaptorValue_;
    std::function<options::Value(LiteNodeRef)> nodeValue_;
    std::function<options::Value(const PropertyArray&)> arrayValue_;
    std::function<PropertyArray(options::Value)> arrayFromValue_;
    std::function<Property(options::Value)> propertyFromValue_;
    std::function<LiteNodeRef(options::Value)> nodeFromValue_;
    std::function<void(options::Value,const std::function<void(options::Value)>&)> forEachIterable_;
    std::function<bool(options::Value,NodeKind)> instanceOf_;
public:
    explicit LiteParser(std::shared_ptr<entities::EntityModule> entities = {});
    void staticPropertyReader(std::function<Property(std::u16string_view)> reader,
                              std::function<Property(const Property&, std::u16string_view)> element,
                              std::function<bool(const Property&, std::u16string_view)> hasOwn);
    // Context-bound Source call bridge. Internal calls consult the receiver's
    // current method property and pass the actual node/array identities.
    void contextMethodBridge(
        std::function<options::Value(std::u16string_view,std::span<const options::Value>)> call,
        std::function<options::Value(std::u16string_view)> getMethod,
        std::function<options::Value(options::Value,std::u16string_view)> getProperty,
        std::function<void(options::Value,std::u16string_view,options::Value)> setProperty,
        std::function<options::Value(options::Value,std::span<const options::Value>)> callMethod,
        std::function<options::Value(AbstractDOMAdaptor&,std::u16string_view)> getAdaptorMethod,
        std::function<options::Value(options::Value,options::Value,std::span<const options::Value>)> callFunction,
        std::function<options::Value(std::function<options::Value(options::Value)>)> makeCallback,
        std::function<options::Value(NodeKind,std::span<const options::Value>)> constructNode,
        std::function<options::Value(const Property&)> valueFromProperty,
        std::function<std::u16string(const options::Value&)> stringValue,
        std::function<options::Value(AbstractDOMAdaptor&)> adaptorValue,
        std::function<options::Value(LiteNodeRef)> nodeValue,
        std::function<options::Value(const PropertyArray&)> arrayValue,
        std::function<PropertyArray(options::Value)> arrayFromValue,
        std::function<Property(options::Value)> propertyFromValue,
        std::function<LiteNodeRef(options::Value)> nodeFromValue,
        std::function<void(options::Value,const std::function<void(options::Value)>&)> forEachIterable,
        std::function<bool(options::Value,NodeKind)> instanceOf);
    LiteNodeRef parseFromString(std::u16string_view text, std::u16string_view format, AbstractDOMAdaptor& adaptor);
    LiteNodeRef addText(AbstractDOMAdaptor& adaptor, LiteNodeRef node, std::u16string_view text);
    LiteNodeRef addComment(AbstractDOMAdaptor& adaptor, LiteNodeRef node, std::u16string_view comment);
    LiteNodeRef closeTag(AbstractDOMAdaptor& adaptor, LiteNodeRef node, std::u16string_view tag);
    LiteNodeRef openTag(AbstractDOMAdaptor& adaptor, LiteNodeRef node, std::u16string_view tag, PropertyArray& parts);
    void addAttributes(AbstractDOMAdaptor& adaptor, LiteNodeRef node, PropertyArray& attributes);
    void handlePCDATA(AbstractDOMAdaptor& adaptor, LiteNodeRef node, std::u16string_view kind, PropertyArray& parts);
    void checkDocument(AbstractDOMAdaptor& adaptor, LiteNodeRef document);
    LiteNodeRef getOnlyChild(AbstractDOMAdaptor& adaptor, LiteNodeRef body);
    std::vector<AttributeData> allAttributes(AbstractDOMAdaptor& adaptor, LiteNodeRef node, bool xml=false);
    std::u16string serialize(AbstractDOMAdaptor& adaptor, LiteNodeRef node, bool xml=false);
    std::u16string serializeInner(AbstractDOMAdaptor& adaptor, LiteNodeRef node, bool xml=false);
    std::u16string protectHTML(std::u16string_view text) const;
    std::u16string protectAttribute(std::u16string_view text, bool xml=false) const;
};
}
