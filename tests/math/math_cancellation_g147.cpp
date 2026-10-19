#include "support/folia_test.hpp"
import folia.math;
import folia.math.cancellation;
import folia.math.output.common;
import folia.math.output.svg.native_document;

namespace {
using namespace boost::ut;
using folia::math::RenderError;
using folia::math::Renderer;
using folia::math::RenderOptions;
namespace cancellation=folia::math::cancellation;
namespace common=folia::math::output::common;
namespace svg=folia::math::output::svg;
namespace opts=folia::math::options;
namespace lite=folia::math::adaptors::lite;

std::string errorKind(auto&& action) {
    try { action(); }
    catch(const RenderError& error) { return error.name; }
    throw std::runtime_error("Expected RenderError");
}
bool cancelled(auto&& action) {
    try { action(); }
    catch(const cancellation::Cancelled&) { return true; }
    return false;
}
common::FontLoadHost fontHost() {
    common::FontLoadHost host;
    host.asyncIsSynchronous=[] { return true; };
    host.warn=[](const std::any&) {};
    return host;
}

suite math_cancellation_g147 = [] {
    "brace_initialized_existing_options_calls_remain_unambiguous"_test = [] {
        Renderer renderer;
        const auto rendered=renderer.render(u"x",{});
        expect(!rendered.svg.empty());
        expect(!renderer.toSvg(u"x",{}).empty());
        expect(!renderer.render(std::string_view("x"),{}).svg.empty());
        expect(!renderer.toSvg(std::string_view("x"),{}).empty());
        expect(!renderer.renderScene(u"x",{}).roots().empty());
        expect(!renderer.renderScene(std::string_view("x"),{}).roots().empty());
    };
    "pre_cancel_and_expired_deadline_leave_session_reusable"_test = [] {
        Renderer renderer;
        std::stop_source source; source.request_stop();
        expect(eq(errorKind([&]{(void)renderer.render(std::string_view("x"),source.get_token(),{});}),std::string("Cancelled")));
        RenderOptions expired; expired.deadline=std::chrono::steady_clock::now()-std::chrono::milliseconds(1);
        expect(eq(errorKind([&]{(void)renderer.render(std::string_view("x"),{},expired);}),std::string("Cancelled")));
        expect(eq(errorKind([&]{(void)renderer.renderScene(std::u16string_view(u"x"),source.get_token(),{});}),std::string("Cancelled")));
        expect(!renderer.render(std::string_view("x")).svg.empty());
    };
    "public_renderer_observes_deadline_during_large_input"_test = [] {
        Renderer renderer;
        std::u16string tex(1'000'000,u'x');
        RenderOptions options;
        options.deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(5);
        expect(eq(errorKind([&]{(void)renderer.render(std::u16string_view(tex),{},options);}),std::string("Cancelled")));
        expect(eq(errorKind([&]{(void)renderer.render(std::string_view("x"));}),std::string("SessionUnusable")));
    };
    "actual_source_text_content_getter_can_request_native_session_cancellation"_test = [] {
        svg::NativeRenderSession session(fontHost(),{
            svg::NewcmFontExtension::bboldx,
            svg::NewcmFontExtension::bbm,
            svg::NewcmFontExtension::dsfont});
        auto document=svg::createNativeSvgDocument(session);
        auto jax=session.outputJax();
        auto adaptor=session.adaptor();
        expect(jax!=nullptr&&adaptor!=nullptr);
        if(!jax||!adaptor)return;

        auto original=adaptor->exchangeSourceTextContent({});
        expect(static_cast<bool>(original));
        if(!original)return;
        std::stop_source source;
        cancellation::Request request(source.get_token());
        cancellation::Scope scope(request);
        bool getterCalled=false;
        adaptor->exchangeSourceTextContent(
            [&,sourceGetter=original](lite::LiteNodeRef node) {
                auto value=sourceGetter(node);
                if(!getterCalled){getterCalled=true;source.request_stop();}
                return value;
            });
        struct RestoreGetter {
            std::shared_ptr<lite::LiteAdaptor> adaptor;
            std::function<folia::math::Property(lite::LiteNodeRef)> previous;
            ~RestoreGetter(){(void)adaptor->exchangeSourceTextContent(std::move(previous));}
        } restore{adaptor,std::move(original)};

        auto& context=session.context();
        const auto settings=context.object();
        context.set(settings,opts::Key(u"em"),opts::Value(16));
        context.set(settings,opts::Key(u"ex"),opts::Value(8));
        context.set(settings,opts::Key(u"containerWidth"),opts::Value(1280));
        bool observed=false;
        try {
            (void)document->convert(opts::Value(u"\\text{漢字😀 é}"),opts::Value(settings));
        } catch(const cancellation::Cancelled&) { observed=true; }
        expect(getterCalled&&observed);
    };
    "nested_request_scopes_restore_prior_context_and_are_thread_local"_test = [] {
        std::stop_source outerSource;
        cancellation::Request activeOuter(outerSource.get_token());
        { cancellation::Scope outerScope(activeOuter);
          std::stop_source innerSource; innerSource.request_stop();
          cancellation::Request inner(innerSource.get_token());
          { cancellation::Scope innerScope(inner); expect(cancelled([]{cancellation::checkpoint();})); }
          expect(!cancelled([]{cancellation::checkpoint();}));
          bool otherThreadCancelled=false;
          std::jthread worker([&]{try{cancellation::checkpoint();}catch(const cancellation::Cancelled&){otherThreadCancelled=true;}}); worker.join();
          expect(!otherThreadCancelled);
          outerSource.request_stop(); expect(cancelled([]{cancellation::checkpoint();}));
        }
    };
    "successful_native_results_remain_complete_and_to_svg_matches_render"_test = [] {
        Renderer renderer;
        const auto result=renderer.render(std::u16string_view(u"\\frac{a}{b}"));
        expect(!result.svg.empty()); expect(!result.scene.roots().empty());
        expect(eq(renderer.toSvg(std::u16string_view(u"\\frac{a}{b}")),result.svg));
        expect(!renderer.renderScene(std::string_view("x+1")).roots().empty());
    };
};
}
