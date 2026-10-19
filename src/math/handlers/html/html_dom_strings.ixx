/*************************************************************
 * Native port of MathJax 4.1.3 handlers/html/HTMLDomStrings.ts.
 *************************************************************/
export module folia.math.handlers.html.html_dom_strings;
export import folia.math.core.dom_adaptor;
import folia.math.util.option_types;
import folia.math.util.object_types;
import folia.math.util.context_property_bindings;
import folia.math.util.runtime_modules;
import folia.math.util.regexp_host;
import folia.math.util.regexp_binding;
import std;

export namespace folia::math::handlers::html {
using HTMLNodeList=std::vector<std::pair<DOMNodeRef,std::size_t>>;
struct HTMLDomStringsResult {std::vector<std::u16string> strings;std::vector<HTMLNodeList> nodes;};

class HTMLDomStrings final:public NativeObject,public std::enable_shared_from_this<HTMLDomStrings> {
    options::Context* context_;
    ContextPropertyBindings* properties_;
    regexp_host::RegExpBindings* regexp_;
    options::ObjectRef identity_;
    std::shared_ptr<AbstractDOMAdaptor> adaptorOwner_;
    void init();
    void getPatterns();
    void pushString();
    void extendString(DOMNodeRef,options::Value);
    DOMNodeValue handleText(DOMNodeRef,bool);
    DOMNodeValue handleTag(DOMNodeRef,bool);
    std::pair<DOMNodeValue,bool> handleContainer(DOMNodeRef,bool);
    DOMNodeValue handleOther(DOMNodeRef,bool);
    options::Value findSource(DOMNodeRef);
    HTMLDomStrings(options::Context&,ContextPropertyBindings&,regexp_host::RegExpBindings&,
        options::ObjectRef);
    friend class HTMLDomStringsModule;
public:
    options::ObjectRef identity()const{return identity_;}
    ContextPropertyBindings& properties()const{return *properties_;}
    options::ObjectRef optionsIdentity()const;
    options::Value adaptorIdentity()const;
    options::Value optionsValue()const;
    void setAdaptor(options::Value identity,std::shared_ptr<AbstractDOMAdaptor> owner);
    HTMLDomStringsResult find(DOMNodeRef);
};

class HTMLDomStringsModule final:public NativeObject,public std::enable_shared_from_this<HTMLDomStringsModule> {
    options::Context* context_;
    ContextPropertyBindings* properties_;
    regexp_host::RegExpBindings* regexp_;
    options::ObjectRef identity_;
    explicit HTMLDomStringsModule(options::Context&,ContextPropertyBindings&,
        regexp_host::RegExpBindings&);
    void initialize();
public:
    static std::shared_ptr<HTMLDomStringsModule> initializeModule(util::RuntimeModules&,
        ContextPropertyBindings&,regexp_host::RegExpBindings&);
    options::Context& context()const{return *context_;}
    options::ObjectRef identity()const{return identity_;}
    std::shared_ptr<HTMLDomStrings> create(options::Value suppliedOptions=options::Undefined{});
    void installTexHtmlHook();
};
}
