// Whole-profile observer for the real NativeRenderSession -> HTMLMathDocument
// -> MathItem -> CommonWrapper -> SVG Scene path.  Input is one tab-delimited
// row per case; string fields are base64 UTF-8 supplied by the companion CJS
// runner.  No expected layout values enter this translation unit.
import std;
import folia.math.output.svg.native_document;
import folia.math.output.svg.output_jax;
import folia.math.output.scene;
import folia.math.core.mml_tree.mml_node;
import folia.math.port.common_layout_native_observer;
import folia.math.util.property_host;
import folia.math.util.options;

namespace {
using namespace folia::math;
namespace opts=folia::math::options;
namespace svg=folia::math::output::svg;
namespace common=folia::math::output::common;
namespace tex=folia::math::tex;
namespace scene=folia::math::output::scene;
namespace layoutdump=folia::math::port::common_layout;

std::string jsonString(std::string_view value){
    std::string out="\"";
    constexpr char hex[]="0123456789abcdef";
    for(std::size_t i=0;i<value.size();){
        const auto c=static_cast<unsigned char>(value[i]);
        if(c=='\\'){out+="\\\\";++i;}
        else if(c=='"'){out+="\\\"";++i;}
        else if(c=='\n'){out+="\\n";++i;}
        else if(c=='\r'){out+="\\r";++i;}
        else if(c=='\t'){out+="\\t";++i;}
        else if(c<0x20){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];++i;}
        else if(i+2<value.size()&&c==0xed&&
                static_cast<unsigned char>(value[i+1])>=0xa0&&
                static_cast<unsigned char>(value[i+1])<=0xbf&&
                (static_cast<unsigned char>(value[i+2])&0xc0)==0x80){
            const auto unit=static_cast<unsigned>(((c&15)<<12)|
                ((static_cast<unsigned char>(value[i+1])&63)<<6)|
                (static_cast<unsigned char>(value[i+2])&63));
            out+="\\u";out+=hex[(unit>>12)&15];out+=hex[(unit>>8)&15];
            out+=hex[(unit>>4)&15];out+=hex[unit&15];i+=3;
        }else{out+=static_cast<char>(c);++i;}
    }
    return out+'"';
}
std::string number(double value){
    if(std::isnan(value))return "{\"$\":\"NaN\"}";
    if(value==std::numeric_limits<double>::infinity())return "{\"$\":\"Infinity\"}";
    if(value==-std::numeric_limits<double>::infinity())return "{\"$\":\"-Infinity\"}";
    if(value==0&&std::signbit(value))return "{\"$\":\"-0\"}";
    std::ostringstream out;out<<std::setprecision(std::numeric_limits<double>::max_digits10)<<value;
    return out.str();
}
std::string propertyJson(const Property&,std::unordered_set<const void*>&);
std::string listJson(const PropertyList& list,std::unordered_set<const void*>& active){
    std::string out="{";bool first=true;
    for(const auto& key:list.keys()){
        if(!first)out+=',';first=false;
        out+=jsonString(key)+":"+propertyJson(list.getOwn(key),active);
    }
    return out+"}";
}
std::string arrayJson(const PropertyArray& array,std::unordered_set<const void*>& active){
    std::string out="[";
    for(std::size_t i=0;i<array.size();++i){
        if(i)out+=',';
        out+=array.has(i)?propertyJson(array.get(i),active):"null";
    }
    return out+"]";
}
std::string propertyJson(const Property& value,std::unordered_set<const void*>& active){
    if(value.missing())return "{\"$\":\"undefined\"}";
    if(std::holds_alternative<PropertyNull>(value.value))return "null";
    if(const auto* item=std::get_if<bool>(&value.value))return *item?"true":"false";
    if(const auto* item=std::get_if<double>(&value.value))return number(*item);
    if(const auto* item=std::get_if<std::string>(&value.value))return jsonString(*item);
    if(const auto* item=std::get_if<BuiltinProperty>(&value.value))
        return "{\"$\":\"builtin\",\"value\":"+
            std::to_string(static_cast<unsigned>(*item))+"}";
    if(const auto* item=std::get_if<FunctionRef>(&value.value))
        return *item?"{\"$\":\"function\"}":"null";
    if(const auto* item=std::get_if<opts::SymbolRef>(&value.value))
        return "{\"$\":\"symbol\",\"context\":"+std::to_string(item->context)+
            ",\"index\":"+std::to_string(item->index)+"}";
    const auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);
    if(!storage||!*storage)return "null";
    if(!active.insert(storage->get()).second)
        throw opts::Exception("TypeError",u"Circular Property value in native layout output");
    std::string result;
    if((*storage)->array){
        const auto array=PropertyArray::array(value);
        result=array?arrayJson(*array,active):"[]";
    }else{
        const auto record=PropertyList::record(value);
        result=record?listJson(*record,active):"{}";
    }
    active.erase(storage->get());return result;
}
std::string listJson(const PropertyList& value){
    std::unordered_set<const void*> active;return listJson(value,active);
}
std::string mmlJson(const mml::NodeValue& value){
    if(value.kind()!=tree::ChildKind::Node)return "null";
    const auto& node=*value.requireNode("native MML dump");
    const auto attributes=node.attributes();
    std::string out="{\"kind\":"+jsonString(node.kind())+
        ",\"attributes\":"+(attributes?listJson(attributes->getAllAttributes()):"null")+
        ",\"properties\":"+listJson(node.getAllProperties());
    if(const auto* textNode=dynamic_cast<const mml::TextNode*>(&node))
        out+=",\"text\":"+jsonString(property_host::debugText(textNode->getText()));
    else if(const auto* token=dynamic_cast<const mml::AbstractMmlTokenNode*>(&node))
        out+=",\"text\":"+jsonString(property_host::debugText(token->getText()));
    out+=",\"children\":[";
    if(node.childNodes){
        for(std::size_t i=0;i<node.childNodes->size();++i){
            if(i)out+=',';
            out+=mmlJson(node.childNodes->get(i));
        }
    }
    return out+"]}";
}
std::string errorJson(std::string_view name,std::string_view message){
    return "{\"name\":"+jsonString(name)+",\"message\":"+jsonString(message)+"}";
}
std::string warningJson(const std::any& warning){
    if(const auto* error=std::any_cast<opts::Exception>(&warning))
        return errorJson(error->name,property_host::debugText(error->message));
    if(const auto* text=std::any_cast<std::string>(&warning))return jsonString(*text);
    if(const auto* text=std::any_cast<std::u16string>(&warning))
        return jsonString(property_host::debugText(*text));
    if(const auto* arguments=std::any_cast<std::vector<std::any>>(&warning)){
        std::string result="[";
        for(std::size_t i=0;i<arguments->size();++i){
            if(i)result+=',';
            result+=warningJson((*arguments)[i]);
        }
        return result+"]";
    }
    return "{\"name\":\"UnobservedWarningType\",\"type\":"+
        jsonString(warning.has_value()?warning.type().name():"empty")+"}";
}
struct InputCase {std::string name,tex;bool display=false;double width=1280,em=16,ex=8;};
struct LayoutObserverReset {
    svg::SvgOutputJax& jax;
    ~LayoutObserverReset(){
        jax.setLayoutObserver({});
        jax.setSceneLayoutObserver({});
    }
};
struct WarningSinkReset {
    std::function<void(std::u16string)> previous;
    ~WarningSinkReset(){opts::OPTIONS.warningSink=std::move(previous);}
};
int base64Digit(char c){
    if(c>='A'&&c<='Z')return c-'A';if(c>='a'&&c<='z')return c-'a'+26;
    if(c>='0'&&c<='9')return c-'0'+52;if(c=='+')return 62;if(c=='/')return 63;
    return -1;
}
std::string decode64(std::string_view text){
    std::string out;unsigned accumulator=0,bits=0;
    for(const auto c:text){if(c=='=')break;const int d=base64Digit(c);
        if(d<0)throw std::runtime_error("Invalid base64 test input");
        accumulator=(accumulator<<6)|static_cast<unsigned>(d);bits+=6;
        if(bits>=8){bits-=8;out.push_back(static_cast<char>((accumulator>>bits)&255));}
    }
    return out;
}
std::vector<std::string_view> splitRow(std::string_view row){
    std::vector<std::string_view> result;std::size_t start=0;
    while(true){const auto end=row.find('\t',start);result.push_back(row.substr(start,
        end==std::string_view::npos?row.size()-start:end-start));
        if(end==std::string_view::npos)break;start=end+1;}
    return result;
}
InputCase parseCase(std::string_view row){
    const auto fields=splitRow(row);if(fields.size()!=6)throw std::runtime_error("Expected six profile fields");
    InputCase item;
    item.name=property_host::debugText(property_host::text(decode64(fields[0])));
    item.tex=property_host::debugText(property_host::text(decode64(fields[1])));
    item.display=fields[2]=="1";
    item.width=std::stod(std::string(fields[3]));item.em=std::stod(std::string(fields[4]));
    item.ex=std::stod(std::string(fields[5]));return item;
}
}

