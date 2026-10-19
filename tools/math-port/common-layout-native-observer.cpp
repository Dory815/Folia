// Candidate-only native wrapper snapshot for the pinned Source layout oracle.
module folia.math.port.common_layout_native_observer;
import folia.math.output.common;
import folia.math.output.scene;
import folia.math.util.property_host;
import std;

namespace folia::math::port::common_layout {
namespace scene=folia::math::output::scene;
namespace {
using folia::math::output::common::CommonWrapper;
using folia::math::output::common::LayoutJob;

std::string jsonString(std::string_view value) {
    std::string out="\"";
    constexpr char hex[]="0123456789abcdef";
    for(const unsigned char c:value){
        switch(c){
        case '\\':out+="\\\\";break;
        case '"':out+="\\\"";break;
        case '\n':out+="\\n";break;
        case '\r':out+="\\r";break;
        case '\t':out+="\\t";break;
        default:
            if(c<0x20){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}
            else out+=static_cast<char>(c);
        }
    }
    return out+'"';
}

std::string number(double value) {
    if(std::isnan(value))return "{\"$\":\"NaN\"}";
    if(value==std::numeric_limits<double>::infinity())return "{\"$\":\"Infinity\"}";
    if(value==-std::numeric_limits<double>::infinity())return "{\"$\":\"-Infinity\"}";
    if(value==0&&std::signbit(value))return "{\"$\":\"-0\"}";
    std::ostringstream out;
    out<<std::setprecision(std::numeric_limits<double>::max_digits10)<<value;
    return out.str();
}

std::string bboxJson(const folia::math::BBox& box) {
    return "{\"w\":"+number(box.w)+",\"h\":"+number(box.h)+
        ",\"d\":"+number(box.d)+",\"scale\":"+number(box.scale)+
        ",\"rscale\":"+number(box.rscale)+",\"L\":"+number(box.L)+
        ",\"R\":"+number(box.R)+",\"pwidth\":"+jsonString(box.pwidth)+
        ",\"ic\":"+number(box.ic)+",\"oc\":"+number(box.oc)+
        ",\"sk\":"+number(box.sk)+",\"dx\":"+number(box.dx)+"}";
}

std::string propertyJson(const Property&,std::unordered_set<const void*>&);
std::string propertyListJson(const PropertyList& value,std::unordered_set<const void*>& active){
    std::string out="{";bool first=true;
    for(const auto& key:value.keys()){
        if(!first)out+=',';
        first=false;out+=jsonString(key)+":"+propertyJson(value.getOwn(key),active);
    }
    return out+"}";
}
std::string propertyArrayJson(const PropertyArray& value,std::unordered_set<const void*>& active){
    std::string out="[";
    for(std::size_t i=0;i<value.size();++i){
        if(i)out+=',';
        out+=value.has(i)?propertyJson(value.get(i),active):"null";
    }
    return out+"]";
}
std::string propertyJson(const Property& value,std::unordered_set<const void*>& active){
    if(std::holds_alternative<Missing>(value.value))return "{\"$\":\"undefined\"}";
    if(std::holds_alternative<PropertyNull>(value.value))return "null";
    if(const auto* item=std::get_if<bool>(&value.value))return *item?"true":"false";
    if(const auto* item=std::get_if<double>(&value.value))return number(*item);
    if(const auto* item=std::get_if<std::string>(&value.value))return jsonString(*item);
    if(const auto* item=std::get_if<BuiltinProperty>(&value.value))
        return "{\"$\":\"builtin\",\"value\":"+std::to_string(static_cast<unsigned>(*item))+"}";
    if(const auto* item=std::get_if<FunctionRef>(&value.value))
        return *item?"{\"$\":\"function\"}":"null";
    if(const auto* item=std::get_if<options::SymbolRef>(&value.value))
        return "{\"$\":\"symbol\",\"context\":"+std::to_string(item->context)+
            ",\"index\":"+std::to_string(item->index)+"}";
    const auto* storage=std::get_if<std::shared_ptr<PropertyStorage>>(&value.value);
    if(!storage||!*storage)return "null";
    if(!active.insert(storage->get()).second)
        throw options::Exception("TypeError",u"Circular property graph in native layout observer");
    std::string result;
    if((*storage)->array){
        const auto array=PropertyArray::array(value);
        result=array?propertyArrayJson(*array,active):"[]";
    }else{
        const auto record=PropertyList::record(value);
        result=record?propertyListJson(*record,active):"{}";
    }
    active.erase(storage->get());
    return result;
}
}

struct LayoutTrace::State {
    struct PlaceCall {std::size_t wrapper=0;double x=0,y=0;bool explicitElement=false;
        std::optional<std::string> kind;};
    struct PlaceCharCall {std::size_t wrapper=0;std::uint32_t codepoint=0;double x=0,y=0;
        std::string variant;std::optional<bool> buffer;std::optional<double> returnedWidth;};
    using Call=std::variant<PlaceCall,PlaceCharCall>;
    WrapperIds ids;
    std::vector<Call> calls;
    std::vector<std::string> paints;

