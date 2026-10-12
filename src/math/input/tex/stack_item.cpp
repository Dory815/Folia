module folia.math.input.tex.stack_item;
import folia.math.input.tex.stack_item_host;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
void MmlStack::Push(std::span<const mml::NodeValue> values){
    const auto receiver=stack_item_host::array(nodes_,"push");
    for(const auto& value:values)receiver->push_back(value);
}
mml::NodeValue MmlStack::Pop(){
    const auto receiver=stack_item_host::array(nodes_,"pop");
    return receiver->pop_back();
}
mml::NodeValue MmlStack::First()const{
    const auto receiver=nodes_;
    const double index=Size()-1;
    const std::string property=stack_item_host::key(index);
    const auto array=stack_item_host::array(receiver,property);
    return array->getProperty(property);
}
void MmlStack::setFirst(mml::NodeValue value){
    const auto receiver=nodes_;
    const double index=Size()-1;
    const std::string property=stack_item_host::key(index);
    const auto array=stack_item_host::writeArray(receiver,property);
    array->setProperty(property,std::move(value));
}
mml::NodeValue MmlStack::Last()const{
    const auto receiver=stack_item_host::array(nodes_,"0");
    return receiver->getProperty("0");
}
void MmlStack::setLast(mml::NodeValue value){
    const auto receiver=stack_item_host::writeArray(nodes_,"0");
    receiver->setProperty("0",std::move(value));
}
mml::Children MmlStack::Peek(Property count)const{
    const auto receiver=nodes_;
    if(property_host::nullish(count))count=1;
    const auto array=stack_item_host::array(receiver,"slice");
    const double size=Size();
    const double numericCount=property_host::number(count);
    const double start=size-numericCount;
    return array->sliceNumber(start);
}
double MmlStack::Size()const{
    const auto receiver=stack_item_host::array(nodes_,"length");
    return static_cast<double>(receiver->size());
}
void MmlStack::Clear(){
    nodes_=std::make_shared<tree::ChildArray<mml::MmlNode>>();
}
mml::NodeValue MmlStack::toMml(Property inferred,Property forceRow){
    if(inferred.missing())inferred=true;
    const auto receiver=stack_item_host::array(nodes_,"length");
    if(receiver->size()==1&&!property_host::truthy(forceRow))return First();
    const Property kind=property_host::truthy(inferred)?Property("inferredMrow"):Property("mrow");
    const std::array<std::any,3> arguments{std::any(kind),std::any(receiver),std::any(Property(PropertyList{}))};
    return create("node",arguments);
}
mml::NodeValue MmlStack::create(std::string_view kind,FunctionArgs rest){
    const auto factoryReference=factory();
    auto& service=stack_item_host::require<StackItemFactoryService>(factoryReference,"configuration");
    const auto configurationReference=service.configuration;
    auto& configuration=stack_item_host::require<StackItemFactoryConfiguration>(configurationReference,"nodeFactory");
    const auto nodeFactoryReference=configuration.nodeFactoryReference();
    auto& nodeFactory=stack_item_host::require<NodeFactory>(nodeFactoryReference,"create");
    return nodeFactory.create(kind,rest);
}
}
