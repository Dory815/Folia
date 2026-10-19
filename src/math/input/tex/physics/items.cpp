// Native port of MathJax 4.1.3 input/tex/physics/PhysicsItems.ts.
module folia.math.input.tex.physics.items;
import folia.math.input.tex.node_util;
import folia.math.input.tex.parser;
import folia.math.input.tex.stack_property;
import folia.math.input.tex.base_item_host;
import folia.math.input.tex.tex_constants;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view s){return Key(std::u16string(s));}
CheckType checked(CheckList values,bool success){auto r=std::make_shared<CheckTuple>();r->items=std::move(values);r->success=success;return r;}
CheckType complete(BaseItem& item,mml::NodeValue node){
    const auto factory=item.sourceFactory();auto* service=std::get_if<StackItemFactoryService*>(&factory);
    if(!service||!*service)property_host::typeError("Cannot read properties of undefined (reading 'create')");
    const std::any arg(node);auto* raw=(*service)->itemFactory().create("mml",tree::FactoryArgs(&arg,1));
    auto out=std::make_shared<std::vector<StackEntry>>();out->emplace_back((*service)->reference(raw));return checked(std::move(out),true);
}
Property autoOpenErrors(){
    static Property value=[] {
        auto record=PropertyList::withPrototype(PropertyList::record(BaseItem::baseErrors()).value());
        record.setOwn("stop",PropertyArray{"ExtraOrMissingDelims","Extra open or missing close delimiter"});
        return Property(std::move(record));
    }();
    return value;
}
}
std::string AutoOpen::kind()const{return "auto open";}
bool AutoOpen::isOpen()const{return true;}
bool AutoOpen::closing(std::u16string_view fence){
    const bool same=stack_property_host::scalar(getProperty("close"))==Property(property_host::debugText(fence));
    if(!same)return false;
    const double previous=openCount_--;
    return !property_host::truthy(Property(previous));
}
CheckType AutoOpen::checkItem(BaseItem* raw){
    BaseItem& item=base_item_host::item(raw,"getProperty");
    if(property_host::truthy(stack_property_host::scalar(item.getProperty("pre-autoclose"))))return BaseItem::fail;
    if(property_host::truthy(stack_property_host::scalar(item.getProperty("autoclose")))){
        if(property_host::truthy(stack_property_host::scalar(getProperty("ignore")))){Clear();return checked(std::make_shared<std::vector<StackEntry>>(),true);}
        return complete(*this,toMml());
    }
    if(item.isKind("mml")&&item.Size()==1){
        const auto node=item.toMml();
        if(node.isNode()){
            auto* m=node.requireNode("auto open nested delimiter");
            if(auto* token=dynamic_cast<mml::AbstractMmlTokenNode*>(m);token&&m->kind()=="mo"&&
               token->getText()==property_host::string(stack_property_host::scalar(getProperty("open"))))++openCount_;
        }
    }
    return BaseItem::checkItem(raw);
}
mml::NodeValue AutoOpen::toMml(Property inferred,Property forceRow){
    const bool useInferred=inferred.missing()||property_host::truthy(inferred);
    if(!useInferred)return BaseItem::toMml(inferred,std::move(forceRow));
    const auto factory=sourceFactory();
    auto* service=std::get_if<StackItemFactoryService*>(&factory);
    if(!service||!*service)property_host::typeError("Cannot read properties of undefined (reading 'configuration')");
    auto* configRef=std::get_if<StackItemFactoryConfiguration*>(&(*service)->configuration);
    auto* config=configRef&&*configRef?dynamic_cast<ParseOptions*>(*configRef):nullptr;
    if(!config)property_host::typeError("Cannot read properties of undefined (reading 'nodeFactory')");
    const auto parserRef=config->parser();const auto* parserValue=std::get_if<std::shared_ptr<TexParser>>(&parserRef);
    if(!parserValue||!*parserValue)property_host::typeError("Cannot read properties of undefined (reading 'stack')");
    auto& parser=**parserValue;
    const auto right=stack_property_host::scalar(getProperty("right"));
    if(property_host::truthy(stack_property_host::scalar(getProperty("smash")))){
        auto mml=BaseItem::toMml();
        auto child=std::make_shared<tree::ChildArray<mml::MmlNode>>();child->push_back(mml);
        const std::array<std::any,3> args{std::any(Property("mpadded")),std::any(child),std::any(Property(PropertyList{{"height",0},{"depth",0}}))};
        auto smash=parser.create(u"node",args);Clear();
        auto atomChild=std::make_shared<tree::ChildArray<mml::MmlNode>>();atomChild->push_back(smash);
        const std::array<std::any,2> atomArgs{std::any(Property("TeXAtom")),std::any(atomChild)};
        Push(std::array<mml::NodeValue,1>{parser.create(u"node",atomArgs)});
    }
    if(property_host::truthy(right)){
        const auto stackValue=parser.stack;const auto* currentStack=std::get_if<std::shared_ptr<Stack>>(&stackValue);
        if(!currentStack||!*currentStack)property_host::typeError("Cannot read properties of null or undefined (reading 'env')");
        TexParserRef nested=TexParser::make(property_host::string(right),(*currentStack)->env(),*config);
        Push(std::array<mml::NodeValue,1>{nested->mml()});
    }
    const auto bigValue=stack_property_host::scalar(getProperty("big"));
    const auto big=property_host::truthy(bigValue)?property_host::string(bigValue):std::u16string{};
    auto result=ParseUtil::fenced(*config,property_host::string(stack_property_host::scalar(getProperty("open"))),BaseItem::toMml(),
        property_host::string(stack_property_host::scalar(getProperty("close"))),big);
    const std::array<std::string,3> removed{"open","close","texClass"};
    NodeUtil::instance().removeProperties(result,removed);
    return result;
}
tree::FactoryClassRef<BaseItem> AutoOpen::initializeModule(ParserRuntime& runtime){
    const auto baseModule=runtime.stackItemModule;
    const auto* module=std::get_if<options::ObjectRef>(&baseModule.data);
    if(!module)throw options::Exception("TypeError",u"BaseItems module exports are unavailable");
    const auto baseValue=runtime.context().get(options::Value(*module),options::Key(u"BaseItem"));
    const auto* base=std::get_if<options::ObjectRef>(&baseValue.data);
    if(!base)throw options::Exception("TypeError",u"BaseItem constructor is unavailable");
    return StackItemFactoryBindings::registerSubclass(runtime,u"AutoOpen","auto open",0,typeid(AutoOpen),
        []{return std::make_unique<AutoOpen>();},[](BaseItem&,tree::FactoryArgs){},
        [](const BaseItem* p){return dynamic_cast<const AutoOpen*>(p)!=nullptr;},
        []{return autoOpenErrors();},*base,true,std::nullopt,true);
}
options::Value PhysicsItems::initializeModule(ParserRuntime& runtime){
    auto& cx=runtime.context();auto module=cx.object();
    const auto type=AutoOpen::initializeModule(runtime);const auto constructor=runtime.itemClassIdentities.find(type);
    if(!constructor)throw options::Exception("Unimplemented",u"AutoOpen constructor identity was not registered");
    cx.set(options::Value(module),options::Key(u"AutoOpen"),options::Value(*constructor));
    return options::Value(module);
}
}
