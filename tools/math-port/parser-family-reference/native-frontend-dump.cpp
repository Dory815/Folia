// Writes a normalized dump from the real native TexParser/TextParser package
// graph. Root's Node comparator compares this file with locked CJS trees.
import std;
import folia.math.input.tex.parser;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.textmacros.configuration;
import folia.math.input.tex.node_util;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.mml_tree.mml_factory;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.options;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
namespace tex=folia::math::tex;
namespace opts=folia::math::options;

std::string quote(std::string_view value){
    std::string out="\"";
    constexpr char hex[]="0123456789abcdef";
    for(const unsigned char ch:value){
        switch(ch){
        case '\\':out+="\\\\";break;case '"':out+="\\\"";break;
        case '\b':out+="\\b";break;case '\f':out+="\\f";break;
        case '\n':out+="\\n";break;case '\r':out+="\\r";break;case '\t':out+="\\t";break;
        default:if(ch<0x20){out+="\\u00";out.push_back(hex[ch>>4]);out.push_back(hex[ch&15]);}else out.push_back(static_cast<char>(ch));
        }
    }
    return out+'"';
}
std::string number(double value){
    if(std::isnan(value))return "{\"$\":\"NaN\"}";
    if(std::isinf(value))return value<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}";
    if(value==0&&std::signbit(value))return "{\"$\":\"-0\"}";
    std::ostringstream out;out<<std::setprecision(17)<<value;return out.str();
}
struct Serializer{
    opts::Context& context;
    OptionFunctionBindings& functions;
    std::string record(const PropertyList& value){
    std::string out="{";bool first=true;
    for(const auto& key:value.keys()){
        if(!first)out+=',';first=false;
        out+=quote(key);out+=':';out+=property(value.getOwn(key));
    }
    return out+'}';
    }
    std::string property(const Property& value){
    if(std::holds_alternative<Missing>(value.value))return "{\"$\":\"undefined\"}";
    if(std::holds_alternative<PropertyNull>(value.value))return "null";
    if(const auto* item=std::get_if<bool>(&value.value))return *item?"true":"false";
    if(const auto* item=std::get_if<double>(&value.value))return number(*item);
    if(const auto* item=std::get_if<std::string>(&value.value))return quote(*item);
    if(const auto* item=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);item&&*item){
        if((*item)->array){
            const auto array=PropertyArray::array(value);std::string out="[";
            for(std::size_t i=0;i<array->size();++i){
                if(i)out+=',';
                if(!array->has(i))out+="{\"$\":\"hole\"}";else out+=property(array->get(i));
            }
            return out+']';
        }
        return record(PropertyList::record(value).value());
    }
    if(const auto* fn=std::get_if<FunctionRef>(&value.value)){
        if(!*fn)throw std::runtime_error("MML tree contains an empty FunctionRef");
        const auto identity=functions.find(*fn);
        if(!identity)throw std::runtime_error("MML tree FunctionRef has no canonical Context metadata");
        const opts::Value name=context.get(*identity,opts::Key(u"name"));
        const opts::Value length=context.get(*identity,opts::Key(u"length"));
        const auto* text=std::get_if<std::u16string>(&name.data);
        const auto* arity=std::get_if<double>(&length.data);
        if(!text||!arity)throw std::runtime_error("canonical FunctionRef lacks Source name/length metadata");
        return "{\"$\":\"function\",\"name\":"+quote(property_host::debugText(*text))+",\"length\":"+number(*arity)+"}";
    }
    throw std::runtime_error("unsupported Property variant in strict MML tree dump");
    }
    std::string node(mml::NodeValue value){
    if(!value.isNode())return "null";
    auto* current=value.requireNode();
    const auto attrs=current->attributes();
    const auto children=current->childNodes;
    std::string out="{\"kind\":"+quote(current->kind())+",\"inferred\":"+
        (current->isInferred()?"true":"false")+",\"token\":"+(current->isToken()?"true":"false");
    if(const auto* token=dynamic_cast<mml::AbstractMmlTokenNode*>(current))
        out+=",\"text\":"+quote(property_host::debugText(token->getText()));
    out+=",\"attrs\":"+
        record(attrs?attrs->getAllAttributes():PropertyList{})+",\"properties\":"+
        record(current->getAllProperties())+",\"children\":[";
    if(children)for(std::size_t i=0;i<children->size();++i){if(i)out+=',';out+=node(children->get(i));}
    return out+"]}";
    }
};
struct ParserConfigurationAppendSpy final:tex::ParserConfiguration{
    using tex::ParserConfiguration::ParserConfiguration;
    std::vector<std::pair<std::string,double>> calls;
    void append(tex::ConfigurationRef configuration,opts::Value priority=opts::Undefined{})override{
        if(!configuration)throw std::runtime_error("append spy received a null Configuration");
        double numericPriority=tex::ParserConfiguration::runtime().context().toNumber(priority);
        calls.emplace_back(property_host::debugText(configuration->name()),numericPriority);
        tex::ParserConfiguration::append(std::move(configuration),std::move(priority));
    }
};
std::string node(Serializer& serializer,mml::NodeValue value){return serializer.node(value);}
opts::Value stringArray(opts::Context& context,std::initializer_list<std::u16string_view> values){
    const auto array=context.array(static_cast<std::uint32_t>(values.size()));std::uint32_t i=0;
    for(const auto value:values){const auto index=std::to_string(i++);context.set(array,opts::Key(std::u16string(index.begin(),index.end())),opts::Value(std::u16string(value)));}
    return opts::Value(array);
}
struct RuntimeGraph{
    std::string& stage;
    opts::Context context;OptionFunctionBindings functions{context};regexp_host::RegExpBindings regex{context};
    Unicode16Normalization unicode;core::InputJaxModule input{context};core::FindMathModule findMath{context};
    tex::FindTeXModule findTeX{context,findMath,regex};tex::ParserRuntime runtime{context,functions,regex,unicode,input,findTeX};
    std::shared_ptr<mml::MmlFactory> factory=std::make_shared<mml::MmlFactory>();tex::TeXRef jax;
    explicit RuntimeGraph(std::string& currentStage):stage(currentStage){
        stage="runtime.initializeParserRuntime";runtime.initializeParserRuntime();
        stage="BasePackage::initializeModule";tex::BasePackage::initializeModule(runtime);
        stage="TextMacrosConfiguration::initializeModule";tex::TextMacrosConfiguration::initializeModule(runtime);
        stage="ParserConfiguration::source-constructor-append-spy";
        const auto spyPackages=stringArray(context,{u"base",u"textmacros"});
        const auto spyParsers=stringArray(context,{u"tex"});
        auto appendSpy=std::make_shared<ParserConfigurationAppendSpy>(runtime);
        appendSpy->initializeParserConfiguration(spyPackages,spyParsers);
        const std::vector<std::pair<std::string,double>> expectedAppendCalls{{"textmacros",1.0},{"base",5.0}};
        if(appendSpy->calls!=expectedAppendCalls)
            throw std::runtime_error("ParserConfiguration constructor did not dispatch exactly one append per Source package record");
        std::cerr<<"tex-constructor-append-calls="<<appendSpy->calls.size()<<"\n";
        const auto config=context.object();context.set(config,opts::Key(u"packages"),stringArray(context,{u"base",u"textmacros"}));
        stage="TeX::make";jax=tex::TeX::make(runtime,opts::Value(config));
        stage="TeX::setMmlFactory";jax->setMmlFactory(factory);
        if(const auto* parser=std::get_if<tex::ParserConfigurationRef>(&jax->configuration);parser&&*parser){
            const auto records=(*parser)->sourceField(u"configurations");
            if(const auto* object=std::get_if<opts::ObjectRef>(&records.data)){
                const auto& payload=context.payload(*object);
                if(const auto* list=std::any_cast<std::shared_ptr<PrioritizedList<tex::ConfigurationRef>>>(&payload);list&&*list){
                    std::size_t configurationCount=0;
                    std::vector<std::pair<std::string,double>> actualConfigurations;
                    for(auto current=(*list)->begin(),finish=(*list)->end();current!=finish;++current)
                    {
                        ++configurationCount;
                        const auto record=current.record();
                        if(!record.present())throw std::runtime_error("null Configuration priority record");
                        const auto item=record.readItem();
                        tex::ConfigurationRef configuration;
                        if(const auto* native=std::get_if<tex::ConfigurationRef>(&item))configuration=*native;
                        else if(const auto* raw=std::get_if<opts::Value>(&item)){
                            if(const auto* identity=std::get_if<opts::ObjectRef>(&raw->data))
                                configuration=runtime.configurationIdentities.resolve(*identity);
                        }
                        if(!configuration)throw std::runtime_error("Configuration priority record item is not a Configuration");
                        const auto priorityValue=record.readPriorityValue();
                        const auto* priority=std::get_if<double>(&priorityValue.data);
                        if(!priority)throw std::runtime_error("Configuration priority is not numeric");
                        actualConfigurations.emplace_back(property_host::debugText(configuration->name()),*priority);
                    }
                    std::cerr<<"tex-config-count="<<configurationCount<<'\n';
                    const std::vector<std::pair<std::string,double>> expectedConfigurations{{"textmacros",1.0},{"base",5.0}};
                    if(actualConfigurations!=expectedConfigurations)
                        throw std::runtime_error("real ParserConfiguration package list did not preserve Source config records/priorities");
                    for(auto current=(*list)->begin(),finish=(*list)->end();current!=finish;++current){
                        const auto record=current.record();if(!record.present())continue;
                        const auto item=record.readItem();
                        tex::ConfigurationRef configuration;
                        if(const auto* native=std::get_if<tex::ConfigurationRef>(&item))configuration=*native;
                        else if(const auto* raw=std::get_if<opts::Value>(&item)){
                            if(const auto* identity=std::get_if<opts::ObjectRef>(&raw->data))
                                configuration=runtime.configurationIdentities.resolve(*identity);
                        }
                        if(!configuration)continue;
                        const auto priorityValue=record.readPriorityValue();
                        const auto* priority=std::get_if<double>(&priorityValue.data);
                        if(!priority)throw std::runtime_error("Configuration priority is not numeric");
                        const auto pre=configuration->field(u"preprocessors");
                        std::uint32_t length=0;
                        if(const auto* array=std::get_if<opts::ObjectRef>(&pre.data);array&&context.kind(*array)==opts::ObjectKind::array)length=context.arrayLength(*array);
                        std::cerr<<"tex-config name="<<property_host::debugText(configuration->name())
                                 <<" priority="<<*priority<<" preprocessors="<<length<<'\n';
                    }
                }else std::cerr<<"tex-config-list payload mismatch\\n";
            }else std::cerr<<"tex-config-list value is not object\\n";
        }else std::cerr<<"tex-parser-configuration unavailable\\n";
        if(jax->preFilters->storageRef()->size()!=1)
            throw std::runtime_error("real TextMacros configuration did not register exactly one pre-filter");
        // The pinned CJS TextMacros observer runs its real preprocessor before
        // constructing TextParser. Mirror that phase through the actual filter
        // list so the inner NodeFactory receives the same live MmlFactory.
        stage="TextMacros.preFilters.execute";
        const auto parseOptions=jax->parseOptions();
        const auto* actual=std::get_if<tex::ParseOptionsRef>(&parseOptions);
        if(!actual||!*actual)throw std::runtime_error("missing real ParseOptions for TextMacros preprocessor");
        auto filter=std::make_shared<core::InputJaxFilterData>();
        filter->initialize(core::MathItemReference{Missing{}},core::MathDocumentReference{Missing{}},std::any(*actual));
        filter->encodeData=[this](const std::any& value)->opts::Value{
            if(const auto* contextValue=std::any_cast<opts::Value>(&value))return *contextValue;
            if(const auto* owner=std::any_cast<tex::ParseOptionsRef>(&value);owner&&*owner){
                const auto identity=runtime.parseOptionsIdentities.find(*owner);
                if(!identity)throw opts::Exception("TypeError",u"ParseOptions lacks its canonical Context identity");
                return opts::Value(*identity);
            }
            throw opts::Exception("TypeError",u"filter data has no canonical Context value");
        };
        filter->decodeData=[this](opts::Value value)->std::any{
            const auto* identity=std::get_if<opts::ObjectRef>(&value.data);
            if(!identity)throw opts::Exception("TypeError",u"filter data is not ParseOptions");
            return std::any(runtime.parseOptionsIdentities.resolve(*identity));
        };
        filter->bindSourceRecord(context,filter,runtime.executionLifetime());
        std::array<std::any,1> arguments{std::any(filter)};
        jax->preFilters->execute(arguments);
        const auto* jaxFactory=std::get_if<mml::MmlFactory*>(&jax->mmlFactory);
        std::cerr<<"textmacros-prefilter count="<<jax->preFilters->storageRef()->size()
                 <<" jax-factory="<<(jaxFactory&&*jaxFactory?"set":"null")<<'\n';
        if(!jaxFactory||!*jaxFactory)throw std::runtime_error("real TextMacros pre-filter did not preserve the outer MmlFactory");
    }
    tex::ParseOptionsRef options(){
        const auto value=jax->parseOptions();const auto* result=std::get_if<tex::ParseOptionsRef>(&value);
        if(!result||!*result)throw std::runtime_error("missing real ParseOptions");return *result;
    }
    std::shared_ptr<SourceStringMap<opts::Value>> data(){
        const auto value=options()->packageData;const auto* result=std::get_if<std::shared_ptr<SourceStringMap<opts::Value>>>(&value);
        if(!result||!*result)throw std::runtime_error("missing real packageData");return *result;
    }
};
}

