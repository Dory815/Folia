module folia.math.input.tex.stack_item;
import folia.math.input.tex.stack_item_host;
import folia.math.util.property_host;
import std;
namespace folia::math::tex {
void MmlStack::Push(std::span<const mml::NodeValue> values){
    if(sourceAccess_){std::vector<options::Value> raw;raw.reserve(values.size());for(auto value:values)raw.push_back(sourceAccess_->encodeNode(value));sourcePushValues(raw);return;}
    const auto receiver=stack_item_host::array(nodes_,"push");for(const auto& value:values)receiver->push_back(value);
}
void MmlStack::sourcePushValues(std::span<const options::Value> values)const{
    auto& access=sourceAccess();const auto array=sourceNodesCurrentValue();const auto method=access.get(array,options::Key(u"push"));(void)access.callApply(method,array,values);
}
options::Value MmlStack::sourcePopValue()const{auto& access=sourceAccess();const auto array=sourceNodesCurrentValue();const auto method=access.get(array,options::Key(u"pop"));return access.callMember(method,array,{});}
mml::NodeValue MmlStack::Pop(){if(sourceAccess_)return sourceAccess_->decodeNode(sourcePopValue());const auto receiver=stack_item_host::array(nodes_,"pop");return receiver->pop_back();}
options::Value MmlStack::sourceFirstValue()const{
    auto& access=sourceAccess();const auto array=sourceNodesCurrentValue();const auto method=access.getReceiver(u"Size");const auto size=access.callMember(method,access.receiver(),{});return access.getIndex(array,access.toNumber(size)-1.0);
}
void MmlStack::sourceSetFirstValue(options::Value value)const{
    auto& access=sourceAccess();const auto array=sourceNodesCurrentValue();const auto method=access.getReceiver(u"Size");const auto size=access.callMember(method,access.receiver(),{});access.setIndex(array,access.toNumber(size)-1.0,std::move(value));
}
mml::NodeValue MmlStack::First()const{if(sourceAccess_)return sourceAccess_->decodeNode(sourceFirstValue());const auto receiver=nodes_;const double index=Size()-1;const std::string property=stack_item_host::key(index);return stack_item_host::array(receiver,property)->getProperty(property);}
void MmlStack::setFirst(mml::NodeValue value){if(sourceAccess_){sourceSetFirstValue(sourceAccess_->encodeNode(value));return;}const auto receiver=nodes_;const double index=Size()-1;const std::string property=stack_item_host::key(index);stack_item_host::writeArray(receiver,property)->setProperty(property,std::move(value));}
options::Value MmlStack::sourceLastValue()const{auto& access=sourceAccess();return access.getIndex(sourceNodesCurrentValue(),0);}
void MmlStack::sourceSetLastValue(options::Value value)const{auto& access=sourceAccess();access.setIndex(sourceNodesCurrentValue(),0,std::move(value));}
mml::NodeValue MmlStack::Last()const{if(sourceAccess_)return sourceAccess_->decodeNode(sourceLastValue());return stack_item_host::array(nodes_,"0")->getProperty("0");}
void MmlStack::setLast(mml::NodeValue value){if(sourceAccess_){sourceSetLastValue(sourceAccess_->encodeNode(value));return;}stack_item_host::writeArray(nodes_,"0")->setProperty("0",std::move(value));}
options::Value MmlStack::sourcePeekValue(std::optional<options::Value> count)const{
    if(!count||std::holds_alternative<options::Null>(count->data)||std::holds_alternative<options::Undefined>(count->data))count=options::Value(1.0);
    auto& access=sourceAccess();const auto array=sourceNodesCurrentValue();
    const auto slice=access.get(array,options::Key(u"slice"));const auto sizeMethod=access.getReceiver(u"Size");const auto size=access.callMember(sizeMethod,access.receiver(),{});
    const std::array<options::Value,1> args{options::Value(access.toNumber(size)-access.toNumber(*count))};return access.callMember(slice,array,args);
}
mml::Children MmlStack::Peek(Property count)const{if(sourceAccess_)throw options::Exception("TypeError",u"Source-backed Peek must be consumed through the raw Context slice API");const auto receiver=nodes_;if(property_host::nullish(count))count=1;const auto array=stack_item_host::array(receiver,"slice");return array->sliceNumber(Size()-property_host::number(count));}
double MmlStack::Size()const{if(sourceAccess_)return sourceAccess_->toNumber(sourceAccess_->get(sourceNodesCurrentValue(),options::Key(u"length")));return static_cast<double>(stack_item_host::array(nodes_,"length")->size());}
void MmlStack::Clear(){if(sourceAccess_){sourceAccess_->setReceiver(u"_nodes",options::Value(sourceAccess_->makeArray()));return;}nodes_=std::make_shared<tree::ChildArray<mml::MmlNode>>();}
options::Value MmlStack::sourceToMmlValue(options::Value inferred,options::Value forceRow)const{
    if(std::holds_alternative<options::Undefined>(inferred.data))inferred=options::Value(true);
    auto& access=sourceAccess();const auto array=sourceNodesCurrentValue();const auto length=access.get(array,options::Key(u"length"));
    const auto* number=std::get_if<double>(&length.data);
    if(number&&*number==1.0&&!access.truthy(forceRow))return access.getReceiver(u"First");
    const auto create=access.getReceiver(u"create");const std::array<options::Value,4> args{options::Value(u"node"),options::Value(access.truthy(inferred)?u"inferredMrow":u"mrow"),sourceNodesCurrentValue(),options::Value(access.makeObject())};
    return access.callMember(create,access.receiver(),args);
}
mml::NodeValue MmlStack::toMml(Property inferred,Property forceRow){
    if(sourceAccess_){const auto value=sourceToMmlValue(sourceAccess_->encodeProperty(std::move(inferred)),sourceAccess_->encodeProperty(std::move(forceRow)));return sourceAccess_->decodeNode(value);}
    if(inferred.missing())inferred=true;const auto receiver=stack_item_host::array(nodes_,"length");if(receiver->size()==1&&!property_host::truthy(forceRow))return First();const Property kind=property_host::truthy(inferred)?Property("inferredMrow"):Property("mrow");const std::array<std::any,3> arguments{std::any(kind),std::any(receiver),std::any(Property(PropertyList{}))};return create("node",arguments);
}
options::Value MmlStack::sourceCreateValues(std::span<const options::Value> args)const{
    auto& access=sourceAccess();const auto factoryValue=access.getReceiver(u"factory");const auto configuration=access.get(factoryValue,options::Key(u"configuration"));const auto nodeFactory=access.get(configuration,options::Key(u"nodeFactory"));const auto create=access.get(nodeFactory,options::Key(u"create"));std::vector<options::Value> callArgs;callArgs.reserve(args.size());callArgs.insert(callArgs.end(),args.begin(),args.end());return access.callApply(create,nodeFactory,callArgs);
}
mml::NodeValue MmlStack::create(std::string_view kind,FunctionArgs rest){
    if(sourceAccess_){std::vector<options::Value> args;args.reserve(rest.size()+1);args.emplace_back(property_host::text(kind));for(const auto& entry:rest){if(const auto* property=std::any_cast<Property>(&entry))args.push_back(sourceAccess_->encodeProperty(*property));else if(const auto* child=std::any_cast<mml::NodeValue>(&entry))args.push_back(sourceAccess_->encodeNode(*child));else if(const auto* children=std::any_cast<mml::Children>(&entry))args.push_back(sourceAccess_->encodeChildren(*children));else if(const auto* raw=std::any_cast<options::Value>(&entry))args.push_back(*raw);else throw options::Exception("TypeError",u"StackItem create argument has no Source Value conversion");}return sourceAccess_->decodeNode(sourceCreateValues(args));}
    const auto factoryReference=factory();auto& service=stack_item_host::require<StackItemFactoryService>(factoryReference,"configuration");auto& configuration=stack_item_host::require<StackItemFactoryConfiguration>(service.configuration,"nodeFactory");auto& nodeFactory=stack_item_host::require<NodeFactory>(configuration.nodeFactoryReference(),"create");return nodeFactory.create(kind,rest);
}
}
