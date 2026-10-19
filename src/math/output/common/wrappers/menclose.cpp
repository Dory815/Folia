/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/menclose.ts.
 *************************************************************/
module folia.math.output.common;
import :menclose;
import :wrapper;
import :msqrt;
import folia.math.util.property_host;
import folia.math.util.string;
import folia.math.util.native_identities;
import std;

namespace folia::math::output::common {
namespace {
constexpr double kPadding=.2;
constexpr double kThickness=.067;
constexpr double kArrowX=4;
constexpr double kArrowY=2;
constexpr double kArrowDX=1;
double number(CommonMenclose const& self,const options::Value& value){
    return self.jax().optionsContext().toNumber(value);
}
options::Key indexKey(std::size_t index){
    const auto text=std::to_string(index);std::u16string key;key.reserve(text.size());
    for(const auto ch:text)key.push_back(static_cast<char16_t>(ch));
    return options::Key(std::move(key));
}
std::vector<std::u16string> splitLiteralSpace(std::u16string_view text){
    std::vector<std::u16string> out;std::size_t start=0;
    for(std::size_t i=0;i<=text.size();++i)if(i==text.size()||text[i]==u' '){out.emplace_back(text.substr(start,i-start));start=i+1;}
    return out;
}
std::u16string splitArgument(const Property& value){
    if(const auto* text=std::get_if<std::string>(&value.value))
        return property_host::text(*text);
    if(value.missing())
        throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'trim')");
    if(std::holds_alternative<PropertyNull>(value.value))
        throw options::Exception("TypeError",u"Cannot read properties of null (reading 'trim')");
    throw options::Exception("TypeError",u"x.trim is not a function");
}
}

options::ObjectRef CommonMenclose::sourceInstance()const{return factory().objectFor(*this);}
options::Value CommonMenclose::sourceField(std::u16string_view key)const{
    return jax().optionsContext().get(sourceInstance(),options::Key(std::u16string(key)));
}
void CommonMenclose::setSourceField(std::u16string_view key,options::Value value){
    jax().optionsContext().set(sourceInstance(),options::Key(std::u16string(key)),std::move(value));
}
options::Value CommonMenclose::callSourceMethod(
        std::u16string_view name,std::span<const options::Value> args)const{
    auto& context=jax().optionsContext();
    const auto instance=sourceInstance();
    const auto value=context.get(instance,options::Key(std::u16string(name)));
    const auto* function=std::get_if<options::ObjectRef>(&value.data);
    if(!function)throw options::Exception("TypeError",u"Source menclose method is not callable");
    return context.call(*function,options::Value(instance),args);
}
options::Value CommonMenclose::notation(std::u16string_view name)const{
    auto& context=jax().optionsContext();
    const auto selected=selectedClass();
    if(!selected)throw options::Exception("TypeError",u"Menclose wrapper has no selected Source class");
    const auto* metadata=std::any_cast<CommonWrapperClassData>(&selected->staticData);
    if(!metadata||!metadata->sourceConstructor)
        throw options::Exception("TypeError",u"Menclose constructor has no Source identity");
    const auto mapValue=context.get(*metadata->sourceConstructor,options::Key(u"notations"));
    const auto* map=std::get_if<options::ObjectRef>(&mapValue.data);
    if(!map)throw options::Exception("TypeError",u"Menclose notations is not a Map");
    const auto getValue=context.get(*map,options::Key(u"get"));
    const auto* function=std::get_if<options::ObjectRef>(&getValue.data);
    if(!function)throw options::Exception("TypeError",u"Notations.get is not callable");
    const std::array<options::Value,1> args={options::Value(std::u16string(name))};
    return context.call(*function,options::Value(*map),args);
}
options::Value CommonMenclose::callNotation(const options::Value& definition,
        std::u16string_view method,std::span<const options::Value> args)const{
    auto& context=jax().optionsContext();
    const auto* receiver=std::get_if<options::ObjectRef>(&definition.data);
    if(!receiver)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'call')");
    const auto callable=context.get(*receiver,options::Key(std::u16string(method)));
    const auto* function=std::get_if<options::ObjectRef>(&callable.data);
    if(!function)throw options::Exception("TypeError",u"Notation callback is not callable");
    return context.call(*function,options::Value(*receiver),args);
}
double CommonMenclose::paddingValue()const{return number(*this,sourceField(u"padding"));}
double CommonMenclose::thicknessValue()const{return number(*this,sourceField(u"thickness"));}
std::array<double,3> CommonMenclose::arrowheadValue()const{
    auto& cx=jax().optionsContext();const auto value=sourceField(u"arrowhead");
    const auto* object=std::get_if<options::ObjectRef>(&value.data);
    if(!object)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'x')");
    return {cx.toNumber(cx.get(*object,options::Key(u"x"))),
            cx.toNumber(cx.get(*object,options::Key(u"y"))),
            cx.toNumber(cx.get(*object,options::Key(u"dx")))};
}
void CommonMenclose::setArrowhead(std::array<double,3> value){
    auto& cx=jax().optionsContext();auto object=cx.object();
    cx.set(object,options::Key(u"x"),value[0]);cx.set(object,options::Key(u"y"),value[1]);
    cx.set(object,options::Key(u"dx"),value[2]);setSourceField(u"arrowhead",options::Value(object));
}
std::vector<std::u16string> CommonMenclose::notationNames()const{
    auto& cx=jax().optionsContext();const auto value=sourceField(u"notations");
    const auto* object=std::get_if<options::ObjectRef>(&value.data);
    if(!object)throw options::Exception("TypeError",u"Cannot convert undefined or null to object");
    std::vector<std::u16string> result;
    for(const auto& key:cx.ownKeys(*object)){
        const auto descriptor=cx.descriptor(*object,key);
        if(!descriptor||!descriptor->enumerable)continue;
        if(const auto* text=std::get_if<std::u16string>(&key.data))result.push_back(*text);
    }
    return result;
}
CommonMenclose::CommonMenclose(CommonWrapperFactory& factory,mml::MmlNode& node,CommonWrapper* parent)
    :CommonWrapper(factory,node,parent){}

