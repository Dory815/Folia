/*************************************************************
 * Copyright (c) 2017-2026 The MathJax Consortium.
 * Native implementation of MathJax 4.1.3 input/tex/base/BaseMethods.ts.
 * Licensed under Apache-2.0; see src/math/LICENSE-MathJax.txt.
 *************************************************************/
module folia.math.input.tex.base_methods;

import folia.math.input.tex.parser;
import folia.math.input.tex.stack_item_factory;
import folia.math.input.tex.stack_host;
import folia.math.input.tex.node_util;
import folia.math.input.tex.tex_constants;
import folia.math.input.tex.handler_types;
import folia.math.input.tex.tex_error;
import folia.math.core.mml_tree.operator_dictionary;
import folia.math.core.mml_tree.mml_nodes.mo;
import folia.math.core.mml_tree.mml_nodes.msubsup;
import folia.math.core.mml_tree.mml_nodes.munderover;
import folia.math.core.mml_tree.mo_patterns;
import folia.math.util.lengths;
import folia.math.input.tex.unit_util;
import folia.math.util.property_host;
import folia.math.util.string;
import folia.math.util.js_numeric_input;
import folia.math.util.js_number;
import folia.math.util.option_functions;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view value){return Key(std::u16string(value));}

TexParser& parserFrom(ParserContext& context){
    auto* parser=dynamic_cast<TexParser*>(&context);
    if(!parser)property_host::typeError("Base parse method requires a TexParser");
    return *parser;
}
StackItemFactory& itemFactory(TexParser& parser){
    const auto factory=parser.itemFactory();
    const auto* value=std::get_if<std::shared_ptr<StackItemFactory>>(&factory);
    if(!value||!*value)property_host::typeError("Cannot read properties of null or undefined (reading 'create')");
    return **value;
}
Stack& stack(TexParser& parser){
    const auto value=parser.stack;
    const auto* result=std::get_if<std::shared_ptr<Stack>>(&value);
    if(!result||!*result)property_host::typeError("Cannot read properties of null or undefined (reading 'Top')");
    return **result;
}
Tags& tags(TexParser& parser){
    const auto value=parser.tags();
    const auto* tagsValue=std::get_if<std::shared_ptr<Tags>>(&value);
    if(!tagsValue||!*tagsValue)property_host::typeError("Cannot read properties of null or undefined (reading 'allLabels')");
    return **tagsValue;
}
ParseOptions& configuration(TexParser& parser){
    const auto* value=std::get_if<ParseOptions*>(&parser.configuration);
    if(!value||!*value)property_host::typeError("Cannot read properties of null or undefined (reading 'options')");
    return **value;
}
std::u16string sliceFrom(std::u16string value,double index){
    const double size=static_cast<double>(value.size());
    if(std::isnan(index))index=0;
    else if(!std::isfinite(index))index=index<0?-size:size;
    else index=std::trunc(index);
    const double start=index<0?std::max(size+index,0.0):std::min(index,size);
    return value.substr(static_cast<std::size_t>(start));
}
Property valueAt(ParseRest rest,std::size_t index){
    if(index>=rest.size())return Missing{};
    if(const auto* value=std::any_cast<Property>(&rest[index]))return *value;
    if(const auto* value=std::any_cast<std::u16string>(&rest[index]))return property_host::debugText(*value);
    if(const auto* value=std::any_cast<bool>(&rest[index]))return *value;
    if(const auto* value=std::any_cast<double>(&rest[index]))return *value;
    return Missing{};
}
std::u16string textAt(ParseRest rest,std::size_t index){return property_host::string(valueAt(rest,index));}
std::u16string atomText(const ParseAtom& atom){
    if(const auto* property=std::get_if<Property>(&atom))return property_host::string(*property);
    if(const auto* value=std::get_if<TokenRef>(&atom);value&&*value)return property_host::string((*value)->token());
    property_host::typeError("Base parse method requires a string token");
}
bool boolAt(ParseRest rest,std::size_t index){return property_host::truthy(valueAt(rest,index));}
std::any tokenArgs(std::u16string_view kind,Property definition,Property text){
    return std::any(std::array<std::any,3>{std::any(Property(property_host::debugText(kind))),std::any(std::move(definition)),std::any(std::move(text))});
}
mml::NodeValue token(TexParser& parser,std::u16string_view kind){
    const std::array<std::any,1> args{std::any(Property(property_host::debugText(kind)))};
    return parser.create(u"token",args);
}
mml::NodeValue token(TexParser& parser,std::u16string_view kind,Property definition){
    const std::array<std::any,2> args{std::any(Property(property_host::debugText(kind))),std::any(std::move(definition))};
    return parser.create(u"token",args);
}
mml::NodeValue token(TexParser& parser,std::u16string_view kind,Property definition,Property text){
    const std::array<std::any,3> args{std::any(Property(property_host::debugText(kind))),std::any(std::move(definition)),std::any(std::move(text))};
    return parser.create(u"token",args);
}
ParseResult pushed(TexParser& parser,mml::NodeValue node){parser.Push(std::move(node));return Property(Missing{});}
mml::NodeValue node(TexParser& parser,std::u16string_view kind){
    const std::array<std::any,1> args{std::any(Property(property_host::debugText(kind)))};
    return parser.create(u"node",args);
}
mml::NodeValue node(TexParser& parser,std::u16string_view kind,mml::Children children){
    const std::array<std::any,2> args{std::any(Property(property_host::debugText(kind))),std::any(std::move(children))};
    return parser.create(u"node",args);
}
mml::NodeValue node(TexParser& parser,std::u16string_view kind,mml::Children children,Property attributes){
    const std::array<std::any,3> args{std::any(Property(property_host::debugText(kind))),std::any(children),std::any(std::move(attributes))};
    return parser.create(u"node",args);
}
mml::Children childList(std::initializer_list<mml::NodeValue> values){
    auto result=std::make_shared<tree::ChildArray<mml::MmlNode>>();for(const auto& value:values)result->push_back(value);return result;
}
mml::Children emptyChildren(){return std::make_shared<tree::ChildArray<mml::MmlNode>>();}
BaseItem* makeItem(TexParser& parser,std::string_view kind,FunctionArgs rest={}){
    const auto factory=parser.itemFactory();
    const auto* value=std::get_if<std::shared_ptr<StackItemFactory>>(&factory);
    if(!value||!*value)property_host::typeError("Cannot read properties of null or undefined (reading 'create')");
    return (*value)->create(kind,tree::FactoryArgs(rest));
}
template<std::size_t N>
BaseItem* makeItem(TexParser& parser,std::string_view kind,const std::array<std::any,N>& rest){
    return makeItem(parser,kind,FunctionArgs(rest));
}
std::optional<regexp_host::Match> exec(ParserRuntime& runtime,std::u16string pattern,const std::u16string& input){
    const auto object=runtime.regexp().create(std::move(pattern));
    return runtime.regexp().exec(Value(object),input);
}
ParseResult pushedItem(TexParser& parser,std::string_view kind){
    const auto ref=itemFactory(parser).reference(itemFactory(parser).create(kind,{}));
    parser.Push(ref);return Property(Missing{});
}
[[noreturn]] void texError(std::u16string id,std::u16string_view message,std::span<const Property> args={}){
    throw TexError(std::move(id),message,args);
}

using Body=ParseBody;
struct MethodTable {
    std::vector<std::pair<std::u16string,FunctionRef>> entries;
    FunctionRef find(std::u16string_view name)const{
        const auto it=std::find_if(entries.begin(),entries.end(),[&](const auto& entry){return entry.first==name;});
        return it==entries.end()?FunctionRef{}:it->second;
    }
};

std::size_t sourceMethodLength(std::u16string_view name){
    // CJS function metadata for the pinned BaseMethods object (the emitted
    // method lengths, including defaulted trailing parameters).
    static constexpr std::array<std::pair<std::u16string_view,std::size_t>,75> lengths{{
        {u"Open",2},{u"Close",2},{u"Bar",2},{u"Tilde",2},{u"Space",2},
        {u"Superscript",2},{u"Subscript",2},{u"Prime",2},{u"Comment",2},{u"Hash",2},
        {u"MathFont",4},{u"SetFont",3},{u"SetStyle",5},{u"SetSize",3},{u"Spacer",3},
        {u"DiscretionaryTimes",2},{u"AllowBreak",2},{u"Break",2},{u"Linebreak",3},{u"LeftRight",2},
        {u"NamedFn",3},{u"NamedOp",3},{u"Limits",3},{u"Over",4},{u"Frac",2},{u"Sqrt",2},
        {u"Root",2},{u"MoveRoot",3},{u"Accent",4},{u"UnderOver",4},{u"Overset",2},
        {u"Underset",2},{u"Overunderset",2},{u"TeXAtom",3},{u"VBox",3},{u"Hsize",2},
        {u"ParBox",2},{u"BreakAlign",2},{u"MmlToken",2},{u"Strut",2},{u"Phantom",4},
        {u"Smash",2},{u"Lap",2},{u"RaiseLower",2},{u"MoveLeftRight",2},{u"Hskip",3},
        {u"Nonscript",2},{u"Rule",3},{u"rule",2},{u"MakeBig",4},{u"BuildRel",2},
        {u"HBox",4},{u"FBox",2},{u"FrameBox",2},{u"MakeBox",2},{u"Not",2},{u"Dots",2},
        {u"Matrix",10},{u"Entry",2},{u"Cr",2},{u"CrLaTeX",3},{u"HLine",3},{u"HFill",2},
        {u"NewColumnType",2},{u"BeginEnd",2},{u"Array",9},{u"AlignedArray",3},{u"IndentAlign",2},
        {u"Equation",4},{u"EqnArray",7},{u"HandleNoTag",2},{u"HandleLabel",2},{u"HandleRef",3},
        {u"Macro",5},{u"MathChoice",2}
    }};
    const auto found=std::find_if(lengths.begin(),lengths.end(),[&](const auto& item){return item.first==name;});
    if(found==lengths.end())throw Exception("TypeError",u"BaseMethods source member has no metadata");
    return found->second;
}

FunctionRef method(Body body){return makeParseCallable(std::move(body));}

