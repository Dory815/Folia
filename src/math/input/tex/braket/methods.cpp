// Native port of MathJax 4.1.3 input/tex/braket/BraketMethods.ts.
module folia.math.input.tex.braket.methods;
import folia.math.input.tex.braket.items;
import folia.math.input.tex.base_methods;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.token_map;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}
TexParser& parser(ParserContext& value){auto* p=dynamic_cast<TexParser*>(&value);if(!p)property_host::typeError("Braket requires a TexParser");return *p;}
std::u16string atomText(const ParseAtom& atom){if(const auto* p=std::get_if<Property>(&atom))return property_host::string(*p);if(const auto* token=std::get_if<TokenRef>(&atom);token&&*token)return property_host::string((*token)->character());property_host::typeError("Braket parse method token is not a string");}
Property restAt(ParseRest rest,std::size_t i){if(i>=rest.size())return Missing{};if(const auto* p=std::any_cast<Property>(&rest[i]))return *p;if(const auto* s=std::any_cast<std::u16string>(&rest[i]))return property_host::debugText(*s);if(const auto* b=std::any_cast<bool>(&rest[i]))return *b;if(const auto* n=std::any_cast<double>(&rest[i]))return *n;return Missing{};}
Stack& stack(TexParser& p){const auto value=p.stack;const auto* s=std::get_if<std::shared_ptr<Stack>>(&value);if(!s||!*s)property_host::typeError("Cannot read properties of null or undefined (reading 'height')");return **s;}
StackItemFactory& items(TexParser& p){const auto value=p.itemFactory();const auto* f=std::get_if<std::shared_ptr<StackItemFactory>>(&value);if(!f||!*f)property_host::typeError("Cannot read properties of null or undefined (reading 'create')");return **f;}
mml::NodeValue token(TexParser& p,std::string_view kind,Property attrs,Property text){const std::array<std::any,3> a{std::any(Property(std::string(kind))),std::any(std::move(attrs)),std::any(std::move(text))};return p.create(u"token",a);}
FunctionRef braket(){return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest rest)->ParseResult{
    auto& p=parser(context);const auto name=atomText(atom);const double index=p.i;
    (void)p.GetArgument(name);p.i=index;
    const auto next=p.GetNext();bool single=true;if(next==u"{"){p.i+=1;single=false;}
    const auto open=restAt(rest,0),close=restAt(rest,1),stretchy=restAt(rest,2),barmax=restAt(rest,3),space=restAt(rest,4);
    auto* item=dynamic_cast<BraketItem*>(items(p).itemFactory().create("braket",{}));
    if(!item)property_host::typeError("BraketItem constructor is not registered");
    PropertyList props{{"barcount",0},{"barmax",barmax},{"open",open},{"close",close},
        {"stretchy",stretchy},{"single",single},{"space",space}};
    item->setProperties(StackProperty(props));
    p.Push(items(p).reference(item));
    // Source pushes first, then writes through the pushed item's live env.
    // Stack::Push can change which environment is current, so assigning the
    // pre-push Stack.env changes both the receiver and bar lookup behavior.
    property_host::set(item->env(),"braketItem",Property(stack(p).numericHeight()-1));
    return Property(Missing{});
});}
FunctionRef bar(){return makeParseCallable([](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
    auto& p=parser(context);std::u16string mark=atomText(atom)==u"|"?u"|":u"\u2016";
    const Property env=stack(p).env();const auto envRecord=PropertyList::record(env);
    const Property start=envRecord?envRecord->get("braketItem"):Property(Missing{});
    const double n=stack(p).numericHeight()-property_host::number(start);
    const auto top=stack(p).Top(Property(n));auto* topItem=std::get_if<StackItemRef>(&top);
    auto* base=topItem&&*topItem?dynamic_cast<BaseItem*>(topItem->get()):nullptr;
    if(!base||!base->isKind("braket"))return Property(false);
    auto* target=dynamic_cast<BraketItem*>(base);if(!target)property_host::typeError("braket stack item identity is not registered");
    const double count=property_host::number(stack_property_host::scalar(target->getProperty("barcount")));
    const double maximum=property_host::number(stack_property_host::scalar(target->getProperty("barmax")));
    if(count>=maximum)return Property(false);
    if(mark==u"|"&&p.GetNext()==u"|"){p.i+=1;mark=u"\u2016";}
    if(!stack_property_host::truthy(target->getProperty("stretchy"))){
        p.Push(token(p,"mo",Property(PropertyList{{"stretchy",false},{"data-braketbar",true},{"texClass",mml::TEXCLASS.ORD}}),property_host::debugText(mark)));
        return Property(true);
    }
    auto* close=items(p).itemFactory().create("close",{});if(!close)property_host::typeError("CloseItem constructor is not registered");
    close->setProperty("braketbar",StackProperty(true));p.Push(items(p).reference(close));
    const auto leftChildren=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    const auto leftAttrs=Property(PropertyList{{"texClass",mml::TEXCLASS.CLOSE}});
    const std::array<std::any,3> leftArgs{std::any(Property("TeXAtom")),std::any(leftChildren),std::any(leftAttrs)};
    const auto left=target->create("node",FunctionArgs(leftArgs));
    const auto middle=token(p,"mo",Property(PropertyList{{"stretchy",true},{"data-braketbar",true},{"texClass",mml::TEXCLASS.BIN}}),property_host::debugText(mark));
    const auto rightChildren=std::make_shared<tree::ChildArray<mml::MmlNode>>();
    const auto rightAttrs=Property(PropertyList{{"texClass",mml::TEXCLASS.OPEN}});
    const std::array<std::any,3> rightArgs{std::any(Property("TeXAtom")),std::any(rightChildren),std::any(rightAttrs)};
    const auto right=target->create("node",FunctionArgs(rightArgs));
    target->barNodes().push_back(left);target->barNodes().push_back(middle);target->barNodes().push_back(right);
    target->setProperty("barcount",StackProperty(count+1));return Property(true);
});}
}
BraketMethodSet BraketMethods::create(ParserRuntime& runtime){
    const auto bracketMethod=braket();
    const auto barMethod=bar();
    // These FunctionRefs are stored directly in the Braket token maps. Bind
    // those same bodies to the runtime's canonical Context identities so
    // reflection and invocation both observe the actual source callable.
    for(const auto& [body,name,length]:std::array{
            std::tuple{bracketMethod,std::u16string_view(u"Braket"),7.0},
            std::tuple{barMethod,std::u16string_view(u"Bar"),2.0}}){
        const auto bound=BaseMethods::bind(runtime,body,std::u16string(name));
        const auto* identity=std::get_if<options::ObjectRef>(&bound.data);
        if(!identity)throw options::Exception("TypeError",u"Braket method has no Context identity");
        runtime.context().define(*identity,options::Key(u"length"),options::Descriptor::data(
            options::Value(length),false,false,true));
    }
    return {bracketMethod,barMethod,BaseMethods::get(runtime,u"Macro")};
}
}
