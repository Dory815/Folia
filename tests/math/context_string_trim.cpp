#include "support/folia_test.hpp"
import folia.math.util.option_types;

namespace {
using namespace boost::ut;
namespace opts=folia::math::options;
suite stringTrim=[] {
    "context_string_trim_preserves_utf16_and_ecmascript_whitespace"_test=[] {
        opts::Context context;
        const auto method=std::get<opts::ObjectRef>(context.get(opts::Value(u""),opts::Key(u"trim")).data);
        const std::u16string whitespace=u"\t\n\v\f\r \u00a0\u1680\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u2028\u2029\u202f\u205f\u3000\ufeff";
        const std::u16string content=u"\u200b\u0085\u180e\U0001f642x y\u200b";
        expect(context.call(method,opts::Value(whitespace+content+whitespace))==opts::Value(content));
        expect(context.call(method,opts::Value(whitespace))==opts::Value(u""));
        expect(context.call(method,opts::Value(0.0))==opts::Value(u"0"));
        expect(context.call(method,opts::Value(true))==opts::Value(u"true"));
    };
    "context_string_trim_uses_string_conversion_and_live_override"_test=[] {
        opts::Context context;
        const auto method=std::get<opts::ObjectRef>(context.get(opts::Value(u""),opts::Key(u"trim")).data);
        const auto object=context.object();
        std::string trace;
        const auto toString=context.function(u"toString",[&](opts::Context&,opts::Value receiver,std::span<const opts::Value>){
            expect(receiver==opts::Value(object));trace+='s';return opts::Value(u" x ");
        });
        const auto valueOf=context.function(u"valueOf",[&](opts::Context&,opts::Value,std::span<const opts::Value>){trace+='v';return opts::Value(3.0);});
        context.set(object,opts::Key(u"toString"),opts::Value(toString));
        context.set(object,opts::Key(u"valueOf"),opts::Value(valueOf));
        expect(context.call(method,opts::Value(object))==opts::Value(u"x"));
        expect(trace=="s");
        for(const auto receiver:{opts::Value(opts::Undefined{}),opts::Value(opts::Null{}),opts::Value(context.symbol(u"x"))}){
            bool threw=false;try{(void)context.call(method,receiver);}catch(const opts::Exception& error){threw=error.name=="TypeError";}
            expect(threw);
        }
        const auto descriptor=context.descriptor(opts::Value(context.intrinsics().stringPrototype),opts::Key(u"trim"));
        expect(descriptor.has_value()&&descriptor->writable&&!descriptor->enumerable&&descriptor->configurable);
        expect(context.get(opts::Value(method),opts::Key(u"name"))==opts::Value(u"trim"));
        expect(context.get(opts::Value(method),opts::Key(u"length"))==opts::Value(0.0));
        const auto replacement=context.function(u"replacement",[](opts::Context&,opts::Value,std::span<const opts::Value>){return opts::Value(u"changed");});
        context.set(context.intrinsics().stringPrototype,opts::Key(u"trim"),opts::Value(replacement));
        expect(context.get(opts::Value(u" x "),opts::Key(u"trim"))==opts::Value(replacement));
    };
};
}
