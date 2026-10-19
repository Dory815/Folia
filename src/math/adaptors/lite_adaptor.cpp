module folia.math.adaptors.lite_adaptor;
import folia.math.util.property_host;
import folia.math.util.utf16;
import folia.math.util.styles;
import folia.math.util.js_numeric_input;
import folia.math.util.regexp_host;
import std;

namespace folia::math::adaptors::lite {
namespace {
std::string keyOf(std::u16string_view text){return property_host::debugText(text);}
std::u16string textOf(const Property& value){return property_host::string(value);}
std::u16string trimEcma(std::u16string_view text){std::size_t first=0,last=text.size();while(first<last&&js_input::isWhitespace(text[first]))++first;while(last>first&&js_input::isWhitespace(text[last-1]))--last;return std::u16string(text.substr(first,last-first));}
std::u16string joinWithSpace(const std::vector<std::u16string>& values){std::u16string result;for(std::size_t i=0;i<values.size();++i){if(i)result.push_back(u' ');result+=values[i];}return result;}
bool isNode(const LiteNodeValue& value){return std::holds_alternative<LiteNodeRef>(value);}
LiteNodeRef nodeValue(const LiteNodeValue& value){return std::get<LiteNodeRef>(value);}
void appendScalar(std::u16string& out,std::uint32_t value){
    if(value<=0xFFFF){out.push_back(static_cast<char16_t>(value));return;}
    value-=0x10000;out.push_back(static_cast<char16_t>(0xD800+(value>>10)));out.push_back(static_cast<char16_t>(0xDC00+(value&0x3FF)));
}
std::u16string nodeMixinCjkPattern(){
    std::u16string pattern=u"[";
    const std::array<std::pair<std::uint32_t,std::uint32_t>,16> ranges{{
        {0x1100,0x115F},{0x2329,0x232A},{0x2E80,0x303E},{0x3040,0x3247},
        {0x3250,0x4DBF},{0x4E00,0xA4C6},{0xA960,0xA97C},{0xAC00,0xD7A3},
        {0xF900,0xFAFF},{0xFE10,0xFE19},{0xFE30,0xFE6B},{0xFF01,0xFF60},
        {0xFFE0,0xFFE6},{0x1B000,0x1B001},{0x1F200,0x1F251},{0x20000,0x3FFFD}
    }};
    for(const auto [first,last]:ranges){appendScalar(pattern,first);if(first!=last){pattern.push_back(u'-');appendScalar(pattern,last);}}
    pattern.push_back(u']');return pattern;
}
std::u16string regexpReplaceWithEmpty(regexp_host::RegExp& expression,std::u16string_view input){
    const std::u16string text(input);const bool global=expression.flags().find(u'g')!=std::u16string::npos;
    expression.lastIndex=0;std::size_t copied=0;std::u16string output;
    while(const auto match=expression.exec(text)){
        const auto whole=match->at(0);if(!whole)break;
        const auto end=match->index+whole->size();
        if(match->index>text.size()||end>text.size())property_host::typeError("Invalid RegExp match range");
        output.append(text,copied,match->index-copied);copied=end;
        if(!global)break;
        if(whole->empty()){
            // String.prototype.replace advances empty global matches using the
            // regexp's Unicode-aware AdvanceStringIndex operation.
            auto next=match->index;
            const bool unicode=expression.flags().find(u'u')!=std::u16string::npos||expression.flags().find(u'v')!=std::u16string::npos;
            if(next<text.size()){
                if(unicode&&text[next]>=0xD800&&text[next]<=0xDBFF&&next+1<text.size()&&text[next+1]>=0xDC00&&text[next+1]<=0xDFFF)next+=2;
                else ++next;
            }else ++next;
            expression.lastIndex=static_cast<double>(next);
        }
    }
    output.append(text,copied,std::u16string::npos);
    if(global)expression.lastIndex=0;
    return output;
}
std::size_t codePointLength(std::u16string_view text){
    std::size_t count=0;
    for(std::size_t i=0;i<text.size();++i){
        if(text[i]>=0xD800&&text[i]<=0xDBFF&&i+1<text.size()&&text[i+1]>=0xDC00&&text[i+1]<=0xDFFF)++i;
        ++count;
    }
    return count;
}
Property documentArgument(const LiteNodeValue& input, const LiteNodeValue& defaultDocument, std::string_view member) {
    Property doc;
    if (std::holds_alternative<Missing>(input)) {
        if(const auto* node=std::get_if<LiteNodeRef>(&defaultDocument))doc=nodeProperty(*node);
        else if(std::holds_alternative<PropertyNull>(defaultDocument))doc=Property(PropertyNull{});
        else if(const auto* raw=std::get_if<Property>(&defaultDocument))doc=*raw;
        else doc=Property(Missing{});
    }
    else if (std::holds_alternative<PropertyNull>(input)) doc = Property(PropertyNull{});
    else if (const auto* node = std::get_if<LiteNodeRef>(&input)) doc = nodeProperty(*node);
    else doc = std::get<Property>(input);
    if (doc.missing() || std::holds_alternative<PropertyNull>(doc.value)) {
        const auto kind = doc.missing() ? "undefined" : "null";
        property_host::typeError(std::string("Cannot read properties of ") + kind + " (reading '" + std::string(member) + "')");
    }
    return doc;
}
Property documentField(const Property& document, std::string_view name) {
    if (auto object = PropertyList::record(document)) return object->get(name);
    return Property(Missing{});
}
LiteNodeValue domValue(Property value) { return propertyNode(std::move(value)); }
std::u16string jsSlice(std::u16string_view s,double start,double end){
    auto integer=[](double n,std::size_t len)->std::int64_t{
        if(std::isnan(n)||n==0)return 0;
        if(std::isinf(n))return n<0?-static_cast<std::int64_t>(len):static_cast<std::int64_t>(len);
        double v=std::trunc(n);if(v< -static_cast<double>(len))return 0;if(v<0)return static_cast<std::int64_t>(len)+static_cast<std::int64_t>(v);if(v>static_cast<double>(len))return static_cast<std::int64_t>(len);return static_cast<std::int64_t>(v);
    };
    const auto n=static_cast<std::int64_t>(s.size());auto a=integer(start,s.size()),b=integer(end,s.size());a=std::clamp<std::int64_t>(a,0,n);b=std::clamp<std::int64_t>(b,0,n);if(b<a)b=a;return std::u16string(s.substr(static_cast<std::size_t>(a),static_cast<std::size_t>(b-a)));
}
std::vector<LiteNodeRef> search(LiteNodeRef root,const std::function<bool(LiteNodeRef)>& predicate,std::optional<std::size_t> stop={}){
    std::vector<LiteNodeRef> result,stack{root};
    while(!stack.empty()){
        auto current=stack.front();stack.erase(stack.begin());
        auto& node=record(current);
        if(node.nodeKind==NodeKind::Element){
            if(predicate(current)){result.push_back(current);if(stop&&result.size()==*stop)return result;}
            auto children=nodeArrayValues(childArray(current));
            stack.insert(stack.begin(),children.begin(),children.end());
        }
    }
    return result;
}
std::optional<std::pair<std::u16string,std::u16string>> parseAttributeSelector(std::u16string_view s){
    if(s.size()<5||s.front()!=u'['||s.back()!=u']')return{};auto eq=s.find(u"=\"");if(eq==std::u16string_view::npos||s.size()<eq+4||s[s.size()-2]!=u'"')return{};
    return std::pair{std::u16string(s.substr(1,eq-1)),std::u16string(s.substr(eq+2,s.size()-eq-4))};
}
bool tagName(std::u16string_view s){if(s.empty())return false;auto alpha=[](char16_t c){return(c>=u'a'&&c<=u'z')||(c>=u'A'&&c<=u'Z');};auto extra=[&](char16_t c){return alpha(c)||(c>=u'0'&&c<=u'9')||c==u'-';};if(!alpha(s.front())&&s.front()!=u'-')return false;for(auto c:s)if(!extra(c))return false;return true;}
void splice(PropertyArray& array,std::size_t start,std::size_t removeCount,std::span<const Property> insertions={}){
    const auto old=array.size();start=std::min(start,old);removeCount=std::min(removeCount,old-start);
    const auto tailStart=start+removeCount;const auto tailCount=old-tailStart;
    if(insertions.size()<removeCount){
        for(std::size_t i=0;i<tailCount;++i){const auto src=tailStart+i,dst=start+insertions.size()+i;if(array.has(src))array.set(dst,array.get(src));else array.erase(dst);}
        array.resize(old-removeCount+insertions.size());
    }else if(insertions.size()>removeCount){
        const auto grow=insertions.size()-removeCount;array.resize(old+grow);
        for(std::size_t i=tailCount;i>0;--i){const auto src=tailStart+i-1,dst=src+grow;if(array.has(src))array.set(dst,array.get(src));else array.erase(dst);}
    }
    for(std::size_t i=0;i<insertions.size();++i)array.set(start+i,insertions[i]);
}
void removeChildAt(LiteNodeRef parent,std::ptrdiff_t index){auto ids=childArray(parent);if(index<0)index+=static_cast<std::ptrdiff_t>(ids.size());if(index>=0&&static_cast<std::size_t>(index)<ids.size())splice(ids,static_cast<std::size_t>(index),1);}
}

LiteBase::LiteBase(std::shared_ptr<LiteNodeStore> store,std::shared_ptr<entities::EntityModule> entities):store_(store?std::move(store):LiteNodeStore::create()) {
    window_=LiteWindow(store_).ref();
    parser_=std::make_shared<LiteParser>(std::move(entities));
}
LiteNodeRef LiteBase::create(std::u16string_view kind,std::u16string_view){return LiteElement(std::u16string(kind),{}, {},store_).ref();}
LiteNodeRef LiteBase::parse(std::u16string_view text,std::u16string_view format){return parser_->parseFromString(text,format,*this);}
LiteNodeRef LiteBase::text(std::u16string_view text){return LiteText(std::u16string(text),false,store_).ref();}
void LiteBase::setProperty(LiteNodeRef node,std::string_view name,Property value){liteStore(node)->object(node).set(name,std::move(value));}
Property LiteBase::getProperty(LiteNodeRef node,std::string_view name)const{return liteStore(node)->object(node).get(name);}
LiteNodeRef LiteBase::comment(std::u16string_view text){return LiteText(std::u16string(text),true,store_).ref();}
LiteNodeRef LiteBase::createDocument(){return LiteDocument(LiteNodeValue(PropertyNull{}),store_).ref();}
LiteNodeValue LiteBase::head(LiteNodeValue doc)const{return domValue(documentField(documentArgument(doc,document,"head"),"head"));}
LiteNodeValue LiteBase::body(LiteNodeValue doc)const{return domValue(documentField(documentArgument(doc,document,"body"),"body"));}
LiteNodeValue LiteBase::root(LiteNodeValue doc)const{return domValue(documentField(documentArgument(doc,document,"root"),"root"));}
std::u16string LiteBase::doctype(LiteNodeValue doc)const{return property_host::string(documentField(documentArgument(doc,document,"type"),"type"));}
std::vector<LiteNodeRef> LiteBase::tags(LiteNodeRef node,std::u16string_view name,std::u16string_view ns)const{return tags(node,name,ns,{});}
std::vector<LiteNodeRef> LiteBase::tags(LiteNodeRef node,std::u16string_view name,std::u16string_view ns,std::optional<std::size_t> stop)const{if(!ns.empty())return{};const Property wanted(property_host::debugText(name));return search(node,[&](LiteNodeRef n){return recordConst(n).nodeKind==NodeKind::Element&&nodeField(n,"kind")==wanted;},stop);}
LiteNodeRef LiteBase::elementById(LiteNodeRef node,std::u16string_view id)const{auto found=search(node,[&](LiteNodeRef n){auto a=attributeList(n).get("id");const auto* text=std::get_if<std::string>(&a.value);return text&&property_host::text(*text)==id;},1);return found.empty()?LiteNodeRef{}:found.front();}
std::vector<LiteNodeRef> LiteBase::elementsByClass(LiteNodeRef node,std::u16string_view name,std::optional<std::size_t> stop)const{
    return search(node,[&](LiteNodeRef n){auto value=attributeList(n).get("class");std::u16string s;if(property_host::truthy(value)){const auto* raw=std::get_if<std::string>(&value.value);if(!raw)property_host::typeError("class.trim is not a function");s=trimEcma(property_host::text(*raw));}std::vector<std::u16string> words;if(s.empty())words.emplace_back();else{std::size_t i=0;while(i<s.size()){auto e=s.find(u' ',i);if(e==std::u16string::npos)e=s.size();words.emplace_back(s.substr(i,e-i));i=e;while(i<s.size()&&s[i]==u' ')++i;}}return std::find(words.begin(),words.end(),name)!=words.end();},stop);
}
std::vector<LiteNodeRef> LiteBase::elementsByAttribute(LiteNodeRef node,std::u16string_view name,std::u16string_view value,std::optional<std::size_t> stop)const{
    return search(node,[&](LiteNodeRef n){auto a=attributeList(n).get(keyOf(name));return std::holds_alternative<std::string>(a.value)&&textOf(a)==value;},stop);
}
std::vector<LiteNodeRef> LiteBase::getElements(const std::vector<LiteSearchInput>& selectors,LiteNodeRef doc)const{
    auto bodyValue=body(LiteNodeValue(doc));
    if(!std::holds_alternative<LiteNodeRef>(bodyValue))property_host::typeError("document.body is not a node");
    auto container=std::get<LiteNodeRef>(std::move(bodyValue));std::vector<LiteNodeRef> out;
    for(const auto& selector:selectors){
        if(auto s=std::get_if<std::u16string>(&selector)){
            auto& t=*s;if(t.empty())continue;
            if(t.front()==u'#'){auto n=elementById(container,std::u16string_view(t).substr(1));if(n)out.push_back(n);}
            else if(t.front()==u'.'){auto found=elementsByClass(container,std::u16string_view(t).substr(1));out.insert(out.end(),found.begin(),found.end());}
            else if(tagName(t)){auto found=tags(container,t);out.insert(out.end(),found.begin(),found.end());}
            else if(auto attr=parseAttributeSelector(t)){auto found=elementsByAttribute(container,attr->first,attr->second);out.insert(out.end(),found.begin(),found.end());}
        } else if(auto array=std::get_if<std::vector<LiteNodeRef>>(&selector))out.insert(out.end(),array->begin(),array->end());
        else if(auto node=std::get_if<LiteNodeRef>(&selector)){
            const auto& r=recordConst(*node);
            if(r.nodeKind==NodeKind::List){auto nodes=nodeArrayValues(r.listNodes);out.insert(out.end(),nodes.begin(),nodes.end());}
            else out.push_back(*node);
        }
    }
    return out;
}
LiteNodeValue LiteBase::getElement(std::u16string_view selector,LiteNodeValue input)const{
    if(std::holds_alternative<Missing>(input))input=document;
    LiteNodeRef node;
    if(const auto* ref=std::get_if<LiteNodeRef>(&input))node=*ref;
    else if(const auto* property=std::get_if<Property>(&input)){auto ref=LiteNodeStore::fromProperty(*property);if(ref)node=*ref;}
    if(!node)property_host::typeError("Cannot read properties of undefined (reading 'attributes')");
    if(recordConst(node).nodeKind==NodeKind::Document){auto bodyValue=body(LiteNodeValue(node));if(!std::holds_alternative<LiteNodeRef>(bodyValue))property_host::typeError("document.body is not a node");node=std::get<LiteNodeRef>(std::move(bodyValue));}
    if(!selector.empty()&&selector.front()==u'#'){auto found=elementById(node,selector.substr(1));return found?LiteNodeValue(found):LiteNodeValue(PropertyNull{});}
    if(!selector.empty()&&selector.front()==u'.'){auto found=elementsByClass(node,selector.substr(1),1);return found.empty()?LiteNodeValue(Missing{}):LiteNodeValue(found.front());}
    if(tagName(selector)){auto found=tags(node,selector,{},1);return found.empty()?LiteNodeValue(Missing{}):LiteNodeValue(found.front());}
    if(auto attr=parseAttributeSelector(selector)){auto found=elementsByAttribute(node,attr->first,attr->second,1);return found.empty()?LiteNodeValue(Missing{}):LiteNodeValue(found.front());}
    return PropertyNull{};
}
bool LiteBase::contains(LiteNodeRef container,LiteNodeRef node)const{while(node&&node!=container){auto p=parentValue(node);if(!std::holds_alternative<LiteNodeRef>(p))return false;node=std::get<LiteNodeRef>(p);}return node==container;}
LiteNodeValue LiteBase::parent(LiteNodeRef node)const{return parentValue(node);}
std::ptrdiff_t LiteBase::childIndex(LiteNodeRef node)const{auto p=parentValue(node);if(!std::holds_alternative<LiteNodeRef>(p))return -1;auto parentNode=std::get<LiteNodeRef>(p);const auto ids=childArray(parentNode);for(std::size_t i=0;i<ids.size();++i)if(ids.has(i)&&nodeFromProperty(ids.get(i))==node)return static_cast<std::ptrdiff_t>(i);return -1;}
LiteNodeRef LiteBase::append(LiteNodeRef node,LiteNodeRef child){if(!node||!child)property_host::typeError("Cannot read properties of null (reading 'parent')");auto p=parentValue(child);if(std::holds_alternative<LiteNodeRef>(p))remove(child);auto children=childArray(node);appendNode(children,child);setNodeField(child,"parent",nodeProperty(node));return child;}
void LiteBase::insert(LiteNodeRef child,LiteNodeRef sibling){if(!child)property_host::typeError("Cannot read properties of null (reading 'parent')");auto p=parentValue(child);if(std::holds_alternative<LiteNodeRef>(p))remove(child);if(sibling){auto sp=parentValue(sibling);if(std::holds_alternative<LiteNodeRef>(sp)){auto parentNode=std::get<LiteNodeRef>(sp);auto index=childIndex(sibling);std::array<Property,1> inserted{nodeProperty(child)};auto children=childArray(parentNode);const auto length=static_cast<std::ptrdiff_t>(children.size());const auto normalized=index<0?std::max<std::ptrdiff_t>(0,length+index):std::min(index,length);splice(children,static_cast<std::size_t>(normalized),0,inserted);setNodeField(child,"parent",nodeProperty(parentNode));}}}
LiteNodeRef LiteBase::remove(LiteNodeRef child){auto i=childIndex(child);auto p=parentValue(child);if(i>=0&&std::holds_alternative<LiteNodeRef>(p))removeChildAt(std::get<LiteNodeRef>(p),i);setNodeField(child,"parent",Property(PropertyNull{}));return child;}
LiteNodeRef LiteBase::replace(LiteNodeRef node,LiteNodeRef oldNode){auto i=childIndex(oldNode);if(i>=0){auto p=std::get<LiteNodeRef>(parentValue(oldNode));auto children=childArray(p);children.set(static_cast<std::size_t>(i),nodeProperty(node));setNodeField(node,"parent",nodeProperty(p));setNodeField(oldNode,"parent",Property(PropertyNull{}));}return oldNode;}
LiteNodeRef LiteBase::clone(LiteNodeRef node,bool deep)const{
    if(recordConst(node).nodeKind!=NodeKind::Element)property_host::typeError("LiteAdaptor clone expects an element");
    auto store=liteStore(node);auto source=recordConst(node);auto sourceAttributes=attributeList(node);auto sourceChildren=childArray(node);auto copy=LiteElement(nodeField(node,"kind"),{}, {},store).ref();record(copy).attributes=PropertyList{};for(const auto& key:sourceAttributes.keys())record(copy).attributes.set(key,sourceAttributes.get(key));
    if(deep){
        const auto length=sourceChildren.size();
        PropertyArray clonedChildren;clonedChildren.resize(length);
        // Array.map captures length, checks HasProperty per index, then Gets
        // live. Holes stay holes; an own undefined still reaches `n.kind` and
        // throws as the Source callback does.
        for(std::size_t i=0;i<length;++i){
            if(!sourceChildren.has(i))continue;
            const auto rawChild=sourceChildren.get(i);
            const auto childValue=propertyNode(rawChild);
            if(!std::holds_alternative<LiteNodeRef>(childValue)){
                if(std::holds_alternative<PropertyNull>(rawChild.value))
                    property_host::typeError("Cannot read properties of null (reading 'kind')");
                property_host::typeError("Cannot read properties of undefined (reading 'kind')");
            }
            const auto child=std::get<LiteNodeRef>(childValue);
            const auto& childRecord=recordConst(child);
            LiteNodeRef cloned;
            if(childRecord.nodeKind==NodeKind::Text)
                cloned=LiteText(childRecord.value,false,store).ref();
            else if(childRecord.nodeKind==NodeKind::Comment)
                cloned=LiteText(childRecord.value,true,store).ref();
            else{
                cloned=clone(child,true);
                setNodeField(cloned,"parent",nodeProperty(copy));
            }
            clonedChildren.set(i,nodeProperty(cloned));
        }
        record(copy).children=std::move(clonedChildren);
    }
    return copy;
}
LiteNodeRef LiteBase::split(LiteNodeRef textNode,double index){auto& node=record(textNode);if(node.nodeKind!=NodeKind::Text&&node.nodeKind!=NodeKind::Comment)property_host::typeError("Cannot read properties of undefined (reading 'children')");auto store=liteStore(textNode);const auto* raw=std::get_if<std::string>(&node.value.value);if(!raw)property_host::typeError("node.value.slice is not a function");auto old=property_host::text(*raw);auto newText=LiteText(Property(property_host::debugText(jsSlice(old,index,static_cast<double>(old.size())))),false,store).ref();node.value=Property(property_host::debugText(jsSlice(old,0,index)));auto parent=propertyNode(nodeField(textNode,"parent"));if(!std::holds_alternative<LiteNodeRef>(parent))property_host::typeError("Cannot read properties of null (reading 'children')");auto parentNode=std::get<LiteNodeRef>(parent);auto at=childIndex(textNode)+1;std::array<Property,1> inserted{nodeProperty(newText)};auto children=childArray(parentNode);splice(children,static_cast<std::size_t>(std::max<std::ptrdiff_t>(0,at)),0,inserted);setNodeField(newText,"parent",nodeProperty(parentNode));return newText;}
LiteNodeValue LiteBase::next(LiteNodeRef node)const{auto p=parentValue(node);if(!std::holds_alternative<LiteNodeRef>(p))return PropertyNull{};auto parentNode=std::get<LiteNodeRef>(p);auto i=childIndex(node)+1;const auto ids=childArray(parentNode);if(i<0||static_cast<std::size_t>(i)>=ids.size())return PropertyNull{};return ids.has(static_cast<std::size_t>(i))?LiteNodeValue(nodeFromProperty(ids.get(static_cast<std::size_t>(i)))):LiteNodeValue(Missing{});}
LiteNodeValue LiteBase::previous(LiteNodeRef node)const{auto p=parentValue(node);if(!std::holds_alternative<LiteNodeRef>(p))return PropertyNull{};auto parentNode=std::get<LiteNodeRef>(p);auto i=childIndex(node)-1;const auto ids=childArray(parentNode);if(i<0)return PropertyNull{};return static_cast<std::size_t>(i)<ids.size()&&ids.has(static_cast<std::size_t>(i))?LiteNodeValue(nodeFromProperty(ids.get(static_cast<std::size_t>(i)))):LiteNodeValue(Missing{});}
LiteNodeValue LiteBase::firstChild(LiteNodeRef node)const{const auto ids=childArray(node);return ids.size()&&!ids.has(0)?LiteNodeValue(Missing{}):ids.size()?LiteNodeValue(nodeFromProperty(ids.get(0))):LiteNodeValue(Missing{});}
LiteNodeValue LiteBase::lastChild(LiteNodeRef node)const{const auto ids=childArray(node);return ids.size()&&ids.has(ids.size()-1)?LiteNodeValue(nodeFromProperty(ids.get(ids.size()-1))):LiteNodeValue(Missing{});}
DOMNodeArray LiteBase::childNodes(LiteNodeRef node)const{
    const auto source=childNodesArray(node);
    PropertyArray result;
    for(std::size_t i=0;i<source.size();++i) result.push(source.get(i));
    return {std::move(result),liteStore(node)};
}
std::u16string LiteBase::kind(LiteNodeRef node)const{const auto& r=recordConst(node);if(r.nodeKind==NodeKind::Text)return u"#text";if(r.nodeKind==NodeKind::Comment)return u"#comment";if(r.nodeKind==NodeKind::Document)return u"#document";return property_host::string(nodeField(node,"kind"));}
std::u16string LiteBase::value(LiteNodeRef node)const{const auto& r=recordConst(node);if(r.nodeKind==NodeKind::Text)return property_host::string(nodeField(node,"value"));if(r.nodeKind==NodeKind::Comment){auto text=property_host::string(nodeField(node,"value"));static regexp_host::RegExp comment(u"^<!(--)?((?:.|\\n)*)\\1>$");auto match=comment.exec(text);if(match){if(auto capture=match->at(2))return *capture;}return text;}return{};}
std::u16string LiteBase::textContent(LiteNodeRef node)const{
    const auto& receiver=recordConst(node);
    // Source reads `node.children` and immediately calls `.reduce`. Only
    // LiteElement owns that field; Text, Comment, and Document receivers fail
    // at the same missing-property operation.
    if(receiver.nodeKind!=NodeKind::Element)
        property_host::typeError("Cannot read properties of undefined (reading 'reduce')");
    std::u16string out;
    const auto children=childArray(node);
    for(std::size_t i=0;i<children.size();++i){
        if(!children.has(i))continue;
        const auto childValue=propertyNode(children.get(i));
        const auto* child=std::get_if<LiteNodeRef>(&childValue);
        if(!child){
            if(std::holds_alternative<Missing>(childValue))
                property_host::typeError("Cannot read properties of undefined (reading 'kind')");
            if(std::holds_alternative<PropertyNull>(childValue))
                property_host::typeError("Cannot read properties of null (reading 'kind')");
            property_host::typeError("Cannot read properties of undefined (reading 'kind')");
        }
        const auto kindValue=recordConst(*child).nodeKind;
        if(kindValue==NodeKind::Text)out+=property_host::string(recordConst(*child).value);
        else if(kindValue!=NodeKind::Comment)out+=textContent(*child);
    }
    return out;
}
std::u16string LiteBase::innerHTML(LiteNodeRef node)const{return parser_->serializeInner(*const_cast<LiteBase*>(this),node);}
std::u16string LiteBase::outerHTML(LiteNodeRef node)const{return parser_->serialize(*const_cast<LiteBase*>(this),node);}
std::u16string LiteBase::serializeXML(LiteNodeRef node)const{
    if(serializeXmlObserver_)serializeXmlObserver_();
    return parser_->serialize(*const_cast<LiteBase*>(this),node,true);
}
void LiteBase::setAttribute(LiteNodeRef node,std::u16string_view name,Property value,std::u16string_view ns){auto key=std::u16string(name);if(!ns.empty()){auto slash=ns.find_last_of(u'/');auto prefix=ns.substr(slash==std::u16string_view::npos?0:slash+1);auto colon=key.find_last_of(u':');key=std::u16string(prefix)+u":"+key.substr(colon==std::u16string::npos?0:colon+1);}attributeList(node).set(keyOf(key),Property(property_host::debugText(textOf(value))));if(key==u"style")liteStore(node)->setNodeStyles(node,{});}
Property LiteBase::getAttribute(LiteNodeRef node,std::u16string_view name)const{return attributeList(node).get(keyOf(name));}
void LiteBase::removeAttribute(LiteNodeRef node,std::u16string_view name){attributeList(node).erase(keyOf(name));}
bool LiteBase::hasAttribute(LiteNodeRef node,std::u16string_view name)const{return attributeList(node).hasOwn(keyOf(name));}
std::vector<AttributeData> LiteBase::allAttributes(LiteNodeRef node)const{std::vector<AttributeData> out;auto attrs=attributeList(node);for(const auto& name:attrs.keys())out.push_back({property_host::text(name),attrs.get(name)});return out;}
void LiteBase::addClass(LiteNodeRef node,std::u16string_view name){auto raw=getAttribute(node,u"class");if(raw.missing()||std::holds_alternative<PropertyNull>(raw.value)){setAttribute(node,u"class",Property(property_host::debugText(name)));return;}if(!std::holds_alternative<std::string>(raw.value))property_host::typeError("class.split is not a function");auto classes=textOf(raw);std::vector<std::u16string> split;std::size_t pos=0;for(;;){auto end=classes.find(u' ',pos);if(end==std::u16string::npos){split.push_back(classes.substr(pos));break;}split.push_back(classes.substr(pos,end-pos));pos=end+1;}if(std::find(split.begin(),split.end(),name)==split.end()){split.emplace_back(name);setAttribute(node,u"class",Property(property_host::debugText(joinWithSpace(split))));}}
void LiteBase::removeClass(LiteNodeRef node,std::u16string_view name){auto raw=getAttribute(node,u"class");if(raw.missing()||std::holds_alternative<PropertyNull>(raw.value))return;if(!std::holds_alternative<std::string>(raw.value))property_host::typeError("class.split is not a function");auto classes=textOf(raw);std::vector<std::u16string> split;std::size_t pos=0;for(;;){auto end=classes.find(u' ',pos);if(end==std::u16string::npos){split.push_back(classes.substr(pos));break;}split.push_back(classes.substr(pos,end-pos));pos=end+1;}auto it=std::find(split.begin(),split.end(),name);if(it!=split.end()){split.erase(it);setAttribute(node,u"class",Property(property_host::debugText(joinWithSpace(split))));}}
bool LiteBase::hasClass(LiteNodeRef node,std::u16string_view name)const{auto raw=getAttribute(node,u"class");if(!property_host::truthy(raw))raw=Property(std::string{});if(!std::holds_alternative<std::string>(raw.value))property_host::typeError("class.split is not a function");auto classes=textOf(raw);std::vector<std::u16string> split;std::size_t pos=0;for(;;){auto end=classes.find(u' ',pos);if(end==std::u16string::npos){split.push_back(classes.substr(pos));break;}split.push_back(classes.substr(pos,end-pos));pos=end+1;}return std::find(split.begin(),split.end(),name)!=split.end();}
void LiteBase::setStyle(LiteNodeRef node,std::u16string_view name,std::u16string_view value){auto store=liteStore(node);auto style=store->nodeStyles(node);auto attrs=attributeList(node);if(!style){style=std::make_shared<styles::Styles>(textOf(attrs.get("style")));store->setNodeStyles(node,style);}style->set(name,styles::StyleValue(std::u16string(value)));attrs.set("style",Property(property_host::debugText(style->cssText())));}
std::u16string LiteBase::getStyle(LiteNodeRef node,std::u16string_view name){auto store=liteStore(node);auto style=store->nodeStyles(node);if(!style){auto raw=attributeList(node).get("style");if(!property_host::truthy(raw))return{};style=std::make_shared<styles::Styles>(textOf(raw));store->setNodeStyles(node,style);}return style->get(name);}
Property LiteBase::allStyles(LiteNodeRef node)const{return attributeList(node).get("style");}
void LiteBase::insertRules(LiteNodeRef node,const std::vector<std::u16string>& rules){auto content=textContent(node)+u"\n\n";for(std::size_t i=0;i<rules.size();++i){if(i)content+=u"\n\n";content+=rules[i];}auto textNode=LiteText(std::move(content),false,liteStore(node)).ref();setNodeField(node,"children",Property(PropertyArray{}));auto children=childArray(node);appendNode(children,textNode);setNodeField(textNode,"parent",nodeProperty(node));}
double LiteBase::fontSize(LiteNodeRef)const{return 0;}
std::u16string LiteBase::fontFamily(LiteNodeRef)const{return{};}
std::pair<double,double> LiteBase::nodeSize(LiteNodeRef,double,std::optional<bool>)const{return{0,0};}
PageBBox LiteBase::nodeBBox(LiteNodeRef)const{return{};}

PropertyList LiteAdaptor::classOptions(){static PropertyList options=[](){PropertyList o;o.set("fontSize",16.0);o.set("fontFamily","Times");o.set("cjkCharWidth",1.0);o.set("unknownCharWidth",.6);o.set("unknownCharHeight",.8);return o;}();return options;}
PropertyList LiteAdaptor::mixinOptions(){static PropertyList options=[](){PropertyList o;o.set("badCSS",true);o.set("badSizes",true);return o;}();return options;}
LiteAdaptor::LiteAdaptor(LiteNodeRef window,PropertyList options,std::shared_ptr<LiteNodeStore> store,std::shared_ptr<entities::EntityModule> entities):LiteBase(store?std::move(store):(window?liteStore(window):LiteNodeStore::create()),std::move(entities)){(void)window;canMeasureNodes=false;auto cls=classOptions();for(auto& key:cls.keys())options_.set(key,cls.get(key));for(auto& key:options.keys())options_.set(key,options.get(key));auto b=mixinOptions();auto bc=b.get("badCSS"),bs=b.get("badSizes");badCSS_=property_host::truthy(bc);badSizes_=property_host::truthy(bs);}
std::u16string LiteAdaptor::cjkPatternSource(){return nodeMixinCjkPattern();}
std::u16string LiteAdaptor::replaceCjk(std::u16string_view text,regexp_host::RegExp& pattern){return regexpReplaceWithEmpty(pattern,text);}
double LiteAdaptor::fontSize(LiteNodeRef node)const{return badCSS_?(sourceNumberOption_?sourceNumberOption_("fontSize"):property_host::number(options_.get("fontSize"))):LiteBase::fontSize(node);}
std::u16string LiteAdaptor::fontFamily(LiteNodeRef node)const{return badCSS_?property_host::string(sourceOption_?sourceOption_("fontFamily"):options_.get("fontFamily")):LiteBase::fontFamily(node);}
Property LiteAdaptor::fontFamilyProperty()const{
    if(badCSS_)return sourceOption_?sourceOption_("fontFamily"):options_.get("fontFamily");
    return Property(utf16::encode(LiteBase::fontFamily(LiteNodeRef{})));
}
std::pair<double,double> LiteAdaptor::nodeSize(LiteNodeRef node,double,std::optional<bool>)const{
    if(!badSizes_)return LiteBase::nodeSize(node);
    const Property source=sourceTextContent_?sourceTextContent_(node):Property(property_host::debugText(textContent(node)));
    const auto* rawText=std::get_if<std::string>(&source.value);
    if(!rawText){
        if(source.missing())property_host::typeError("Cannot read properties of undefined (reading 'replace')");
        if(std::holds_alternative<PropertyNull>(source.value))property_host::typeError("Cannot read properties of null (reading 'replace')");
        property_host::typeError("text.replace is not a function");
    }
    const auto text=property_host::text(*rawText);
    std::u16string withoutCjk;
    if(cjkStripper_)withoutCjk=cjkStripper_(text);
    else {regexp_host::RegExp pattern(cjkPatternSource(),u"gu");withoutCjk=replaceCjk(text,pattern);}
    const auto non=codePointLength(withoutCjk);
    const auto cjk=codePointLength(text)-non;
    const auto numberOption=[&](std::string_view name){return sourceNumberOption_?sourceNumberOption_(name):property_host::number(options_.get(name));};
    const auto cjkWidth=numberOption("cjkCharWidth");
    const auto unknownWidth=numberOption("unknownCharWidth");
    const auto unknownHeight=numberOption("unknownCharHeight");
    return {static_cast<double>(cjk)*cjkWidth+static_cast<double>(non)*unknownWidth,unknownHeight};
}
PageBBox LiteAdaptor::nodeBBox(LiteNodeRef node)const{return badSizes_?PageBBox{}:LiteBase::nodeBBox(node);}
}
