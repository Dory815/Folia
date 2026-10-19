// Draft native probe paired with attributes-original-oracle.cjs.
// Root-owned shared MathJax graph probe for Gate 114; capture its JSON with the
// companion comparator after the ABI-consistent graph has been built.
import std;
import folia.math.core.mml_tree.attributes_bindings;
import folia.math.util.property_host;

namespace {
using namespace folia::math;
using namespace folia::math::options;

std::string quote(std::string_view value){
    std::string out="\"";
    for(const unsigned char c:value){
        if(c=='"'||c=='\\'){out.push_back('\\');out.push_back(static_cast<char>(c));}
        else if(c=='\n')out+="\\n";
        else if(c=='\r')out+="\\r";
        else if(c=='\t')out+="\\t";
        else out.push_back(static_cast<char>(c));
    }
    out.push_back('"');return out;
}
std::string quote(std::u16string_view value){return quote(property_host::debugText(value));}
std::string yes(bool value){return value?"true":"false";}
std::string array(const std::vector<std::string>& values){
    std::string out="[";for(std::size_t i=0;i<values.size();++i){if(i)out+=',';out+=quote(values[i]);}return out+"]";
}
std::string boolArray(const std::vector<bool>& values){
    std::string out="[";for(std::size_t i=0;i<values.size();++i){if(i)out+=',';out+=yes(values[i]);}return out+"]";
}
std::vector<std::string> keys(Context& context,ObjectRef object){
    std::vector<std::string> result;
    for(const auto& key:context.ownKeys(Value(object)))
        if(const auto* text=std::get_if<std::u16string>(&key.data))result.push_back(property_host::debugText(*text));
    return result;
}
std::string propertyString(const Property& value){return quote(property_host::string(value));}
}

