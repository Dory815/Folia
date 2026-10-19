// Native port of the MML table transformations in EmpheqUtil.ts.
module folia.math.input.tex.empheq.util;
import folia.math.input.tex.parser;
import folia.math.core.mml_tree.mml_node;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
mml::MmlNode& node(mml::NodeValue value,std::string_view member){return *value.requireNode(member);}
mml::NodeValue create(TexParser& parser,std::u16string_view kind,mml::Children children=std::make_shared<tree::ChildArray<mml::MmlNode>>(),Property attributes=Missing{}){
    const std::array<std::any,3> args{std::any(Property(property_host::debugText(kind))),std::any(std::move(children)),std::any(std::move(attributes))};
    return parser.create(u"node",args);
}
mml::Children children(std::initializer_list<mml::NodeValue> values){auto result=std::make_shared<tree::ChildArray<mml::MmlNode>>();for(const auto& value:values)result->push_back(value);return result;}
std::u16string attribute(mml::NodeValue value,std::string_view name){auto& n=node(value,"attributes");const auto attrs=n.attributes();if(!attrs)property_host::typeError("Cannot read properties of null (reading 'get')");return property_host::string(attrs->get(name));}
void setAttribute(mml::NodeValue value,std::string_view name,std::u16string val){auto& n=node(value,"attributes");const auto attrs=n.attributes();if(!attrs)property_host::typeError("Cannot read properties of null (reading 'set')");attrs->set(name,Property(property_host::debugText(val)));}
std::size_t columnCount(mml::NodeValue table){auto& t=node(table,"childNodes");std::size_t max=0;if(!t.childNodes)return max;for(std::size_t i=0;i<t.childNodes->size();++i){const auto row=t.childNodes->get(i);auto& r=node(row,"childNodes");if(!r.childNodes)continue;const std::size_t count=r.childNodes->size()-(r.isKind("mlabeledtr")&&r.childNodes->size()?1:0);max=std::max(max,count);}return max;}
}
namespace {
using namespace options;
Key empheqKey(std::u16string_view value){return Key(std::u16string(value));}
ObjectRef empheqOwner(ParserRuntime& runtime){
    const auto* owner=std::get_if<ObjectRef>(&runtime.empheqUtilModuleExports.data);
    if(!owner)throw Exception("TypeError",u"EmpheqUtil module owner is unavailable");
    return *owner;
}
Value callEmpheq(ParserRuntime& runtime,Value receiver,std::u16string_view name,std::span<const Value> args){
    (void)EmpheqUtil::initializeModule(runtime);Context& cx=runtime.context();
    const Value method=cx.get(receiver,empheqKey(name));const auto* function=std::get_if<ObjectRef>(&method.data);
    if(!function||cx.kind(*function)!=ObjectKind::function)throw Exception("TypeError",u"EmpheqUtil."+std::u16string(name)+u" is not a function");
    return cx.call(*function,receiver,args);
}
Value callEmpheq(ParserRuntime& runtime,std::u16string_view name,std::span<const Value> args){
    (void)EmpheqUtil::initializeModule(runtime);
    return callEmpheq(runtime,Value(empheqOwner(runtime)),name,args);
}
Value empheqNodeValue(ParserRuntime& runtime,mml::NodeValue value){
    if(!value.isNode())return Value(Null{});const auto identity=runtime.mmlNodeIdentities.find(value.requireNode());
    if(!identity)throw Exception("Unimplemented",u"Empheq MML node has no canonical Context identity");return Value(*identity);
}
mml::NodeValue empheqNode(ParserRuntime& runtime,const Value& value){
    const auto* identity=std::get_if<ObjectRef>(&value.data);if(!identity)property_host::typeError("Empheq utility returned a non-node");return runtime.mmlNodeIdentities.resolve(*identity);
}
ParserRuntime& empheqRuntime(TexParser& parser){
    const auto* config=std::get_if<ParseOptions*>(&parser.configuration);if(!config||!*config)property_host::typeError("Cannot read properties of null or undefined (reading 'runtime')");return (**config).runtime();
}
void rowspanCellImpl(ParserRuntime&,Value,mml::NodeValue,std::u16string,mml::NodeValue,TexParser&,std::u16string);
void leftImpl(ParserRuntime&,Value,mml::NodeValue,mml::NodeValue,std::u16string,TexParser&,std::u16string);
void rightImpl(ParserRuntime&,Value,mml::NodeValue,mml::NodeValue,std::u16string,TexParser&,std::u16string);
void adjustTableImpl(ParserRuntime&,Value,BaseItem&,TexParser&);
}
options::Value EmpheqUtil::initializeModule(ParserRuntime& runtime){
    if(!std::holds_alternative<Undefined>(runtime.empheqUtilModuleExports.data))return runtime.empheqUtilModuleExports;
    Context& cx=runtime.context();const ObjectRef owner=cx.object();runtime.empheqUtilModuleExports=Value(owner);
    const auto install=[&](std::u16string name,double length,options::NativeCall body){
        const ObjectRef function=cx.function(name,std::move(body));
        cx.define(function,empheqKey(u"length"),Descriptor::data(Value(length),false,false,true));
        cx.set(Value(owner),empheqKey(name),Value(function));
    };
    // The bodies below are adapters over the existing EmpheqUtil algorithms.
    // Their node/parser conversions use runtime.mmlNodeIdentities and
    // runtime.parserIdentities; no duplicate MML algorithm is introduced.
    const auto textArg=[&runtime](std::span<const Value> a,std::size_t i){if(i>=a.size())return std::u16string{};return property_host::string(runtime.properties.from(a[i]));};
    const auto nodeArg=[&runtime](std::span<const Value> a,std::size_t i){if(i>=a.size())property_host::typeError("Empheq MML argument is missing");const auto* id=std::get_if<ObjectRef>(&a[i].data);if(!id)property_host::typeError("Empheq MML argument is not an object");return runtime.mmlNodeIdentities.resolve(*id);};
    const auto parserArg=[&runtime](std::span<const Value> a,std::size_t i)->TexParser&{if(i>=a.size())property_host::typeError("Empheq parser argument is missing");const auto* id=std::get_if<ObjectRef>(&a[i].data);if(!id)property_host::typeError("Empheq parser argument is not an object");const auto parser=runtime.parserIdentities.resolve(*id);if(!parser)property_host::typeError("Empheq parser argument is unavailable");return *parser;};
    install(u"splitOptions",2,[&runtime](Context&,Value,std::span<const Value> a)->Value{
        const auto text=a.empty()?std::u16string{}:property_host::string(runtime.properties.from(a[0]));
        const Value allowed=a.size()>1?a[1]:Value(Null{});return runtime.properties.to(EmpheqUtil::splitOptions(runtime,std::move(text),allowed));
    });
    install(u"columnCount",1,[&runtime,nodeArg](Context&,Value,std::span<const Value> a)->Value{
        return Value(static_cast<double>(columnCount(nodeArg(a,0))));
    });
    install(u"cellBlock",4,[&runtime,textArg,nodeArg,parserArg](Context&,Value,std::span<const Value> a)->Value{return empheqNodeValue(runtime,EmpheqUtil::cellBlock(textArg(a,0),nodeArg(a,1),parserArg(a,2),textArg(a,3)));});
    install(u"topRowTable",2,[&runtime,nodeArg,parserArg](Context&,Value,std::span<const Value> a)->Value{return empheqNodeValue(runtime,EmpheqUtil::topRowTable(nodeArg(a,0),parserArg(a,1)));});
    install(u"rowspanCell",5,[&runtime,textArg,nodeArg,parserArg](Context&,Value receiver,std::span<const Value> a)->Value{rowspanCellImpl(runtime,std::move(receiver),nodeArg(a,0),textArg(a,1),nodeArg(a,2),parserArg(a,3),textArg(a,4));return Value(Undefined{});});
    install(u"left",5,[&runtime,textArg,nodeArg,parserArg](Context&,Value receiver,std::span<const Value> a)->Value{leftImpl(runtime,std::move(receiver),nodeArg(a,0),nodeArg(a,1),textArg(a,2),parserArg(a,3),textArg(a,4));return Value(Undefined{});});
    install(u"right",5,[&runtime,textArg,nodeArg,parserArg](Context&,Value receiver,std::span<const Value> a)->Value{rightImpl(runtime,std::move(receiver),nodeArg(a,0),nodeArg(a,1),textArg(a,2),parserArg(a,3),textArg(a,4));return Value(Undefined{});});
    install(u"adjustTable",2,[&runtime,parserArg](Context&,Value receiver,std::span<const Value> a)->Value{if(a.empty())property_host::typeError("Empheq begin item is missing");const auto* id=std::get_if<ObjectRef>(&a[0].data);if(!id)property_host::typeError("Empheq begin item is not an object");const auto ref=runtime.stackItemIdentities.resolve(*id);auto* begin=ref?dynamic_cast<BaseItem*>(ref.get()):nullptr;if(!begin)property_host::typeError("Empheq begin item is not a BaseItem");adjustTableImpl(runtime,std::move(receiver),*begin,parserArg(a,1));return Value(Undefined{});});
    const ObjectRef allowEnv=cx.object();
    for(const auto name:{u"equation",u"align",u"gather",u"flalign",u"alignat",u"multline"})cx.set(Value(allowEnv),empheqKey(name),Value(true));
    cx.set(Value(owner),empheqKey(u"allowEnv"),Value(allowEnv));
    install(u"checkEnv",1,[&runtime](Context& context,Value receiver,std::span<const Value> a)->Value{
        const Value allowed=context.get(receiver,empheqKey(u"allowEnv"));if(a.empty())return Value(false);
        std::u16string env=property_host::string(runtime.properties.from(a[0]));if(!env.empty()&&env.back()==u'*')env.pop_back();
        return Value(context.hasOwn(allowed,empheqKey(env)));
    });
    return runtime.empheqUtilModuleExports;
}
options::Value EmpheqUtil::call(ParserRuntime& runtime,std::u16string_view name,std::span<const options::Value> args){
    (void)initializeModule(runtime);return callEmpheq(runtime,name,args);
}

