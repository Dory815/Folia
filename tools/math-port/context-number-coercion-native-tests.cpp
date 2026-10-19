#include "../../tests/support/folia_test.hpp"
import folia.math.util.option_types;

namespace {
using namespace boost::ut;
namespace opts=folia::math::options;

suite contextNumberConversion=[] {
    "context_number_uses_exotic_number_hint_and_actual_receiver"_test=[] {
        opts::Context context;
        std::vector<std::string> events;
        const auto object=context.object();
        const auto exotic=context.function(u"",[&](opts::Context&,opts::Value receiver,
            std::span<const opts::Value> args)->opts::Value {
            expect(receiver==opts::Value(object));
            expect(args.size()==1_u);
            expect(args[0]==opts::Value(u"number"));
            events.push_back("exotic number true");return opts::Value(u"12");
        });
        const auto getter=context.function(u"",[&](opts::Context&,opts::Value receiver,
            std::span<const opts::Value>)->opts::Value {
            expect(receiver==opts::Value(object));
            events.push_back("get exotic");return opts::Value(exotic);
        });
        context.define(object,opts::Key(context.intrinsics().symbolToPrimitive),
            opts::Descriptor::access(getter,std::nullopt));
        const auto unused=context.function(u"",[](opts::Context&,opts::Value,
            std::span<const opts::Value>)->opts::Value {
            throw opts::Exception("Error",u"must not get valueOf");
        });
        context.define(object,opts::Key(u"valueOf"),opts::Descriptor::access(unused,std::nullopt));
        expect(context.toNumber(opts::Value(object))==12.0);
        expect(events==std::vector<std::string>{"get exotic","exotic number true"});
    };

    "context_number_observes_live_ordinary_conversion_order"_test=[] {
        opts::Context context;
        std::vector<std::string> events;
        const auto object=context.object();
        const auto valueOf=context.function(u"",[&](opts::Context&,opts::Value receiver,
            std::span<const opts::Value>)->opts::Value {
            expect(receiver==opts::Value(object));
            events.push_back("valueOf true");return opts::Value(object);
        });
        const auto toString=context.function(u"",[&](opts::Context&,opts::Value receiver,
            std::span<const opts::Value>)->opts::Value {
            expect(receiver==opts::Value(object));
            events.push_back("toString true");return opts::Value(u"17");
        });
        const auto valueGetter=context.function(u"",[&](opts::Context&,opts::Value,
            std::span<const opts::Value>)->opts::Value {
            events.push_back("get valueOf");return opts::Value(valueOf);
        });
        const auto textGetter=context.function(u"",[&](opts::Context&,opts::Value,
            std::span<const opts::Value>)->opts::Value {
            events.push_back("get toString");return opts::Value(toString);
        });
        context.define(object,opts::Key(u"valueOf"),opts::Descriptor::access(valueGetter,std::nullopt));
        context.define(object,opts::Key(u"toString"),opts::Descriptor::access(textGetter,std::nullopt));
        expect(context.toNumber(opts::Value(object))==17.0);
        expect(events==std::vector<std::string>{"get valueOf","valueOf true","get toString","toString true"});
    };

    "context_number_preserves_exotic_abrupt_completion"_test=[] {
        opts::Context context;
        const auto object=context.object();
        std::vector<std::string> events;
        const auto exotic=context.function(u"",[&](opts::Context&,opts::Value,
            std::span<const opts::Value> args)->opts::Value {
            expect(args[0]==opts::Value(u"number"));
            events.push_back("exotic number");throw opts::Exception("RangeError",u"number-probe");
        });
        context.set(object,opts::Key(context.intrinsics().symbolToPrimitive),opts::Value(exotic));
        bool caught=false;
        try {(void)context.toNumber(opts::Value(object));}
        catch(const opts::Exception& error) {
            caught=true;expect(error.name=="RangeError");
            expect(bool(error.message==u"number-probe"));
        }
        expect(caught);expect(events==std::vector<std::string>{"exotic number"});
    };

    "context_number_rejects_symbol_exotic_result"_test=[] {
        opts::Context context;
        const auto object=context.object();
        const auto symbol=context.symbol(u"probe");
        const auto exotic=context.function(u"",[symbol](opts::Context&,opts::Value,
            std::span<const opts::Value>)->opts::Value {return opts::Value(symbol);});
        context.set(object,opts::Key(context.intrinsics().symbolToPrimitive),opts::Value(exotic));
        bool caught=false;
        try {(void)context.toNumber(opts::Value(object));}
        catch(const opts::Exception& error) {
            caught=true;expect(error.name=="TypeError");
            expect(bool(error.message==u"Cannot convert a Symbol value to a number"));
        }
        expect(caught);
    };

    "context_number_skips_noncallable_valueOf_and_preserves_negative_zero"_test=[] {
        opts::Context context;
        const auto object=context.object();
        context.set(object,opts::Key(u"valueOf"),opts::Value(4));
        std::size_t calls=0;
        const auto toString=context.function(u"",[&](opts::Context&,opts::Value,
            std::span<const opts::Value>)->opts::Value {++calls;return opts::Value(u"-0");});
        context.set(object,opts::Key(u"toString"),opts::Value(toString));
        const auto number=context.toNumber(opts::Value(object));
        expect(number==0.0);expect(std::signbit(number));expect(calls==1_u);
    };

    "context_number_rejects_object_exotic_result_with_source_error"_test=[] {
        opts::Context context;
        const auto object=context.object(),result=context.object();
        const auto exotic=context.function(u"",[result](opts::Context&,opts::Value,
            std::span<const opts::Value>)->opts::Value {return opts::Value(result);});
        context.set(object,opts::Key(context.intrinsics().symbolToPrimitive),opts::Value(exotic));
        bool caught=false;
        try {(void)context.toNumber(opts::Value(object));}
        catch(const opts::Exception& error) {
            caught=true;expect(error.name=="TypeError");
            expect(bool(error.message==u"Cannot convert object to primitive value"));
        }
        expect(caught);
    };
};
}
