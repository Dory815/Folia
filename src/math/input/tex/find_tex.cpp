// MathJax 4.1.3 input/tex/FindTeX.ts; Apache-2.0.
module folia.math.input.tex.find_tex;
import std;
import folia.math.util.string;
import folia.math.util.property_host;
import folia.math.util.js_number;
import folia.math.util.js_numeric_input;

namespace {
using namespace folia::math;
using namespace folia::math::options;
using folia::math::regexp_host::Match;

[[noreturn]] void type_error(std::u16string message) {
    throw options::Exception("TypeError",std::move(message));
}

std::u16string widen_ascii(std::string_view value) {
    std::u16string result;
    result.reserve(value.size());
    for (const unsigned char unit : value) result.push_back(static_cast<char16_t>(unit));
    return result;
}

std::u16string primitive_string(const Value& value) {
    if (std::holds_alternative<Undefined>(value.data)) return u"undefined";
    if (std::holds_alternative<Null>(value.data)) return u"null";
    if (const auto* text=std::get_if<std::u16string>(&value.data)) return *text;
    if (const auto* boolean=std::get_if<bool>(&value.data)) return *boolean?u"true":u"false";
    if (const auto* number=std::get_if<double>(&value.data)) return widen_ascii(js_number::toString(*number));
    if (std::holds_alternative<SymbolRef>(value.data))
        type_error(u"Cannot convert a Symbol value to a string");
    type_error(u"Object-to-primitive string conversion has no native binding");
}

double primitive_number(const Value& value) {
    if (std::holds_alternative<Undefined>(value.data)) return std::numeric_limits<double>::quiet_NaN();
    if (std::holds_alternative<Null>(value.data)) return 0;
    if (const auto* boolean=std::get_if<bool>(&value.data)) return *boolean?1:0;
    if (const auto* number=std::get_if<double>(&value.data)) return *number;
    if (const auto* text=std::get_if<std::u16string>(&value.data)) return js_input::number(*text);
    if (std::holds_alternative<SymbolRef>(value.data))
        type_error(u"Cannot convert a Symbol value to a number");
    type_error(u"Object-to-primitive number conversion has no native binding");
}

const std::u16string& require_text(const Value& value,std::u16string_view operation) {
    const auto* text=std::get_if<std::u16string>(&value.data);
    if (!text) {
        std::u16string message(operation);
        message+=u" is not a function";
        type_error(std::move(message));
    }
    return *text;
}

Property to_property(const Value& value) {
    if (std::holds_alternative<Undefined>(value.data)) return Missing{};
    if (std::holds_alternative<Null>(value.data)) return PropertyNull{};
    if (const auto* boolean=std::get_if<bool>(&value.data)) return *boolean;
    if (const auto* number=std::get_if<double>(&value.data)) return *number;
    if (const auto* text=std::get_if<std::u16string>(&value.data))
        return property_host::debugText(*text);
    type_error(u"Unsupported native ProtoItem field value");
}

Key index_key(std::uint64_t index) {
    const auto ascii=std::to_string(index);
    return Key(std::u16string(ascii.begin(),ascii.end()));
}

class OrdinaryArrayValueIterator {
    Context& context_;
    ObjectRef array_;
    std::uint64_t index_=0;
    bool done_=false;
public:
    OrdinaryArrayValueIterator(Context& context,ObjectRef array)
        :context_(context),array_(array){}
    Value next(){
        if (done_) return Undefined{};
        // Array iterators re-read length on each next(), but once a next call
        // observes completion, later growth does not revive the iterator.
        const std::uint32_t length=context_.arrayLength(array_);
        if (index_>=length) {
            done_=true;
            return Undefined{};
        }
        const std::uint64_t index=index_++;
        return context_.get(Value(array_),index_key(index));
    }
};

std::size_t capture_index(const Value& value) {
    const auto* number=std::get_if<double>(&value.data);
    if (!number || !std::isfinite(*number) || *number<0 || std::trunc(*number)!=*number)
        type_error(u"Invalid native RegExp capture index");
    return static_cast<std::size_t>(*number);
}

std::u16string capture_or_empty(const Match& match,std::size_t index) {
    const auto capture=match.at(index);
    return capture?*capture:std::u16string{};
}

std::u16string join(const std::vector<std::u16string>& strings) {
    std::u16string result;
    for (std::size_t i=0;i<strings.size();++i) {
        if (i) result.push_back(u'|');
        result+=strings[i];
    }
    return result;
}

std::u16string substring(const Value& text,std::size_t start,std::size_t finish) {
    const auto& source=require_text(text,u"text.substring");
    start=std::min(start,source.size());
    finish=std::min(finish,source.size());
    if (start>finish) std::swap(start,finish);
    return source.substr(start,finish-start);
}

double item_end_n(const Property& item) {
    const auto record=PropertyList::record(item);
    if (!record) type_error(u"ProtoItem.end is not an object");
    const auto end=record->get("end");
    const auto value=property_host::get(end,"n");
    const auto* number=std::get_if<double>(&value.value);
    if (!number) type_error(u"ProtoItem.end.n is not a number");
    return *number;
}
}