std::shared_ptr<const MethodTable> makeMethods(){
    auto table=std::make_shared<MethodTable>();
    const auto add=[&](std::u16string name,Body body){table->entries.emplace_back(std::move(name),method(std::move(body)));};
    add(u"Open",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{return pushedItem(parserFrom(c),"open");});
    add(u"Close",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{return pushedItem(parserFrom(c),"close");});
    add(u"Bar",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto* character=std::get_if<Property>(&atom);if(!character)property_host::typeError("Bar character must be a string");
        PropertyList def{{"stretchy",false},{"texClass",mml::TEXCLASS.ORD}};
        const auto node=token(p,u"mo",Property(def),*character);NodeUtil::instance().setProperty(node,"keep-attrs",Property("stretchy"));
        return pushed(p,node);
    });
    add(u"Tilde",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{return pushed(parserFrom(c),token(parserFrom(c),u"mtext",PropertyList{},Property(std::string("\xC2\xA0"))));});
    add(u"Space",[](const FunctionReceiver&,ParserContext&,const ParseAtom&,ParseRest)->ParseResult{return Property(Missing{});});
    auto script=[&](std::u16string methodName,bool superscript){
        add(std::move(methodName),[superscript](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
            auto& p=parserFrom(context);
            if(const auto next=p.GetNext();!next.empty()&&next[0]>=u'0'&&next[0]<=u'9'){
                auto source=p.string();const auto at=static_cast<std::size_t>(std::max(0.0,p.i+1));
                source.insert(std::min(at,source.size()),1,u' ');p.setString(std::move(source));
            }
            const StackEntry topEntry=stack(p).Top();BaseItem& top=stack_host::item(topEntry,"isKind");
            mml::NodeValue primes=Missing{};mml::NodeValue base=PropertyNull{};
            if(top.isKind("prime")){
                if(top.hasSourceNodesOwner()){
                    const auto values=top.sourceReadPeek(options::Value(2.0),2);
                    base=top.sourceNodeFromSlice(values,0);primes=top.sourceNodeFromSlice(values,1);
                }else{
                    const auto values=top.Peek(2);if(values){base=values->get(0);primes=values->get(1);}
                }
                stack(p).Pop();
            }else{
                base=stack(p).Prev();
                if(!base.isNode())base=token(p,u"mi",Property(PropertyList{}),Property(""));
            }
            const Property moves=NodeUtil::instance().getProperty(base,"movesupsub");
            const bool subSup=NodeUtil::instance().isType(base,"msubsup");
            const bool isSup=NodeUtil::instance().isType(base,"msup");
            const bool underOver=NodeUtil::instance().isType(base,"munderover");
            const bool isMover=NodeUtil::instance().isType(base,"mover");
            std::size_t position=superscript?2:1;
            if(subSup){auto* scripted=dynamic_cast<mml::MmlMsubsup*>(base.requireNode("sup"));position=superscript?scripted->sup():scripted->sub();}
            else if(underOver){auto* scripted=dynamic_cast<mml::MmlMunderover*>(base.requireNode("over"));position=superscript?scripted->over():scripted->under();}
            const mml::NodeValue existing=(subSup||underOver)?NodeUtil::instance().getChildAt(base,position):mml::NodeValue(Missing{});
            if(((subSup&&!isSup)&&existing.isNode())||
               (underOver&&!isMover&&existing.isNode()&&!property_host::truthy(NodeUtil::instance().getProperty(base,"subsupOK"))))
                texError(superscript?u"DoubleExponent":u"DoubleSubscripts",superscript?u"Double exponent: use braces to clarify":u"Double subscripts: use braces to clarify");
            std::size_t slot=superscript?2:1;
            if(!subSup||isSup){
                if(property_host::truthy(moves)){
                    if(!underOver||isMover||existing.isNode())base=node(p,u"munderover",childList({base}),Property(PropertyList{{"movesupsub",true}}));
                    slot=superscript?dynamic_cast<mml::MmlMunderover*>(base.requireNode("over"))->over():dynamic_cast<mml::MmlMunderover*>(base.requireNode("under"))->under();
                }else {base=node(p,u"msubsup",childList({base}));slot=superscript?2:1;}
            }
            BaseItem* item=makeItem(p,"subsup",std::array<std::any,1>{std::any(base)});
            StackPropertyList properties{{"position",static_cast<double>(slot)},{"primes",primes},{"movesupsub",moves}};
            item->setProperties(properties);p.Push(itemFactory(p).reference(item));return Property(Missing{});
        });
    };
    script(u"Superscript",true);script(u"Subscript",false);
    add(u"Limits",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(context);const bool limits=boolAt(rest,0);mml::NodeValue op=stack(p).Prev(true);
        if(!op.isNode()||(NodeUtil::instance().getTexClass(NodeUtil::instance().getCoreMO(op))!=mml::TEXCLASS.OP&&
                          property_host::nullish(NodeUtil::instance().getProperty(op,"movesupsub")))){
            const std::array<Property,1> args{p.currentCS()};
            texError(u"MisplacedLimits",u"%1 is allowed only on operators",args);
        }
        const StackEntry topEntry=stack(p).Top();BaseItem& top=stack_host::item(topEntry,"First");
        if(NodeUtil::instance().isType(op,"munderover")&&!limits){
            auto converted=node(p,u"msubsup");NodeUtil::instance().copyChildren(op,converted);top.setFirst(converted);op=converted;
        }else if(NodeUtil::instance().isType(op,"msubsup")&&limits){
            auto converted=node(p,u"munderover");NodeUtil::instance().copyChildren(op,converted);top.setFirst(converted);op=converted;
        }
        NodeUtil::instance().setProperty(op,"movesupsub",Property(limits));
        const auto core=NodeUtil::instance().getCoreMO(op);NodeUtil::instance().setProperties(core,Property(PropertyList{{"movablelimits",false}}));
        const Property movable=NodeUtil::instance().isType(op,"mo")?NodeUtil::instance().getMoAttribute(op,"movableLimits"):
            NodeUtil::instance().getAttribute(op,"movablelimits");
        if(property_host::truthy(movable)||property_host::truthy(NodeUtil::instance().getProperty(op,"movablelimits")))
            NodeUtil::instance().setProperties(op,Property(PropertyList{{"movablelimits",false}}));
        return Property(Missing{});
    });
    add(u"Comment",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{
        auto& p=parserFrom(c);auto value=p.string();double i=p.i;
        while(i<static_cast<double>(value.size())&&value[static_cast<std::size_t>(i)]!=u'\n')++i;
        p.i=i;return Property(Missing{});
    });
    add(u"Hash",[](const FunctionReceiver&,ParserContext&,const ParseAtom&,ParseRest)->ParseResult{
        texError(u"CantUseHash1",u"You can't use 'macro parameter character #' in math mode");
    });
    add(u"Prime",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);mml::NodeValue base=stack(p).Prev();
        if(!base.isNode())base=token(p,u"mi");
        const auto kind=NodeUtil::instance().isType(base,"msubsup")?std::string_view("msubsup"):
                        NodeUtil::instance().isType(base,"munderover")?std::string_view("munderover"):std::string_view{};
        if(!kind.empty()){
            const auto position=NodeUtil::instance().getChildAt(base,2);
            const bool eligible=kind=="msubsup"?!NodeUtil::instance().isType(base,"msup"):
                (!NodeUtil::instance().isType(base,"mover")&&!property_host::truthy(NodeUtil::instance().getProperty(base,"subsupOK")));
            if(position.isNode()&&eligible)
                texError(u"DoubleExponentPrime",u"Prime causes double exponent: use braces to clarify");
        }
        std::u16string superscript;double index=p.i-1;p.i=index;
        for(;;){superscript+=u"\u2032";p.i+=1;const auto next=p.GetNext();if(next!=u"'"&&next!=u"\u2019")break;}
        switch(superscript.size()){
        case 1:superscript=u"\u2032";break;case 2:superscript=u"\u2033";break;
        case 3:superscript=u"\u2034";break;case 4:superscript=u"\u2057";break;default:break;
        }
        const auto prime=token(p,u"mo",Property(PropertyList{{"variantForm",true}}),Property(property_host::debugText(superscript)));
        BaseItem* item=makeItem(p,"prime",std::array<std::any,2>{std::any(base),std::any(prime)});
        p.Push(itemFactory(p).reference(item));return Property(Missing{});
    });
    add(u"SetFont",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const Property font=valueAt(rest,0);
        property_host::set(stack(p).env(),"font",font);return pushedItem(p,"null");
    });
    add(u"SetStyle",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto style=textAt(rest,0);const auto display=valueAt(rest,1);const auto level=valueAt(rest,2);
        property_host::set(stack(p).env(),"style",Property(property_host::debugText(style)));property_host::set(stack(p).env(),"level",level);
        PropertyList attrs{{"displaystyle",display},{"scriptlevel",level}};
        BaseItem* item=itemFactory(p).create("style",{});item->setProperty("styles",StackProperty(Property(attrs)));p.Push(itemFactory(p).reference(item));return Property(Missing{});
    });
    add(u"SetSize",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const Property size=valueAt(rest,0);property_host::set(stack(p).env(),"size",size);
        BaseItem* item=itemFactory(p).create("style",{});
        item->setProperty("styles",StackProperty(Property(PropertyList{{"mathsize",lengths::em(property_host::number(size))}})));
        p.Push(itemFactory(p).reference(item));return Property(Missing{});
    });
    add(u"MathFont",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto text=property_host::string(p.GetArgument(name));
        const Value options=p.options();
        const auto* config=std::get_if<ParseOptions*>(&p.configuration);
        if(!config||!*config)property_host::typeError("Cannot read properties of null or undefined (reading 'options')");
        ParserRuntime& runtime=(**config).runtime();
        const Value identifiers=runtime.context().get(options,key(u"identifierPattern"));
        PropertyList childEnv{{"multiLetterIdentifiers",runtime.properties.from(identifiers)}};
        const Property parentEnv=stack(p).env();
        for(const auto& key:property_host::ownKeys(parentEnv))childEnv.setOwn(key,property_host::get(parentEnv,key));
        childEnv.set("font",valueAt(rest,0));
        const Property italic=rest.size()>1?valueAt(rest,1):Property(Missing{});
        childEnv.set("italicFont",italic.missing()?Property(""):italic);
        childEnv.set("noAutoOP",Property(true));
        Property env=Property(childEnv);
        const auto nested=TexParser::make(text,env,**config);
        return pushed(p,node(p,u"TeXAtom",childList({nested->mml()})));
    });
    add(u"Spacer",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const Property space=valueAt(rest,0);
        const std::array<std::any,4> spaceArgs{std::any(Property("mspace")),std::any(std::make_shared<tree::ChildArray<mml::MmlNode>>()),std::any(Property(PropertyList{{"width",lengths::em(property_host::number(space))}})),std::any(Property(Missing{}))};
        const auto mspace=p.create(u"node",spaceArgs);
        const auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();children->push_back(mspace);
        const std::array<std::any,3> styleArgs{std::any(Property("mstyle")),std::any(children),std::any(Property(PropertyList{{"scriptlevel",0}}))};
        return pushed(p,p.create(u"node",styleArgs));
    });
    add(u"DiscretionaryTimes",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{
        return pushed(parserFrom(c),token(parserFrom(c),u"mo",PropertyList{{"linebreakmultchar",Property("\xC3\x97")}},Property("\xE2\x81\xA2")));
    });
    add(u"AllowBreak",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{return pushed(parserFrom(c),token(parserFrom(c),u"mspace"));});
    add(u"Break",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{return pushed(parserFrom(c),token(parserFrom(c),u"mspace",PropertyList{{"linebreak","newline"}}));});
    add(u"Linebreak",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);
        const Property linebreak=valueAt(rest,0);
        bool insert=true;
        const mml::NodeValue previous=stack(p).Prev(Property(true));
        if(previous.isNode()&&previous.requireNode("isKind")->isKind("mo")){
            const Property style=NodeUtil::instance().getMoAttribute(previous,"linebreakstyle");
            const Property linebreakStyles=property_host::get(TexConstant,"LineBreakStyle");
            const Property before=property_host::get(linebreakStyles,"BEFORE");
            if(style!=before){
                mml::MmlNode& receiver=*previous.requireNode("attributes");
                const auto attributes=receiver.attributes();
                if(!attributes)property_host::typeError("Cannot read properties of null (reading 'set')");
                attributes->set("linebreak",linebreak);
                insert=false;
            }
        }
        const std::array<std::any,2> args{std::any(linebreak),std::any(Property(insert))};
        const auto item=itemFactory(p).reference(makeItem(p,"break",args));
        p.Push(item);
        return Property(Missing{});
    });
    add(u"Hskip",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto width=p.GetDimen(name);const auto result=node(p,u"mspace",emptyChildren(),Property(PropertyList{{"width",Property(property_host::debugText(width))}}));
        if(boolAt(rest,0))NodeUtil::instance().setAttribute(result,"linebreak",Property("nobreak"));return pushed(p,result);
    });
    add(u"Nonscript",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{return pushedItem(parserFrom(c),"nonscript");});
    add(u"Rule",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto width=p.GetDimen(name),height=p.GetDimen(name),depth=p.GetDimen(name);
        PropertyList attrs{{"width",Property(property_host::debugText(width))},{"height",Property(property_host::debugText(height))},{"depth",Property(property_host::debugText(depth))}};const auto style=textAt(rest,0);
        if(style!=u"blank"){const auto color=property_host::get(stack(p).env(),"color");attrs.set("mathbackground",property_host::truthy(color)?color:Property("black"));}
        return pushed(p,node(p,u"mspace",emptyChildren(),Property(attrs)));
    });
    add(u"rule",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const Property vertical=p.GetBrackets(name);const auto width=p.GetDimen(name),height=p.GetDimen(name);
        const auto color=property_host::get(stack(p).env(),"color");const auto space=node(p,u"mspace",emptyChildren(),Property(PropertyList{{"width",Property(property_host::debugText(width))},{"height",Property(property_host::debugText(height))},{"mathbackground",property_host::truthy(color)?color:Property("black")}}));
        if(!property_host::truthy(vertical))return pushed(p,space);
        auto padded=node(p,u"mpadded",childList({space}),Property(PropertyList{{"voffset",vertical}}));
        const std::u16string v=property_host::string(vertical);
        if(!v.empty()&&v.front()==u'-'){
            NodeUtil::instance().setAttribute(padded,"height",vertical);
            NodeUtil::instance().setAttribute(padded,"depth",Property(property_host::debugText(u"+"+v.substr(1))));
        }else NodeUtil::instance().setAttribute(padded,"height",Property(property_host::debugText(u"+"+v)));
        return pushed(p,padded);
    });
    add(u"HBox",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto text=property_host::string(p.GetArgument(atomText(atom)));
        const Property style=valueAt(rest,0),font=valueAt(rest,1);
        const auto* config=std::get_if<ParseOptions*>(&p.configuration);if(!config||!*config)property_host::typeError("Cannot read properties of null or undefined (reading 'internalMath')");
        const auto children=ParseUtil::internalMath(p,text,style,font);
        p.PushAll(children);return Property(Missing{});
    });
    add(u"FBox",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto text=property_host::string(p.GetArgument(atomText(atom)));
        const auto* config=std::get_if<ParseOptions*>(&p.configuration);if(!config||!*config)property_host::typeError("Cannot read properties of null or undefined (reading 'internalMath')");
        const auto children=ParseUtil::internalMath(p,text,Missing{},Missing{});
        return pushed(p,node(p,u"menclose",children,Property(PropertyList{{"notation","box"}})));
    });
    add(u"Strut",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto row=node(p,u"mrow");
        return pushed(p,node(p,u"mpadded",childList({row}),Property(PropertyList{{"height","8.6pt"},{"depth","3pt"},{"width",0}})));
    });
    add(u"Phantom",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto box=node(p,u"mphantom",childList({p.ParseArg(name)}));
        const bool v=property_host::truthy(valueAt(rest,0)),h=property_host::truthy(valueAt(rest,1));mml::NodeValue shaped=box;
        if(v||h){shaped=node(p,u"mpadded",childList({box}));if(h)NodeUtil::instance().setAttribute(shaped,"height",Property(0));if(h)NodeUtil::instance().setAttribute(shaped,"depth",Property(0));if(v)NodeUtil::instance().setAttribute(shaped,"width",Property(0));}
        return pushed(p,node(p,u"TeXAtom",childList({shaped})));
    });
    add(u"Smash",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto bt=UnitUtil::instance().trimSpaces(p.GetBrackets(name,Property("")));const auto shaped=node(p,u"mpadded",childList({p.ParseArg(name)}));
        if(bt==Property("b"))NodeUtil::instance().setAttribute(shaped,"depth",Property(0));
        else if(bt==Property("t"))NodeUtil::instance().setAttribute(shaped,"height",Property(0));
        else{NodeUtil::instance().setAttribute(shaped,"height",Property(0));NodeUtil::instance().setAttribute(shaped,"depth",Property(0));}
        return pushed(p,node(p,u"TeXAtom",childList({shaped})));
    });
    add(u"Lap",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto shaped=node(p,u"mpadded",childList({p.ParseArg(name)}),Property(PropertyList{{"width",0}}));
        if(name==u"\\llap")NodeUtil::instance().setAttribute(shaped,"lspace",Property("-1width"));
        return pushed(p,node(p,u"TeXAtom",childList({shaped})));
    });
    add(u"Macro",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);
        const Property macroValue=valueAt(rest,0);
        const Property count=valueAt(rest,1);const Property def=valueAt(rest,2);
        if(property_host::truthy(count)){
            std::vector<Property> args;
            if(!property_host::nullish(def)){
                const Property optional=p.GetBrackets(name);
                args.push_back(property_host::nullish(optional)?def:optional);
            }
            while(static_cast<double>(args.size())<property_host::number(count))
                args.push_back(p.GetArgument(name));
            auto& config=configuration(p);Context& context=config.runtime().context();
            const auto array=context.array(static_cast<std::uint32_t>(args.size()));
            for(std::size_t i=0;i<args.size();++i)context.set(array,key(property_host::text(std::to_string(i))),config.runtime().properties.to(args[i]));
            // Source passes the original macro value into substituteArgs;
            // that method reads length and calls charAt without coercing it.
            // Preserve the raw value here for parameterized definitions.
            const auto substituted=ParseUtil::substituteArgs(p,Value(array),macroValue);
            const auto suffix=sliceFrom(p.string(),p.i);
            p.setString(ParseUtil::addArgs(p,substituted,suffix));
        }else{
            const auto suffix=sliceFrom(p.string(),p.i);
            p.setString(ParseUtil::addArgs(p,macroValue,suffix));
        }
        p.i=0;ParseUtil::checkMaxMacros(p);return Property(Missing{});
    });
    add(u"UnderOver",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto character=textAt(rest,0);
        const auto entity=NodeUtil::instance().createEntity(character);
        const auto accent=token(p,u"mo",Property(PropertyList{{"stretchy",true},{"accent",true}}),Property(property_host::debugText(entity)));
        if(mml::matchMoPattern(mml::MoPattern::mathaccentsWithWidth,entity))NodeUtil::instance().setProperty(accent,"mathaccent",Property(false));
        const std::u16string position=name.size()>1&&name[1]==u'o'?u"over":u"under";
        const auto base=p.ParseArg(name);
        return pushed(p,ParseUtil::underOver(p,base,accent,position,boolAt(rest,1)));
    });
    add(u"MmlToken",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);
        const auto kind=property_host::string(p.GetArgument(name));
        std::u16string attributes=property_host::string(p.GetBrackets(name,Property("")));
        if(const auto match=exec(configuration(p).runtime(),u"^\\s+",attributes);match&&match->at(0))
            attributes.erase(0,match->at(0)->size());
        const auto sourceText=property_host::string(p.GetArgument(name));
        const std::unordered_set<std::u16string> allowed={u"fontfamily",u"fontsize",u"fontweight",u"fontstyle",u"color",u"background",u"id",u"class",u"href",u"style",
            // MmlTokenAllow is an object literal, so a few inherited Object
            // names are truthy there just as they are in the pinned Source.
            u"constructor",u"hasOwnProperty",u"isPrototypeOf",u"propertyIsEnumerable",u"toLocaleString",u"toString",u"valueOf"};
        PropertyList definition;std::vector<std::u16string> keep;
        mml::NodeValue created=PropertyNull{};
        try{const std::array<std::any,1> args{std::any(Property(property_host::debugText(kind)))};created=p.create(u"node",args);}catch(...){created=PropertyNull{};}
        if(!created.isNode()||!created.requireNode()->isToken()){
            const std::array<Property,1> args{Property(property_host::debugText(kind))};texError(u"NotMathMLToken",u"%1 is not a token element",args);
        }
        while(!attributes.empty()){
            static constexpr std::u16string_view pattern=u"^([A-Za-z]+)\\s*=\\s*('[^'\\n]*'|\"[^\"\\n]*\"|[^ ,\\n]*)[\\s\\n]*,?[\\s\\n]*";
            const auto match=exec(configuration(p).runtime(),std::u16string(pattern),attributes);
            if(!match||!match->at(0)||match->at(0)->empty()){
                const auto first=exec(configuration(p).runtime(),u"^[^\\s\\n=]*",attributes);
                const std::u16string invalid=first&&first->at(0)?*first->at(0):std::u16string{};
                const std::array<Property,1> args{Property(property_host::debugText(invalid))};texError(u"InvalidMathMLAttr",u"Invalid MathML attribute: %1",args);
            }
            const std::u16string keyText=match->at(1).value_or(u"");
            std::string keyUtf8;keyUtf8.reserve(keyText.size());for(const char16_t ch:keyText)keyUtf8.push_back(ch<=0x7f?static_cast<char>(ch):'?');
            const auto attributesPtr=created.requireNode("attributes")->attributes();
            if(!attributesPtr)property_host::typeError("Cannot read properties of null (reading 'hasDefault')");
            const bool recognized=attributesPtr->hasDefault(keyUtf8)||allowed.contains(keyText);
            if(!recognized){
                const std::array<Property,2> args{Property(property_host::debugText(keyText)),Property(property_host::debugText(kind))};
                texError(u"UnknownAttrForElement",u"%1 is not a recognized attribute for %2",args);
            }
            std::u16string raw=match->at(2).value_or(u"");
            if(raw.size()>=2&&(raw.front()==u'\''||raw.front()==u'\"')){
                const char16_t quote=raw.front();
                if(raw.back()==quote){
                    const auto unquoted=exec(configuration(p).runtime(),u"^(['\"])(.*)\\1$",raw);
                    if(unquoted&&unquoted->at(2))raw=*unquoted->at(2);
                }
            }
            std::u16string value=ParseUtil::mmlFilterAttribute(p,keyText,std::move(raw));
            if(!value.empty()){
                std::u16string lower=value;for(auto& ch:lower)if(ch>=u'A'&&ch<=u'Z')ch=static_cast<char16_t>(ch-u'A'+u'a');
                Property stored=lower==u"true"?Property(true):lower==u"false"?Property(false):Property(property_host::debugText(value));
                definition.setOwn(keyUtf8,std::move(stored));keep.push_back(keyText);
            }
            attributes.erase(0,match->at(0)->size());
        }
        if(!keep.empty()){
            std::u16string joined;for(std::size_t i=0;i<keep.size();++i){if(i)joined.push_back(u' ');joined+=keep[i];}
            NodeUtil::instance().setProperty(created,"keep-attrs",Property(property_host::debugText(joined)));
        }
        const auto replaced=string_util::replaceUnicode(sourceText);
        const std::array<std::any,1> textArgs{std::any(Property(property_host::debugText(replaced)))};
        const auto textNode=p.create(u"text",textArgs);
        created.requireNode("appendChild")->appendChild(textNode);
        NodeUtil::instance().setProperties(created,Property(definition));
        p.Push(created);return Property(Missing{});
    });
    add(u"RaiseLower",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);std::u16string name=atomText(atom);std::u16string height=p.GetDimen(name);
        BaseItem* position=itemFactory(p).create("position",{});
        PropertyList initial{{"name",p.currentCS()},{"move","vertical"}};
        position->setProperties(StackProperty(Property(initial)));
        if(!height.empty()&&height.front()==u'-'){
            height.erase(height.begin());name=name.size()>1&&name.substr(1)==u"raise"?u"\\lower":u"\\raise";
        }
        if(name==u"\\lower"){
            position->setProperty("dh",StackProperty(Property(property_host::debugText(u"-"+height))));
            position->setProperty("dd",StackProperty(Property(property_host::debugText(u"+"+height))));
        }else{
            position->setProperty("dh",StackProperty(Property(property_host::debugText(u"+"+height))));
            position->setProperty("dd",StackProperty(Property(property_host::debugText(u"-"+height))));
        }
        p.Push(itemFactory(p).reference(position));return Property(Missing{});
    });
    add(u"MoveLeftRight",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);std::u16string width=p.GetDimen(name);
        std::u16string opposite=!width.empty()&&width.front()==u'-'?width.substr(1):u"-"+width;
        if(name==u"\\moveleft")std::swap(width,opposite);
        BaseItem* position=itemFactory(p).create("position",{});
        const Property currentCommand=p.currentCS();
        const auto left=node(p,u"mspace",emptyChildren(),Property(PropertyList{{"width",Property(property_host::debugText(width))}}));
        const auto right=node(p,u"mspace",emptyChildren(),Property(PropertyList{{"width",Property(property_host::debugText(opposite))}}));
        StackPropertyList values{{"name",StackProperty(currentCommand)},{"move",StackProperty("horizontal")},{"left",StackProperty(left)},{"right",StackProperty(right)}};
        position->setProperties(StackProperty(values));
        p.Push(itemFactory(p).reference(position));return Property(Missing{});
    });
    add(u"FrameBox",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const Property width=p.GetBrackets(name);
        Property position=p.GetBrackets(name);if(!property_host::truthy(position))position=Property("c");
        auto inner=ParseUtil::internalMath(p,property_host::string(p.GetArgument(name)));
        if(property_host::truthy(width)){
            std::u16string align=u"center";const auto pos=property_host::string(position);
            if(pos==u"l")align=u"left";else if(pos==u"r")align=u"right";
            const auto padded=node(p,u"mpadded",inner,Property(PropertyList{{"width",width},{"data-align",Property(property_host::debugText(align))}}));
            inner=childList({padded});
        }
        const auto enclosed=node(p,u"menclose",inner,Property(PropertyList{{"notation","box"}}));
        return pushed(p,node(p,u"TeXAtom",childList({enclosed}),Property(PropertyList{{"texClass",mml::TEXCLASS.ORD}})));
    });
    add(u"MakeBox",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const Property width=p.GetBrackets(name);
        const Property position=p.GetBrackets(name,Property("c"));
        const auto inner=ParseUtil::internalMath(p,property_host::string(p.GetArgument(name)));
        const auto padded=node(p,u"mpadded",inner);
        if(property_host::truthy(width))NodeUtil::instance().setAttribute(padded,"width",width);
        const auto pos=property_host::string(position);std::u16string lower=pos;
        for(auto& ch:lower)if(ch>=u'A'&&ch<=u'Z')ch=static_cast<char16_t>(ch-u'A'+u'a');
        const std::u16string align=lower==u"c"?u"center":lower==u"r"?u"right":u"";
        if(!align.empty())NodeUtil::instance().setAttribute(padded,"data-align",Property(property_host::debugText(align)));
        if(lower!=pos)NodeUtil::instance().setAttribute(padded,"data-overflow",Property("linebreak"));
        return pushed(p,padded);
    });
    add(u"MoveRoot",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto id=textAt(rest,0);
        const Property inRoot=property_host::get(stack(p).env(),"inRoot");
        if(!property_host::truthy(inRoot)){const std::array<Property,1> args{p.currentCS()};texError(u"MisplacedMoveRoot",u"%1 can appear only within a root",args);}
        if(property_host::truthy(property_host::get(stack(p).global,property_host::debugText(id)))){
            const std::array<Property,1> args{p.currentCS()};texError(u"MultipleMoveRoot",u"Multiple use of %1",args);
        }
        std::u16string number=property_host::string(p.GetArgument(name));
        const auto valid=exec(configuration(p).runtime(),u"-?[0-9]+",number).has_value();
        if(!valid){const std::array<Property,1> args{p.currentCS()};texError(u"IntegerArg",u"The argument to %1 must be an integer",args);}
        const double parsed=js_input::parseInt(number,js_input::IntegerRadix::Decimal)/15.0;
        std::string formatted=js_number::toString(parsed);std::u16string result;
        for(const unsigned char ch:formatted)result.push_back(static_cast<char16_t>(ch));
        if(result.empty()||result.front()!=u'-')result.insert(result.begin(),u'+');
        result+=u"em";property_host::set(stack(p).global,property_host::debugText(id),Property(property_host::debugText(result)));
        return Property(Missing{});
    });
    add(u"HandleRef",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const Property label=p.GetArgument(name);
        Tags& tagger=tags(p);ParserRuntime& runtime=configuration(p).runtime();Context& context=runtime.context();
        const Value taggerObject(tagger.sourceIdentity());
        const Value allLabels=context.get(taggerObject,key(u"allLabels"));
        Value ref=context.get(allLabels,Key(property_host::string(label)));
        if(!options::truthy(ref)){
            const Value labels=context.get(taggerObject,key(u"labels"));
            ref=context.get(labels,Key(property_host::string(label)));
        }
        if(!options::truthy(ref)){
            if(!options::truthy(context.get(taggerObject,key(u"refUpdate"))))
                context.set(tagger.sourceIdentity(),key(u"redo"),true);
            const auto* constructor=std::get_if<ObjectRef>(&runtime.labelClass.data);
            if(!constructor)property_host::typeError("Label is not a constructor");
            ref=Value(context.construct(*constructor));
        }
        const Value rawTag=context.get(ref,key(u"tag"));
        Value formatted=rawTag;
        if(boolAt(rest,0)){
            const Value formatRef=context.get(taggerObject,key(u"formatRef"));
            const auto* function=std::get_if<ObjectRef>(&formatRef.data);
            if(!function||context.kind(*function)!=ObjectKind::function)property_host::typeError("this.formatRef is not a function");
            const std::array<Value,1> args{rawTag};
            formatted=context.call(*function,taggerObject,args);
        }
        std::u16string text;
        if(const auto* object=std::get_if<ObjectRef>(&formatted.data);object&&context.kind(*object)==ObjectKind::array)
            text=context.arrayJoin(*object,Value(u""));
        else text=property_host::string(runtime.properties.from(context.toPrimitive(formatted,options::PrimitiveHint::stringHint)));
        const auto children=ParseUtil::internalMath(p,std::move(text));
        const Value rawId=context.get(ref,key(u"id"));
        const Value optionsObject=p.options();
        const Value baseUrl=context.get(optionsObject,key(u"baseURL"));
        const Value formatUrl=context.get(taggerObject,key(u"formatUrl"));
        const auto* function=std::get_if<ObjectRef>(&formatUrl.data);
        if(!function||context.kind(*function)!=ObjectKind::function)property_host::typeError("this.formatUrl is not a function");
        const std::array<Value,2> hrefArgs{rawId,baseUrl};
        const Value href=context.call(*function,taggerObject,hrefArgs);
        const std::u16string hrefText=property_host::string(runtime.properties.from(context.toPrimitive(href,options::PrimitiveHint::stringHint)));
        return pushed(p,node(p,u"mrow",children,Property(PropertyList{{"href",Property(property_host::debugText(hrefText))},{"class",Property("MathJax_ref")}})));
    });
    add(u"LeftRight",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto first=name.substr(1);const auto delimiter=p.GetDelimiter(name);
        const auto args=std::array<std::any,2>{std::any(delimiter),std::any(property_host::get(stack(p).env(),"color"))};
        const auto ref=itemFactory(p).reference(itemFactory(p).create(property_host::debugText(first),tree::FactoryArgs(args)));p.Push(ref);return Property(Missing{});
    });
    add(u"NamedFn",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);auto id=textAt(rest,0);if(id.empty())id=atomText(atom).substr(1);
        const auto node=token(p,u"mi",PropertyList{{"texClass",mml::TEXCLASS.OP}},Property(property_host::debugText(id)));
        const auto ref=itemFactory(p).reference(itemFactory(p).create("fn",std::array<std::any,1>{std::any(node)}));p.Push(ref);return Property(Missing{});
    });
    add(u"NamedOp",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);auto id=textAt(rest,0);if(id.empty())id=atomText(atom).substr(1);
        const auto pos=id.find(u"&thinsp;");if(pos!=std::u16string::npos)id.replace(pos,7,u"\u2006");
        const auto node=token(p,u"mo",PropertyList{{"movablelimits",true},{"movesupsub",true},{"form","prefix"},{"texClass",mml::TEXCLASS.OP}},Property(property_host::debugText(id)));
        return pushed(p,node);
    });
    add(u"MathChoice",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=property_host::string(p.currentCS());
        const std::array<mml::NodeValue,4> args{p.ParseArg(name),p.ParseArg(name),p.ParseArg(name),p.ParseArg(name)};
        const auto children=std::make_shared<tree::ChildArray<mml::MmlNode>>();for(const auto& child:args)children->push_back(child);
        const std::array<std::any,3> nodeArgs{std::any(Property("MathChoice")),std::any(children),std::any(Property(PropertyList{}))};
        return pushed(p,p.create(u"node",nodeArgs));
    });
    add(u"Frac",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);
        const auto numerator=p.ParseArg(name);const auto denominator=p.ParseArg(name);
        return pushed(p,node(p,u"mfrac",childList({numerator,denominator})));
    });
    add(u"Sqrt",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const Property index=p.GetBrackets(name);
        std::u16string source=property_host::string(p.GetArgument(name));
        if(source==u"\\frac"){
            source+=u"{"+property_host::string(p.GetArgument(u"\\frac"))+u"}{"+
                property_host::string(p.GetArgument(u"\\frac"))+u"}";
        }
        const auto nested=TexParser::make(source,stack(p).env(),configuration(p));
        mml::NodeValue radicand=nested->mml();
        if(!property_host::truthy(index))return pushed(p,node(p,u"msqrt",childList({radicand})));

        Property env=stack(p).env();const Property oldInRoot=property_host::get(env,"inRoot");
        property_host::set(env,"inRoot",Property(true));
        const auto rootParser=TexParser::make(property_host::string(index),env,configuration(p));
        mml::NodeValue root=rootParser->mml();
        Stack& rootStack=stack(*rootParser);
        const Property left=property_host::get(rootStack.global,"leftRoot");
        const Property up=property_host::get(rootStack.global,"upRoot");
        if(property_host::truthy(left)||property_host::truthy(up)){
            PropertyList attrs;
            if(property_host::truthy(left))attrs.setOwn("width",left);
            if(property_host::truthy(up)){attrs.setOwn("voffset",up);attrs.setOwn("height",up);}
            root=node(p,u"mpadded",childList({root}),Property(attrs));
        }
        property_host::set(env,"inRoot",oldInRoot);
        return pushed(p,node(p,u"mroot",childList({radicand,root})));
    });
    add(u"Root",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto index=p.GetUpTo(name,u"\\of");
        const auto radicand=p.ParseArg(name);Property env=stack(p).env();
        const Property oldInRoot=property_host::get(env,"inRoot");property_host::set(env,"inRoot",Property(true));
        const auto rootParser=TexParser::make(index,env,configuration(p));mml::NodeValue root=rootParser->mml();
        Stack& rootStack=stack(*rootParser);const Property left=property_host::get(rootStack.global,"leftRoot");
        const Property up=property_host::get(rootStack.global,"upRoot");
        if(property_host::truthy(left)||property_host::truthy(up)){
            PropertyList attrs;if(property_host::truthy(left))attrs.setOwn("width",left);
            if(property_host::truthy(up)){attrs.setOwn("voffset",up);attrs.setOwn("height",up);}
            root=node(p,u"mpadded",childList({root}),Property(attrs));
        }
        property_host::set(env,"inRoot",oldInRoot);
        return pushed(p,node(p,u"mroot",childList({radicand,root})));
    });
    add(u"Overset",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto over=p.ParseArg(name);const auto base=p.ParseArg(name);
        const auto core=NodeUtil::instance().getCoreMO(over);
        const bool accent=NodeUtil::instance().isType(core,"mo")&&NodeUtil::instance().getMoAttribute(core,"accent")==Property(true);
        ParseUtil::checkMovableLimits(base);return pushed(p,node(p,u"mover",childList({base,over}),Property(PropertyList{{"accent",accent}})));
    });
    add(u"Underset",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto under=p.ParseArg(name);const auto base=p.ParseArg(name);
        const auto core=NodeUtil::instance().getCoreMO(under);
        const bool accent=NodeUtil::instance().isType(core,"mo")&&NodeUtil::instance().getMoAttribute(core,"accent")==Property(true);
        ParseUtil::checkMovableLimits(base);return pushed(p,node(p,u"munder",childList({base,under}),Property(PropertyList{{"accentunder",accent}})));
    });
    add(u"Overunderset",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto over=p.ParseArg(name);const auto under=p.ParseArg(name);const auto base=p.ParseArg(name);
        const auto overCore=NodeUtil::instance().getCoreMO(over);const auto underCore=NodeUtil::instance().getCoreMO(under);
        const bool accent=NodeUtil::instance().isType(overCore,"mo")&&NodeUtil::instance().getMoAttribute(overCore,"accent")==Property(true);
        const bool accentunder=NodeUtil::instance().isType(underCore,"mo")&&NodeUtil::instance().getMoAttribute(underCore,"accent")==Property(true);
        ParseUtil::checkMovableLimits(base);return pushed(p,node(p,u"munderover",childList({base,under,over}),Property(PropertyList{{"accent",accent},{"accentunder",accentunder}})));
    });
    add(u"Accent",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto accentCode=textAt(rest,0);const Property stretchyValue=valueAt(rest,1);
        const auto child=p.ParseArg(name);const Property font=ParseUtil::getFontDef(p);
        PropertyList definition;for(const auto& key:property_host::ownKeys(font))definition.setOwn(key,property_host::get(font,key));
        definition.setOwn("accent",Property(true));definition.setOwn("mathaccent",stretchyValue.missing()?Property(true):stretchyValue);
        const auto entity=NodeUtil::instance().createEntity(accentCode);
        const auto accent=token(p,u"mo",Property(definition),Property(property_host::debugText(entity)));
        NodeUtil::instance().setAttribute(accent,"stretchy",Property(property_host::truthy(stretchyValue)));
        const auto mo=NodeUtil::instance().isEmbellished(child)?NodeUtil::instance().getCoreMO(child):child;
        if(NodeUtil::instance().isType(mo,"mo")||property_host::truthy(NodeUtil::instance().getProperty(mo,"movablelimits")))
            NodeUtil::instance().setProperties(mo,Property(PropertyList{{"movablelimits",false}}));
        const auto underover=node(p,u"munderover");NodeUtil::instance().setChild(underover,0,child);
        NodeUtil::instance().setChild(underover,1,mml::NodeValue(PropertyNull{}));NodeUtil::instance().setChild(underover,2,accent);
        return pushed(p,node(p,u"TeXAtom",childList({underover})));
    });
    add(u"TeXAtom",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const Property rawClass=valueAt(rest,0);
        const Property opClass=Property(mml::TEXCLASS.OP);const bool isOp=rawClass==opClass;
        PropertyList definition;definition.setOwn("texClass",rawClass);
        if(isOp){definition.setOwn("movesupsub",Property(true));definition.setOwn("movablelimits",Property(true));}
        mml::NodeValue nodeValue=PropertyNull{};
        if(isOp){
            const auto source=property_host::string(p.GetArgument(name));
        const auto match=exec(configuration(p).runtime(),u"^\\s*\\\\rm\\s+([a-zA-Z0-9 ]+)$",source);
            if(match&&match->at(1)){
                definition.setOwn("mathvariant",Property("normal"));
                nodeValue=token(p,u"mi",Property(definition),Property(property_host::debugText(*match->at(1))));
            }else{
                const auto parsed=TexParser::make(source,stack(p).env(),configuration(p))->mml();
                nodeValue=node(p,u"TeXAtom",childList({parsed}),Property(definition));
            }
            BaseItem* item=makeItem(p,"fn",std::array<std::any,1>{std::any(nodeValue)});p.Push(itemFactory(p).reference(item));
            return Property(Missing{});
        }
        return pushed(p,node(p,u"TeXAtom",childList({p.ParseArg(name)}),Property(definition)));
    });
    add(u"Over",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);BaseItem* item=makeItem(p,"over",{});
        item->setProperty("name",StackProperty(p.currentCS()));
        const Property open=valueAt(rest,0),close=valueAt(rest,1);
        if(property_host::truthy(open)||property_host::truthy(close)){
            item->setProperty("ldelim",StackProperty(open));item->setProperty("rdelim",StackProperty(close));
        }else if(name.ends_with(u"withdelims")){
            item->setProperty("ldelim",StackProperty(p.GetDelimiter(name)));
            item->setProperty("rdelim",StackProperty(p.GetDelimiter(name)));
        }
        if(name.starts_with(u"\\above"))item->setProperty("thickness",StackProperty(Property(property_host::debugText(p.GetDimen(name)))));
        else if(name.starts_with(u"\\atop")||property_host::truthy(open)||property_host::truthy(close))
            item->setProperty("thickness",StackProperty(Property(0)));
        p.Push(itemFactory(p).reference(item));return Property(Missing{});
    });
    add(u"Not",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{return pushedItem(parserFrom(c),"not");});
    add(u"Dots",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto left=token(p,u"mo",Property(PropertyList{{"stretchy",false}}),Property(property_host::debugText(NodeUtil::instance().createEntity(u"2026"))));
        const auto center=token(p,u"mo",Property(PropertyList{{"stretchy",false}}),Property(property_host::debugText(NodeUtil::instance().createEntity(u"22EF"))));
        BaseItem* item=makeItem(p,"dots",{});item->setProperties(StackProperty(StackPropertyList{{"ldots",StackProperty(left)}, {"cdots",StackProperty(center)}}));
        p.Push(itemFactory(p).reference(item));return Property(Missing{});
    });
    add(u"HFill",[](const FunctionReceiver&,ParserContext& c,const ParseAtom&,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const StackEntry top=stack(p).Top();BaseItem& base=stack_host::item(top,"kind");
        auto* array=dynamic_cast<ArrayItem*>(&base);
        if(!array){const std::array<Property,1> args{p.currentCS()};texError(u"UnsupportedHFill",u"Unsupported use of %1",args);}
        const auto* fillArray=std::get_if<ObjectRef>(&array->hfill.data);
        if(!fillArray)property_host::typeError("Cannot read properties of undefined (reading 'push')");
        const std::array<Value,1> value{Value(array->Size())};
        configuration(p).runtime().context().arrayPush(*fillArray,value);return Property(Missing{});
    });
    add(u"Entry",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(context);const auto name=atomText(atom);
        BaseItem* cell=makeItem(p,"cell",{});cell->setProperties(StackPropertyList{{"isEntry",true},{"name",property_host::debugText(name)}});p.Push(itemFactory(p).reference(cell));
        const StackEntry topEntry=stack(p).Top();BaseItem& top=stack_host::item(topEntry,"getProperty");const StackProperty casesValue=top.getProperty("isCases"),envValue=top.getProperty("casesEnv");
        if(!stack_property_host::truthy(casesValue)&&!stack_property_host::truthy(envValue))return Property(Missing{});
        const auto source=p.string();std::size_t i=static_cast<std::size_t>(std::max(0.0,p.i)),m=source.size();int braces=0;std::ptrdiff_t close=-1;
        std::optional<std::u16string> endPattern;
        if(stack_property_host::truthy(envValue)){
            Property env=stack_property_host::scalar(envValue);auto escaped=property_host::string(env);
            const auto at=escaped.find(u'*');if(at!=std::u16string::npos)escaped.insert(at,1,u'\\');
            endPattern=u"^\\\\end\\s*\\{"+escaped+u"\\}";
        }
        while(i<m){const char16_t ch=source[i];
            if(ch==u'{'){++braces;++i;}
            else if(ch==u'}'){if(braces==0)m=0;else{--braces;if(braces==0&&close<0)close=static_cast<std::ptrdiff_t>(i)-static_cast<std::ptrdiff_t>(p.i);++i;}}
            else if(ch==u'&'&&braces==0)texError(u"ExtraAlignTab",u"Extra alignment tab in \\cases text");
            else if(ch==u'\\'){
                const std::u16string_view rest(source.data()+i,source.size()-i);
                bool stop=rest.starts_with(u"\\\\");
                if(rest.starts_with(u"\\cr")){const auto pos=i+3;stop=pos==source.size()||!((source[pos]>=u'a'&&source[pos]<=u'z')||(source[pos]>=u'A'&&source[pos]<=u'Z'));}
                if(!stop&&endPattern){const auto match=exec(configuration(p).runtime(),*endPattern,std::u16string(rest));stop=match&&match->at(0).has_value();}
                if(stop)m=0;else i=std::min(i+2,source.size());
            }else ++i;
        }
        const std::u16string text=source.substr(static_cast<std::size_t>(std::max(0.0,p.i)),i-static_cast<std::size_t>(std::max(0.0,p.i)));
        auto white=[](char16_t c){return (c>=u'\t'&&c<=u'\r')||c==u' '||c==u'\u00a0'||(c>=u'\u2000'&&c<=u'\u200a')||c==u'\u2028'||c==u'\u2029'||c==u'\u202f'||c==u'\u205f'||c==u'\u3000'||c==u'\ufeff';};
        std::size_t first=0;while(first<text.size()&&white(text[first]))++first;
        bool textMacro=first+5<text.size()&&text.compare(first,5,u"\\text")==0&&!((text[first+5]>=u'a'&&text[first+5]<=u'z')||(text[first+5]>=u'A'&&text[first+5]<=u'Z'));
        std::size_t trimmed=text.size();while(trimmed&&white(text[trimmed-1]))--trimmed;
        if(!textMacro||close!=static_cast<std::ptrdiff_t>(trimmed)-1){
            const Property cleaned=UnitUtil::instance().trimSpaces(Property(property_host::debugText(text)));
            const auto internal=ParseUtil::internalMath(p,property_host::string(cleaned),Property(0));p.PushAll(internal);p.i=static_cast<double>(i);
        }
        return Property(Missing{});
    });
    add(u"Cr",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(context);const auto name=atomText(atom);BaseItem* cell=makeItem(p,"cell",{});
        cell->setProperties(StackPropertyList{{"isCR",true},{"name",property_host::debugText(name)}});p.Push(itemFactory(p).reference(cell));return Property(Missing{});
    });
    add(u"CrLaTeX",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(context);const auto name=atomText(atom);const bool noBrackets=boolAt(rest,0);std::optional<std::u16string> spacing;
        if(!noBrackets){auto source=p.string();const auto index=static_cast<std::size_t>(std::max(0.0,p.i));
            if(index<source.size()&&source[index]==u'*')p.i+=1;
            source=p.string();const auto i=static_cast<std::size_t>(std::max(0.0,p.i));
            if(i<source.size()&&source[i]==u'['){const auto dim=property_host::string(p.GetBrackets(name,Property("")));const auto parts=UnitUtil::instance().matchDimen(Property(property_host::debugText(dim)));
                const std::u16string value=property_host::string(parts[0]),unit=property_host::string(parts[1]);
                if(!dim.empty()&&value.empty()){const std::array<Property,1> args{p.currentCS()};texError(u"BracketMustBeDimension",u"Bracket argument to %1 must be a dimension",args);}
                spacing=value+unit;
            }
        }
        BaseItem* cell=makeItem(p,"cell",{});cell->setProperties(StackPropertyList{{"isCR",true},{"name",property_host::debugText(name)},{"linebreak",true}});p.Push(itemFactory(p).reference(cell));
        const StackEntry topEntry=stack(p).Top();BaseItem& top=stack_host::item(topEntry,"kind");
        if(auto* array=dynamic_cast<ArrayItem*>(&top)){if(spacing)array->addRowSpacing(*spacing);}
        else {PropertyList attrs{{"linebreak",property_host::get(property_host::get(TexConstant,"LineBreak"),"NEWLINE")}};const auto space=node(p,u"mspace",emptyChildren(),Property(attrs));
            if(spacing)NodeUtil::instance().setAttribute(space,"data-lineleading",Property(property_host::debugText(*spacing)));p.Push(space);}
        return Property(Missing{});
    });
    add(u"ParBox",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(context);const auto name=atomText(atom);const Property alignArg=p.GetBrackets(name,Property("c"));
        const auto width=p.GetDimen(name);const auto text=ParseUtil::internalMath(p,property_host::string(p.GetArgument(name)));
        const Value align=splitAlignArray(property_host::string(alignArg),1);
        PropertyList attrs{{"width",Property(property_host::debugText(width))},{"data-overflow",Property("linebreak")},
            {"data-vertical-align",configuration(p).runtime().properties.from(align)}};
        auto padded=node(p,u"mpadded",text,Property(attrs));NodeUtil::instance().setProperty(padded,"vbox",configuration(p).runtime().properties.from(align));
        return pushed(p,padded);
    });
    add(u"BreakAlign",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(context);const auto name=atomText(atom);const StackEntry topEntry=stack(p).Top();BaseItem& base=stack_host::item(topEntry,"kind");
        auto* array=dynamic_cast<ArrayItem*>(&base);if(!array){const std::array<Property,1> args{p.currentCS()};texError(u"BreakNotInArray",u"%1 must be used in an alignment environment",args);}
        const auto type=property_host::string(p.GetArgument(name));const std::u16string trimSet=u" \t\r\n\f\v";
        const auto first=type.find_first_not_of(trimSet);const auto last=type.find_last_not_of(trimSet);const auto trimmed=first==std::u16string::npos?std::u16string{}:type.substr(first,last-first+1);
        const auto error=[&](std::u16string id,std::u16string_view message,std::u16string suffix={}){const std::array<Property,1> args{Property(property_host::debugText(property_host::string(p.currentCS())+suffix))};texError(std::move(id),message,args);};
        const auto store=[&](std::u16string_view keyName,options::Value value){const auto* record=std::get_if<ObjectRef>(&array->breakAlign.data);if(!record)property_host::typeError("Cannot set properties of undefined (setting 'cell')");configuration(p).runtime().context().set(*record,key(keyName),std::move(value));};
        if(trimmed==u"c"){
            if(array->First().isNode())error(u"BreakFirstInEntry",u"%1 must be at the beginning of an alignment entry",u"{c}");
            store(u"cell",splitAlignArray(property_host::string(p.GetArgument(name)),1));
        }else if(trimmed==u"r"){
            const auto row=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&array->row);
            if((row&&*row&&!(*row)->empty())||array->First().isNode())error(u"BreakFirstInRow",u"%1 must be at the beginning of an alignment row",u"{r}");
            store(u"row",splitAlignArray(property_host::string(p.GetArgument(name))));
        }else if(trimmed==u"t"){
            const auto table=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&array->table);
            const auto row=std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&array->row);
            if((table&&*table&&!(*table)->empty())||(row&&*row&&!(*row)->empty())||array->First().isNode())
                error(u"BreakFirstInTable",u"%1 must be at the beginning of an alignment",u"{t}");
            store(u"table",splitAlignArray(property_host::string(p.GetArgument(name))));
        }else error(u"BreakType",u"First argument to %1 must be one of c, r, or t");
        return Property(Missing{});
    });
    add(u"HLine",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(context);const auto name=atomText(atom);Property style=valueAt(rest,0);if(property_host::nullish(style))style=Property("solid");
        const StackEntry topEntry=stack(p).Top();BaseItem& base=stack_host::item(topEntry,"kind");auto* array=dynamic_cast<ArrayItem*>(&base);
        const auto table= array?std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&array->table):nullptr;
        const auto row= array?std::get_if<std::shared_ptr<tree::ChildArray<mml::MmlNode>>>(&array->row):nullptr;
        if(!array||!table||!*table||!row||!*row||!(*row)->empty()){const std::array<Property,1> args{p.currentCS()};texError(u"Misplaced",u"Misplaced %1",args);}
        Context& cx=configuration(p).runtime().context();
        if((*table)->empty()){
            const auto frame=std::get_if<ObjectRef>(&array->frame.data);if(!frame)property_host::typeError("Cannot read properties of undefined (reading 'push')");
            const ObjectRef line=cx.array(2);cx.set(line,key(u"0"),Value(u"top"));cx.set(line,key(u"1"),configuration(p).runtime().properties.to(style));const std::array<Value,1> addition{Value(line)};cx.arrayPush(*frame,addition);
        }else{
            const auto* def=std::get_if<ObjectRef>(&array->arraydef.data);if(!def)property_host::typeError("Cannot read properties of undefined (reading 'rowlines')");
            const Value old=cx.get(Value(*def),key(u"rowlines"));const Property oldProperty=configuration(p).runtime().properties.from(old);std::u16string lineText;
            if(property_host::truthy(oldProperty))lineText=property_host::string(oldProperty);
            std::vector<std::u16string> lines;std::size_t start=0;while(start<lineText.size()){
                while(start<lineText.size()&&lineText[start]==u' ')++start;if(start==lineText.size())break;
                auto end=lineText.find(u' ',start);if(end==std::u16string::npos)end=lineText.size();lines.push_back(lineText.substr(start,end-start));start=end;
            }
            const auto rows=(*table)->size();while(lines.size()<rows)lines.emplace_back(u"none");
            if(rows)lines[rows-1]=property_host::string(style);
            std::u16string joined;for(const auto& line:lines){if(!joined.empty())joined.push_back(u' ');joined+=line;}
            cx.set(*def,key(u"rowlines"),Value(joined));
        }
        return Property(Missing{});
    });
    add(u"Matrix",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(context);const auto name=atomText(atom);const auto open=textAt(rest,0),close=textAt(rest,1),align=textAt(rest,2),spacing=textAt(rest,3),vspacing=textAt(rest,4),style=textAt(rest,5);
        const bool cases=boolAt(rest,6),numbered=boolAt(rest,7);const auto c=p.GetNext();
        if(c.empty()){const std::array<Property,1> args{p.currentCS()};texError(u"MissingArgFor",u"Missing argument for %1",args);}
        if(c==u"{")p.i+=1;else{const auto source=p.string();const auto index=static_cast<std::size_t>(std::max(0.0,p.i+1));p.setString(c+u"}"+source.substr(std::min(index,source.size())));p.i=0;}
        BaseItem* raw=makeItem(p,"array",{});auto* array=dynamic_cast<ArrayItem*>(raw);if(!array)property_host::typeError("Base array constructor did not create an ArrayItem");
        raw->setProperty("requireClose",StackProperty(true));
        if(!open.empty()||align.empty())raw->setProperty("arrayPadding",StackProperty(".2em .125em"));
        Context& cx=configuration(p).runtime().context();const ObjectRef definition=cx.object();
        cx.set(definition,key(u"rowspacing"),Value(vspacing.empty()?u"4pt":vspacing));cx.set(definition,key(u"columnspacing"),Value(spacing.empty()?u"1em":spacing));array->arraydef=definition;
        if(cases)raw->setProperty("isCases",StackProperty(true));
        if(numbered){raw->setProperty("isNumbered",StackProperty(true));cx.set(definition,key(u"side"),configuration(p).runtime().properties.to(valueAt(rest,7)));}
        if(!open.empty()||!close.empty()){raw->setProperty("open",StackProperty(property_host::debugText(open)));raw->setProperty("close",StackProperty(property_host::debugText(close)));}
        if(style==u"D")cx.set(definition,key(u"displaystyle"),Value(true));
        if(!property_host::nullish(valueAt(rest,2)))cx.set(definition,key(u"columnalign"),configuration(p).runtime().properties.to(valueAt(rest,2)));
        p.Push(itemFactory(p).reference(raw));return Property(Missing{});
    });
    add(u"BeginEnd",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(context);const auto name=atomText(atom);const Property env=p.GetArgument(name);const auto envText=property_host::string(env);
        if(envText.find(u'\\')!=std::u16string::npos){const std::array<Property,1> args{env};texError(u"InvalidEnv",u"Invalid environment name '%1'",args);}
        ParseOptions& config=configuration(p);const auto* handlers=std::get_if<std::shared_ptr<SubHandlers>>(&config.handlers);
        if(!handlers||!*handlers)property_host::typeError("Cannot read properties of null or undefined (reading 'get')");
        const auto handler=(*handlers)->get("environment");if(!handler)property_host::typeError("Cannot read properties of undefined (reading 'lookup')");
        const TokenLookup found=handler->lookup(env);const auto* macro=std::get_if<MacroRef>(&found); 
        if(macro&&*macro&&name==u"\\end"){
            const Property args=(*macro)->args();const Property first=property_host::get(args,"0");
            if(!property_host::truthy(first)){
                BaseItem* end=makeItem(p,"end",{});end->setProperty("name",StackProperty(env));p.Push(itemFactory(p).reference(end));return Property(Missing{});
            }
            property_host::set(stack(p).env(),"closing",env);
        }
        ParseUtil::checkMaxMacros(p,false);return p.parse(property_host::get(HandlerType,"ENVIRONMENT"),ParseInput{p,env});
    });
    add(u"HandleNoTag",[](const FunctionReceiver&,ParserContext& context,const ParseAtom&,ParseRest)->ParseResult{
        tags(parserFrom(context)).notag();return Property(Missing{});
    });
    add(u"HandleLabel",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(context);const auto name=atomText(atom);const Property label=p.GetArgument(name);if(property_host::string(label).empty())return Property(Missing{});
        Tags& tagState=tags(p);if(options::truthy(tagState.label())){const std::array<Property,1> args{p.currentCS()};texError(u"MultipleCommand",u"Multiple %1",args);}
        tagState.setLabel(configuration(p).runtime().properties.to(label));
        const auto identity=tagState.sourceIdentity();Context& cx=configuration(p).runtime().context();const Value refUpdate=cx.get(identity,key(u"refUpdate"));
        if(!options::truthy(refUpdate)){
            const Value allLabels=cx.get(identity,key(u"allLabels")),labels=cx.get(identity,key(u"labels"));
            const Value existingAll=cx.get(allLabels,key(property_host::string(label)));const Value existing=cx.get(labels,key(property_host::string(label)));
            const Value opts=cx.get(p.options(),key(u"ignoreDuplicateLabels"));
            if((options::truthy(existingAll)||options::truthy(existing))&&!options::truthy(opts)){
                const std::array<Property,1> args{label};texError(u"MultipleLabel",u"Label '%1' multiply defined",args);
            }
            const auto* ctor=std::get_if<ObjectRef>(&configuration(p).runtime().labelClass.data);
            if(!ctor)property_host::typeError("Label constructor is unavailable");
            const std::array<Value,0> args{};const ObjectRef labelObject=cx.construct(*ctor,args);cx.set(labels,key(property_host::string(label)),Value(labelObject));
        }
        return Property(Missing{});
    });
    add(u"Array",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(context);const auto* beginRef=std::get_if<StackItemRef>(&atom);if(!beginRef||!*beginRef)property_host::typeError("Cannot read properties of null or undefined (reading 'getName')");
        BaseItem& begin=stack_host::item(StackEntry(*beginRef),"getName");const auto name=property_host::string(stack_property_host::scalar(begin.getName()));
        const Property rawOpen=valueAt(rest,0),rawClose=valueAt(rest,1),rawAlign=valueAt(rest,2),rawSpacing=valueAt(rest,3),rawVspacing=valueAt(rest,4),rawStyle=valueAt(rest,5);
        const bool ragged=boolAt(rest,6);
        const Property alignValue=property_host::truthy(rawAlign)?rawAlign:p.GetArgument(u"\\begin{"+name+u"}");
        const std::u16string align=property_host::string(alignValue);
        BaseItem* raw=makeItem(p,"array",{});auto* array=dynamic_cast<ArrayItem*>(raw);if(!array)property_host::typeError("Base array constructor did not create an ArrayItem");
        if(name==u"array")raw->setProperty("arrayPadding",StackProperty(".5em .125em"));
        array->parser=&p;Context& cx=configuration(p).runtime().context();const ObjectRef definition=cx.object();
        const Property columnSpacing=property_host::truthy(rawSpacing)?rawSpacing:Property(property_host::debugText(u"1em"));
        const Property rowSpacing=property_host::truthy(rawVspacing)?rawVspacing:Property(property_host::debugText(u"4pt"));
        cx.set(definition,key(u"columnspacing"),configuration(p).runtime().properties.to(columnSpacing));
        cx.set(definition,key(u"rowspacing"),configuration(p).runtime().properties.to(rowSpacing));array->arraydef=definition;
        ParseOptions& config=configuration(p);config.columnParserOwner().process(p,align,*array);
        if(property_host::truthy(rawOpen))raw->setProperty("open",StackProperty(p.convertDelimiter(property_host::string(rawOpen))));
        if(property_host::truthy(rawClose))raw->setProperty("close",StackProperty(p.convertDelimiter(property_host::string(rawClose))));
        std::u16string style=property_host::truthy(rawStyle)?property_host::string(rawStyle):std::u16string{};
        if(style.size()>1&&style[1]==u'\''){cx.set(definition,key(u"data-cramped"),Value(true));style.resize(1);}
        if(style==u"D")cx.set(definition,key(u"displaystyle"),Value(true));else if(!style.empty())cx.set(definition,key(u"displaystyle"),Value(false));
        cx.set(definition,key(u"scriptlevel"),Value(style==u"S"?1:0));if(ragged)cx.set(definition,key(u"useHeight"),Value(false));
        p.Push(*beginRef);array->StartEntry();return itemFactory(p).reference(raw);
    });
    add(u"AlignedArray",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(context);const auto* beginRef=std::get_if<StackItemRef>(&atom);if(!beginRef||!*beginRef)property_host::typeError("Cannot read properties of null or undefined (reading 'getName')");
        const auto name=property_host::string(stack_property_host::scalar(stack_host::item(StackEntry(*beginRef),"getName").getName()));
        const Property align=p.GetBrackets(u"\\begin{"+name+u"}");const Property style=valueAt(rest,0);
        const ParseMethod arrayMethod=functions::fromProperty(BaseMethods::get(configuration(p).runtime(),u"Array"));
        const std::array<std::any,6> arrayRest{std::any(Property(PropertyNull{})),std::any(Property(PropertyNull{})),std::any(Property(PropertyNull{})),
            std::any(Property(PropertyNull{})),std::any(Property(PropertyNull{})),std::any(style)};
        const ParseResult parsed=invokeParseMethod(arrayMethod,FunctionReceiver{Missing{}},static_cast<ParserContext&>(p),ParseAtom(*beginRef),arrayRest);
        const auto* item=std::get_if<StackItemRef>(&parsed);if(!item||!*item)property_host::typeError("Array method did not return a stack item");
        auto* raw=dynamic_cast<BaseItem*>(item->get());auto* array=dynamic_cast<ArrayItem*>(raw);if(!array)property_host::typeError("Array method did not return an ArrayItem");
        ParseUtil::setArrayAlign(*array,property_host::truthy(align)?property_host::string(align):std::u16string{});return *item;
    });
    add(u"IndentAlign",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(context);const auto* beginRef=std::get_if<StackItemRef>(&atom);if(!beginRef||!*beginRef)property_host::typeError("Cannot read properties of null or undefined (reading 'getName')");
        auto* begin=dynamic_cast<BaseItem*>(beginRef->get());if(!begin)property_host::typeError("Cannot read properties of null (reading 'getName')");
        const auto beginName=property_host::string(stack_property_host::scalar(begin->getName()));const std::u16string name=u"\\begin{"+beginName+u"}";
        const Property first=p.GetBrackets(name,Property("")),shift=p.GetBrackets(name,Property("")),last=p.GetBrackets(name,Property(""));
        for(const Property value:{first,shift,last})if(property_host::truthy(value)&&!property_host::truthy(UnitUtil::instance().matchDimen(value)[0])){
            const std::array<Property,1> args{Property(property_host::debugText(name))};texError(u"BracketMustBeDimension",u"Bracket argument to %1 must be a dimension",args);
        }
        const Property spec=p.GetArgument(name);const auto text=property_host::string(spec);
        if(!text.empty()&&(text.size()>3||std::any_of(text.begin(),text.end(),[](char16_t c){return c!=u'l'&&c!=u'c'&&c!=u'r';})))
            texError(u"BadAlignment",u"Alignment must be one to three copies of l, c, or r");
        std::vector<std::u16string> alignment;for(char16_t c:text)alignment.emplace_back(c==u'l'?u"left":c==u'c'?u"center":u"right");if(alignment.size()==1)alignment.push_back(alignment[0]);
        PropertyList attrs;
        if(property_host::truthy(first))attrs.setOwn("indentshiftfirst",first);
        const Property middle=property_host::truthy(shift)?shift:first;if(property_host::truthy(middle))attrs.setOwn("indentshift",middle);
        if(property_host::truthy(last))attrs.setOwn("indentshiftlast",last);
        const std::array<std::string_view,3> keys{"indentalignfirst","indentalign","indentalignlast"};
        for(std::size_t i=0;i<alignment.size()&&i<keys.size();++i)attrs.setOwn(keys[i],Property(property_host::debugText(alignment[i])));
        const std::array<std::any,2> args{std::any(Property(attrs)),std::any(Property(property_host::debugText(beginName)))};
        BaseItem* item=itemFactory(p).create("mstyle",tree::FactoryArgs(args));p.Push(itemFactory(p).reference(item));return Property(Missing{});
    });
    add(u"Equation",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(context);const auto* beginRef=std::get_if<StackItemRef>(&atom);if(!beginRef||!*beginRef)property_host::typeError("Cannot read properties of null or undefined (reading 'getName')");
        BaseItem& begin=stack_host::item(StackEntry(*beginRef),"getName");const bool numbered=boolAt(rest,0);Property display=valueAt(rest,1);if(display.missing())display=true;ParseOptions& config=configuration(p);
        const auto mathItem=config.mathItemOwner();
        if(const auto* math=std::get_if<core::MathItem*>(&mathItem);math&&*math)(*math)->display=display;else property_host::typeError("Cannot set properties of undefined (setting 'display')");
        property_host::set(stack(p).env(),"display",display);ParseUtil::checkEqnEnv(p);p.Push(*beginRef);
    const std::array<std::any,1> args{std::any(numbered)};
    BaseItem* item=makeItem(p,"equation",args);item->setProperty("name",begin.getName());return itemFactory(p).reference(item);
    });
    add(u"EqnArray",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(context);const auto* beginRef=std::get_if<StackItemRef>(&atom);if(!beginRef||!*beginRef)property_host::typeError("Cannot read properties of null or undefined (reading 'getName')");
        BaseItem& begin=stack_host::item(StackEntry(*beginRef),"getName");const auto name=property_host::string(stack_property_host::scalar(begin.getName()));
        const bool numbered=boolAt(rest,0),taggable=boolAt(rest,1);std::u16string align=textAt(rest,2),balign=textAt(rest,3);const Property spacing=valueAt(rest,4);
        const bool gather=name==u"gather"||name==u"gather*";if(taggable)ParseUtil::checkEqnEnv(p,!gather);p.Push(*beginRef);
        std::u16string normalized;for(char16_t c:align)if(c==u'c'||c==u'l'||c==u'r'){if(!normalized.empty())normalized.push_back(u' ');normalized+=(c==u'l'?u"left":c==u'r'?u"right":u"center");}
        const Property vertical=configuration(p).runtime().properties.from(splitAlignArray(balign));
        const auto* stk=std::get_if<std::shared_ptr<Stack>>(&p.stack);Property global=stk&&*stk?(*stk)->global:Property(Missing{});
        const std::array<std::any,4> args{std::any(Property(property_host::debugText(name))),std::any(Property(numbered)),std::any(Property(taggable)),std::any(global)};
        BaseItem* item=itemFactory(p).create("eqnarray",tree::FactoryArgs(args));auto* array=dynamic_cast<ArrayItem*>(item);if(!array)property_host::typeError("EqnArray constructor did not create an ArrayItem");
        Context& cx=configuration(p).runtime().context();const ObjectRef definition=cx.object();
        cx.set(definition,key(u"displaystyle"),Value(true));cx.set(definition,key(u"columnalign"),Value(normalized));const Property columnSpacing=property_host::truthy(spacing)?spacing:Property(property_host::debugText(u"1em"));cx.set(definition,key(u"columnspacing"),configuration(p).runtime().properties.to(columnSpacing));
        cx.set(definition,key(u"rowspacing"),Value(u"3pt"));cx.set(definition,key(u"data-break-align"),configuration(p).runtime().properties.to(vertical));
        const Value parserOptions=p.options();cx.set(definition,key(u"side"),cx.get(parserOptions,key(u"tagSide")));cx.set(definition,key(u"minlabelspacing"),cx.get(parserOptions,key(u"tagIndent")));array->arraydef=definition;
        if(gather)item->setProperty("nestable",true);return itemFactory(p).reference(item);
    });
    add(u"NewColumnType",[](const FunctionReceiver&,ParserContext& context,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(context);const auto name=atomText(atom);const std::u16string column=property_host::string(p.GetArgument(name));
        const Property countValue=p.GetBrackets(name,Property("0"));const std::u16string countText=property_host::string(countValue);
        const std::u16string macro=property_host::string(p.GetArgument(name));
        if(column.size()!=1){const std::array<Property,1> args{Property(property_host::debugText(column))};texError(u"BadColumnName",u"Column specifier must be exactly one character: %1",args);}
        if(countText.empty()||std::any_of(countText.begin(),countText.end(),[](char16_t c){return c<u'0'||c>u'9';})){
            const std::array<Property,1> args{Property(property_host::debugText(countText))};texError(u"PositiveIntegerArg",u"Argument to %1 must be a positive integer",args);
        }
        auto& runtime=configuration(p).runtime();ColumnParser& columnParser=configuration(p).columnParserOwner();Context& cx=runtime.context();
        const ObjectRef owner=columnParser.sourceIdentity();const Value handlerValue=cx.get(Value(owner),key(u"columnHandler"));const auto* handler=std::get_if<ObjectRef>(&handlerValue.data);
        if(!handler)property_host::typeError("Cannot set properties of undefined (setting column handler)");
        const double number=js_input::parseInt(countText,js_input::IntegerRadix::Decimal);
        const ObjectRef callback=cx.function(u"",[&runtime,owner,macro,number](Context& active,Value,std::span<const Value> args){
            const Value state=args.empty()?Value(Undefined{}):args[0];const auto method=active.get(Value(owner),key(u"macroColumn"));const auto* methodObject=std::get_if<ObjectRef>(&method.data);
            if(!methodObject||active.kind(*methodObject)!=ObjectKind::function)property_host::typeError("this.macroColumn is not a function");
            const std::array<Value,3> callArgs{state,Value(macro),Value(number)};(void)active.call(*methodObject,Value(owner),callArgs);return Value(Undefined{});
        });
        cx.set(Value(*handler),key(column),Value(callback));return pushedItem(p,"null");
    });
    add(u"MakeBig",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const Property rawClass=valueAt(rest,0);const double factor=property_host::number(valueAt(rest,1));
        const double size=factor*(1.2/0.85);std::string number=js_number::toString(size);std::u16string sizeText;
        for(const unsigned char ch:number)sizeText.push_back(static_cast<char16_t>(ch));
        const auto dot=sizeText.find(u'.');if(dot!=std::u16string::npos&&sizeText.size()>dot+4)sizeText.erase(dot+4);
        sizeText+=u"em";const auto delimiter=p.GetDelimiter(name,true);
        PropertyList definition{{"minsize",Property(property_host::debugText(sizeText))},{"maxsize",Property(property_host::debugText(sizeText))},
            {"fence",true},{"stretchy",true},{"symmetric",true}};
        return pushed(p,node(p,u"TeXAtom",childList({token(p,u"mo",Property(definition),delimiter)}),Property(PropertyList{{"texClass",rawClass}})));
    });
    add(u"BuildRel",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto top=p.ParseUpTo(name,u"\\over");const auto bottom=p.ParseArg(name);
        const auto underover=node(p,u"munderover");NodeUtil::instance().setChild(underover,0,bottom);
        NodeUtil::instance().setChild(underover,1,mml::NodeValue(PropertyNull{}));NodeUtil::instance().setChild(underover,2,top);
        return pushed(p,node(p,u"TeXAtom",childList({underover}),Property(PropertyList{{"texClass",mml::TEXCLASS.REL}})));
    });
    add(u"Hsize",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);if(p.GetNext()==u"=")p.i+=1;
        property_host::set(stack(p).env(),"hsize",Property(property_host::debugText(p.GetDimen(name))));return pushedItem(p,"null");
    });
    add(u"VBox",[](const FunctionReceiver&,ParserContext& c,const ParseAtom& atom,ParseRest rest)->ParseResult{
        auto& p=parserFrom(c);const auto name=atomText(atom);const auto align=textAt(rest,0);
        const auto inner=TexParser::make(property_host::string(p.GetArgument(name)),stack(p).env(),configuration(p));
        PropertyList definition{{"data-vertical-align",Property(property_host::debugText(align))},{"texClass",mml::TEXCLASS.ORD}};
        const Property width=property_host::get(stack(*inner).env(),"hsize");
        if(property_host::truthy(width)){definition.setOwn("width",width);definition.setOwn("data-overflow",Property("linebreak"));}
        const auto result=node(p,u"mpadded",childList({inner->mml()}),Property(definition));
        NodeUtil::instance().setProperty(result,"vbox",Property(property_host::debugText(align)));return pushed(p,result);
    });
    // BaseMethods is an exported Source object. Keep its own-key order equal
    // to the object literal in BaseMethods.ts, independently of the helper
    // grouping used above to implement related bodies.
    constexpr std::array<std::u16string_view,75> sourceOrder{
        u"Open",u"Close",u"Bar",u"Tilde",u"Space",u"Superscript",u"Subscript",u"Prime",
        u"Comment",u"Hash",u"MathFont",u"SetFont",u"SetStyle",u"SetSize",u"Spacer",
        u"DiscretionaryTimes",u"AllowBreak",u"Break",u"Linebreak",u"LeftRight",u"NamedFn",
        u"NamedOp",u"Limits",u"Over",u"Frac",u"Sqrt",u"Root",u"MoveRoot",u"Accent",
        u"UnderOver",u"Overset",u"Underset",u"Overunderset",u"TeXAtom",u"VBox",u"Hsize",
        u"ParBox",u"BreakAlign",u"MmlToken",u"Strut",u"Phantom",u"Smash",u"Lap",
        u"RaiseLower",u"MoveLeftRight",u"Hskip",u"Nonscript",u"Rule",u"rule",u"MakeBig",
        u"BuildRel",u"HBox",u"FBox",u"FrameBox",u"MakeBox",u"Not",u"Dots",u"Matrix",
        u"Entry",u"Cr",u"CrLaTeX",u"HLine",u"HFill",u"NewColumnType",u"BeginEnd",u"Array",
        u"AlignedArray",u"IndentAlign",u"Equation",u"EqnArray",u"HandleNoTag",u"HandleLabel",
        u"HandleRef",u"Macro",u"MathChoice"
    };
    std::vector<std::pair<std::u16string,FunctionRef>> ordered;ordered.reserve(sourceOrder.size());
    for(const auto name:sourceOrder){
        const auto found=std::find_if(table->entries.begin(),table->entries.end(),[&](const auto& entry){return entry.first==name;});
        if(found==table->entries.end())throw Exception("TypeError",u"BaseMethods source member is missing");
        ordered.push_back(*found);
    }
    if(ordered.size()!=table->entries.size())throw Exception("TypeError",u"BaseMethods contains a non-source member");
    table->entries=std::move(ordered);
    return table;
}

