module folia.math.input.tex.tex_parser_context_fields;
import std;

namespace folia::math::tex {
namespace {
using namespace options;
Key key(std::u16string_view name){return Key(std::u16string(name));}
[[noreturn]] void unsupported(std::u16string message){
    throw Exception("Unimplemented",std::move(message));
}
std::uint32_t checkedLength(std::size_t size){
    if(size>std::numeric_limits<std::uint32_t>::max())
        throw Exception("RangeError",u"TextParser.envStack exceeds the Context Array length range");
    return static_cast<std::uint32_t>(size);
}
void active(const std::weak_ptr<std::atomic_bool>& lease){
    const auto state=lease.lock();
    if(!state||!state->load(std::memory_order_acquire))
        throw Exception("TypeError",u"TexParser Context runtime has ended");
}
}

TexParserContextFields::TexParserContextFields(Context& context,ObjectRef identity,
        std::shared_ptr<void> actualOwner,std::weak_ptr<std::atomic_bool> executionLease)
    :context_(&context),identity_(identity),owner_(std::move(actualOwner)),lease_(std::move(executionLease)){
    if(!owner_)throw Exception("TypeError",u"TexParser Context fields require their actual owner");
    (void)context_->kind(identity_);
}

void TexParserContextFields::assign(std::u16string_view name,Value initial,
        std::function<Value()> read,std::function<void(Value)> write){
    if(!read||!write)throw Exception("TypeError",u"TexParser Context field requires live read/write operations");
    auto guardedRead=[lease=lease_,read=std::move(read)](){active(lease);return read();};
    auto guardedWrite=[lease=lease_,write=std::move(write)](Value value){active(lease);write(std::move(value));};
    context_->assignNativeData(identity_,key(name),std::make_shared<NativeDataField>(
        owner_,std::move(guardedRead),std::move(guardedWrite)),std::move(initial));
}

TextParserEnvStackView::TextParserEnvStackView(ContextPropertyBindings& properties,
        std::shared_ptr<void> actualOwner,std::shared_ptr<std::vector<Property>> actualValues,
        std::weak_ptr<std::atomic_bool> executionLease)
    :properties_(&properties),owner_(std::move(actualOwner)),valuesOwner_(std::move(actualValues)),
     values_(valuesOwner_.get()),lease_(std::move(executionLease)){
    if(!owner_||!valuesOwner_)throw Exception("TypeError",u"TextParser.envStack view requires its actual owner");
}

std::uint32_t TextParserEnvStackView::length()const{active(lease_);return checkedLength(values_->size());}
Descriptor TextParserEnvStackView::lengthDescriptor()const{
    return Descriptor::data(Value(static_cast<double>(length())),true,false,false);
}
std::optional<Descriptor> TextParserEnvStackView::indexDescriptor(std::uint32_t index)const{
    active(lease_);
    if(index>=values_->size())return std::nullopt;
    return Descriptor::data(properties_->to((*values_)[index]),true,true,true);
}
std::vector<std::uint32_t> TextParserEnvStackView::ownIndexKeys()const{
    active(lease_);
    std::vector<std::uint32_t> result;result.reserve(values_->size());
    for(std::uint32_t i=0;i<length();++i)result.push_back(i);
    return result;
}
void TextParserEnvStackView::defineIndex(std::uint32_t index,Descriptor descriptor){
    active(lease_);
    if(descriptor.accessor)unsupported(u"TextParser.envStack cannot store accessor array entries in its typed Property vector");
    if(!descriptor.enumerable||!descriptor.configurable||!descriptor.writable)
        unsupported(u"TextParser.envStack cannot represent non-default array index descriptors");
    if(index>values_->size())unsupported(u"TextParser.envStack cannot represent sparse array holes");
    Property value=properties_->from(descriptor.value);
    if(index==values_->size())values_->push_back(std::move(value));
    else (*values_)[index]=std::move(value);
}
void TextParserEnvStackView::defineLength(Descriptor descriptor){
    active(lease_);
    if(descriptor.accessor)unsupported(u"TextParser.envStack length cannot be an accessor");
    const auto* number=std::get_if<double>(&descriptor.value.data);
    if(!number||!std::isfinite(*number)||*number<0||std::floor(*number)!=*number||
       *number>std::numeric_limits<std::uint32_t>::max())
        throw Exception("RangeError",u"Invalid TextParser.envStack array length");
    if(!descriptor.writable)unsupported(u"TextParser.envStack cannot represent a non-writable array length");
    const auto requested=static_cast<std::size_t>(*number);
    if(requested>values_->size())unsupported(u"TextParser.envStack cannot represent array-length expansion holes in its typed Property vector");
    values_->resize(requested);
}
void TextParserEnvStackView::deleteIndex(std::uint32_t index){
    active(lease_);
    if(index>=values_->size())return;
    if(std::size_t(index)+1==values_->size()){
        values_->pop_back();
        return;
    }
    unsupported(u"TextParser.envStack cannot represent interior deletion holes in its typed Property vector");
}

TextParserEnvStackOwner::TextParserEnvStackOwner(Context& context,ContextPropertyBindings& properties,
        std::shared_ptr<void> parserOwner,std::weak_ptr<std::atomic_bool> executionLease)
    :context_(&context),properties_(&properties),parserOwner_(std::move(parserOwner)),lease_(std::move(executionLease)){
    if(!parserOwner_)throw Exception("TypeError",u"TextParser.envStack requires its actual parser owner");
    (void)reset();
}

ObjectRef TextParserEnvStackOwner::reset(){
    active_=VectorArray{};
    auto values=std::make_shared<std::vector<Property>>();
    const ObjectRef identity=context_->nativeArray(std::make_shared<TextParserEnvStackView>(
        *properties_,parserOwner_,values,lease_));
    active_=VectorArray{std::move(values),identity};
    return identity;
}

ObjectRef TextParserEnvStackOwner::identity()const{
    if(const auto* values=std::get_if<VectorArray>(&active_))return values->identity;
    return std::get<ContextArray>(active_).identity;
}

void TextParserEnvStackOwner::replace(Value value){
    active(lease_);
    const auto* identity=std::get_if<ObjectRef>(&value.data);
    if(!identity)throw Exception("TypeError",u"TextParser.envStack must be an Array");
    // arrayLength validates that this is an actual Context Array. Keep that
    // exact object as the sole active store; do not copy its current elements.
    (void)context_->arrayLength(*identity);
    active_=ContextArray{*identity};
}

void TextParserEnvStackOwner::push(Property value){
    active(lease_);
    if(auto* vector=std::get_if<VectorArray>(&active_)){vector->values->push_back(std::move(value));return;}
    const std::array<Value,1> values{properties_->to(value)};
    (void)context_->arrayPush(std::get<ContextArray>(active_).identity,values);
}

bool TextParserEnvStackOwner::empty()const{
    active(lease_);
    if(const auto* vector=std::get_if<VectorArray>(&active_))return vector->values->empty();
    return context_->arrayLength(std::get<ContextArray>(active_).identity)==0;
}

Property TextParserEnvStackOwner::back()const{
    active(lease_);
    if(const auto* vector=std::get_if<VectorArray>(&active_)){
        if(vector->values->empty())throw Exception("RangeError",u"TextParser.envStack is empty");
        return vector->values->back();
    }
    const ObjectRef identity=std::get<ContextArray>(active_).identity;
    const auto length=context_->arrayLength(identity);
    if(length==0)throw Exception("RangeError",u"TextParser.envStack is empty");
    const auto digits=std::to_string(length-1);
    std::u16string index;index.reserve(digits.size());
    for(const char digit:digits)index.push_back(static_cast<char16_t>(digit));
    return properties_->from(context_->get(Value(identity),Key(std::move(index))));
}

Property TextParserEnvStackOwner::pop(){
    active(lease_);
    if(auto* vector=std::get_if<VectorArray>(&active_)){
        if(vector->values->empty())throw Exception("RangeError",u"TextParser.envStack is empty");
        Property value=std::move(vector->values->back());vector->values->pop_back();return value;
    }
    const Value value=context_->arrayPop(std::get<ContextArray>(active_).identity);
    return properties_->from(value);
}
}