int main(){
  try {
    using namespace folia::math;
    std::vector<std::string> packageNames;
    std::vector<InputCase> cases;
    bool sawPackageHeader=false;
    for(std::string row;std::getline(std::cin,row);){
        if(row.empty())continue;
        if(!sawPackageHeader){
            const auto fields=splitRow(row);
            if(fields.empty()||fields.front()!="@packages")
                throw std::runtime_error("Expected a leading @packages input row");
            for(std::size_t i=1;i<fields.size();++i)
                packageNames.push_back(decode64(fields[i]));
            sawPackageHeader=true;
        }else cases.push_back(parseCase(row));
    }
    if(!sawPackageHeader)throw std::runtime_error("Missing @packages input row");
    if(cases.empty())return 2;
    std::vector<std::any> warnings;
    std::vector<std::string> configurationWarnings;
    std::vector<std::string> loadRequests;
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] {return true;};
    host.consoleWarn=[&warnings](std::vector<std::any> arguments){
        warnings.emplace_back(std::move(arguments));
    };
    host.warn=[&warnings](const std::any& value){warnings.push_back(value);};
    // The actual session replaces sync/async request functions with its
    // generated NewCM module provider. Keep this observer at the exact host
    // call site so resolved and rejected requests are both Source ordered.
    host.syncLoad=[](common::FontData&,const common::DynamicFileRef&,std::string){
        throw options::Exception("Unimplemented",u"Native NewCM provider was not installed");
    };
    host.asyncLoad=[](std::shared_ptr<common::FontData>,common::DynamicFileRef,
                      std::string)->async::PromiseRef{
        throw options::Exception("Unimplemented",u"Native NewCM provider was not installed");
    };
    host.requestObserver=[&loadRequests](std::string_view request){loadRequests.emplace_back(request);};

    auto session=std::make_unique<svg::NativeRenderSession>(std::move(host),
        svg::NewcmFontProfile{svg::NewcmFontExtension::bboldx,
            svg::NewcmFontExtension::bbm,svg::NewcmFontExtension::dsfont});
    auto& cx=session->context();
    WarningSinkReset warningSinkReset{opts::OPTIONS.warningSink};
    opts::OPTIONS.warningSink=[&configurationWarnings](std::u16string message){
        configurationWarnings.push_back(property_host::debugText(message));
    };
    const auto jaxOptions=cx.object();
    cx.set(jaxOptions,options::Key(u"fontCache"),options::Value(u"none"));
    // The pinned Source profile supplies this option to SVG explicitly. The
    // NewCM class default remains its own CJS-transpiled JS export path.
    cx.set(jaxOptions,options::Key(u"dynamicPrefix"),
           options::Value(u"@mathjax/mathjax-newcm-font/cjs/svg/dynamic"));
    cx.set(jaxOptions,options::Key(u"exFactor"),options::Value(.5));
    cx.set(jaxOptions,options::Key(u"displayOverflow"),options::Value(u"linebreak"));
    const auto linebreaks=cx.object();
    cx.set(linebreaks,options::Key(u"inline"),options::Value(true));
    cx.set(jaxOptions,options::Key(u"linebreaks"),options::Value(linebreaks));
    const auto jax=session->construct(options::Value(jaxOptions));
    const auto inputJaxOptions=cx.object();
    const auto packageArray=cx.array(packageNames.size());
    for(std::size_t i=0;i<packageNames.size();++i){
        const auto index=std::to_string(i);
        cx.set(packageArray,options::Key(std::u16string(index.begin(),index.end())),
               options::Value(property_host::text(packageNames[i])));
    }
    cx.set(inputJaxOptions,options::Key(u"packages"),options::Value(packageArray));
    auto document=svg::createNativeSvgDocument(*session,options::Value(options::Undefined{}),
                                               options::Value(inputJaxOptions));
    auto& contextRef=document->sourceContext();
    const auto inputJaxList=contextRef.get(options::Value(document->sourceIdentity()),
                                           options::Key(u"inputJax"));
    const auto* inputJaxArray=std::get_if<options::ObjectRef>(&inputJaxList.data);
    if(!inputJaxArray)throw options::Exception("TypeError",u"MathDocument.inputJax is not an array");
    if(contextRef.arrayLength(*inputJaxArray)==0)
        throw options::Exception("TypeError",u"MathDocument.inputJax is empty");
    const auto inputJax=contextRef.get(inputJaxList,options::Key(u"0"));
    const auto actualInputOptions=contextRef.get(inputJax,options::Key(u"options"));
    const auto actualPackages=contextRef.get(actualInputOptions,options::Key(u"packages"));
    const auto* actualPackageArray=std::get_if<options::ObjectRef>(&actualPackages.data);
    if(!actualPackageArray)
        throw options::Exception("TypeError",u"InputJax.options.packages is not an array");
    std::string configuredPackages="[";
    const auto actualPackageCount=contextRef.arrayLength(*actualPackageArray);
    std::vector<std::string> registeredPackages;
    auto& parserRuntime=session->services().parser();
    for(std::uint32_t i=0;i<actualPackageCount;++i){
        if(i)configuredPackages+=',';
        const auto index=std::to_string(i);
        const auto package=contextRef.get(actualPackages,
            options::Key(std::u16string(index.begin(),index.end())));
        const auto* name=std::get_if<std::u16string>(&package.data);
        if(!name)throw options::Exception("TypeError",u"InputJax package name is not a string");
        configuredPackages+=jsonString(property_host::debugText(*name));
        const auto found=parserRuntime.configurationHandler.get(*name);
        if(const auto* configuration=std::get_if<tex::ConfigurationRef>(&found);
           configuration&&*configuration)
            registeredPackages.push_back(property_host::debugText((*configuration)->name()));
    }
    configuredPackages+=']';
    std::string registeredPackageJson="[";
    for(std::size_t i=0;i<registeredPackages.size();++i){
        if(i)registeredPackageJson+=',';
        registeredPackageJson+=jsonString(registeredPackages[i]);
    }
    registeredPackageJson+=']';
    std::string configurationWarningJson="[";
    for(std::size_t i=0;i<configurationWarnings.size();++i){
        if(i)configurationWarningJson+=',';
        configurationWarningJson+=jsonString(configurationWarnings[i]);
    }
    configurationWarningJson+=']';
    const auto contextLease=session->contextOwner();
    (void)contextLease;
    auto* svgJax=dynamic_cast<svg::SvgOutputJax*>(
        std::get<core::MathOutputJaxService*>(document->outputJaxReference()));
    if(!svgJax)throw std::runtime_error("Document did not retain its actual SvgOutputJax");
    std::string activeMml="null";
    // MathDocument.convert() creates a temporary MathItem and does not append
    // it to document.math. Wrap the actual Source prototype method in this
    // diagnostic Context, call the original with the same receiver and args,
    // then serialize the receiver's real root field at the compile boundary.
    // Exceptions propagate unchanged and leave MML null for failed compiles.
    const auto documentOptions=contextRef.get(options::Value(document->sourceIdentity()),options::Key(u"options"));
    const auto itemConstructor=contextRef.get(documentOptions,options::Key(u"MathItem"));
    const auto itemPrototypeValue=contextRef.get(itemConstructor,options::Key(u"prototype"));
    const auto* itemPrototype=std::get_if<options::ObjectRef>(&itemPrototypeValue.data);
    if(!itemPrototype)throw options::Exception("TypeError",u"MathItem prototype is unavailable");
    const auto originalCompileValue=contextRef.get(options::Value(*itemPrototype),options::Key(u"compile"));
    const auto* originalCompile=std::get_if<options::ObjectRef>(&originalCompileValue.data);
    if(!originalCompile||contextRef.kind(*originalCompile)!=options::ObjectKind::function)
        throw options::Exception("TypeError",u"MathItem.prototype.compile is not callable");
    const auto originalCompileRef=*originalCompile;
    contextRef.set(*itemPrototype,options::Key(u"compile"),options::Value(contextRef.function(
        u"layout-profile-observed-compile",
        [originalCompileRef,&session,&activeMml](options::Context& context,options::Value receiver,
                                                 std::span<const options::Value> args)->options::Value{
            const options::Value result=context.call(originalCompileRef,receiver,args);
            const options::Value root=context.get(receiver,options::Key(u"root"));
            if(const auto* identity=std::get_if<options::ObjectRef>(&root.data)){
                auto& node=session->services().parser().resolveMmlNode(*identity);
                activeMml=mmlJson(mml::NodeValue(&node));
            }else if(std::holds_alternative<options::Null>(root.data)||
                     std::holds_alternative<options::Undefined>(root.data)){
                activeMml="null";
            }else throw options::Exception("TypeError",u"MathItem.root is not a canonical MML identity");
            return result;
        })));
    std::string activeWrappers="[]",activeCalls="[]",activePaint="[]";
    std::string svgText="null",error="null",loads="[]",warningValues="[]";
    std::optional<std::size_t> layoutRoot;
    std::cout<<"{\"configuredPackages\":"<<configuredPackages
             <<",\"registeredPackages\":"<<registeredPackageJson
             <<",\"configurationWarnings\":"<<configurationWarningJson
             <<",\"records\":[";
    std::cout.flush();
    bool emittedRecord=false;
    for(std::size_t index=0;index<cases.size();++index){
        const auto& item=cases[index];
        std::cerr<<"[native-layout] case "<<(index+1)<<"/"<<cases.size()<<" "
                 <<item.name<<" begin"<<std::endl;
        layoutdump::LayoutTrace trace;
        svgJax->setSceneLayoutObserver(std::static_pointer_cast<scene::LayoutEventObserver>(trace.observerOpaque()));
        LayoutObserverReset reset{*svgJax};
        activeMml="null";activeWrappers="[]";activeCalls="[]";activePaint="[]";
        svgText="null";error="null";layoutRoot.reset();warnings.clear();loadRequests.clear();
        svgJax->setLayoutObserver([&](const common::CommonWrapper& root,const common::LayoutJob& job){
            auto snap=layoutdump::snapshotWrappers(&root,&job,trace.ids());
            layoutRoot=snap.rootId;activeWrappers=std::move(snap.wrappersJson);
        });
        const auto settings=contextRef.object();
        contextRef.set(settings,options::Key(u"display"),options::Value(item.display));
        contextRef.set(settings,options::Key(u"em"),options::Value(item.em));
        contextRef.set(settings,options::Key(u"ex"),options::Value(item.ex));
        contextRef.set(settings,options::Key(u"containerWidth"),options::Value(item.width));
        try{
            std::cerr<<"[native-layout] case "<<(index+1)<<" "<<item.name
                     <<" convert begin"<<std::endl;
            const auto result=document->convert(options::Value(property_host::text(item.tex)),
                                                options::Value(settings));
            std::cerr<<"[native-layout] case "<<(index+1)<<" "<<item.name
                     <<" convert returned; serialize begin"<<std::endl;
            const auto dom=propertyNode(document->propertyBindings().from(result));
            const auto* node=std::get_if<DOMNodeRef>(&dom);
            if(!node||!*node)throw options::Exception("TypeError",u"Document conversion returned no DOM node");
            svgText=jsonString(property_host::debugText(document->domAdaptor().serializeXML(*node)));
            std::cerr<<"[native-layout] case "<<(index+1)<<" "<<item.name
                     <<" serialize returned"<<std::endl;
        }catch(const options::Exception& caught){
            std::cerr<<"[native-layout] case "<<(index+1)<<" "<<item.name
                     <<" caught options exception"<<std::endl;
            error=errorJson(caught.name,property_host::debugText(caught.message));
        }catch(const std::exception& caught){
            std::cerr<<"[native-layout] case "<<(index+1)<<" "<<item.name
                     <<" caught std exception"<<std::endl;
            error=errorJson("std::exception",caught.what());
        }catch(...){
            std::cerr<<"[native-layout] case "<<(index+1)<<" "<<item.name
                     <<" caught unknown exception"<<std::endl;
            error=errorJson("unknown","unclassified native exception");
        }
        activeCalls=trace.callsJson();activePaint=trace.paintJson();
        std::string warningJsonValues="[";
        for(std::size_t i=0;i<warnings.size();++i){if(i)warningJsonValues+=',';warningJsonValues+=warningJson(warnings[i]);}
        warningJsonValues+=']';
        std::string loadJson="[";
        for(std::size_t i=0;i<loadRequests.size();++i){if(i)loadJson+=',';loadJson+=jsonString(loadRequests[i]);}
        loadJson+=']';
        std::ostringstream record;
        record<<"{\"name\":"<<jsonString(item.name)<<",\"tex\":"<<jsonString(item.tex)
            <<",\"display\":"<<(item.display?"true":"false")<<",\"em\":"<<number(item.em)
            <<",\"ex\":"<<number(item.ex)<<",\"width\":"<<number(item.width)
            <<",\"layoutRoot\":"<<(layoutRoot?std::to_string(*layoutRoot):"null")
            <<",\"wrappers\":"<<activeWrappers<<",\"calls\":"<<activeCalls
            <<",\"paint\":"<<activePaint<<",\"loads\":"<<loadJson<<",\"svg\":"<<svgText
            <<",\"error\":"<<error<<",\"mml\":"<<activeMml<<",\"warnings\":"
            <<warningJsonValues<<",\"intendedFeatures\":[]}";
        if(emittedRecord)std::cout<<',';
        emittedRecord=true;
        std::cout<<record.str()<<std::flush;
        std::cerr<<"[native-layout] case "<<(index+1)<<" "<<item.name
                 <<" end"<<std::endl;
    }
    std::cout<<"]}\n";
    return 0;
  }catch(const folia::math::options::Exception& error){
    std::cerr<<"native-layout bootstrap error: "<<error.name<<": "
             <<folia::math::property_host::debugText(error.message)<<'\n';
  }catch(const std::exception& error){
    std::cerr<<"native-layout bootstrap error: std::exception: "<<error.what()<<'\n';
  }catch(...){
    std::cerr<<"native-layout bootstrap error: unknown exception\n";
  }
  return 1;
}