mml::NodeValue EmpheqUtil::cellBlock(std::u16string tex,mml::NodeValue table,TexParser& parser,std::u16string env){
    auto mpadded=create(parser,u"mpadded",children({}),Property(PropertyList{{"height",0},{"depth",0},{"voffset","-1height"}}));
    const Property environment=std::visit([](const auto& value)->Property{using T=std::decay_t<decltype(value)>;if constexpr(std::is_same_v<T,std::shared_ptr<Stack>>)return value?value->env():Property(Missing{});else return Missing{};},parser.stack);
    const auto* configured=std::get_if<ParseOptions*>(&parser.configuration);if(!configured||!*configured)property_host::typeError("Cannot read properties of undefined (reading 'configuration')");
    auto nested=TexParser::make(std::move(tex),environment,**configured);const auto math=nested->mml();auto& result=node(math,"childNodes");
    if(!env.empty()){
        const auto tags=nested->tags();if(const auto* active=std::get_if<std::shared_ptr<Tags>>(&tags);active&&*active&&property_host::truthy((**configured).runtime().properties.from((**active).label()))){
            Context& context=(**configured).runtime().context();const Value current=context.get(Value((**active).sourceIdentity()),Key(u"currentTag"));
            context.set(current,Key(u"env"),Value(env));(void)(**active).getTag(Value(true));
        }
    }
    if(result.isInferred()&&result.childNodes){for(std::size_t i=0;i<result.childNodes->size();++i)node(mpadded,"appendChild").appendChild(result.childNodes->get(i));}
    else node(mpadded,"appendChild").appendChild(math);
    auto sized=create(parser,u"mpadded",children({table}),Property(PropertyList{{"width",0}}));
    auto phantom=create(parser,u"mphantom",children({sized}));node(mpadded,"appendChild").appendChild(phantom);
    return mpadded;
}
mml::NodeValue EmpheqUtil::topRowTable(mml::NodeValue original,TexParser& parser){
    auto table=ParseUtil::copyNode(original,parser);auto& copied=node(table,"childNodes");
    const auto copiedChildren=copied.childArrayOwner();
    if(!copiedChildren)property_host::typeError("Cannot read properties of null (reading 'slice')");
    copied.setChildren(copiedChildren->slice(0,1));
    setAttribute(table,"align",u"baseline 1");
    auto padded=create(parser,u"mpadded",children({table}),Property(PropertyList{{"width",0}}));
    return create(parser,u"mphantom",children({padded}));
}
namespace {
void rowspanCellImpl(ParserRuntime& runtime,Value receiver,mml::NodeValue mtd,std::u16string tex,mml::NodeValue table,TexParser& parser,std::u16string env){
    const std::array<Value,4> contentArgs{Value(tex),empheqNodeValue(runtime,ParseUtil::copyNode(table,parser)),Value(parser.instanceIdentity()),Value(env)};
    const auto content=empheqNode(runtime,callEmpheq(runtime,receiver,u"cellBlock",contentArgs));
    const std::array<Value,2> topArgs{empheqNodeValue(runtime,table),Value(parser.instanceIdentity())};
    const auto top=empheqNode(runtime,callEmpheq(runtime,receiver,u"topRowTable",topArgs));
    const auto padded=create(parser,u"mpadded",children({content,top}),Property(PropertyList{{"height",0},{"depth",0},{"voffset","height"}}));
    node(mtd,"appendChild").appendChild(padded);
}
void leftImpl(ParserRuntime& runtime,Value receiver,mml::NodeValue table,mml::NodeValue original,std::u16string tex,TexParser& parser,std::u16string env){
    setAttribute(table,"columnalign",u"right "+attribute(table,"columnalign"));
    setAttribute(table,"columnspacing",u"0em "+attribute(table,"columnspacing"));
    auto& t=node(table,"childNodes");if(!t.childNodes||t.childNodes->empty())t.appendChild(create(parser,u"mtr"));
    mml::NodeValue last(Missing{});
    for(std::size_t index=t.childNodes->size();index>0;--index){
        const auto row=t.childNodes->get(index-1);auto& r=node(row,"childNodes");
        const auto cell=create(parser,u"mtd");last=cell;
        if(!r.childNodes)property_host::typeError("Cannot read properties of null (reading 'unshift')");
        const std::array<mml::NodeValue,1> added{cell};(void)r.childNodes->splice(0,0,added);cell.requireNodeForWrite("parent")->setParent(row);
        if(r.isKind("mlabeledtr")){const auto label=r.childNodes->get(1),newCell=r.childNodes->get(0);r.childNodes->set(0,label);r.childNodes->set(1,newCell);}
    }
    const std::array<Value,5> args{empheqNodeValue(runtime,last),Value(tex),empheqNodeValue(runtime,original),Value(parser.instanceIdentity()),Value(env)};
    (void)callEmpheq(runtime,receiver,u"rowspanCell",args);
}
void rightImpl(ParserRuntime& runtime,Value receiver,mml::NodeValue table,mml::NodeValue original,std::u16string tex,TexParser& parser,std::u16string env){
    auto& t=node(table,"childNodes");if(!t.childNodes||t.childNodes->empty())t.appendChild(create(parser,u"mtr"));
    const auto row=t.childNodes->get(0);auto& r=node(row,"childNodes");
    const std::array<Value,1> countArgs{empheqNodeValue(runtime,table)};
    const auto m=static_cast<std::size_t>(runtime.context().toNumber(callEmpheq(runtime,u"columnCount",countArgs)))+(r.isKind("mlabeledtr")?1:0);
    if(!r.childNodes)property_host::typeError("Cannot read properties of null (reading 'length')");
    while(r.childNodes->size()<m)r.appendChild(create(parser,u"mtd"));
    const auto cell=create(parser,u"mtd");r.appendChild(cell);
    const std::array<Value,5> spanArgs{empheqNodeValue(runtime,cell),Value(tex),empheqNodeValue(runtime,original),Value(parser.instanceIdentity()),Value(env)};
    (void)callEmpheq(runtime,u"rowspanCell",spanArgs);
    auto split=[](const std::u16string& value){std::vector<std::u16string> words;std::size_t at=0;for(;;){const auto end=value.find(u' ',at);words.emplace_back(value.substr(at,end==std::u16string::npos?value.size()-at:end-at));if(end==std::u16string::npos)break;at=end+1;}return words;};
    auto join=[](const std::vector<std::u16string>& words){std::u16string result;for(std::size_t i=0;i<words.size();++i){if(i)result.push_back(u' ');result+=words[i];}return result;};
    auto align=split(attribute(table,"columnalign"));align.resize(std::min(align.size(),m));align.push_back(u"left");setAttribute(table,"columnalign",join(align));
    auto spacing=split(attribute(table,"columnspacing"));const std::size_t spacingLimit=m?m-1:(spacing.empty()?0:spacing.size()-1);spacing.resize(std::min(spacing.size(),spacingLimit));spacing.push_back(u"0em");setAttribute(table,"columnspacing",join(spacing));
}
void adjustTableImpl(ParserRuntime& runtime,Value receiver,BaseItem& empheq,TexParser& parser){
    const Property left=stack_property_host::scalar(empheq.getProperty("left"));
    const Property right=stack_property_host::scalar(empheq.getProperty("right"));
    if(property_host::truthy(left)||property_host::truthy(right)){
        const mml::NodeValue table=empheq.Last();const mml::NodeValue original=ParseUtil::copyNode(table,parser);
        if(property_host::truthy(left)){const std::array<Value,5> args{empheqNodeValue(runtime,table),empheqNodeValue(runtime,original),Value(property_host::string(left)),Value(parser.instanceIdentity()),Value(u"")};(void)callEmpheq(runtime,receiver,u"left",args);}
        if(property_host::truthy(right)){const std::array<Value,5> args{empheqNodeValue(runtime,table),empheqNodeValue(runtime,original),Value(property_host::string(right)),Value(parser.instanceIdentity()),Value(u"")};(void)callEmpheq(runtime,receiver,u"right",args);}
    }
}
}
void EmpheqUtil::rowspanCell(mml::NodeValue mtd,std::u16string tex,mml::NodeValue table,TexParser& parser,std::u16string env){auto& runtime=empheqRuntime(parser);const std::array<Value,5> args{empheqNodeValue(runtime,mtd),Value(std::move(tex)),empheqNodeValue(runtime,table),Value(parser.instanceIdentity()),Value(std::move(env))};(void)callEmpheq(runtime,u"rowspanCell",args);}
void EmpheqUtil::left(mml::NodeValue table,mml::NodeValue original,std::u16string tex,TexParser& parser,std::u16string env){auto& runtime=empheqRuntime(parser);const std::array<Value,5> args{empheqNodeValue(runtime,table),empheqNodeValue(runtime,original),Value(std::move(tex)),Value(parser.instanceIdentity()),Value(std::move(env))};(void)callEmpheq(runtime,u"left",args);}
void EmpheqUtil::right(mml::NodeValue table,mml::NodeValue original,std::u16string tex,TexParser& parser,std::u16string env){auto& runtime=empheqRuntime(parser);const std::array<Value,5> args{empheqNodeValue(runtime,table),empheqNodeValue(runtime,original),Value(std::move(tex)),Value(parser.instanceIdentity()),Value(std::move(env))};(void)callEmpheq(runtime,u"right",args);}
void EmpheqUtil::adjustTable(BaseItem& empheq,TexParser& parser){
    auto& runtime=empheqRuntime(parser);const auto factoryReference=empheq.sourceFactory();
    const auto* service=std::get_if<StackItemFactoryService*>(&factoryReference);
    if(!service||!*service)property_host::typeError("Empheq begin item has no source factory");
    const auto reference=(*service)->reference(&empheq);const auto identity=runtime.stackItemIdentities.find(reference);
    if(!identity)throw Exception("Unimplemented",u"Empheq begin item has no canonical Context identity");
    const std::array<Value,2> args{Value(*identity),Value(parser.instanceIdentity())};(void)callEmpheq(runtime,u"adjustTable",args);
}
Property EmpheqUtil::splitOptions(ParserRuntime& runtime,std::u16string text,Value allowed){return ParseUtil::keyvalOptions(runtime,std::move(text),std::move(allowed),true);}
bool EmpheqUtil::checkEnv(ParserRuntime& runtime,std::u16string env){const std::array<options::Value,1> args{options::Value(env)};return property_host::truthy(runtime.properties.from(call(runtime,u"checkEnv",args)));}
}
