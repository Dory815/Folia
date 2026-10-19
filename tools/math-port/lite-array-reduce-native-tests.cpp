#include "../../tests/support/folia_test.hpp"
import folia.math.util.option_types;
import folia.math.adaptors.lite.source_text_content;

namespace {
using namespace boost::ut;
namespace options=folia::math::options;

suite arrayReduce=[] {
    "reduce_uses_live_indices_and_captures_length_once"_test=[] {
        options::Context context;
        const auto array=context.array();
        context.set(options::Value(array),options::Key(u"0"),options::Value(1));
        context.set(options::Value(array),options::Key(u"1"),options::Value(2));
        context.set(options::Value(array),options::Key(u"2"),options::Value(3));
        std::vector<double> indexes;
        const auto callback=context.function(u"callback",
            [&indexes,array](options::Context& cx,options::Value receiver,
                             std::span<const options::Value> args)->options::Value {
                expect(std::holds_alternative<options::Undefined>(receiver.data));
                expect(args.size()==4u);
                expect(args[3]==options::Value(array));
                const auto index=std::get<double>(args[2].data);
                indexes.push_back(index);
                if(index==0){
                    cx.erase(array,options::Key(u"1"));
                    cx.set(options::Value(array),options::Key(u"2"),options::Value(7));
                    cx.set(options::Value(array),options::Key(u"3"),options::Value(11));
                }
                return options::Value(std::get<double>(args[0].data)+std::get<double>(args[1].data));
            });
        const auto method=context.get(options::Value(array),options::Key(u"reduce"));
        const std::array<options::Value,2> args{options::Value(callback),options::Value(0)};
        const auto result=context.callMember(method,options::Value(array),args);
        expect(result==options::Value(8));
        expect(indexes==std::vector<double>{0,2});
        expect(context.arrayLength(array)==4u);
    };

    "reduce_uses_inherited_indices_as_initial_accumulator"_test=[] {
        options::Context context;
        const auto array=context.array(2);
        const auto prototype=context.objectWithPrototype(context.intrinsics().arrayPrototype);
        context.define(prototype,options::Key(u"0"),
            options::Descriptor::data(options::Value(4)));
        context.setPrototype(array,prototype);
        context.set(options::Value(array),options::Key(u"1"),options::Value(3));
        std::vector<double> indexes;
        const auto callback=context.function(u"callback",
            [&indexes](options::Context&,options::Value,std::span<const options::Value> args){
                indexes.push_back(std::get<double>(args[2].data));
                return options::Value(std::get<double>(args[0].data)+std::get<double>(args[1].data));
            });
        const auto method=context.get(options::Value(array),options::Key(u"reduce"));
        const std::array<options::Value,1> args{options::Value(callback)};
        expect(context.callMember(method,options::Value(array),args)==options::Value(7));
        expect(indexes==std::vector<double>{1});
    };

    "reduce_reads_length_before_rejecting_non_callable_callback"_test=[] {
        options::Context context;
        std::vector<std::u16string> trace;
        const auto receiver=context.object();
        const auto getter=context.function(u"get length",
            [&trace](options::Context&,options::Value,std::span<const options::Value>){
                trace.push_back(u"length");return options::Value(1);
            });
        context.define(receiver,options::Key(u"length"),
            options::Descriptor::access(getter,std::nullopt,true,true));
        const auto method=context.get(options::Value(context.intrinsics().arrayPrototype),options::Key(u"reduce"));
        bool caught=false;
        try{
            const std::array<options::Value,2> args{options::Value(3),options::Value(0)};
            (void)context.callMember(method,options::Value(receiver),args);
        }catch(const options::Exception& error){caught=error.name=="TypeError";}
        expect(caught);
        expect(trace==std::vector<std::u16string>{u"length"});
    };

    "text_content_reads_children_and_reduce_dynamically"_test=[] {
        options::Context context;
        const auto adaptor=context.object();
        const auto text=context.object();
        const auto missingReduce=[&]{
            try{(void)folia::math::adaptors::lite::sourceTextContent(
                context,options::Value(adaptor),options::Value(text));}
            catch(const options::Exception& error){
                return error.name=="TypeError"&&
                    error.message==u"Cannot read properties of undefined (reading 'reduce')";
            }
            return false;
        };
        expect(missingReduce());

        const auto children=context.array();
        const auto child=context.object();
        context.set(options::Value(child),options::Key(u"kind"),options::Value(u"#text"));
        context.set(options::Value(child),options::Key(u"value"),options::Value(u"dynamic"));
        context.arrayPush(children,std::array<options::Value,1>{options::Value(child)});
        context.set(options::Value(text),options::Key(u"children"),options::Value(children));
        const auto result=folia::math::adaptors::lite::sourceTextContent(
            context,options::Value(adaptor),options::Value(text));
        expect(result==options::Value(u"dynamic"));

        const auto replacement=context.function(u"reduce",
            [children](options::Context&,options::Value receiver,std::span<const options::Value> args){
                expect(receiver==options::Value(children));
                expect(args.size()==2u);
                return options::Value(u"replacement reduce");
            });
        context.set(options::Value(children),options::Key(u"reduce"),options::Value(replacement));
        const auto replaced=folia::math::adaptors::lite::sourceTextContent(
            context,options::Value(adaptor),options::Value(text));
        expect(replaced==options::Value(u"replacement reduce"));
    };

    "text_content_recurses_through_the_live_receiver_method"_test=[] {
        options::Context context;
        const auto adaptor=context.object();
        const auto nestedMethod=context.function(u"textContent",
            [adaptor](options::Context&,options::Value receiver,std::span<const options::Value> args){
                expect(receiver==options::Value(adaptor));
                expect(args.size()==1u);
                return options::Value(u"nested");
            });
        context.set(options::Value(adaptor),options::Key(u"textContent"),options::Value(nestedMethod));
        const auto nested=context.object();
        context.set(options::Value(nested),options::Key(u"kind"),options::Value(u"element"));
        const auto children=context.array();
        context.arrayPush(children,std::array<options::Value,1>{options::Value(nested)});
        const auto node=context.object();
        context.set(options::Value(node),options::Key(u"children"),options::Value(children));
        const auto result=folia::math::adaptors::lite::sourceTextContent(
            context,options::Value(adaptor),options::Value(node));
        expect(result==options::Value(u"nested"));
    };

    "text_content_reads_custom_reduce_and_child_properties_in_source_order"_test=[] {
        options::Context context;
        const auto adaptor=context.object();
        const auto node=context.object();
        const auto child=context.object();
        std::vector<std::u16string> trace;
        const auto kindGetter=context.function(u"get kind",
            [&trace](options::Context&,options::Value,std::span<const options::Value>){
                trace.push_back(u"kind");return options::Value(u"#text");
            });
        const auto valueGetter=context.function(u"get value",
            [&trace](options::Context&,options::Value,std::span<const options::Value>){
                trace.push_back(u"value");return options::Value(u"x");
            });
        context.define(child,options::Key(u"kind"),
            options::Descriptor::access(kindGetter,std::nullopt,true,true));
        context.define(child,options::Key(u"value"),
            options::Descriptor::access(valueGetter,std::nullopt,true,true));
        const auto children=context.array();
        context.arrayPush(children,std::array<options::Value,1>{options::Value(child)});
        const auto reduce=context.get(options::Value(context.intrinsics().arrayPrototype),options::Key(u"reduce"));
        const auto tracedReduce=context.function(u"reduce",
            [&trace,reduce,children](options::Context& cx,options::Value receiver,
                                    std::span<const options::Value> args){
                expect(receiver==options::Value(children));
                expect(args.size()==2u);
                trace.push_back(u"reduce-call");
                const auto* function=std::get_if<options::ObjectRef>(&reduce.data);
                expect(function!=nullptr);
                const auto callback=args[0];
                const auto wrapped=cx.function(u"wrapped callback",
                    [&trace,callback](options::Context& inner,options::Value thisValue,
                                      std::span<const options::Value> callbackArgs){
                        expect(callbackArgs.size()==4u);
                        trace.push_back(u"callback");
                        const auto* original=std::get_if<options::ObjectRef>(&callback.data);
                        expect(original!=nullptr);
                        return inner.call(*original,thisValue,callbackArgs);
                    });
                const std::array<options::Value,2> forwarded{options::Value(wrapped),args[1]};
                return cx.call(*function,receiver,forwarded);
            });
        context.define(children,options::Key(u"reduce"),
            options::Descriptor::data(options::Value(tracedReduce)));
        context.set(options::Value(node),options::Key(u"children"),options::Value(children));
        expect(folia::math::adaptors::lite::sourceTextContent(
            context,options::Value(adaptor),options::Value(node))==options::Value(u"x"));
        expect(trace==std::vector<std::u16string>{u"reduce-call",u"callback",u"kind",u"value"});
    };

    "text_receiver_can_use_a_dynamically_assigned_children_array"_test=[] {
        options::Context context;
        const auto adaptor=context.object();
        const auto text=context.object();
        const auto children=context.array();
        const auto child=context.object();
        context.set(options::Value(child),options::Key(u"kind"),options::Value(u"#text"));
        context.set(options::Value(child),options::Key(u"value"),options::Value(u"assigned"));
        context.arrayPush(children,std::array<options::Value,1>{options::Value(child)});
        context.set(options::Value(text),options::Key(u"children"),options::Value(children));
        expect(folia::math::adaptors::lite::sourceTextContent(
            context,options::Value(adaptor),options::Value(text))==options::Value(u"assigned"));
    };

    "text_content_plus_uses_default_hint_and_left_to_right_primitive_order"_test=[] {
        options::Context context;
        const auto adaptor=context.object();
        const auto node=context.object();
        const auto child=context.object();
        context.set(options::Value(child),options::Key(u"kind"),options::Value(u"#text"));
        std::vector<std::u16string> trace;
        const auto left=context.object();
        const auto leftValueOf=context.function(u"valueOf",[&trace](options::Context&,options::Value,
            std::span<const options::Value> args){expect(args.empty());trace.push_back(u"left");return options::Value(2);});
        context.set(options::Value(left),options::Key(u"valueOf"),options::Value(leftValueOf));
        const auto right=context.object();
        const auto rightValueOf=context.function(u"valueOf",[&trace](options::Context&,options::Value,
            std::span<const options::Value> args){expect(args.empty());trace.push_back(u"right");return options::Value(3);});
        context.set(options::Value(right),options::Key(u"valueOf"),options::Value(rightValueOf));
        context.set(options::Value(child),options::Key(u"value"),options::Value(right));
        const auto children=context.array();
        context.arrayPush(children,std::array<options::Value,1>{options::Value(child)});
        const auto reduce=context.function(u"reduce",[left,children](options::Context& cx,
            options::Value receiver,std::span<const options::Value> args){
            expect(receiver==options::Value(children));expect(args.size()==2u);
            const auto* callback=std::get_if<options::ObjectRef>(&args[0].data);expect(callback!=nullptr);
            const std::array<options::Value,4> callbackArgs{options::Value(left),
                cx.get(options::Value(children),options::Key(u"0")),options::Value(0),options::Value(children)};
            return cx.call(*callback,options::Undefined{},callbackArgs);
        });
        context.set(options::Value(children),options::Key(u"reduce"),options::Value(reduce));
        context.set(options::Value(node),options::Key(u"children"),options::Value(children));
        expect(folia::math::adaptors::lite::sourceTextContent(context,options::Value(adaptor),
            options::Value(node))==options::Value(5));
        expect(trace==std::vector<std::u16string>{u"left",u"right"});
    };

    "context_to_primitive_uses_symbol_method_and_rejects_object_result"_test=[] {
        options::Context context;
        std::u16string hint;
        const auto object=context.object();
        const auto method=context.function(u"[Symbol.toPrimitive]",[&hint](options::Context&,
            options::Value,std::span<const options::Value> args){
            expect(args.size()==1u);hint=std::get<std::u16string>(args[0].data);
            return options::Value(u"font");
        });
        context.set(options::Value(object),options::Key(context.intrinsics().symbolToPrimitive),
            options::Value(method));
        expect(context.toPrimitive(options::Value(object))==options::Value(u"font"));
        expect(hint==u"default");

        const auto bad=context.object();
        const auto returnsObject=context.function(u"[Symbol.toPrimitive]",[bad](options::Context&,
            options::Value,std::span<const options::Value>){return options::Value(bad);});
        context.set(options::Value(bad),options::Key(context.intrinsics().symbolToPrimitive),
            options::Value(returnsObject));
        bool rejected=false;
        try{(void)context.toPrimitive(options::Value(bad));}
        catch(const options::Exception& error){rejected=error.name=="TypeError"&&
            error.message==u"Cannot convert object to primitive value";}
        expect(rejected);
    };

    "context_to_primitive_ordinary_methods_follow_the_hint_order"_test=[] {
        options::Context context;
        const auto object=context.object();
        std::vector<std::u16string> trace;
        const auto toString=context.function(u"toString",[&trace,object](options::Context&,
            options::Value receiver,std::span<const options::Value> args){
            expect(receiver==options::Value(object));expect(args.empty());trace.push_back(u"toString");
            return options::Value(object);
        });
        const auto valueOf=context.function(u"valueOf",[&trace](options::Context&,
            options::Value,std::span<const options::Value> args){
            expect(args.empty());trace.push_back(u"valueOf");return options::Value(7);
        });
        context.set(options::Value(object),options::Key(u"toString"),options::Value(toString));
        context.set(options::Value(object),options::Key(u"valueOf"),options::Value(valueOf));
        expect(context.toPrimitive(options::Value(object),options::PrimitiveHint::stringHint)==options::Value(7));
        expect(trace==std::vector<std::u16string>{u"toString",u"valueOf"});
    };
};
}