    std::size_t beginPlaceChar(CommonWrapper& wrapper,std::uint32_t cp,double x,double y,
        const scene::NodeRef&,std::string_view variant,std::optional<bool> buffer){
        calls.emplace_back(PlaceCharCall{ids.idOf(&wrapper),cp,x,y,std::string(variant),buffer,{}});
        return calls.size()-1;
    }
    void finishPlaceChar(std::size_t token,double width){
        if(token>=calls.size()||!std::holds_alternative<PlaceCharCall>(calls[token]))
            throw std::logic_error("Mismatched native placeChar observer token");
        std::get<PlaceCharCall>(calls[token]).returnedWidth=width;
    }
    void place(CommonWrapper& wrapper,double x,double y,const scene::NodeRef& element,
               bool explicitElement){
        std::optional<std::string> kind;
        if(element)kind=property_host::debugText(element->kind);
        calls.emplace_back(PlaceCall{ids.idOf(&wrapper),x,y,explicitElement,std::move(kind)});
    }
    void paint(CommonWrapper*,std::u16string_view kind,const PropertyList& attributes,
               std::size_t childCount){
        std::unordered_set<const void*> active;
        paints.push_back("{\"kind\":"+jsonString(property_host::debugText(kind))+
            ",\"attributes\":"+propertyListJson(attributes,active)+
            ",\"childCount\":"+std::to_string(childCount)+"}");
    }
    std::string callsJson()const{
        std::string out="[";bool first=true;
        for(const auto& call:calls){
            if(!first)out+=',';first=false;
            if(const auto* place=std::get_if<PlaceCall>(&call)){
                out+="{\"method\":\"place\",\"wrapper\":"+std::to_string(place->wrapper)+
                    ",\"x\":"+number(place->x)+",\"y\":"+number(place->y)+
                    ",\"explicitElement\":"+(place->explicitElement?"true":"false")+
                    ",\"kind\":"+(place->kind?jsonString(*place->kind):"null")+"}";
            }else{
                const auto& character=std::get<PlaceCharCall>(call);
                out+="{\"method\":\"placeChar\",\"wrapper\":"+std::to_string(character.wrapper)+
                    ",\"codepoint\":"+std::to_string(character.codepoint)+
                    ",\"x\":"+number(character.x)+",\"y\":"+number(character.y)+
                    ",\"variant\":"+jsonString(character.variant)+
                    ",\"buffer\":"+(character.buffer?std::string(*character.buffer?"true":"false"):std::string("{\"$\":\"undefined\"}"));
                if(character.returnedWidth)out+=",\"returnedWidth\":"+number(*character.returnedWidth);
                out+='}';
            }
        }
        return out+"]";
    }
    std::string paintJson()const{
        std::string out="[";
        for(std::size_t i=0;i<paints.size();++i){if(i)out+=',';out+=paints[i];}
        return out+"]";
    }
};

std::size_t WrapperIds::idOf(const void* wrapper){
    if(!wrapper)throw std::logic_error("Source observer encountered a null wrapper");
    const auto [entry,inserted]=ids_.emplace(wrapper,next_);
    if(inserted)++next_;
    return entry->second;
}

WrapperSnapshot snapshotWrappers(const void* rootPointer,const void* jobPointer,
                                 WrapperIds& ids) {
    const auto& root=*static_cast<const CommonWrapper*>(rootPointer);
    const auto& job=*static_cast<const LayoutJob*>(jobPointer);
    if(!job.nodeMap)throw std::logic_error("actual layout job has no nodeMap");
    // The Source observer assigns IDs from place/placeChar/paint before it
    // snapshots wrappers. Do not seed a new registry or force the root to 0.
    const auto rootId=ids.idOf(&root);
    std::string records="[";
    bool firstRecord=true;
    job.nodeMap->forEachValue([&](CommonWrapper& wrapper){
        const auto wrapperId=ids.idOf(&wrapper);
        const auto kind=wrapper.node->kind();
        if(!wrapper.childNodes)
            throw folia::math::options::Exception("TypeError",u"Cannot read properties of undefined (reading 'map')");
        const auto children=*wrapper.childNodes;
        if(!children)
            throw folia::math::options::Exception("TypeError",u"Cannot read properties of null (reading 'map')");
        std::string childIds="[";
        for(std::size_t i=0;i<children->size();++i){
            if(i)childIds+=',';
            childIds+=std::to_string(ids.idOf(children->get(i).requireNode("map")));
        }
        childIds+=']';
        const auto variant=wrapper.variant;
        const auto innerJson=bboxJson(*wrapper.getBBox());
        const auto outerJson=bboxJson(*wrapper.getOuterBBox());
        if(!firstRecord)records+=',';
        firstRecord=false;
        records+="{\"id\":"+std::to_string(wrapperId)+
            ",\"kind\":"+jsonString(kind)+",\"children\":"+childIds+
            ",\"variant\":"+jsonString(variant)+",\"bbox\":"+innerJson+
            ",\"outerBBox\":"+outerJson+"}";
    });
    records+=']';
    return WrapperSnapshot{rootId,std::move(records)};
}

LayoutTrace::LayoutTrace():state_(std::make_shared<State>()),
    observer_(std::make_shared<scene::LayoutEventObserver>()){
    auto actualObserver=std::static_pointer_cast<scene::LayoutEventObserver>(observer_);
    std::weak_ptr<State> weak=state_;
    actualObserver->place=[weak](CommonWrapper& wrapper,double x,double y,
                            const scene::NodeRef& element,bool explicitElement){
        if(auto state=weak.lock())state->place(wrapper,x,y,element,explicitElement);
    };
    actualObserver->beginPlaceChar=[weak](CommonWrapper& wrapper,std::uint32_t cp,double x,double y,
        const scene::NodeRef& parent,std::string_view variant,std::optional<bool> buffer){
        if(auto state=weak.lock())return state->beginPlaceChar(wrapper,cp,x,y,parent,variant,buffer);
        return std::size_t{};
    };
    actualObserver->finishPlaceChar=[weak](std::size_t token,double width){
        if(auto state=weak.lock())state->finishPlaceChar(token,width);
    };
    actualObserver->paint=[weak](CommonWrapper* wrapper,std::u16string_view kind,
        const PropertyList& attributes,std::size_t childCount){
        if(auto state=weak.lock())state->paint(wrapper,kind,attributes,childCount);
    };
}
WrapperIds& LayoutTrace::ids()const noexcept{return state_->ids;}
std::string LayoutTrace::callsJson()const{return state_->callsJson();}
std::string LayoutTrace::paintJson()const{return state_->paintJson();}
}