int main(){
    Context context;
    OptionFunctionBindings functions(context);
    ContextPropertyBindings properties(context,functions);
    auto lease=std::make_shared<std::atomic_bool>(true);
    AttributesContextBindings binding(context,functions,properties,lease);
    PropertyList defaults{{"mathcolor",Property("blue")},{"mathsize",Property(12)}};
    PropertyList globals{{"mathcolor",Property("red")},{"dir",Property("ltr")}};
    auto owner=std::make_shared<Attributes>(defaults,globals);
    const auto object=binding.bind(owner);
    const auto proto=binding.prototype();
    const auto ctor=binding.constructor();
    const auto ctorKeys=keys(context,ctor);
    std::vector<std::array<bool,3>> ctorDescriptors;
    for(const auto& field:ctorKeys){
        const auto descriptor=context.descriptor(Value(ctor),Key(std::u16string(field.begin(),field.end())));
        ctorDescriptors.push_back({descriptor&&descriptor->writable,descriptor&&descriptor->enumerable,descriptor&&descriptor->configurable});
    }
    const auto own=keys(context,object),methods=keys(context,proto);
    std::vector<std::array<bool,3>> fieldDescriptors;
    for(const auto& field:own){
        const auto descriptor=context.descriptor(Value(object),Key(std::u16string(field.begin(),field.end())));
        fieldDescriptors.push_back({descriptor&&descriptor->writable,descriptor&&descriptor->enumerable,descriptor&&descriptor->configurable});
    }
    const std::array<std::string_view,21> methodNames={"set","setList","unset","get","getExplicit","hasExplicit","hasOneOf","getList","setInherited","getInherited","getDefault","isSet","hasDefault","getExplicitNames","getInheritedNames","getDefaultNames","getGlobalNames","getAllAttributes","getAllInherited","getAllDefaults","getAllGlobals"};
    std::vector<std::string> methodsInSourceOrder;
    for(const auto& method:methods)if(std::find(methodNames.begin(),methodNames.end(),method)!=methodNames.end())methodsInSourceOrder.push_back(method);
    std::vector<std::pair<std::string,int>> arities;
    std::vector<std::pair<std::string,std::string>> methodFunctionNames;
    std::vector<std::array<bool,3>> methodDescriptors;
    for(const auto method:methodNames){
        const auto function=context.get(Value(proto),Key(std::u16string(method.begin(),method.end())));
        const auto functionRef=std::get<ObjectRef>(function.data);
        const auto descriptor=context.descriptor(Value(proto),Key(std::u16string(method.begin(),method.end())));
        arities.emplace_back(std::string(method),static_cast<int>(std::get<double>(context.get(Value(functionRef),Key(u"length")).data)));
        methodFunctionNames.emplace_back(std::string(method),property_host::debugText(std::get<std::u16string>(context.get(Value(functionRef),Key(u"name")).data)));
        methodDescriptors.push_back({descriptor&&descriptor->writable,descriptor&&descriptor->enumerable,descriptor&&descriptor->configurable});
    }
    const auto constructorDescriptor=context.descriptor(Value(proto),Key(u"constructor"));
    const bool prototypeConstructorIdentity=constructorDescriptor&&constructorDescriptor->value==Value(ctor);
    const std::array<std::string_view,4> layerNames={"global","defaults","inherited","attributes"};
    std::vector<bool> layerLinks;
    std::vector<ObjectRef> layerObjects;
    for(const auto layer:layerNames)layerObjects.push_back(std::get<ObjectRef>(context.get(Value(object),Key(std::u16string(layer.begin(),layer.end()))).data));
    layerLinks.push_back(context.prototype(layerObjects[0])==context.intrinsics().objectPrototype);
    for(std::size_t i=1;i<layerObjects.size();++i)layerLinks.push_back(context.prototype(layerObjects[i])==layerObjects[i-1]);
    const auto set=context.get(Value(proto),Key(u"set"));
    const std::array<Value,2> setArgs{Value(u"actiontype"),Value(u"toggle")};
    context.callMember(set,Value(object),setArgs);
    const auto explicitNames=owner->getExplicitNames();
    const auto allAttributes=std::get<ObjectRef>(context.get(Value(object),Key(u"attributes")).data);
    const auto listed=context.callMember(context.get(Value(proto),Key(u"getAllAttributes")),Value(object));
    std::vector<bool> layerAliases;
    for(const auto method:std::array<std::string_view,4>{"getAllGlobals","getAllDefaults","getAllInherited","getAllAttributes"}){
        const std::string_view field=std::string_view(method)=="getAllGlobals"?"global":
            std::string_view(method)=="getAllDefaults"?"defaults":
            std::string_view(method)=="getAllInherited"?"inherited":"attributes";
        const auto actual=context.get(Value(object),Key(std::u16string(field.begin(),field.end())));
        const auto result=context.callMember(context.get(Value(proto),Key(std::u16string(method.begin(),method.end()))),Value(object));
        layerAliases.push_back(actual==result);
    }
    const auto get=context.get(Value(proto),Key(u"get"));
    const std::array<Value,1> colorArg{Value(u"mathcolor")};
    const auto color=context.callMember(get,Value(object),colorArg);

    PropertyList matrixDefaults{{"explicit",Property("default-explicit")},{"defaulted",Property("D")},{"fallback",Property("default-fallback")}};
    PropertyList matrixGlobals{{"explicit",Property("global-explicit")},{"fallback",Property("global-fallback")},{"globalOnly",Property("G")}};
    auto matrixOwner=std::make_shared<Attributes>(matrixDefaults,matrixGlobals);
    const auto matrixObject=binding.bind(matrixOwner);
    auto callMatrix=[&](std::string_view method,std::vector<Value> args={}){
        return context.callMember(context.get(Value(proto),Key(std::u16string(method.begin(),method.end()))),
            Value(matrixObject),args);
    };
    callMatrix("set",{Value(u"explicit"),Value(u"E")});
    callMatrix("set",{Value(u"fallback"),Value(u"_inherit_")});
    callMatrix("setInherited",{Value(u"inherited"),Value(u"I")});
    std::vector<std::string> matrixGet;
    for(const auto name:std::array<std::u16string_view,6>{u"explicit",u"inherited",u"defaulted",u"fallback",u"globalOnly",u"missing"})
        matrixGet.push_back(property_host::debugText(property_host::string(binding.fromValue(callMatrix("get",{Value(std::u16string(name))})))));
    std::vector<std::string> matrixGetExplicit;
    for(const auto name:std::array<std::u16string_view,2>{u"explicit",u"inherited"})
        matrixGetExplicit.push_back(property_host::debugText(property_host::string(binding.fromValue(callMatrix("getExplicit",{Value(std::u16string(name))})))));
    std::vector<bool> matrixHasExplicit,matrixIsSet,matrixHasDefault,matrixHasOneOf;
    for(const auto name:std::array<std::u16string_view,2>{u"explicit",u"inherited"})
        matrixHasExplicit.push_back(std::get<bool>(callMatrix("hasExplicit",{Value(std::u16string(name))}).data));
    for(const auto name:std::array<std::u16string_view,2>{u"inherited",u"missing"}){
        const auto names=context.array(2);
        context.set(names,Key(u"0"),Value(u"missing"));context.set(names,Key(u"1"),Value(std::u16string(name)));
        matrixHasOneOf.push_back(std::get<bool>(callMatrix("hasOneOf",{Value(names)}).data));
    }
    const std::array<std::u16string_view,3> checkNames={u"explicit",u"inherited",u"defaulted"};
    for(const auto name:checkNames)matrixIsSet.push_back(std::get<bool>(callMatrix("isSet",{Value(std::u16string(name))}).data));
    for(const auto name:std::array<std::u16string_view,3>{u"defaulted",u"globalOnly",u"missing"})
        matrixHasDefault.push_back(std::get<bool>(callMatrix("hasDefault",{Value(std::u16string(name))}).data));
    std::vector<std::string> matrixInheritedValues;
    for(const auto name:std::array<std::u16string_view,2>{u"inherited",u"defaulted"})
        matrixInheritedValues.push_back(property_host::debugText(property_host::string(binding.fromValue(callMatrix("getInherited",{Value(std::u16string(name))})))));
    std::vector<std::string> matrixDefaultValues;
    for(const auto name:std::array<std::u16string_view,2>{u"defaulted",u"globalOnly"})
        matrixDefaultValues.push_back(property_host::debugText(property_host::string(binding.fromValue(callMatrix("getDefault",{Value(std::u16string(name))})))));
    const auto matrixList=callMatrix("getList",{Value(u"explicit"),Value(u"inherited"),Value(u"defaulted"),Value(u"fallback"),Value(u"missing")});
    const auto matrixListObject=std::get<ObjectRef>(matrixList.data);
    std::vector<std::string> matrixListValues;
    for(const auto name:std::array<std::u16string_view,5>{u"explicit",u"inherited",u"defaulted",u"fallback",u"missing"})
        matrixListValues.push_back(property_host::debugText(property_host::string(binding.fromValue(context.get(Value(matrixListObject),Key(std::u16string(name)))))));
    callMatrix("unset",{Value(u"explicit")});
    const auto matrixUnsetGet=callMatrix("get",{Value(u"explicit")});
    const auto oldDefaults=context.get(Value(matrixObject),Key(u"defaults"));
    const auto oldInherited=std::get<ObjectRef>(context.get(Value(matrixObject),Key(u"inherited")).data);
    const auto replacement=context.object();context.set(replacement,Key(u"replacement"),Value(u"R"));
    context.set(matrixObject,Key(u"defaults"),Value(replacement));
    const auto matrixReplacementDefault=callMatrix("getDefault",{Value(u"replacement")});
    const auto matrixReplacementFlags=std::vector<bool>{
        std::get<bool>(callMatrix("hasDefault",{Value(u"defaulted")}).data),
        context.prototype(oldInherited)==std::get<ObjectRef>(oldDefaults.data),
        context.get(Value(matrixObject),Key(u"defaults"))==Value(replacement),
        std::get<ObjectRef>(callMatrix("getAllDefaults",{}).data)==replacement};
    const auto matrixOldChain=callMatrix("get",{Value(u"defaulted")});

    auto ordered=std::make_shared<Attributes>(PropertyList{},PropertyList{});
    const auto orderedObject=binding.bind(ordered),source=context.object();
    context.set(source,Key(u"first"),Value(1));
    bool laterSawWrite=false;
    const auto getter=context.function(u"later",[&](Context&,Value,std::span<const Value>)->Value{
        laterSawWrite=ordered->get("first")==Property(1);
        throw Exception("TypeError",u"later getter failed");
    });
    context.define(source,Key(u"later"),Descriptor::access(getter,{}));
    bool getterThrew=false;
    try{
        const std::array<Value,1> listArg{Value(source)};
        context.callMember(context.get(Value(proto),Key(u"setList")),Value(orderedObject),listArg);
    }catch(const Exception& error){getterThrew=error.name=="TypeError";}

    const std::array<Value,2> nullArgs{Value(Undefined{}),Value(Null{})};
    const auto nullGlobal=context.construct(binding.constructor(),nullArgs);
    const auto nullDefaults=std::get<ObjectRef>(context.get(Value(nullGlobal),Key(u"defaults")).data);
    bool nullNamesThrew=false;
    try{context.callMember(context.get(Value(proto),Key(u"getGlobalNames")),Value(nullGlobal));}
    catch(const Exception& error){nullNamesThrew=error.name=="TypeError";}
    const auto stringGlobals=context.object();
    const std::array<Value,2> stringArgs{Value(u"ab"),Value(stringGlobals)};
    const auto stringDefaults=context.construct(binding.constructor(),stringArgs);
    const auto getDefault=context.get(Value(proto),Key(u"getDefault"));
    const std::array<Value,1> zeroArg{Value(u"0")},oneArg{Value(u"1")};
    const auto zero=context.callMember(getDefault,Value(stringDefaults),zeroArg);
    const auto one=context.callMember(getDefault,Value(stringDefaults),oneArg);

    std::cout<<"{\"sourceHashes\":{\"ts\":\"c1e363944e0c059c7ed554b4b6aeb8ea56d43c844f84934c267ccc79b090468a\",\"cjs\":\"efa567c0c573f13f136f1d37b0af6bdc1a675f70cc13a9ae1cd7334321f30749\"},"
        <<"\"reflection\":{\"constructorName\":"
        <<quote(property_host::debugText(std::get<std::u16string>(context.get(Value(ctor),Key(u"name")).data)))
        <<",\"constructorLength\":"
        <<std::get<double>(context.get(Value(ctor),Key(u"length")).data)<<",\"constructorOwnKeys\":"<<array(ctorKeys)<<",\"constructorDescriptors\":{";
    for(std::size_t i=0;i<ctorKeys.size();++i){if(i)std::cout<<',';std::cout<<quote(ctorKeys[i])<<":{\"writable\":"<<yes(ctorDescriptors[i][0])<<",\"enumerable\":"<<yes(ctorDescriptors[i][1])<<",\"configurable\":"<<yes(ctorDescriptors[i][2])<<'}';}
    const auto ctorParent=context.prototype(ctor);
    const auto ctorPrototype=std::get<ObjectRef>(context.get(Value(ctor),Key(u"prototype")).data);
    const bool ctorPrototypeIdentity=ctorPrototype==proto;
    const bool instancePrototypeIdentity=context.prototype(object)==proto;
    std::cout<<"},\"constructorPrototypeParent\":"<<yes(ctorParent&&*ctorParent==context.intrinsics().functionPrototype)
        <<",\"constructorPrototypeIdentity\":"<<yes(ctorPrototypeIdentity)
        <<",\"instanceOwnKeys\":"<<array(own)<<",\"fieldDescriptors\":{";
    for(std::size_t i=0;i<own.size();++i){if(i)std::cout<<',';std::cout<<quote(own[i])<<":{\"writable\":"<<yes(fieldDescriptors[i][0])<<",\"enumerable\":"<<yes(fieldDescriptors[i][1])<<",\"configurable\":"<<yes(fieldDescriptors[i][2])<<'}';}
    std::cout<<"},\"instancePrototypeIdentity\":"<<yes(instancePrototypeIdentity)
        <<",\"methodNames\":"<<array(methodsInSourceOrder)
        <<",\"prototypeConstructorDescriptor\":{\"writable\":"<<yes(constructorDescriptor&&constructorDescriptor->writable)
        <<",\"enumerable\":"<<yes(constructorDescriptor&&constructorDescriptor->enumerable)
        <<",\"configurable\":"<<yes(constructorDescriptor&&constructorDescriptor->configurable)
        <<"},\"prototypeConstructorIdentity\":"<<yes(prototypeConstructorIdentity)<<",\"arity\":{";
    for(std::size_t i=0;i<arities.size();++i){if(i)std::cout<<',';std::cout<<quote(arities[i].first)<<':'<<arities[i].second;}
    std::cout<<"},\"methodFunctionNames\":{";
    for(std::size_t i=0;i<methodFunctionNames.size();++i){if(i)std::cout<<',';std::cout<<quote(methodFunctionNames[i].first)<<':'<<quote(methodFunctionNames[i].second);}
    std::cout<<"},\"methodDescriptors\":{";
    for(std::size_t i=0;i<methodNames.size();++i){if(i)std::cout<<',';std::cout<<quote(methodNames[i])<<":{\"writable\":"<<yes(methodDescriptors[i][0])<<",\"enumerable\":"<<yes(methodDescriptors[i][1])<<",\"configurable\":"<<yes(methodDescriptors[i][2])<<'}';}
    std::cout<<"},\"layerPrototypeLinks\":[";
    for(std::size_t i=0;i<layerNames.size();++i){if(i)std::cout<<',';std::cout<<"{\"name\":"<<quote(layerNames[i])<<",\"parent\":"<<(i?quote(layerNames[i-1]):"null")<<",\"correct\":"<<yes(layerLinks[i])<<'}';}
    std::cout<<"]},\"behavior\":{\"get\":"<<quote(std::get<std::u16string>(color.data))
        <<",\"explicitNames\":"<<array(explicitNames)<<",\"allAttributesAlias\":"<<yes(Value(allAttributes)==listed)
        <<",\"allLayerAliases\":[";
    for(std::size_t i=0;i<layerAliases.size();++i){if(i)std::cout<<',';std::cout<<yes(layerAliases[i]);}
    std::cout<<"],\"matrix\":{\"get\":"<<array(matrixGet)<<",\"getExplicit\":"<<array(matrixGetExplicit)
        <<",\"hasExplicit\":"<<boolArray(matrixHasExplicit)<<",\"hasOneOf\":"<<boolArray(matrixHasOneOf)
        <<",\"getInherited\":"<<array(matrixInheritedValues)<<",\"getDefault\":"<<array(matrixDefaultValues)
        <<",\"isSet\":"<<boolArray(matrixIsSet)<<",\"hasDefault\":"<<boolArray(matrixHasDefault)
        <<",\"getList\":"<<array(matrixListValues)<<",\"unsetGet\":"<<propertyString(binding.fromValue(matrixUnsetGet))
        <<",\"replacedDefaults\":{"
        <<"\"getDefault\":"<<propertyString(binding.fromValue(matrixReplacementDefault))
        <<",\"hasDefaultDefaulted\":"<<yes(matrixReplacementFlags[0])
        <<",\"getStillUsesPriorPrototype\":"<<propertyString(binding.fromValue(matrixOldChain))
        <<",\"inheritedPrototypePreserved\":"<<yes(matrixReplacementFlags[1])
        <<",\"allDefaultsIsReplacement\":"<<yes(matrixReplacementFlags[2]&&matrixReplacementFlags[3])
        <<"}}},\"setListFailure\":{\"name\":"<<quote(getterThrew?"TypeError":"none")
        <<",\"first\":"<<std::get<double>(ordered->getExplicit("first").value)
        <<",\"later\":"<<yes(ordered->hasExplicit("later"))
        <<",\"laterGetterSawEarlierWrite\":"<<yes(laterSawWrite)
        <<"},\"constructorCoercions\":{\"nullGlobalField\":"<<yes(std::holds_alternative<Null>(context.get(Value(nullGlobal),Key(u"global")).data))
        <<",\"nullGlobalPrototype\":"<<yes(!context.prototype(nullDefaults).has_value())
        <<",\"nullGlobalNamesError\":"<<quote(nullNamesThrew?"TypeError":"none")
        <<",\"stringDefault0\":"<<quote(std::get<std::u16string>(zero.data))
        <<",\"stringDefault1\":"<<quote(std::get<std::u16string>(one.data))<<"}}\n";
}
