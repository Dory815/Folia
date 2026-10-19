#include "support/folia_test.hpp"
import folia.math.core.tree.node_context_classes;
import folia.math.util.option_functions;
import folia.math.util.context_property_bindings;
import folia.math.util.native_identities;

namespace {
using namespace boost::ut;
namespace math=folia::math;
namespace tree=folia::math::tree;
namespace o=folia::math::options;
using Value=o::Value;
using Key=o::Key;
struct Product:tree::Node<Product>{using tree::Node<Product>::Node;};
struct Fixture {
    o::Context context;
    math::OptionFunctionBindings functions{context};
    math::ContextPropertyBindings properties{context,functions};
    math::NativeIdentityBindings<Product*> identities{context};
    std::shared_ptr<std::atomic_bool> lease=std::make_shared<std::atomic_bool>(true);
    tree::TreeNodeContextBindings<Product> values{context,identities,properties,lease};
    tree::TreeNodeContextClasses<Product> classes{context,properties,values,lease};
    o::ObjectRef factory=context.object();
    Fixture(){
        context.set(Value(factory),Key(u"nodeIsKind"),Value(context.function(u"",[](auto&,Value,std::span<const Value>){return Value(true);} )));
        context.set(Value(factory),Key(u"create"),Value(context.function(u"",[this](auto&,Value,std::span<const Value>)->Value{
            const std::array<Value,1> args{Value(factory)};
            return Value(context.construct(classes.abstractNodeConstructor(),args));
        })));
    }
    o::ObjectRef node(){const std::array<Value,1> args{Value(factory)};return context.construct(classes.abstractNodeConstructor(),args);}
};
}

suite math_tree_ordinary_call=[] {
    "tree_ordinary_member_calls_ignore_function_own_call_but_explicit_call_does_not"_test=[] {
        Fixture f;
        std::size_t ownCallReads=0;
        const auto trap=f.context.function(u"get call",[&](o::Context&,Value,std::span<const Value>)->Value{
            ++ownCallReads;
            throw o::Exception("TypeError",u"own function.call getter was read");
        });
        const auto setProperty=f.context.get(Value(f.classes.abstractNodePrototype()),Key(u"setProperty"));
        const auto setPropertyRef=std::get<o::ObjectRef>(setProperty.data);
        f.context.define(setPropertyRef,Key(u"call"),o::Descriptor::access(trap,std::nullopt,false,true));
        const auto properties=f.context.object();
        f.context.set(Value(properties),Key(u"x"),Value(1));
        const std::array<Value,2> args{Value(f.factory),Value(properties)};
        const auto root=f.context.construct(f.classes.abstractNodeConstructor(),args);
        expect(f.context.get(Value(root),Key(u"properties"))!=Value(properties));
        expect(ownCallReads==std::size_t{0});
        f.context.erase(setPropertyRef,Key(u"call"));
        const auto create=std::get<o::ObjectRef>(f.context.get(Value(f.factory),Key(u"create")).data);
        f.context.define(create,Key(u"call"),o::Descriptor::access(trap,std::nullopt,false,true));
        const auto copyMethod=std::get<o::ObjectRef>(f.context.get(Value(root),Key(u"copy")).data);
        (void)f.context.call(copyMethod,Value(root),{});
        expect(ownCallReads==std::size_t{0});
        f.context.erase(create,Key(u"call"));

        const auto child=f.node();
        auto children=std::get<o::ObjectRef>(f.context.get(Value(root),Key(u"childNodes")).data);
        f.context.set(Value(children),Key(u"0"),Value(child));
        f.context.set(Value(children),Key(u"length"),Value(1));
        const auto childWalk=f.context.function(u"walkTree",[](o::Context& cx,Value receiver,std::span<const Value> a){
            const auto callback=std::get<o::ObjectRef>(a[0].data);
            const std::array<Value,2> visit{receiver,a.size()>1?a[1]:Value(o::Undefined{})};
            return cx.call(callback,Value(o::Undefined{}),visit);
        });
        const auto walkRef=childWalk;
        f.context.set(Value(child),Key(u"walkTree"),Value(childWalk));
        f.context.define(walkRef,Key(u"call"),o::Descriptor::access(trap,std::nullopt,false,true));
        std::size_t visits=0;
        const auto visitor=f.context.function(u"visitor",[&](o::Context&,Value,std::span<const Value>){++visits;return Value(o::Undefined{});});
        const auto walkMethod=f.context.get(Value(f.classes.abstractNodePrototype()),Key(u"walkTree"));
        const auto walkMethodRef=std::get<o::ObjectRef>(walkMethod.data);
        f.context.define(walkMethodRef,Key(u"call"),o::Descriptor::access(trap,std::nullopt,false,true));
        const std::array<Value,1> walkArgs{Value(visitor)};
        const auto childAccess=f.values.nodeAccess(Value(child));
        (void)childAccess->call(u"walkTree",walkArgs);
        expect(visits==std::size_t{1});
        (void)f.context.call(walkMethodRef,Value(root),walkArgs);
        expect(visits==std::size_t{3});
        expect(ownCallReads==std::size_t{0});
        f.context.erase(walkMethodRef,Key(u"call"));

        const auto selected=f.context.function(u"selected",[](o::Context&,Value,std::span<const Value>){return Value(0);});
        const auto replacement=f.context.function(u"replacement",[](o::Context&,Value,std::span<const Value>){return Value(9);});
        const auto selectedRef=selected;
        const auto explicitGetter=f.context.function(u"get call",[replacement](o::Context&,Value,std::span<const Value>){return Value(replacement);});
        f.context.define(selectedRef,Key(u"call"),o::Descriptor::access(explicitGetter,std::nullopt,false,true));
        const auto explicitResult=f.context.callMember(selected,Value(root),{});
        expect(explicitResult==Value(9));
    };
};
