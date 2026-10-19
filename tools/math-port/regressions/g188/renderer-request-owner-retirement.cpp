import folia.math;
import folia.math.adaptors.lite.lite_module;
import std;

namespace {
using namespace folia::math;
void collect(const output::scene::FrozenNodeRef& node,std::vector<std::uint32_t>& out){
    if(!node)return;
    if(const auto* glyph=std::get_if<output::scene::GlyphPrimitive>(&node->semantic))out.push_back(glyph->codepoint);
    for(const auto& child:node->children)collect(child,out);
}
std::vector<std::uint32_t> glyphs(const RenderResult& result){
    std::vector<std::uint32_t> out;
    for(const auto& root:result.scene.roots())collect(root,out);
    return out;
}
bool contains(const std::vector<std::uint32_t>& values,std::uint32_t value){
    return std::find(values.begin(),values.end(),value)!=values.end();
}
using NamedLiteIdentitySnapshot=std::vector<std::tuple<std::uint64_t,int,std::string>>;
bool hasNamedLiteIdentity(const NamedLiteIdentitySnapshot& values,const std::tuple<std::uint64_t,int,std::string>& identity){
    return std::find(values.begin(),values.end(),identity)!=values.end();
}
NamedLiteIdentitySnapshot newLiteIdentities(
    const NamedLiteIdentitySnapshot& before,const NamedLiteIdentitySnapshot& after){
    std::vector<std::tuple<std::uint64_t,int,std::string>> result;
    for(const auto& identity:after)
        if(std::none_of(before.begin(),before.end(),[&](const auto& old){return std::get<0>(old)==std::get<0>(identity);}))
            result.push_back(identity);
    return result;
}
bool isMathError(const RenderResult& result){
    return result.svg.find("data-mjx-error=")!=std::string::npos||
        result.svg.find("data-mml-node=\"merror\"")!=std::string::npos;
}
void check(bool condition,std::string_view label,bool& ok){
    if(condition)return;
    std::cerr<<"FAIL "<<label<<'\n';ok=false;
}
void checkCount(std::size_t actual,std::size_t expected,std::string_view label,bool& ok){
    if(actual==expected)return;
    std::cerr<<"FAIL "<<label<<": actual="<<actual<<" expected="<<expected<<'\n';ok=false;
}
template<class F>decltype(auto) runPhase(std::string_view phase,F&& operation){
    try{return std::forward<F>(operation)();}
    catch(const RenderError& error){
        std::cerr<<"PHASE "<<phase<<" RenderError "<<error.name<<": "<<error.what()<<'\n';throw;
    }catch(const std::exception& error){
        std::cerr<<"PHASE "<<phase<<" exception: "<<error.what()<<'\n';throw;
    }catch(...){
        std::cerr<<"PHASE "<<phase<<" non-standard exception\n";throw;
    }
}
}

