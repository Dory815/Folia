// Native reference conversion for the same Context graph and real RegExp host.
// No delimiter, parser, or pattern algorithm belongs in this module.
export module folia.math.util.regexp_binding;
export import folia.math.util.regexp_host;
export import folia.math.util.native_identities;
import std;
export namespace folia::math::regexp_host {
class RegExpBindings {
    options::Context& context_;
    options::ObjectRef constructor_;
    NativeIdentityBindings<RegExpRef> identities_;
public:
    // One registration/binding owner per runtime, shared by all real callers.
    explicit RegExpBindings(options::Context& context)
        :context_(context),constructor_(context.constructor(u"RegExp")),identities_(context){
        context.define(constructor_,options::Key(u"length"),options::Descriptor::data(2,false,false,true));
    }
    RegExpBindings(const RegExpBindings&)=delete;
    RegExpBindings&operator=(const RegExpBindings&)=delete;
    options::Context& context()const{return context_;}
    options::ObjectRef constructorIdentity()const{return constructor_;}
    options::ObjectRef bind(RegExpRef reference){
        return identities_.instance(std::move(reference),constructor_);
    }
    options::ObjectRef create(std::u16string pattern,std::u16string flags={}){
        return bind(std::make_shared<RegExp>(std::move(pattern),std::move(flags)));
    }
    RegExpRef resolve(options::Value value)const{
        const auto* object=std::get_if<options::ObjectRef>(&value.data);
        if(!object)throw options::Exception("TypeError",u"Expected an actual native RegExp reference");
        return identities_.resolve(*object);
    }
    void setLastIndex(options::Value value,double cursor){resolve(value)->lastIndex=cursor;}
    double lastIndex(options::Value value)const{return resolve(value)->lastIndex;}
    std::optional<Match> exec(options::Value value,const std::u16string&input){return resolve(value)->exec(input);}
    // Teardown after retained Source dictionary/instance consumers are released.
    void clear(){identities_.clear();}
};
}
