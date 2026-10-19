module folia.math.output.common;
import :mtext;
import :output_jax;
import :wrapper_factory;
import folia.math.util.property_host;
import std;

namespace folia::math::output::common {
namespace {
struct InheritFont { std::string family;bool italic=false,bold=false; };
const std::map<std::string,InheritFont,std::less<>>& inheritFonts(){
    static const std::map<std::string,InheritFont,std::less<>> values{
        {"normal",{"",false,false}},{"bold",{"",false,true}},
        {"italic",{"",true,false}},{"bold-italic",{"",true,true}}
    };
    return values;
}
}
CommonMtext::CommonMtext(CommonWrapperFactory& factory,mml::MmlNode& source,CommonWrapper* parent)
    :CommonWrapper(factory,source,parent){}
std::vector<std::u16string> CommonMtext::splitText(std::u16string_view text)const{
    std::vector<std::u16string> words;std::size_t start=0;
    for(std::size_t i=0;i<text.size();++i)if(text[i]==u' '){words.emplace_back(text.substr(start,i-start));start=i+1;}
    words.emplace_back(text.substr(start));return words;
}
double CommonMtext::textWidth(std::u16string_view text){
    if(!textNode_){
        auto* created=node->factoryOwner().create("text",std::span<const std::any>{});
        if(!created)throw options::Exception("TypeError",u"Text factory returned no node");
        created->setParent(mml::NodeValue(node));
        textNode_=factory().wrap(created);
        if(!textNode_)throw options::Exception("TypeError",u"Wrapper factory returned no text wrapper");
        textNode_->parent=this;
    }
    auto* textNode=dynamic_cast<mml::TextNode*>(textNode_->node);
    if(!textNode)throw options::Exception("TypeError",u"Text wrapper node is not a TextNode");
    textNode->setText(std::u16string(text));
    textNode_->invalidateBBox(false);
    return textNode_->getBBox()->w;
}
std::size_t CommonMtext::breakCount(){return breakPoints.size();}
void CommonMtext::setBreakAt(IndexData index){breakPoints.push_back(std::move(index));}
void CommonMtext::clearBreakPoints(){breakPoints={};}
void CommonMtext::getVariant(){
    const auto& jaxValue=jax();
    const auto options=jaxValue.options;
    auto option=[&](std::u16string_view key){return jaxValue.optionsContext().get(options,options::Key(std::u16string(key)));};
    const auto* item=jaxValue.math;
    Property outputData=item?item->outputData:Property(Missing{});
    const auto data=PropertyList::record(outputData);
    const auto family=data?data->get("mtextFamily"):Property(Missing{});
    const auto errorFamily=data?data->get("merrorFamily"):Property(Missing{});
    const auto mtextFont=option(u"mtextFont"),merrorFont=option(u"merrorFont");
    const auto parentNode=node->Parent();
    const bool merror=(property_host::truthy(errorFamily)||options::truthy(merrorFont))&&
        parentNode.isNode()&&parentNode.nodeOrNull()->isKind("merror");
    if(property_host::truthy(family)||options::truthy(mtextFont)||merror){
        const auto mathvariant=node->attributes()->get("mathvariant");
        const auto key=property_host::debugText(property_host::string(mathvariant));
        const auto found=inheritFonts().find(key);
        const InheritFont inherited=found==inheritFonts().end()?InheritFont{}:found->second;
        if(found==inheritFonts().end()&&!font)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getCssFont')");
        const auto selectedFont=found==inheritFonts().end()?font->getCssFont(key):CssFontData{};
        std::string selectedFamily=inherited.family;
        bool italic=inherited.italic,bold=inherited.bold;
        if(found==inheritFonts().end()){
            const auto value=PropertyArray::array(selectedFont);
            if(!value)throw options::Exception("TypeError",u"CSS font data is not an array");
            selectedFamily=property_host::debugText(property_host::string(value->get(0)));
            italic=property_host::truthy(value->get(1));bold=property_host::truthy(value->get(2));
        }
        if(selectedFamily.empty()){
            if(merror){
                if(property_host::truthy(errorFamily))selectedFamily=property_host::debugText(property_host::string(errorFamily));
                else if(const auto* value=std::get_if<std::u16string>(&merrorFont.data))selectedFamily=property_host::debugText(*value);
                else throw options::Exception("TypeError",u"font family is not a string");
            }else if(property_host::truthy(family))selectedFamily=property_host::debugText(property_host::string(family));
            else if(const auto* value=std::get_if<std::u16string>(&mtextFont.data))selectedFamily=property_host::debugText(*value);
            else throw options::Exception("TypeError",u"font family is not a string");
        }
        variant=explicitVariant(selectedFamily,bold?"bold":"",italic?"italic":"");
        return;
    }
    CommonWrapper::getVariant();
}
std::shared_ptr<LineBBox> CommonMtext::computeLineBBox(std::size_t index){
    const auto linebreakOptionsValue=this->linebreakOptions();
    auto& context=jax().optionsContext();
    const auto rawLeading=context.get(linebreakOptionsValue,options::Key(u"lineleading"));
    const std::optional<double> leading=context.toNumber(rawLeading);
    auto result=std::make_shared<LineBBox>(LineBBox::from(*getOuterBBox(),leading));
    if(!breakCount())return result;
    result->w=getBreakWidth(index);
    if(index==0){result->R=0;addLeftBorders(*result);}
    else{
        result->L=0;result->indentData=std::make_shared<PropertyArray>(PropertyArray{
            Property(PropertyArray{Property("left"),Property("0")}),
            Property(PropertyArray{Property("left"),Property("0")}),
            Property(PropertyArray{Property("left"),Property("0")})});
        if(index==breakCount())addRightBorders(*result);
    }
    return result;
}
double CommonMtext::getBreakWidth(std::size_t index){
    if(!childNodes||!(*childNodes))throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'length')");
    const auto children=*childNodes;
    const auto start=index&&index-1<breakPoints.size()?breakPoints[index-1].value_or(std::array<std::size_t,2>{0,0}):std::array<std::size_t,2>{0,0};
    const auto end=index<breakPoints.size()?breakPoints[index].value_or(std::array<std::size_t,2>{children->size(),0}):std::array<std::size_t,2>{children->size(),0};
    const auto startChild=children->get(start[0]);
    auto* first=dynamic_cast<CommonWrapper*>(startChild.requireNode("node"));
    auto* text=dynamic_cast<mml::TextNode*>(first->node);
    if(!text)throw options::Exception("TypeError",u"Text child is not a TextNode");
    auto words=splitText(text->getText());
    if(start[0]==end[0]){
        std::u16string fragment;bool firstWord=true;
        for(std::size_t j=start[1];j<end[1]&&j<words.size();++j){if(!firstWord)fragment.push_back(u' ');fragment+=words[j];firstWord=false;}
        return textWidth(fragment);
    }
    std::u16string fragment;bool firstWord=true;
    for(std::size_t j=start[1];j<words.size();++j){if(!firstWord)fragment.push_back(u' ');fragment+=words[j];firstWord=false;}
    double width=textWidth(fragment);
    std::size_t childIndex=start[0];
    while(++childIndex<end[0]&&childIndex<children->size()){
        auto* child=dynamic_cast<CommonWrapper*>(children->get(childIndex).requireNode("getBBox"));
        width+=child->getBBox()->w;
    }
    if(childIndex<children->size()){
        auto* child=dynamic_cast<CommonWrapper*>(children->get(childIndex).requireNode("node"));
        auto* childText=dynamic_cast<mml::TextNode*>(child->node);
        if(!childText)throw options::Exception("TypeError",u"Text child is not a TextNode");
        words=splitText(childText->getText());fragment.clear();firstWord=true;
        for(std::size_t j=0;j<end[1]&&j<words.size();++j){if(!firstWord)fragment.push_back(u' ');fragment+=words[j];firstWord=false;}
        width+=textWidth(fragment);
    }
    return width;
}
}
