// Native port of CasesTags from MathJax 4.1.3 CasesConfiguration.ts.
module folia.math.input.tex.cases.tags;
import folia.math.input.tex.parser;
import folia.math.util.js_number;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace { using namespace options; Key key(std::u16string_view s){return Key(std::u16string(s));} }
std::u16string CasesTags::casesNumber(double n,double m) const {
    std::u16string result=property_host::text(js_number::toString(n));
    if(m!=0)result.push_back(static_cast<char16_t>(0x60+static_cast<std::uint32_t>(m)));
    return result;
}
void CasesTags::start(Value env,Value taggable,Value defaultTags){
    subcounter_=0;
    AmsTags::start(std::move(env),std::move(taggable),std::move(defaultTags));
}
void CasesTags::autoTag(){
    Context& cx=runtime().context();
    const Value current=field(u"currentTag");
    const Value tagValue=cx.get(current,key(u"tag"));
    if(!std::holds_alternative<Null>(tagValue.data)&&!std::holds_alternative<Undefined>(tagValue.data))return;
    const Value environment=cx.get(current,key(u"env"));
    const auto* env=std::get_if<std::u16string>(&environment.data);
    const std::u16string_view name=env?std::u16string_view(*env):std::u16string_view{};
    double counter=cx.toNumber(field(u"counter"));
    if(name==u"subnumcases"){
        if(subcounter_==0){counter+=1;setField(u"counter",Value(counter));}
        subcounter_+=1;
        tag(Value(casesNumber(counter,subcounter_)),Value(false));
        return;
    }
    if(name!=u"numcases-left"){
        counter+=1;
        setField(u"counter",Value(counter));
    }
    tag(Value(casesNumber(counter,0)),Value(false));
}
TagsClassRef CasesTags::initializeModule(ParserRuntime& runtime){
    const auto ams=runtime.configurationHandler.get(u"ams");
    const auto* amsConfiguration=std::get_if<ConfigurationRef>(&ams);
    if(!amsConfiguration||!*amsConfiguration)throw Exception("TypeError",u"AMS configuration is unavailable");
    const Value tagConstructors=(*amsConfiguration)->field(u"tags");
    const Value parent=runtime.context().get(tagConstructors,Key(u"ams"));
    const auto* parentConstructor=std::get_if<ObjectRef>(&parent.data);
    if(!parentConstructor)throw Exception("TypeError",u"AmsTags constructor is unavailable");
    const auto descriptor=TagsFactory::registerSubclass(runtime,u"CasesTags",
        [&runtime](){return std::make_shared<CasesTags>(runtime);},*parentConstructor,
        [](Tags& value){auto* tags=dynamic_cast<CasesTags*>(&value);if(!tags)throw Exception("TypeError",u"CasesTags initializer received another class");});
    return descriptor;
}
}
