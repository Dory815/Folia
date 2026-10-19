module folia.math.output.scene.foundation;
import :builder;
import folia.math.util.property_host;
import folia.math.util.styles;

namespace folia::math::output::scene {
namespace {
Property sceneAttribute(Property value){
    // A Scene outlives the Context and layout job that produced it. SVG
    // attributes have already crossed the DOM setAttribute boundary here, so
    // retain their owned string value rather than a live Property handle.
    return Property(property_host::debugText(property_host::string(value)));
}
PropertyList sceneAttributes(const PropertyList& source){
    PropertyList result;
    // DOMAdaptor.setAttributes applies an object-valued style before ordinary
    // attributes. Complete that same conversion while the live record is
    // available, retaining only owned CSS text in the finished Scene.
    const auto style=source.get("style");
    if(const auto record=PropertyList::record(style)){
        styles::Styles owned;
        for(const auto& key:record->keys()){
            const auto name=property_host::text(key);
            std::u16string converted;
            for(std::size_t index=0;index<name.size();++index){
                if(name[index]==u'-'&&index+1<name.size()&&name[index+1]>=u'a'&&name[index+1]<=u'z')
                    converted.push_back(static_cast<char16_t>(name[++index]-32));
                else converted.push_back(name[index]);
            }
            owned.set(converted,property_host::string(record->get(key)));
        }
        if(!record->keys().empty())
            result.set("style",Property(property_host::debugText(owned.cssText())));
    }
    for(const auto& key:source.keys()){
        if(key!="properties"&&(key!="style"||std::holds_alternative<std::string>(style.value)))
            result.set(key,sceneAttribute(source.getOwn(key)));
    }
    return result;
}
}
bool Builder::reaches(const NodeRef& current,const Node* target){
    if(!current)return false;
    if(current.get()==target)return true;
    for(std::size_t index=0;index<current->children.size();++index)
        if(reaches(current->children[index],target))return true;
    return false;
}
NodeRef Builder::node(std::u16string kind,PropertyList attributes,std::u16string text,
                      std::size_t sourceChildCount){
    if(kind!=u"#text"&&observer_&&observer_->paint)
        observer_->paint(currentWrapper_,kind,attributes,sourceChildCount);
    auto result=std::make_shared<Node>();
    result->kind=std::move(kind);result->attributes=sceneAttributes(attributes);
    if(result->kind==u"#text")result->semantic=TextRunPrimitive{std::move(text)};
    return result;
}
NodeRef Builder::adoptExisting(std::u16string kind,PropertyList attributes,std::u16string text){
    auto result=std::make_shared<Node>();
    result->kind=std::move(kind);result->attributes=sceneAttributes(attributes);
    if(result->kind==u"#text")result->semantic=TextRunPrimitive{std::move(text)};
    return result;
}
void Builder::geometry(const NodeRef& node,Geometry geometry){
    if(!node)property_host::typeError("Cannot set properties of null (setting 'geometry')");
    node->geometry=std::move(geometry);
}
void Builder::transform(const NodeRef& node,Transform transform){
    if(!node)property_host::typeError("Cannot set properties of null (setting 'transform')");
    node->transform=transform;
}
void Builder::validateAppend(const NodeRef& parent,const NodeRef& child)const{
    if(!parent)property_host::typeError("Cannot read properties of null (reading 'children')");
    if(!child)property_host::typeError("Cannot read properties of null (reading 'append')");
    if(reaches(child,parent.get()))property_host::typeError("Scene graph cannot contain a cycle");
}
void Builder::append(const NodeRef& parent,const NodeRef& child){
    validateAppend(parent,child);
    if(const auto previous=child->parent.lock()){
        for(std::size_t index=0;index<previous->children.size();++index)
            if(previous->children[index]==child){previous->children.erase(index);break;}
    }
    parent->children.push_back(child);
    child->parent=parent;
    child->parentValue=options::Value(options::Undefined{});
}
Scene Builder::finish()&&{
    Scene result;
    result.freeze(roots_,resources_);
    return result;
}
}
