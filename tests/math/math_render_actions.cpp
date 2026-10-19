#include "support/folia_test.hpp"
import folia.math.core.math_render_actions;
import folia.math.util.option_types;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
namespace o = folia::math::options;
namespace property_host = folia::math::property_host;
using folia::math::Missing;
using folia::math::Property;
using folia::math::core::RenderActions;
using folia::math::core::MathItem;
using folia::math::core::MathItemSourceIdentity;

struct Document final : folia::math::core::MathDocumentService {
    o::Context& context;
    o::ObjectRef identity;
    explicit Document(o::Context& c) : context(c), identity(c.object()) {}
    folia::math::core::MathRenderActionsReference renderActionsReference() const override { return Missing{}; }
    folia::math::core::MathOutputJaxReference outputJaxReference() const override { return Missing{}; }
    o::Context& sourceContext() const override { return context; }
    o::ObjectRef sourceIdentity() const override { return identity; }
};
struct Item final : MathItem, MathItemSourceIdentity {
    o::Context& context;
    o::ObjectRef identity;
    explicit Item(o::Context& c) : context(c), identity(c.object()) {}
    o::Context& sourceContext() const override { return context; }
    o::ObjectRef sourceIdentity() const override { return identity; }
    bool isEscaped() const override { return false; }
    void render(folia::math::core::MathDocumentReference) override {}
    void rerender(folia::math::core::MathDocumentReference,Property=Missing{}) override {}
    void convert(folia::math::core::MathDocumentReference,Property=Missing{}) override {}
    void compile(folia::math::core::MathDocumentReference) override {}
    void typeset(folia::math::core::MathDocumentReference) override {}
    void updateDocument(folia::math::core::MathDocumentReference) override {}
    void removeFromDocument(Property=Missing{}) override {}
    void setMetrics(Property,Property,Property,Property) override {}
    Property state(Property=Missing{},Property=Missing{}) override { return Property(Missing{}); }
    void reset(Property=Missing{}) override {}
    void clear() override {}
};

void setIndex(o::Context& cx,o::ObjectRef object,std::uint32_t index,o::Value value) {
    const auto ascii=std::to_string(index);
    cx.set(object,o::Key(std::u16string(ascii.begin(),ascii.end())),std::move(value));
}
o::Value at(o::Context& cx, o::ObjectRef object, std::u16string_view key) {
    return cx.get(object, o::Key(std::u16string(key)));
}