void CommonMenclose::initializeSource(){
    CommonWrapper::initializeSource();
    auto& cx=jax().optionsContext();
    setSourceField(u"notations",options::Value(cx.object()));
    setSourceField(u"renderChild",options::Value(options::Null{}));
    setSourceField(u"msqrt",options::Value(options::Null{}));
    setSourceField(u"padding",kPadding);setSourceField(u"thickness",kThickness);
    setArrowhead({kArrowX,kArrowY,kArrowDX});
    auto initialTrbl=cx.array(4);
    for(std::size_t i=0;i<4;++i)cx.set(initialTrbl,indexKey(i),0.0);
    setSourceField(u"TRBL",options::Value(initialTrbl));
    // The Source constructor calls these through `this`, so resolve each live
    // prototype member at the call site and preserve its exact returned array.
    (void)callSourceMethod(u"getParameters");
    (void)callSourceMethod(u"getNotations");
    (void)callSourceMethod(u"removeRedundantNotations");
    (void)callSourceMethod(u"initializeNotations");
    setSourceField(u"TRBL",callSourceMethod(u"getBBoxExtenders"));
}
void CommonMenclose::getParameters(){
    const auto attributes=node->attributes();
    const auto padding=attributes->get("data-padding");
    if(!padding.missing())setSourceField(u"padding",length2em(padding,kPadding));
    const auto thickness=attributes->get("data-thickness");
    if(!thickness.missing())setSourceField(u"thickness",length2em(thickness,kThickness));
    const auto arrow=attributes->get("data-arrowhead");
    if(!arrow.missing()){
        const auto parts=string_util::split(splitArgument(arrow));
        auto parse=[&](std::size_t i,double fallback){
            if(i>=parts.size()||parts[i].empty())return fallback;
            const auto text=property_host::debugText(parts[i]);char* end=nullptr;
            const auto number=std::strtod(text.c_str(),&end);return end==text.c_str()?std::numeric_limits<double>::quiet_NaN():number;
        };
        setArrowhead({parse(0,kArrowX),parse(1,kArrowY),parse(2,kArrowDX)});
    }
}
void CommonMenclose::getNotations(){
    auto& cx=jax().optionsContext();const auto names=string_util::split(splitArgument(node->attributes()->get("notation")));
    for(const auto& name:names){
        const auto definition=notation(name);
        if(options::truthy(definition)){
            const auto target=std::get<options::ObjectRef>(sourceField(u"notations").data);
            cx.set(target,options::Key(name),definition);
            const auto child=cx.get(definition,options::Key(u"renderChild"));
            if(options::truthy(child)){
                const auto renderer=cx.get(definition,options::Key(u"renderer"));
                setSourceField(u"renderChild",renderer);
            }
        }
    }
}
void CommonMenclose::removeRedundantNotations(){
    auto& cx=jax().optionsContext();const auto target=std::get<options::ObjectRef>(sourceField(u"notations").data);
    for(const auto& name:notationNames()){
        const auto definition=cx.get(target,options::Key(name));if(!options::truthy(definition))continue;
        const auto removeValue=cx.get(std::get<options::ObjectRef>(definition.data),options::Key(u"remove"));
        std::u16string removeText;
        if(options::truthy(removeValue)){
            const auto* remove=std::get_if<std::u16string>(&removeValue.data);
            if(!remove)throw options::Exception("TypeError",u"remove.split is not a function");
            removeText=*remove;
        }
        for(const auto& key:splitLiteralSpace(removeText))cx.erase(target,options::Key(key));
    }
}
void CommonMenclose::initializeNotations(){
    auto& cx=jax().optionsContext();const auto target=std::get<options::ObjectRef>(sourceField(u"notations").data);
    for(const auto& name:notationNames()){
        const auto def=cx.get(target,options::Key(name));const auto init=cx.get(std::get<options::ObjectRef>(def.data),options::Key(u"init"));
        if(options::truthy(init)){
            const auto* fn=std::get_if<options::ObjectRef>(&init.data);if(!fn)throw options::Exception("TypeError",u"Notation initializer is not callable");
            const std::array<options::Value,1> args={options::Value(sourceInstance())};
            (void)cx.call(*fn,options::Value(options::Undefined{}),args);
        }
    }
}
void CommonMenclose::maximizeEntries(std::array<double,4>& x,const std::array<double,4>& y)const{for(std::size_t i=0;i<4;++i)if(x[i]<y[i])x[i]=y[i];}
std::array<double,4> CommonMenclose::getBBoxExtenders()const{
    std::array<double,4> out{};auto& cx=jax().optionsContext();const auto target=std::get<options::ObjectRef>(sourceField(u"notations").data);
    for(const auto& name:notationNames()){
        const auto def=cx.get(target,options::Key(name));const std::array<options::Value,1> args={options::Value(sourceInstance())};const auto result=callNotation(def,u"bbox",args);
        const auto* array=std::get_if<options::ObjectRef>(&result.data);if(!array)throw options::Exception("TypeError",u"Notation bbox must return an array");
        std::array<double,4> value{};for(std::size_t i=0;i<4;++i)value[i]=cx.toNumber(cx.get(*array,indexKey(i)));
        maximizeEntries(out,value);
    }return out;
}
std::array<double,4> CommonMenclose::getPadding()const{
    std::array<double,4> border{};auto& cx=jax().optionsContext();const auto target=std::get<options::ObjectRef>(sourceField(u"notations").data);
    for(const auto& name:notationNames()){
        const auto def=cx.get(target,options::Key(name));const auto receiver=std::get_if<options::ObjectRef>(&def.data);
        if(!receiver)continue;const auto callback=cx.get(*receiver,options::Key(u"border"));if(!options::truthy(callback))continue;
        const auto* fn=std::get_if<options::ObjectRef>(&callback.data);if(!fn)throw options::Exception("TypeError",u"Notation border is not callable");
        const std::array<options::Value,1> args={options::Value(sourceInstance())};const auto result=cx.call(*fn,def,args);
        const auto* array=std::get_if<options::ObjectRef>(&result.data);if(!array)throw options::Exception("TypeError",u"Notation border must return an array");
        std::array<double,4> value{};for(std::size_t i=0;i<4;++i)value[i]=cx.toNumber(cx.get(*array,indexKey(i)));
        maximizeEntries(border,value);
    }
    const auto trbl=std::get<options::ObjectRef>(sourceField(u"TRBL").data);std::array<double,4> result{};
    for(std::size_t i=0;i<4;++i)result[i]=cx.toNumber(cx.get(trbl,indexKey(i)))-border[i];return result;
}
double CommonMenclose::getOffset(std::string_view direction)const{const auto value=sourceField(u"TRBL");const auto* arr=std::get_if<options::ObjectRef>(&value.data);if(!arr)throw options::Exception("TypeError",u"Cannot read properties of undefined");auto& cx=jax().optionsContext();const double t=cx.toNumber(cx.get(*arr,indexKey(0))),r=cx.toNumber(cx.get(*arr,indexKey(1))),b=cx.toNumber(cx.get(*arr,indexKey(2))),l=cx.toNumber(cx.get(*arr,indexKey(3)));const double d=(direction=="X"?r-l:b-t)/2;return std::abs(d)>.001?d:0;}
std::array<double,2> CommonMenclose::getArgMod(double w,double h)const{return {std::atan2(h,w),std::sqrt(w*w+h*h)};}
options::Value CommonMenclose::arrow(double,double,bool,std::string_view,double)const{return options::Null{};}
CommonMenclose::ArrowData CommonMenclose::arrowData()const{
    const double p=paddingValue(),t=thicknessValue();
    const auto arrowhead=sourceField(u"arrowhead");const auto* arrow=std::get_if<options::ObjectRef>(&arrowhead.data);
    if(!arrow)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'x')");
    auto& context=jax().optionsContext();
    const double xHead=context.toNumber(context.get(*arrow,options::Key(u"x")));
    const double dxHead=context.toNumber(context.get(*arrow,options::Key(u"dx")));
    const double r=t*(xHead+std::max(1.0,dxHead));
    if(!childNodes||!*childNodes||(*childNodes)->size()==0)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'getBBox')");
    auto* child=dynamic_cast<CommonWrapper*>((*childNodes)->get(0).requireNode("getBBox"));if(!child)throw options::Exception("TypeError",u"Child is not a CommonWrapper");
    const auto box=child->getBBox();const double H=box->h+box->d,R=std::sqrt(H*H+box->w*box->w);const double x=std::max(p,r*box->w/R),y=std::max(p,r*H/R);const auto [a,w]=getArgMod(box->w+2*x,H+2*y);return {a,w,x,y};
}
std::array<double,2> CommonMenclose::arrowAW()const{if(!childNodes||!*childNodes||(*childNodes)->size()==0)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'getBBox')");auto* child=dynamic_cast<CommonWrapper*>((*childNodes)->get(0).requireNode("getBBox"));if(!child)throw options::Exception("TypeError",u"Child is not a CommonWrapper");const auto box=child->getBBox();const auto v=std::get<options::ObjectRef>(sourceField(u"TRBL").data);auto&cx=jax().optionsContext();const double top=cx.toNumber(cx.get(v,indexKey(0))),right=cx.toNumber(cx.get(v,indexKey(1))),bottom=cx.toNumber(cx.get(v,indexKey(2))),left=cx.toNumber(cx.get(v,indexKey(3)));return getArgMod(left+box->w+right,top+box->h+box->d+bottom);}
CommonMsqrt& CommonMenclose::createMsqrt(CommonWrapper& child){
    if(!child.node)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'node')");
    const std::array<std::any,0> noArguments{};
    auto* square=node->factoryOwner().create("msqrt",noArguments);
    square->inheritAttributesFrom(mml::NodeValue(node));
    if(!square->childNodes)throw options::Exception("TypeError",u"Cannot set properties of null (setting '0')");
    square->childNodes->set(0,mml::NodeValue(child.node));
    // Source calls this.wrap(mml), which preserves the actual wrapper
    // constructor/prototype selected by the factory and performs the parent,
    // child-array, and node-map updates through the common wrap path.
    auto* wrapped=wrap(square);
    auto* result=dynamic_cast<CommonMsqrt*>(wrapped);
    if(!result)throw options::Exception("TypeError",u"The msqrt wrapper factory returned an incompatible wrapper");
    // Source returns the wrapper and the caller performs `this.msqrt = ...`;
    // leave that assignment to the caller so setters and partial effects keep
    // their Source order.
    return *result;
}
std::array<double,4> CommonMenclose::sqrtTRBL()const{
    auto& context=jax().optionsContext();const auto value=sourceField(u"msqrt");
    const auto* object=std::get_if<options::ObjectRef>(&value.data);
    if(!object)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getBBox')");
    const auto* payload=std::any_cast<NativeBindingPayload<CommonWrapper*>>(&context.payload(*object));
    if(!payload||!payload->identity)throw options::Exception("TypeError",u"msqrt is not a CommonWrapper instance");
    auto* square=dynamic_cast<CommonMsqrt*>(payload->identity);
    if(!square)throw options::Exception("TypeError",u"msqrt is not a CommonMsqrt instance");
    if(!square->childNodes||!*square->childNodes||(*square->childNodes)->size()==0)
        throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'getBBox')");
    auto* child=dynamic_cast<CommonWrapper*>((*square->childNodes)->get(0).requireNode("getBBox"));
    if(!child)throw options::Exception("TypeError",u"msqrt child is not a CommonWrapper");
    const auto outer=square->getBBox(),inner=child->getBBox();
    return {outer->h-inner->h,0,outer->d-inner->d,outer->w-inner->w};
}
void CommonMenclose::computeBBox(BBox& box,bool recompute){
    if(!childNodes||!*childNodes||(*childNodes)->size()==0)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'getBBox')");
    auto* child=dynamic_cast<CommonWrapper*>((*childNodes)->get(0).requireNode("getBBox"));if(!child)throw options::Exception("TypeError",u"Child is not a CommonWrapper");
    const auto trbl=std::get<options::ObjectRef>(sourceField(u"TRBL").data);auto&cx=jax().optionsContext();const double t=cx.toNumber(cx.get(trbl,indexKey(0))),r=cx.toNumber(cx.get(trbl,indexKey(1))),b=cx.toNumber(cx.get(trbl,indexKey(2))),l=cx.toNumber(cx.get(trbl,indexKey(3)));const auto childBox=child->getBBox();box.combine(*childBox,l,0);box.h+=t;box.d+=b;box.w+=r;setChildPWidths(recompute);
}
}
