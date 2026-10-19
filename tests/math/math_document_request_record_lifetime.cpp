#include "support/folia_test.hpp"
import folia.math.output.svg.native_document;
import folia.math.output.svg.render_session;
import folia.math.input.tex.bussproofs.configuration;
import folia.math.util.option_types;

namespace {
using namespace boost::ut;
namespace opts=folia::math::options;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;

bool isLive(opts::Context& context,opts::ObjectRef identity){
    try{(void)context.kind(identity);return true;}
    catch(const opts::Exception& error){
        if(error.name!="TypeError")throw;
        return false;
    }
}
namespace tex=folia::math::tex;

common::FontLoadHost host(){
    common::FontLoadHost value;
    value.asyncIsSynchronous=[] {return true;};
    value.warn=[](const std::any&){};
    return value;
}

suite math_document_request_record_lifetime=[] {
"private_renderer_scope_retires_only_its_reduce_callback_and_actual_item"_test=[] {
    svg::NativeRenderSession session(host());
    auto& cx=session.context();
    const auto jaxOptions=cx.object();
    cx.set(jaxOptions,opts::Key(u"fontCache"),opts::Value(u"none"));
    (void)session.construct(opts::Value(jaxOptions));
    auto& services=session.services();
    (void)tex::BussproofsConfiguration::initializeModule(services.parser(),services.modules());
    const auto inputOptions=cx.object();
    const auto packages=cx.array(2);
    cx.set(opts::Value(packages),opts::Key(u"0"),opts::Value(u"base"));
    cx.set(opts::Value(packages),opts::Key(u"1"),opts::Value(u"bussproofs"));
    cx.set(inputOptions,opts::Key(u"packages"),opts::Value(packages));
    auto document=svg::createNativeSvgDocument(session,opts::Undefined{},opts::Value(inputOptions));

    const auto docOptions=cx.get(opts::Value(document->sourceIdentity()),opts::Key(u"options"));
    const auto originalItemValue=cx.get(docOptions,opts::Key(u"MathItem"));
    const auto* originalItem=std::get_if<opts::ObjectRef>(&originalItemValue.data);
    expect(originalItem!=nullptr);
    if(!originalItem)return;
    auto itemIds=std::make_shared<std::vector<opts::ObjectRef>>();
    auto itemMathArguments=std::make_shared<std::vector<opts::Value>>();
    const auto captureItem=cx.nativeConstructor(u"CaptureMathItem",
        [original=*originalItem,itemIds,itemMathArguments](opts::Context& context,std::span<const opts::Value> args){
            const auto identity=context.construct(original,args);
            itemIds->push_back(identity);
            itemMathArguments->push_back(args.empty()?opts::Value(opts::Undefined{}):args.front());
            return identity;
        });
    cx.set(docOptions,opts::Key(u"MathItem"),opts::Value(captureItem));

    const auto inputList=std::get<opts::ObjectRef>(cx.get(opts::Value(document->sourceIdentity()),opts::Key(u"inputJax")).data);
    const auto originalReduceValue=cx.get(inputList,opts::Key(u"reduce"));
    const auto* originalReduce=std::get_if<opts::ObjectRef>(&originalReduceValue.data);
    expect(originalReduce!=nullptr);
    if(!originalReduce)return;
    auto reducerIds=std::make_shared<std::vector<opts::ObjectRef>>();
    auto savePublicReducer=std::make_shared<bool>(true);
    const auto captureReduce=cx.function(u"CaptureInputJaxReduce",
        [original=*originalReduce,reducerIds,savePublicReducer,inputList](opts::Context& context,opts::Value receiver,
            std::span<const opts::Value> args){
            if(!args.empty())if(const auto* reducer=std::get_if<opts::ObjectRef>(&args.front().data)){
                reducerIds->push_back(*reducer);
                if(*savePublicReducer)context.set(inputList,opts::Key(u"savedReducer"),opts::Value(*reducer));
            }
            return context.call(original,receiver,args);
        });
    cx.set(inputList,opts::Key(u"reduce"),opts::Value(captureReduce));

    const auto settings=cx.object();
    cx.set(settings,opts::Key(u"display"),opts::Value(false));
    cx.set(settings,opts::Key(u"em"),opts::Value(16.0));
    cx.set(settings,opts::Key(u"ex"),opts::Value(8.0));
    cx.set(settings,opts::Key(u"containerWidth"),opts::Value(1280.0));

    const auto proof=opts::Value(u"\\begin{prooftree}\\AxiomC{A}\\UnaryInfC{B}\\end{prooftree}");
    (void)document->convert(proof,opts::Value(settings));
    expect(reducerIds->size()==1u);
    expect(itemIds->size()==2u);
    expect(itemMathArguments->size()==2u);
    if(reducerIds->size()!=1u||itemIds->size()!=2u||itemMathArguments->size()!=2u)return;
    expect((*itemMathArguments)[0]==proof);
    expect((*itemMathArguments)[1]==opts::Value(u""));
    const auto publicReducer=reducerIds->front();
    const auto publicMainItem=(*itemIds)[0];
    const auto publicScratchItem=(*itemIds)[1];
    expect(isLive(cx,publicReducer));
    const auto savedReducer=cx.get(inputList,opts::Key(u"savedReducer"));
    expect(savedReducer==opts::Value(publicReducer));
    const auto firstInput=cx.get(inputList,opts::Key(u"0"));
    const std::array<opts::Value,2> reducerArguments{opts::Value(opts::Null{}),firstInput};
    expect(cx.call(publicReducer,opts::Value(opts::Null{}),reducerArguments)==firstInput);
    document->retireCompletedRequestItem();
    expect(isLive(cx,publicMainItem));
    expect(isLive(cx,publicScratchItem));
    expect(isLive(cx,publicReducer));

    // Drop the Source-visible retained callback before starting private work;
    // the private conversion must not inherit an earlier root.
    *savePublicReducer=false;
    cx.erase(inputList,opts::Key(u"savedReducer"));
    expect(!cx.hasOwn(inputList,opts::Key(u"savedReducer")));
    document->beginRequestOwnerScope();
    (void)document->convert(proof,opts::Value(settings));
    expect(reducerIds->size()==2u);
    expect(itemIds->size()==4u);
    expect(itemMathArguments->size()==4u);
    if(reducerIds->size()!=2u||itemIds->size()!=4u||itemMathArguments->size()!=4u)return;
    expect((*itemMathArguments)[2]==proof);
    expect((*itemMathArguments)[3]==opts::Value(u""));
    const auto privateReducer=reducerIds->back();
    const auto privateMainItem=(*itemIds)[2];
    const auto privateScratchItem=(*itemIds)[3];
    expect(isLive(cx,privateReducer));
    document->retireCompletedRequestItem();
    expect(!isLive(cx,privateReducer));
    expect(!isLive(cx,privateMainItem));
    expect(!isLive(cx,privateScratchItem));
    expect(isLive(cx,publicMainItem));
    expect(isLive(cx,publicScratchItem));
    cx.erase(inputList,opts::Key(u"savedReducer"));
    // The public call's retained reducer stays valid after private cleanup.
    expect(isLive(cx,publicReducer));
};
};
}