suite render_actions_source=[] {
    "create-keeps-priority-order-and-finds-canonical-data"_test=[] {
        o::Context cx;
        auto actionsObject=cx.object();
        auto first=cx.array(1); setIndex(cx,first,0,4);
        auto second=cx.array(1); setIndex(cx,second,0,2);
        auto zero=cx.array(1); setIndex(cx,zero,0,0);
        cx.set(actionsObject,o::Key(u"first"),first);
        cx.set(actionsObject,o::Key(u"second"),second);
        cx.set(actionsObject,o::Key(u"zero"),zero);
        RenderActions render(cx,cx.object());
        render.create(actionsObject);
        Document doc(cx);std::vector<std::u16string> trace;
        auto firstMethod=cx.function(u"first",[&](auto&,auto,auto)->o::Value {trace.push_back(u"first");return false;});
        auto secondMethod=cx.function(u"second",[&](auto&,auto,auto)->o::Value {trace.push_back(u"second");return false;});
        cx.set(doc.identity,o::Key(u"first"),firstMethod);
        cx.set(doc.identity,o::Key(u"second"),secondMethod);
        render.renderDoc(doc);
        const auto firstData=render.findID(u"first");
        const auto secondData=render.findID(u"second");
        const auto skipped=render.findID(u"zero");
        expect(firstData!=o::Value(o::Null{}));
        expect(secondData!=o::Value(o::Null{}));
        expect(skipped==o::Value(o::Null{}));
        expect(at(cx,std::get<o::ObjectRef>(firstData.data),u"id")==o::Value(u"first"));
        expect(at(cx,std::get<o::ObjectRef>(secondData.data),u"id")==o::Value(u"second"));
        const auto expectedOrder=std::vector<std::u16string>{u"second",u"first"};
        expect(static_cast<bool>(trace==expectedOrder));
    };

    "method-actions-use-live-document-receiver-and-priority-filter"_test=[] {
        o::Context cx;
        auto actionsObject=cx.object();
        auto low=cx.array(1); setIndex(cx,low,0,2);
        auto high=cx.array(1); setIndex(cx,high,0,5);
        cx.set(actionsObject,o::Key(u"low"),low);
        cx.set(actionsObject,o::Key(u"high"),high);
        RenderActions render(cx,cx.object()); render.create(actionsObject);
        Document doc(cx); std::vector<std::u16string> trace;
        auto lowFn=cx.function(u"low",[&](auto&,auto self,auto)->o::Value {
            trace.push_back(self==o::Value(doc.identity)?u"low:same":u"low:other"); return false;
        });
        auto highFn=cx.function(u"high",[&](auto&,auto self,auto)->o::Value {
            trace.push_back(self==o::Value(doc.identity)?u"high:same":u"high:other"); return false;
        });
        cx.set(doc.identity,o::Key(u"low"),lowFn); cx.set(doc.identity,o::Key(u"high"),highFn);
        render.renderDoc(doc,Property(3));
        expect(static_cast<bool>(trace==std::vector<std::u16string>{u"high:same"}));
    };

    "direct-action-can-stop-current-prioritized-iteration"_test=[] {
        o::Context cx;
        auto actionsObject=cx.object(); std::vector<std::u16string> trace;
        auto stop=cx.function(u"stop",[&](auto&,auto,auto)->o::Value {trace.push_back(u"stop");return true;});
        auto next=cx.function(u"next",[&](auto&,auto,auto)->o::Value {trace.push_back(u"next");return false;});
        auto first=cx.array(2); setIndex(cx,first,0,1); setIndex(cx,first,1,stop);
        auto second=cx.array(2); setIndex(cx,second,0,2); setIndex(cx,second,1,next);
        cx.set(actionsObject,o::Key(u"first"),first);cx.set(actionsObject,o::Key(u"second"),second);
        RenderActions render(cx,cx.object());render.create(actionsObject);Document doc(cx);
        render.renderDoc(doc);
        expect(static_cast<bool>(trace==std::vector<std::u16string>{u"stop"}));
    };

    "method-action-propagates-the-source-callback-exception"_test=[] {
        o::Context cx;
        auto actionsObject=cx.object();auto definition=cx.array(2);
        setIndex(cx,definition,0,1);setIndex(cx,definition,1,u"fail");
        cx.set(actionsObject,o::Key(u"failure"),definition);
        RenderActions render(cx,cx.object());render.create(actionsObject);Document doc(cx);
        auto failure=cx.function(u"failure",[](auto&,auto,auto)->o::Value {
            throw o::Exception("TypeError",u"render-fault");
        });
        cx.set(doc.identity,o::Key(u"fail"),failure);
        std::string observed="no-error";
        try { render.renderDoc(doc); }
        catch(const o::Exception& error) {
            observed=error.name+":"+property_host::debugText(error.message);
        }
        expect(eq(observed,std::string("TypeError:render-fault")));
    };

    "math-action-preserves-data-item-document-identities-and-convert-gate"_test=[] {
        o::Context cx;auto actionsObject=cx.object();auto definition=cx.array(4);
        std::vector<o::Value> receiverAndArgs;
        auto docCallback=cx.function(u"doc",[](auto&,auto,auto)->o::Value{return false;});
        auto mathCallback=cx.function(u"math",[&](auto&,auto receiver,std::span<const o::Value> args)->o::Value {
            receiverAndArgs.clear();receiverAndArgs.push_back(receiver);
            receiverAndArgs.insert(receiverAndArgs.end(),args.begin(),args.end());return false;
        });
        setIndex(cx,definition,0,5);setIndex(cx,definition,1,docCallback);
        setIndex(cx,definition,2,mathCallback);setIndex(cx,definition,3,true);
        cx.set(actionsObject,o::Key(u"identity"),definition);
        RenderActions render(cx,cx.object());render.create(actionsObject);Document doc(cx);Item item(cx);
        const auto data=render.findID(u"identity");
        render.renderMath(item,&doc,Property(5));
        const auto expected=std::vector<o::Value>{data,o::Value(item.identity),o::Value(doc.identity)};
        expect(static_cast<bool>(receiverAndArgs==expected));
        receiverAndArgs.clear();render.renderConvert(item,&doc,Property(4));
        expect(receiverAndArgs.empty());
        render.renderConvert(item,&doc,Property(5));
        expect(static_cast<bool>(receiverAndArgs==expected));
    };

    "iteration-observes-later-items-added-by-a-live-callback"_test=[] {
        o::Context cx;auto actionsObject=cx.object();auto first=cx.array(1);auto second=cx.array(1);
        setIndex(cx,first,0,1);setIndex(cx,second,0,2);
        cx.set(actionsObject,o::Key(u"first"),first);cx.set(actionsObject,o::Key(u"second"),second);
        RenderActions render(cx,cx.object());render.create(actionsObject);Document doc(cx);
        std::vector<std::u16string> trace;
        auto addLater=cx.function(u"first",[&](auto&,auto,auto)->o::Value {
            trace.push_back(u"first");
            auto definition=cx.array(1);setIndex(cx,definition,0,3);
            auto [data,priority]=render.action(u"late",definition);
            render.add(data,cx.toNumber(priority));
            return false;
        });
        auto secondMethod=cx.function(u"second",[&](auto&,auto,auto)->o::Value {trace.push_back(u"second");return false;});
        auto lateMethod=cx.function(u"late",[&](auto&,auto,auto)->o::Value {trace.push_back(u"late");return false;});
        cx.set(doc.identity,o::Key(u"first"),addLater);
        cx.set(doc.identity,o::Key(u"second"),secondMethod);
        cx.set(doc.identity,o::Key(u"late"),lateMethod);
        render.renderDoc(doc);
        const auto expected=std::vector<std::u16string>{u"first",u"second",u"late"};
        expect(static_cast<bool>(trace==expected));
    };
};
}
