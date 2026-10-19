module folia.math.handlers.html.html_math_item;
import folia.math.core.math_item_host;
import folia.math.util.property_host;
import std;

namespace folia::math::handlers::html {
HTMLMathItem::HTMLMathItem(options::Context& context,options::ObjectRef identity,
    HTMLMathItemHost& document,Property math,core::MathInputJaxReference input,
    Property display,core::MathLocation start,core::MathLocation end)
    :core::AbstractMathItem(std::move(math),std::move(input),std::move(display),
       std::move(start),std::move(end)),context_(&context),identity_(identity),document_(&document){}

bool HTMLMathItem::isEscaped()const{return core::AbstractMathItem::isEscaped();}
options::Context& HTMLMathItem::sourceContext()const{return *context_;}
options::ObjectRef HTMLMathItem::sourceIdentity()const{return identity_;}

void HTMLMathItem::updateDocument(core::MathDocumentReference){
    if(!core::math_item_host::less(state(),core::STATE.get("INSERTED")))return;
    auto& location=document_->locationService();
    const auto startValue=core::readMathLocation(start,"node",&location);
    const auto endValue=core::readMathLocation(end,"node",&location);
    const auto startIndex=property_host::number(core::readMathLocation(start,"n",&location));
    const auto endIndex=property_host::number(core::readMathLocation(end,"n",&location));
    const auto startNodeValue=propertyNode(startValue),endNodeValue=propertyNode(endValue);
    const auto* startNode=std::get_if<DOMNodeRef>(&startNodeValue);
    const auto* endNode=std::get_if<DOMNodeRef>(&endNodeValue);
    if(!startNode||!endNode)throw options::Exception("TypeError",u"HTMLMathItem location node is not an adaptor node");
    const auto* root=std::get_if<DOMNodeRef>(&typesetRoot);
    if(!root)throw options::Exception("TypeError",u"HTMLMathItem typesetRoot is not an adaptor node");
    auto& adaptor=document_->domAdaptor();
    const auto* input=std::get_if<core::MathInputJaxService*>(&inputJax);
    if(!input||!*input)throw options::Exception("TypeError",u"MathItem.inputJax is missing");
    if(document_->processStrings(inputJax)){
        DOMNodeRef node=*startNode;
        if(*startNode==*endNode){
            if(endIndex&&endIndex<static_cast<double>(adaptor.value(*endNode).size()))(void)adaptor.split(*endNode,endIndex);
            if(startIndex)node=adaptor.split(*startNode,startIndex);
            const auto parent=adaptor.parent(node);
            if(const auto* p=std::get_if<DOMNodeRef>(&parent))(void)adaptor.replace(*root,node);
        }else{
            if(startIndex)node=adaptor.split(node,startIndex);
            while(node!=*endNode){
                const auto next=adaptor.next(node);
                const auto* n=std::get_if<DOMNodeRef>(&next);
                if(!n)throw options::Exception("TypeError",u"HTMLMathItem range ended before its end node");
                (void)adaptor.remove(node);node=*n;
            }
            adaptor.insert(*root,node);
            if(endIndex<static_cast<double>(adaptor.value(node).size()))(void)adaptor.split(node,endIndex);
            (void)adaptor.remove(node);
        }
    }else (void)adaptor.replace(*root,*startNode);
    const auto outputProperty=document_->propertyBindings().from(
        document_->propertyBindings().to(nodeProperty(*root)));
    core::writeMathLocation(start,"node",outputProperty,&location);
    core::writeMathLocation(end,"node",outputProperty,&location);
    core::writeMathLocation(start,"n",0,&location);
    core::writeMathLocation(end,"n",0,&location);
    (void)state(core::STATE.get("INSERTED"));
}

void HTMLMathItem::removeFromDocument(Property restore){
    core::AbstractMathItem::removeFromDocument(restore);
    if(!core::math_item_host::greaterEqual(state(),core::STATE.get("TYPESET")))return;
    auto& location=document_->locationService();
    const auto nodeValue=propertyNode(core::readMathLocation(start,"node",&location));
    const auto* node=std::get_if<DOMNodeRef>(&nodeValue);
    if(!node)throw options::Exception("TypeError",u"HTMLMathItem start.node is not an adaptor node");
    auto& adaptor=document_->domAdaptor();
    DOMNodeRef text=adaptor.text(u"");
    if(property_host::truthy(restore)){
        const auto left=property_host::string(core::readMathLocation(start,"delim",&location));
        const auto body=property_host::string(math);
        const auto right=property_host::string(core::readMathLocation(end,"delim",&location));
        const auto joined=left+body+right;
        if(document_->processStrings(inputJax))text=adaptor.text(joined);
        else{
            const auto parsed=adaptor.parse(joined,u"text/html");
            const auto bodyValue=adaptor.body(parsed);
            const auto* bodyNode=std::get_if<DOMNodeRef>(&bodyValue);
            if(!bodyNode)throw options::Exception("TypeError",u"restoration document has no body");
            const auto child=adaptor.firstChild(*bodyNode);
            const auto* childNode=std::get_if<DOMNodeRef>(&child);
            if(!childNode)throw options::Exception("TypeError",u"restoration body has no child");
            text=*childNode;
        }
    }
    const auto parent=adaptor.parent(*node);
    if(std::get_if<DOMNodeRef>(&parent))(void)adaptor.replace(text,*node);
    const auto property=document_->propertyBindings().from(document_->propertyBindings().to(nodeProperty(text)));
    core::writeMathLocation(start,"node",property,&location);
    core::writeMathLocation(end,"node",property,&location);
    core::writeMathLocation(start,"n",0,&location);
    core::writeMathLocation(end,"n",0,&location);
}
}