int main(){
  try{
    RenderResult retained;
    std::vector<std::uint32_t> retainedGlyphs;
    bool ok=true;
    {
        Renderer renderer;
        (void)runPhase("primary/warm-render",[&]{return renderer.render("x");});
        const auto warm=RendererDiagnosticsProbe::snapshot(renderer);
        const auto warmSceneOwners=RendererDiagnosticsProbe::sceneOwnerSnapshot(renderer);
        const auto warmLiteIdentities=RendererDiagnosticsProbe::liteNodeIdentityNamedSnapshot(renderer);
        retained=runPhase("primary/retained-result",[&]{return renderer.render("x+1");});
        retainedGlyphs=glyphs(retained);
        const auto macroDefinition=runPhase("primary/define-macro",[&]{return renderer.render(R"(\newcommand{\FoliaOwnerMacro}{x}\FoliaOwnerMacro)");});
        const auto label=runPhase("primary/define-tag-A-and-label",[&]{return renderer.render(R"(\begin{equation}\tag{A}\label{folia:owner:a}x\end{equation})");});
        check(!macroDefinition.svg.empty(),"persistent macro definition SVG",ok);
        check(!label.svg.empty(),"first tagged label SVG",ok);
        // TagA changes currentTag/history ownership. Match the first formula
        // that follows it before taking the stable-loop baseline.
        const auto normalized=runPhase("primary/normalize-after-tag-A",[&]{return renderer.render(R"(\begin{array}{cc}a&b\\c&d\end{array})");});
        check(!normalized.svg.empty(),"normalization array SVG",ok);
        const auto persistentState=RendererDiagnosticsProbe::snapshot(renderer);
        const auto persistentFamilies=RendererDiagnosticsProbe::parserIdentitySnapshot(renderer);
        const auto persistentSceneOwners=RendererDiagnosticsProbe::sceneOwnerSnapshot(renderer);
        checkCount(persistentFamilies[11],1,"one durable Label identity after tag A placeholder retirement",ok);
        for(std::size_t i=0;i<64;++i){
            const auto n=std::to_string(i);
            const auto matrix=runPhase(std::string("primary/iteration-")+n+"/array",[&]{return renderer.render(R"(\begin{array}{cc}a&b\\c&d\end{array})");});
            const auto ordinary=runPhase(std::string("primary/iteration-")+n+"/fraction",[&]{return renderer.render("x+\\frac{1}{2}");});
            const auto scene=runPhase(std::string("primary/iteration-")+n+"/renderScene",[&]{return renderer.renderScene(R"(\begin{matrix}x&y\\z&w\end{matrix})");});
            check(!matrix.svg.empty(),std::string("iteration ")+n+" array SVG",ok);
            check(!ordinary.svg.empty(),std::string("iteration ")+n+" fraction SVG",ok);
            check(!scene.roots().empty(),std::string("iteration ")+n+" Scene roots",ok);
        }
        const auto afterLoop=RendererDiagnosticsProbe::snapshot(renderer);
        const auto afterLoopFamilies=RendererDiagnosticsProbe::parserIdentitySnapshot(renderer);
        const auto afterLoopSceneOwners=RendererDiagnosticsProbe::sceneOwnerSnapshot(renderer);
        const auto afterLoopLiteIdentities=RendererDiagnosticsProbe::liteNodeIdentityNamedSnapshot(renderer);
        checkCount(afterLoop[2],persistentState[2],"MML products after 64 requests at the same persistent state",ok);
        checkCount(afterLoop[3],persistentState[3],"ParserRuntime registry after 64 requests at the same persistent state",ok);
        checkCount(afterLoop[4],persistentState[4],"wrapper handles after 64 requests",ok);
        checkCount(afterLoop[5],persistentState[5],"retained layout jobs after 64 requests",ok);
        checkCount(afterLoop[6],persistentState[6],"Scene handles after 64 requests",ok);
        checkCount(afterLoop[7],persistentState[7],"Lite records after 64 requests",ok);
        checkCount(afterLoop[8],persistentState[8],"Lite node identities before first malformed formula",ok);
        for(std::size_t i=0;i<afterLoopFamilies.size();++i)
            checkCount(afterLoopFamilies[i],persistentFamilies[i],std::string("ParserRuntime family ")+std::to_string(i)+" after 64 requests",ok);
        for(std::size_t i=0;i<afterLoopSceneOwners.size();++i)
            checkCount(afterLoopSceneOwners[i],persistentSceneOwners[i],std::string("Scene owner family ")+std::to_string(i)+" after 64 requests",ok);
        check(afterLoopLiteIdentities==warmLiteIdentities,"64 mixed requests preserve the existing Lite identity set",ok);

        const auto parseError=runPhase("primary/malformed-formula",[&]{return renderer.render(R"(\frac{)");});
        const auto afterFirstMalformed=RendererDiagnosticsProbe::snapshot(renderer);
        const auto firstMalformedLite=RendererDiagnosticsProbe::liteNodeIdentityNamedSnapshot(renderer);
        const auto newlyMaterializedLite=newLiteIdentities(afterLoopLiteIdentities,firstMalformedLite);
        check(isMathError(parseError),"malformed TeX returns an merror",ok);
        checkCount(afterFirstMalformed[7],persistentState[7],"Lite record count after malformed formula",ok);
        checkCount(afterFirstMalformed[8],persistentState[8]+1,"one pre-existing document-body identity is first materialized on the error path",ok);
        for(const auto& identity:afterLoopLiteIdentities)
            check(hasNamedLiteIdentity(firstMalformedLite,identity),"pre-existing Lite identity remains after malformed formula",ok);
        checkCount(newlyMaterializedLite.size(),1,"malformed path materializes exactly one pre-existing Lite identity",ok);
        if(newlyMaterializedLite.size()==1){
        check(std::get<1>(newlyMaterializedLite.front())==static_cast<int>(adaptors::lite::NodeKind::Element),
                  "newly materialized Lite identity is an Element",ok);
            check(std::get<2>(newlyMaterializedLite.front())=="body",
                  "newly materialized Lite identity is the document body",ok);
        }
        const auto secondParseError=runPhase("primary/repeated-malformed-formula",[&]{return renderer.render(R"(\frac{)");});
        const auto afterSecondMalformed=RendererDiagnosticsProbe::snapshot(renderer);
        const auto secondMalformedLite=RendererDiagnosticsProbe::liteNodeIdentityNamedSnapshot(renderer);
        check(isMathError(secondParseError),"repeated malformed TeX returns an merror",ok);
        check(secondMalformedLite==firstMalformedLite,"repeated error keeps the same canonical Lite identities",ok);
        for(std::size_t i=2;i<afterFirstMalformed.size();++i)
            checkCount(afterSecondMalformed[i],afterFirstMalformed[i],std::string("owner count ")+std::to_string(i)+" after repeated malformed formula",ok);
        const auto macroUse=runPhase("primary/use-persistent-macro",[&]{return renderer.render(R"(\FoliaOwnerMacro)");});
        const auto plainX=runPhase("primary/plain-x",[&]{return renderer.render("x");});
        const auto reference=runPhase("primary/tag-B-label-reference-A",[&]{return renderer.render(R"(\begin{equation}\tag{B}\label{folia:owner:b}\ref{folia:owner:a}\end{equation})");});
        const auto labelBNormalized=runPhase("primary/normalize-after-tag-B",[&]{return renderer.render(R"(\begin{array}{cc}a&b\\c&d\end{array})");});
        check(!labelBNormalized.svg.empty(),"normalization array SVG after tag B",ok);
        const auto after=RendererDiagnosticsProbe::snapshot(renderer);
        const auto finalFamilies=RendererDiagnosticsProbe::parserIdentitySnapshot(renderer);
        const auto afterSceneOwners=RendererDiagnosticsProbe::sceneOwnerSnapshot(renderer);
        const auto referenceGlyphs=glyphs(reference);
        // Context slot count is cumulative. Request-owned counts stay at the
        // normalized post-label-A and post-label-B state; replacement
        // placeholders retire, leaving one additional durable Label.
        checkCount(after[2],afterLoop[2],"MML factory products after persistent label B",ok);
        checkCount(after[3],afterLoop[3]+1,"ParserRuntime registry after adding persistent label B",ok);
        checkCount(after[4],afterLoop[4],"wrapper handles after persistent label B",ok);
        checkCount(after[5],afterLoop[5],"retained layout jobs after persistent label B",ok);
        checkCount(after[6],afterLoop[6],"Scene handles after persistent label B",ok);
        checkCount(after[7],afterLoop[7],"Lite records after persistent label B",ok);
        checkCount(after[8],afterFirstMalformed[8],"Lite identities after persistent label B",ok);
        checkCount(afterSceneOwners[0],afterLoopSceneOwners[0],"Scene handle map after persistent label B",ok);
        checkCount(afterSceneOwners[1],afterLoopSceneOwners[1],"Scene source identity map after persistent label B",ok);
        checkCount(afterSceneOwners[2],afterLoopSceneOwners[2],"Scene source view map after persistent label B",ok);
        for(std::size_t i=0;i<finalFamilies.size();++i){
            const auto expected=afterLoopFamilies[i]+(i==11?1:0);
            checkCount(finalFamilies[i],expected,std::string("ParserRuntime family ")+std::to_string(i)+" after adding label B",ok);
        }
        check(glyphs(macroUse)==glyphs(plainX),"defined macro remains usable after 64 iterations",ok);
        check(contains(referenceGlyphs,static_cast<std::uint32_t>('A')),"prior tag is referenced after 64 iterations",ok);
        check(contains(referenceGlyphs,static_cast<std::uint32_t>('B')),"current tag is present after 64 iterations",ok);
        const auto unresolvedOne=runPhase("primary/unresolved-reference-one",[&]{return renderer.render(R"(\ref{folia:owner:missing})");});
        const auto afterUnresolvedOne=RendererDiagnosticsProbe::parserIdentitySnapshot(renderer);
        const auto unresolvedTwo=runPhase("primary/unresolved-reference-two",[&]{return renderer.render(R"(\ref{folia:owner:missing})");});
        const auto afterUnresolvedTwo=RendererDiagnosticsProbe::parserIdentitySnapshot(renderer);
        check(!unresolvedOne.svg.empty()&&!unresolvedTwo.svg.empty(),"repeated unresolved references produce results",ok);
        for(std::size_t i=0;i<afterUnresolvedOne.size();++i){
            checkCount(afterUnresolvedOne[i],finalFamilies[i],std::string("ParserRuntime family ")+std::to_string(i)+" after unresolved reference",ok);
            checkCount(afterUnresolvedTwo[i],afterUnresolvedOne[i],std::string("ParserRuntime family ")+std::to_string(i)+" after repeated unresolved reference",ok);
        }
        const auto priorLabelUse=runPhase("primary/reuse-prior-label-after-unresolved-references",[&]{return renderer.render(R"(\ref{folia:owner:a})");});
        const auto afterPriorLabelUse=RendererDiagnosticsProbe::parserIdentitySnapshot(renderer);
        check(contains(glyphs(priorLabelUse),static_cast<std::uint32_t>('A')),"prior label remains readable after unresolved references",ok);
        check(afterPriorLabelUse==afterUnresolvedTwo,"prior label use does not grow ParserRuntime identities",ok);
    }
    check(!retained.svg.empty(),"retained SVG remains readable after Renderer destruction",ok);
    check(glyphs(retained)==retainedGlyphs,"retained FrozenScene remains readable after Renderer destruction",ok);
    {
        Renderer cancelled;
        (void)runPhase("cancelled/warm-render",[&]{return cancelled.render("x");});
        const auto before=RendererDiagnosticsProbe::snapshot(cancelled);
        const auto beforeSceneOwners=RendererDiagnosticsProbe::sceneOwnerSnapshot(cancelled);
        // Decode/construct the UTF-16 input before starting the stop request so
        // cancellation is observed inside the native session, not in UTF-8 conversion.
        std::u16string large;large.reserve(36000);
        for(std::size_t i=0;i<12000;++i)large+=u"x+";
        large+=u'x';
        std::stop_source source;
        std::jthread stopper([&]{std::this_thread::sleep_for(std::chrono::milliseconds(1));source.request_stop();});
        bool cancelledDuringRequest=false;
        try{(void)runPhase("cancelled/in-flight-UTF16-render",[&]{return cancelled.render(std::u16string_view(large),source.get_token(),{});});}
        catch(const RenderError& error){
            cancelledDuringRequest=error.name=="Cancelled"&&
                std::string_view(error.what()).find("session is unusable")!=std::string_view::npos;
            if(!cancelledDuringRequest)
                std::cerr<<"FAIL in-flight cancellation error: "<<error.name<<": "<<error.what()<<'\n';
        }
        stopper.join();
        const auto afterCancel=RendererDiagnosticsProbe::snapshot(cancelled);
        const auto afterCancelSceneOwners=RendererDiagnosticsProbe::sceneOwnerSnapshot(cancelled);
        check(cancelledDuringRequest,"stop request is observed inside native session",ok);
        checkCount(afterCancel[2],before[2],"MML products after in-flight cancellation",ok);
        checkCount(afterCancel[3],before[3],"ParserRuntime identity registry after in-flight cancellation",ok);
        checkCount(afterCancel[4],before[4],"wrapper handles after in-flight cancellation",ok);
        checkCount(afterCancel[5],before[5],"retained layout jobs after in-flight cancellation",ok);
        checkCount(afterCancel[6],before[6],"Scene handles after in-flight cancellation",ok);
        checkCount(afterCancel[7],before[7],"Lite records after in-flight cancellation",ok);
        checkCount(afterCancel[8],before[8],"Lite node identities after in-flight cancellation",ok);
        checkCount(afterCancelSceneOwners[0],beforeSceneOwners[0],"Scene handle map after in-flight cancellation",ok);
        checkCount(afterCancelSceneOwners[1],beforeSceneOwners[1],"Scene source identity map after in-flight cancellation",ok);
        checkCount(afterCancelSceneOwners[2],beforeSceneOwners[2],"Scene source view map after in-flight cancellation",ok);
    }
    if(!ok){
        std::cerr<<"FAIL renderer request owner retirement checks\n";
        return 1;
    }
    std::cout<<"request MML/Lite owners return to the warm count; macro and label/reference state persist; retained result remains usable after Renderer destruction\n";
    return 0;
  }catch(const RenderError& error){
    std::cerr<<"renderer request owner test failed with "<<error.name<<": "<<error.what()<<'\n';
    return 1;
  }catch(const std::exception& error){
    std::cerr<<"renderer request owner test failed with exception: "<<error.what()<<'\n';
    return 1;
  }catch(...){
    std::cerr<<"renderer request owner test failed with a non-standard exception\n";
    return 1;
  }
}


