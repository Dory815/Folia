// Native observer for the pinned FontData tuple-presence profile.
import std;
import folia.math.output.svg.font_data;
import folia.math.fonts.newcm_font_data;
import folia.math.util.property_host;
import folia.math.core.tree.factory_types;
import folia.math.util.option_functions;
import folia.math.util.context_property_bindings;
import folia.math.util.map_bindings;
import folia.math.output.common.font_data_bindings;
import folia.math.output.common.font_data_options;

namespace {
using namespace folia::math;
using namespace folia::math::output::common;
using namespace folia::math::output::svg;
namespace math_options=folia::math::options;

math_options::ObjectRef registerNewcmSourceClass(math_options::Context& context,
    folia::math::OptionFunctionBindings& functions,folia::math::ContextPropertyBindings& properties,
    FontDataContextBindings& identities,math_options::MapContextBindings& maps,
    const std::shared_ptr<folia::math::output::common::FontLoadHost>& host,
    const std::shared_ptr<folia::math::fonts::NewcmFontDataClass>& owner){
    (void)functions;
    const auto constructor=identities.registerClass(u"MathJaxNewcmFont",owner,*host,
        std::static_pointer_cast<void>(host),
        [](FontDataClass& actual,FontLoadHost& loadHost,math_options::Context& cx,math_options::Value supplied){
            return std::make_shared<folia::math::fonts::NewcmFontData>(
                static_cast<folia::math::fonts::NewcmFontDataClass&>(actual),loadHost,cx,std::move(supplied));
        });
    owner->bindSourceIdentity(properties,constructor,maps);
    owner->dynamicExtensions=math_options::Value(maps.create());
    context.bindData(constructor,math_options::Key(u"dynamicExtensions"),
        std::make_shared<math_options::NativeDataField>(std::static_pointer_cast<void>(owner),
            [owner]{return owner->dynamicExtensions;},
            [owner](math_options::Value value){owner->dynamicExtensions=std::move(value);}));
    bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),u"defaultVariants",owner->defaultVariants);
    bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),u"defaultSizeVariants",owner->defaultSizeVariants);
    bindSourceOptionArrayField(properties,constructor,std::static_pointer_cast<void>(owner),u"defaultStretchVariants",owner->defaultStretchVariants);
    owner->initializeSourceData();
    return constructor;
}

void quote(std::ostream& out,std::string_view value){
    out.put('"');for(unsigned char c:value){switch(c){case '"':out<<"\\\"";break;case '\\':out<<"\\\\";break;case '\n':out<<"\\n";break;case '\r':out<<"\\r";break;case '\t':out<<"\\t";break;default:out.put(static_cast<char>(c));}}out.put('"');
}
void number(std::ostream& out,double value){
    if(std::isnan(value))out<<"{\"$\":\"NaN\"}";
    else if(std::isinf(value))out<<(value<0?"{\"$\":\"-Infinity\"}":"{\"$\":\"Infinity\"}");
    else if(value==0&&std::signbit(value))out<<"{\"$\":\"-0\"}";
    else out<<std::setprecision(std::numeric_limits<double>::max_digits10)<<value;
}
void value(std::ostream&,const Property&);
void object(std::ostream& out,const PropertyList& record){
    auto keys=record.keys();std::sort(keys.begin(),keys.end());out.put('{');
    for(std::size_t i=0;i<keys.size();++i){if(i)out.put(',');quote(out,keys[i]);out.put(':');value(out,record.getOwn(keys[i]));}out.put('}');
}
void value(std::ostream& out,const Property& item){
    std::visit([&](const auto& v){using T=std::decay_t<decltype(v)>;
        if constexpr(std::is_same_v<T,Missing>)out<<"{\"$\":\"undefined\"}";
        else if constexpr(std::is_same_v<T,PropertyNull>)out<<"null";
        else if constexpr(std::is_same_v<T,bool>)out<<(v?"true":"false");
        else if constexpr(std::is_same_v<T,double>)number(out,v);
        else if constexpr(std::is_same_v<T,std::string>)quote(out,v);
        else if constexpr(std::is_same_v<T,std::shared_ptr<PropertyStorage>>){
            if(!v)throw std::runtime_error("null property object in tuple observer");
            if(v->array){out.put('[');for(std::size_t i=0;i<v->arrayValues.size();++i){if(i)out.put(',');if(i>=v->arrayPresent.size()||!v->arrayPresent[i])out<<"{\"$\":\"hole\"}";else value(out,v->arrayValues[i]);}out.put(']');}
            else {auto r=PropertyList::record(item);if(!r)throw std::runtime_error("invalid property record in tuple observer");object(out,*r);}
        }else throw std::runtime_error("unsupported property value in tuple observer");
    },item.value);
}
PropertyArray makeTuple(std::initializer_list<Property> values){return PropertyArray(values);}
void outcome(std::ostream& out,const std::function<Property()>& f){
    try{const auto result=f();out<<"{\"value\":";value(out,result);out<<'}';}
    catch(const tree::FactoryError& e){out<<"{\"error\":{\"name\":";quote(out,e.name);out<<",\"message\":";quote(out,property_host::debugText(e.message));out<<"}}";}
}
void tupleCase(std::ostream& out,std::string_view name,PropertyArray tuple){
    CharMap map;map.emplace(0x41,Property(tuple));const auto before=std::get<Property>(map.at(0x41));
    const auto fourth=tuple.get(3);
    const auto option=FontDataClass::charOptions(map,0x41);
    quote(out,name);out<<":{\"tuple\":";value(out,before);out<<",\"option\":{\"value\":";value(out,option);out<<'}';
    const auto after=std::get<Property>(map.at(0x41));
    out<<",\"fourthSame\":";if(tuple.size()>3)out<<(fourth==option?"true":"false");else out<<"null";
    out<<'}';
}
}