std::vector<std::any> parseArguments(ParserRuntime& runtime,Context& context,std::span<const Value> args){
    std::vector<std::any> result;result.reserve(args.size()+1);
    if(args.empty())throw Exception("TypeError",u"ParseMethod requires parser argument");
    const auto* parserObject=std::get_if<ObjectRef>(&args[0].data);
    if(!parserObject)throw Exception("TypeError",u"ParseMethod parser must be an object");
    result.emplace_back(std::ref(static_cast<ParserContext&>(*runtime.parserIdentities.resolve(*parserObject))));
    if(args.size()>1)result.emplace_back(ParseAtom(runtime.properties.from(args[1])));
    for(std::size_t i=2;i<args.size();++i)result.emplace_back(runtime.properties.from(args[i]));
    (void)context;
    return result;
}

Value parseResultValue(ParserRuntime& runtime,Context&,const std::any& value){
    if(const auto* result=std::any_cast<ParseResult>(&value)){
        if(const auto* property=std::get_if<Property>(result))return runtime.properties.to(*property);
        if(const auto* item=std::get_if<StackItemRef>(result);item&&*item){
            if(const auto identity=runtime.stackItemIdentities.find(*item))return *identity;
            throw Exception("TypeError",u"ParseMethod returned an unbound StackItem");
        }
        if(const auto* symbol=std::get_if<ParseSymbolRef>(result))return Value(runtime.bindParseSymbol(*symbol));
        throw Exception("TypeError",u"ParseMethod returned an invalid result");
    }
    throw Exception("TypeError",u"ParseMethod returned an invalid result");
}

} // namespace