int main(int argc,char** argv){
    std::string stage="startup";
    try{
        RuntimeGraph graph(stage);Serializer serializer{graph.context,graph.functions};
        std::vector<std::pair<std::string,std::string>> rows;
        struct TexCase{std::u16string_view source;const char* name;};
        constexpr TexCase texCases[]={{u"","empty"},{u"x","x"},{u"x+1","x-plus-one"},
            {u"\\frac{1}{2}","fraction"},{u"😀","astral"},{u"x^2","superscript"}};
        for(const auto& item:texCases){
            stage=std::string("tex/")+item.name+"/construct-parse-stop";
            auto config=graph.options();config->clear();
            const auto parser=tex::TexParser::make(std::u16string(item.source),PropertyList{},*config);
            stage=std::string("tex/")+item.name+"/mml-serialize";
            rows.emplace_back(std::string("tex/")+item.name,node(serializer,parser->mml()));
        }
        const auto data=graph.data()->get(u"textmacros");if(!data)throw std::runtime_error("missing textmacros record");
        const auto* recordObject=std::get_if<opts::ObjectRef>(&data->data);if(!recordObject)throw std::runtime_error("invalid textmacros record");
        auto main=graph.options();main->clear();const auto outer=tex::TexParser::make(u"",PropertyList{},*main);
        graph.context.set(*recordObject,opts::Key(u"texParser"),opts::Value(outer->instanceIdentity()));
        const auto innerValue=graph.context.get(*recordObject,opts::Key(u"parseOptions"));
        const auto* innerIdentity=std::get_if<opts::ObjectRef>(&innerValue.data);if(!innerIdentity)throw std::runtime_error("missing inner ParseOptions identity");
        auto inner=graph.runtime.parseOptionsIdentities.resolve(*innerIdentity);
        // Verify the real preprocessor's factory handoff separately from the
        // first TextParser call. This creates a detached probe node only; it is
        // never inserted in a parse tree or node list.
        stage="TextMacros.inner-nodeFactory-probe";
        const auto nodeFactory=inner->nodeFactoryOwnerRef();
        if(!nodeFactory)throw std::runtime_error("inner ParseOptions has no live NodeFactory");
        const std::array<std::any,1> textProbeArgs{std::any(Property(std::string("factory-probe")))};
        const auto textProbe=nodeFactory->create("text",textProbeArgs);
        if(!textProbe.isNode()||textProbe.requireNode("kind")->kind()!="text")
            throw std::runtime_error("inner NodeFactory did not create a detached text node");
        struct MacroCase{std::u16string_view source;const char* name;};
        constexpr MacroCase macroCases[]={
            {u"\\%","percent"},{u"\\dagger","dagger"},{u"\\'{e}","accent"},{u"\\frak{x}","frak"},
            {u"\\quad","quad"},{u"\\hspace{1em}","hspace"},{u"\\rule{1em}{2pt}","rule"},
            {u"\\ref{missing}","reference"},{u"\\underline{x}","underline"},{u"\\llap{x}","llap"},
            {u"\\phantom{x}","phantom"},{u"\\smash{x}","smash"},{u"\\mmlToken{mi}{x}","mml-token"},
            {u"\\emph emphatic","emph"},{u"\\textbf{bold}","textbf"},
            {u"\\textit{italic}","textit"},{u"\\texttt{mono}","texttt"},
        };
        for(const auto& item:macroCases){
            stage=std::string("macro/")+item.name+"/construct-parse-stop";
            inner->clear();
            const auto parser=tex::TextParser::make(graph.runtime,std::u16string(item.source),PropertyList{},*inner);
            stage=std::string("macro/")+item.name+"/mml-serialize";
            rows.emplace_back(std::string("macro/")+item.name,node(serializer,parser->mml()));
        }
        struct TextCase{std::u16string_view source;const char* name;};
        constexpr TextCase textCases[]={
            {u"\\bf bold","bf"},{u"\\it italic","it"},{u"\\tiny small","tiny"},
            {u"\\emph emphatic","emph"},{u"\\textbf{bold}","textbf"},{u"\\textit{italic}","textit"},
            {u"\\texttt{mono}","texttt"},{u"plain  many   spaces","spaces"},{u"a\u00a0b","nbsp"},
            {u"`open'","quotes"},{u"``pair''","double-quotes"},{u"{group}","group"},{u"$x$","math-delimiter"},
        };
        for(const auto& item:textCases){
            stage=std::string("text/")+item.name;
            inner->clear();const auto parser=tex::TextParser::make(graph.runtime,std::u16string(item.source),PropertyList{},*inner);
            rows.emplace_back(std::string("text/")+item.name,node(serializer,parser->mml()));
        }
        stage="text/level-2";inner->clear();const auto leveled=tex::TextParser::make(graph.runtime,u"abc",PropertyList{},*inner,Property(2.0));
        rows.emplace_back("text/level-2",node(serializer,leveled->mml()));

        const std::filesystem::path output=argc>1?std::filesystem::path(argv[1]):std::filesystem::path("build/math-port/native-frontend.json");
        std::filesystem::create_directories(output.parent_path());std::ofstream stream(output,std::ios::binary);
        if(!stream)throw std::runtime_error("cannot open native frontend dump output");
        stream<<"{\"schema\":1,\"cases\":{";
        for(std::size_t i=0;i<rows.size();++i){if(i)stream<<',';stream<<quote(rows[i].first)<<':'<<rows[i].second;}
        stream<<"}}\n";if(!stream)throw std::runtime_error("failed writing native frontend dump");
        return 0;
    }catch(const opts::Exception& error){
        std::cerr<<"stage="<<stage<<" exception="<<error.name<<" message="
                 <<property_host::debugText(error.message)<<'\n';return 1;
    }catch(const tex::TexError& error){
        std::cerr<<"stage="<<stage<<" exception=TexError id="
                 <<property_host::debugText(error.id)<<" message="
                 <<property_host::debugText(error.message)<<'\n';return 1;
    }catch(const std::exception& error){std::cerr<<"stage="<<stage<<" exception="<<error.what()<<'\n';return 1;}
}