int main(){
    try{
        std::cout<<"{\"results\":{";
        tupleCase(std::cout,"length0",makeTuple({}));std::cout<<',';
        tupleCase(std::cout,"length2",makeTuple({Property(1),Property(0)}));std::cout<<',';
        tupleCase(std::cout,"length3",makeTuple({Property(1),Property(0),Property(.5)}));std::cout<<',';
        tupleCase(std::cout,"length4Undefined",makeTuple({Property(1),Property(0),Property(.5),Property(Missing{})}));std::cout<<',';
        tupleCase(std::cout,"length4Null",makeTuple({Property(1),Property(0),Property(.5),Property(PropertyNull{})}));std::cout<<',';
        tupleCase(std::cout,"length4Object",makeTuple({Property(1),Property(0),Property(.5),Property(PropertyList{{"keep",Property(std::string("same"))}})}));std::cout<<',';
        tupleCase(std::cout,"length5",makeTuple({Property(1),Property(0),Property(.5),Property(PropertyList{{"keep",Property(std::string("same"))}}),Property(std::string("extra"))}));std::cout<<',';
        std::size_t addIndex=0;
        for(const auto& [name,slot]:std::array<std::pair<std::string_view,Property>,3>{{
            {"object",Property(PropertyList{{"keep",Property(std::string("old"))}})},
            {"undefined",Property(Missing{})},{"null",Property(PropertyNull{})}}}){
            PropertyArray tuple=makeTuple({Property(1),Property(0),Property(.5),slot});CharMap map;map.emplace(0x41,Property(tuple));
            const auto before=std::get<Property>(map.at(0x41));const auto fourth=tuple.get(3);
            const auto key=std::string("addPaths_")+std::string(name);quote(std::cout,key);std::cout<<':';
            outcome(std::cout,[&](){auto* result=&AddPaths(map,CharStringMap{{0x41,"M65"}},CharStringMap{{0x41,"A"}});
                const auto after=std::get<Property>(map.at(0x41));const auto afterTuple=PropertyArray::array(after).value();
                PropertyList fields{{"sameMap",Property(result==&map)},{"sameTuple",Property(before==after)},
                    {"sameFourth",Property(fourth==afterTuple.get(3))},{"tuple",Property(afterTuple)}};
                return Property(fields);
            });
            if(++addIndex<3)std::cout<<',';
        }
        std::cout<<',';
        std::cout<<"\"repeatedGetChar\":";
        options::Context context;folia::math::OptionFunctionBindings functions(context);folia::math::ContextPropertyBindings properties(context,functions);FontDataContextBindings identities(context);math_options::MapContextBindings maps(context);auto cls=std::make_shared<folia::math::fonts::NewcmFontDataClass>(context);auto queue=std::make_shared<async::JobQueue>();auto hostOwner=std::make_shared<FontLoadHost>();auto& host=*hostOwner;host.queue=queue;host.asyncIsSynchronous=[] {return false;};(void)registerNewcmSourceClass(context,functions,properties,identities,maps,hostOwner,cls);folia::math::fonts::NewcmFontData font(*cls,host,context);
        const auto first=font.getChar("normal",65),second=font.getChar("normal",65);
        PropertyList repeated{{"sameTuple",Property(first&&second&&*first==*second)}};
        if(first)repeated.setOwn("tuple",*first);else repeated.setOwn("tuple",Property(Missing{}));
        value(std::cout,Property(repeated));
        std::cout<<"}}\n";return 0;
    }catch(const std::exception& e){std::cerr<<"font tuple native observer: "<<e.what()<<'\n';return 2;}
}