options::Value splitAlignArray(std::u16string_view align,double count){
    std::u16string values;
    const auto whitespace=[](char16_t c){return (c>=u'\t'&&c<=u'\r')||c==u' '||c==u'\u00a0'||c==u'\u1680'||
        (c>=u'\u2000'&&c<=u'\u200a')||c==u'\u2028'||c==u'\u2029'||c==u'\u202f'||c==u'\u205f'||c==u'\u3000'||c==u'\ufeff';};
    for(char16_t c:align)if(!whitespace(c))values.push_back(c);
    std::vector<std::u16string_view> result;result.reserve(values.size());
    for(char16_t c:values){
        switch(c){case u't':result.emplace_back(u"top");break;case u'b':result.emplace_back(u"bottom");break;
        case u'm':result.emplace_back(u"middle");break;case u'c':result.emplace_back(u"center");break;
        default:{const std::array<Property,1> args{Property(property_host::debugText(std::u16string(1,c)))};
            texError(u"BadBreakAlign",u"Invalid alignment character: %1",args);}}
    }
    if(static_cast<double>(result.size())>count){const std::array<Property,1> args{Property(property_host::debugText(align))};
        texError(u"TooManyAligns",u"Too many alignment characters: %1",args);}
    if(count==1)return result.empty()?options::Value(options::Undefined{}):options::Value(std::u16string(result.front()));
    std::u16string joined;for(const auto value:result){if(!joined.empty())joined.push_back(u' ');joined+=value;}
    return joined;
}