namespace folia::math::tex {
using options::Context;
using options::Key;
using options::Null;
using options::ObjectKind;
using options::ObjectRef;
using options::Undefined;
using options::Value;
using folia::math::Property;
using folia::math::PropertyList;
using folia::math::PropertyNull;

void FindTeX::initializeFindTeX(){
    this->getPatterns();
}

void FindTeX::getPatterns(){
    const Value options=this->options_;
    const auto starts=std::make_shared<std::vector<std::u16string>>();
    std::vector<std::u16string> parts;
    std::vector<std::u16string> subparts;

    // Source replaces end before resetting the two numeric capture indexes.
    const auto new_end=optionsContext().object();
    this->end_=Value(new_end);
    this->sub_=Value(0);
    this->env_=Value(0);
    double i=1;

    auto run_for_each=[&](std::u16string_view option_name,bool display){
        const Value delimiters=optionsContext().get(options,Key(std::u16string(option_name)));
        const Value method=optionsContext().get(delimiters,Key(u"forEach"));

        const auto* owner=std::any_cast<std::shared_ptr<NativeObject>>(
            &optionsContext().payload(instanceIdentity()));
        if (!owner || !*owner) type_error(u"FindTeX instance has no native owner");
        auto self=std::dynamic_pointer_cast<FindTeX>(*owner);
        if (!self) type_error(u"FindTeX instance has the wrong native owner");

        const ObjectRef callback=optionsContext().function(u"FindTeX.forEach",
            [self,starts,display](Context&,Value,std::span<const Value> arguments)->Value{
                const Value delims=arguments.empty()?Value(Undefined{}):arguments.front();
                self->addPattern(starts,delims,display);
                return Undefined{};
            });
        const auto* method_ref=std::get_if<ObjectRef>(&method.data);
        if (!method_ref) type_error(u"forEach is not a function");
        const std::array<Value,1> arguments{Value(callback)};
        (void)optionsContext().call(*method_ref,delimiters,arguments);
    };

    run_for_each(u"inlineMath",false);
    run_for_each(u"displayMath",true);

    if (!starts->empty()) {
        std::stable_sort(starts->begin(),starts->end(),[](const auto& left,const auto& right){
            return string_util::sortLength(left,right)<0;
        });
        parts.push_back(join(*starts));
    }

    if (options::truthy(optionsContext().get(options,Key(u"processEnvironments")))) {
        parts.emplace_back(u"\\\\begin\\s*\\{([^}]*)\\}");
        this->env_=Value(i);
        i++;
    }
    if (options::truthy(optionsContext().get(options,Key(u"processEscapes"))))
        subparts.emplace_back(u"\\\\([\\\\$])");
    if (options::truthy(optionsContext().get(options,Key(u"processRefs"))))
        subparts.emplace_back(u"(\\\\(?:eq)?ref\\s*\\{[^}]*\\})");
    if (!subparts.empty()) {
        parts.push_back(u"("+join(subparts)+u")");
        this->sub_=Value(i);
    }

    this->start_=Value(regexp_.create(join(parts),u"g"));
    this->hasPatterns_=Value(!parts.empty());
}

void FindTeX::addPattern(StringArray starts,Delims delims,bool display){
    const auto* tuple=std::get_if<ObjectRef>(&delims.data);
    if (!tuple || optionsContext().kind(*tuple)!=ObjectKind::array)
        type_error(u"Delimiter pair is not iterable");

    // Source destructures through the ordinary array iterator. Each yield
    // checks the current length; completion is sticky if an earlier getter
    // shrinks the tuple.
    OrdinaryArrayValueIterator iterator(optionsContext(),*tuple);
    const Value open=iterator.next();
    const Value close=iterator.next();
    const auto& open_text=require_text(open,u"text.replace");
    starts->push_back(string_util::quotePattern(open_text));

    // Evaluate the assignment receiver/key before its RHS, since endPattern is
    // virtual and can observe or replace the current end table.
    const Value end_receiver=this->end_;
    const Key key(open_text);
    const Value pattern=this->endPattern(close,Undefined{});
    const ObjectRef end_tuple=optionsContext().array(3);
    optionsContext().set(end_tuple,Key(u"0"),close);
    optionsContext().set(end_tuple,Key(u"1"),Value(display));
    optionsContext().set(end_tuple,Key(u"2"),pattern);
    optionsContext().set(end_receiver,key,Value(end_tuple));
}

Value FindTeX::endPattern(Value end,Value endp){
    std::u16string prefix;
    if (options::truthy(endp)) {
        prefix=primitive_string(endp);
    } else {
        prefix=string_util::quotePattern(require_text(end,u"text.replace"));
    }
    prefix+=u"|\\\\(?:[a-zA-Z]|.)|[{}]";
    return Value(regexp_.create(std::move(prefix),u"g"));
}

Property FindTeX::findEnd(Value text,double n,const Match& start,EndItem end){
    const auto* tuple=std::get_if<ObjectRef>(&end.data);
    if (!tuple || optionsContext().kind(*tuple)!=ObjectKind::array)
        type_error(u"End delimiter data is not iterable");
    OrdinaryArrayValueIterator iterator(optionsContext(),*tuple);
    const Value close=iterator.next();
    const Value display=iterator.next();
    const Value pattern=iterator.next();
    const auto start_text=start.at(0);
    if (!start_text) type_error(u"RegExp match is missing its full match");
    const double i=static_cast<double>(start.index+start_text->size());
    regexp_.setLastIndex(pattern,i);

    std::size_t braces=0;
    for (;;) {
        auto match=regexp_.exec(pattern,primitive_string(text));
        if (!match) break;
        const auto first=match->at(1);
        const std::u16string candidate=first && !first->empty()?*first:capture_or_empty(*match,0);
        if (Value(candidate)==close && braces==0) {
            const auto full=match->at(0);
            if (!full) type_error(u"RegExp match is missing its full match");
            const auto content=substring(text,static_cast<std::size_t>(i),match->index);
            return core::protoItem(
                Property(property_host::debugText(*start_text)),
                Property(property_host::debugText(content)),
                Property(property_host::debugText(*full)),Property(n),
                Property(static_cast<double>(start.index)),
                Property(static_cast<double>(match->index+full->size())),to_property(display));
        }
        const auto full=capture_or_empty(*match,0);
        if (full==u"{") {
            ++braces;
        } else if (full==u"}" && braces) {
            --braces;
        }
    }
    return PropertyNull{};
}

void FindTeX::findMathInString(core::ProtoMathItems math,double n,Value text){
    regexp_.setLastIndex(this->start_,0);
    for (;;) {
        const auto start=regexp_.exec(this->start_,primitive_string(text));
        if (!start) break;

        Property match=PropertyNull{};
        const std::size_t env_index=capture_index(this->env_);
        const auto environment=start->at(env_index);
        if (environment && options::truthy(this->env_)) {
            const std::u16string ending=u"\\\\end\\s*(\\{"+
                string_util::quotePattern(*environment)+u"\\})";
            const Value end_pattern=this->endPattern(Value(Null{}),Value(ending));
            const ObjectRef tuple=optionsContext().array(3);
            const std::u16string close=u"{"+*environment+u"}";
            optionsContext().set(tuple,Key(u"0"),Value(close));
            optionsContext().set(tuple,Key(u"1"),Value(true));
            optionsContext().set(tuple,Key(u"2"),end_pattern);
            match=this->findEnd(text,n,*start,Value(tuple));
            if (property_host::truthy(match)) {
                auto record=PropertyList::record(match);
                if (!record) type_error(u"ProtoItem is not an object");
                const auto open=record->get("open");
                const auto content=record->get("math");
                const auto close_value=record->get("close");
                const auto open_text=property_host::text(std::get<std::string>(open.value));
                const auto content_text=property_host::text(std::get<std::string>(content.value));
                const auto close_text=property_host::text(std::get<std::string>(close_value.value));
                record->setOwn("math",property_host::debugText(open_text+content_text+close_text));
                // Source's chained assignment writes close before open.
                record->setOwn("close","");
                record->setOwn("open","");
            }
        } else {
            const std::size_t sub_index=capture_index(this->sub_);
            const auto sub=start->at(sub_index);
            if (sub && options::truthy(this->sub_)) {
                const double end=static_cast<double>(start->index+sub->size());
                if (sub->size()==2) {
                    match=core::protoItem(Property("\\"),Property(property_host::debugText(sub->substr(1))),
                        Property(""),Property(n),Property(static_cast<double>(start->index)),Property(end));
                } else {
                    match=core::protoItem(Property(""),Property(property_host::debugText(*sub)),
                        Property(""),Property(n),Property(static_cast<double>(start->index)),Property(end),Property(false));
                }
            } else {
                const auto open=start->at(0);
                if (!open) type_error(u"RegExp match is missing its full match");
                const Value end_item=optionsContext().get(this->end_,Key(*open));
                match=this->findEnd(text,n,*start,end_item);
            }
        }

        if (property_host::truthy(match)) {
            math->emplace_back(match);
            regexp_.setLastIndex(this->start_,item_end_n(match));
        }
    }
}

core::ProtoMathItems FindTeX::findMath(std::any strings){
    auto math=std::make_shared<std::vector<core::ProtoMathItem>>();
    if (!options::truthy(this->hasPatterns_)) return math;

    Value input=Undefined{};
    if (const auto* supplied=std::any_cast<Value>(&strings)) input=*supplied;
    const Value length=optionsContext().get(input,Key(u"length"));
    const double limit=primitive_number(length);
    for (double i=0;i<limit;++i) {
        const Value text=optionsContext().get(input,index_key(static_cast<std::uint64_t>(i)));
        this->findMathInString(math,i,text);
    }
    return math;
}

}
