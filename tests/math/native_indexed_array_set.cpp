#include "support/folia_test.hpp"
import folia.math.util.option_types;

namespace {
namespace opts=folia::math::options;
using namespace boost::ut;

struct IndexedArray final:opts::NativeIndexedView{
    std::vector<std::optional<opts::Descriptor>> slots;
    bool lengthWritable=true;
    std::uint32_t length()const override{return static_cast<std::uint32_t>(slots.size());}
    opts::Descriptor lengthDescriptor()const override{return opts::Descriptor::data(opts::Value(static_cast<double>(length())),lengthWritable,false,false);}
    std::optional<opts::Descriptor> indexDescriptor(std::uint32_t i)const override{return i<slots.size()?slots[i]:std::nullopt;}
    std::vector<std::uint32_t> ownIndexKeys()const override{
        std::vector<std::uint32_t> result;for(std::uint32_t i=0;i<slots.size();++i)if(slots[i])result.push_back(i);return result;
    }
    void defineIndex(std::uint32_t i,opts::Descriptor descriptor)override{
        if(i>=slots.size())slots.resize(static_cast<std::size_t>(i)+1);
        slots[i]=std::move(descriptor);
    }
    void defineLength(opts::Descriptor descriptor)override{
        const auto requested=static_cast<std::uint32_t>(std::get<double>(descriptor.value.data));
        slots.resize(requested);lengthWritable=descriptor.writable;
    }
    void deleteIndex(std::uint32_t i)override{if(i<slots.size())slots[i].reset();}
};

struct Fixture{
    opts::Context context;
    std::shared_ptr<IndexedArray> view=std::make_shared<IndexedArray>();
    opts::ObjectRef array=context.nativeArray(view);
    static opts::Key index(std::uint32_t i){const auto s=std::to_string(i);return opts::Key(std::u16string(s.begin(),s.end()));}
};

suite native_indexed_array_set_suite=[] {
    "native_indexed_set_and_define_update_the_provider_and_length"_test=[] {
        Fixture f;
        f.context.set(f.array,Fixture::index(2),u"set");
        expect(eq(f.view->length(),3u));
        expect(!f.view->slots[0]&&!f.view->slots[1]);
        expect(f.context.get(f.array,Fixture::index(2))==opts::Value(u"set"));
        f.context.define(f.array,Fixture::index(4),opts::Descriptor::data(u"define"));
        expect(eq(f.view->length(),5u));
        expect(f.context.get(f.array,Fixture::index(4))==opts::Value(u"define"));
    };
    "native_indexed_set_calls_inherited_setter_before_length_growth"_test=[] {
        Fixture f;auto prototype=f.context.object();f.context.setPrototype(f.array,prototype);
        opts::Value received;opts::Value assigned;
        const auto setter=f.context.function(u"set0",[&](opts::Context&,opts::Value receiver,std::span<const opts::Value> args){
            received=receiver;assigned=args.empty()?opts::Value(opts::Undefined{}):args[0];return opts::Value(opts::Undefined{});
        });
        f.context.define(prototype,Fixture::index(0),opts::Descriptor::access({},setter,true,true));
        f.context.set(f.array,Fixture::index(0),u"value");
        expect(received==opts::Value(f.array));
        expect(assigned==opts::Value(u"value"));
        expect(eq(f.view->length(),0u));
        expect(!f.context.hasOwn(f.array,Fixture::index(0)));
    };
    "native_indexed_set_respects_inherited_readonly_and_array_length"_test=[] {
        Fixture f;auto prototype=f.context.object();f.context.setPrototype(f.array,prototype);
        f.context.define(prototype,Fixture::index(0),opts::Descriptor::data(u"fixed",false,true,true));
        expect(throws<opts::Exception>([&]{f.context.set(f.array,Fixture::index(0),u"overwrite");}));
        expect(eq(f.view->length(),0u));
        f.context.define(f.array,opts::Key(u"length"),opts::Descriptor::data(0,false,false,false));
        expect(throws<opts::Exception>([&]{f.context.set(f.array,Fixture::index(1),u"grow");}));
        expect(eq(f.view->length(),0u));
    };
    "native_indexed_array_length_descriptor_is_non_enumerable_and_non_configurable"_test=[] {
        Fixture f;
        const auto descriptor=f.context.descriptor(opts::Value(f.array),opts::Key(u"length"));
        expect(descriptor.has_value());
        if(descriptor){
            expect(!descriptor->enumerable);
            expect(!descriptor->configurable);
            expect(descriptor->writable);
        }
    };
};
}