Property BaseMethods::initializeModule(ParserRuntime& runtime){
    if(!runtime.baseMethods.missing())return runtime.baseMethods;
    const auto methodTable=makeMethods();
    PropertyList table;
    for(const auto& [name,body]:methodTable->entries){
        OptionCallBinding binding;
        binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
        binding.arguments=[&runtime](Context& context,std::span<const Value> args){return parseArguments(runtime,context,args);};
        binding.result=[&runtime](Context& context,const std::any& value){return parseResultValue(runtime,context,value);};
        const Value function=runtime.functions().bind(body,std::move(binding),name);
        const auto* identity=std::get_if<ObjectRef>(&function.data);
        if(!identity)throw Exception("TypeError",u"BaseMethods function has no Context identity");
        runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(static_cast<double>(sourceMethodLength(name))),false,false,true));
        table.setOwn(property_host::debugText(name),Property(body));
    }
    runtime.baseMethods=Property(table);
    return runtime.baseMethods;
}

Property BaseMethods::get(const Property& table,std::u16string_view name){
    return property_host::get(table,property_host::debugText(name));
}
Property BaseMethods::get(ParserRuntime& runtime,std::u16string_view name){
    return property_host::get(runtime.baseMethods,property_host::debugText(name));
}
options::Value BaseMethods::bind(ParserRuntime& runtime,FunctionRef function,std::u16string label){
    OptionCallBinding binding;
    binding.receiver=[](Context&,Value receiver){return FunctionReceiver(std::move(receiver));};
    binding.arguments=[&runtime](Context& context,std::span<const options::Value> args){return parseArguments(runtime,context,args);};
    binding.result=[&runtime](Context& context,const std::any& value){return parseResultValue(runtime,context,value);};
    const auto name=label;
    const Value bound=runtime.functions().bind(std::move(function),std::move(binding),std::move(label));
    std::optional<std::size_t> sourceLength;
    if(name==u"variable"||name==u"digit"||name==u"controlSequence"||name==u"lcGreek"||name==u"ucGreek"||
       name==u"mathchar0mi"||name==u"mathchar0mo"||name==u"mathchar7"||name==u"delimiter")sourceLength=2;
    else if(name==u"environment")sourceLength=4;
    else if(name==u"Other"||name==u"csUndefined"||name==u"envUndefined")sourceLength=2;
    if(sourceLength){
        const auto* identity=std::get_if<ObjectRef>(&bound.data);
        if(!identity)throw Exception("TypeError",u"Base callable has no Context identity");
        runtime.context().define(*identity,Key(u"length"),Descriptor::data(Value(static_cast<double>(*sourceLength)),false,false,true));
    }
    return bound;
}

} // namespace folia::math::tex
