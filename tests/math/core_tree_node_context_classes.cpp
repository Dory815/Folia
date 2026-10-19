#include "support/folia_test.hpp"
import folia.math.core.tree.node_context_classes;
import folia.math.util.option_functions;
import folia.math.util.context_property_bindings;
import folia.math.util.native_identities;
import folia.math.util.property_host;

namespace {
using namespace boost::ut;
namespace math=folia::math;
namespace tree=folia::math::tree;
namespace math_options=folia::math::options;
using Value=math_options::Value;
using Key=math_options::Key;
struct Product:tree::Node<Product>{using tree::Node<Product>::Node;};
struct Fixture {
    math_options::Context context;
    math::OptionFunctionBindings functions{context};
    math::ContextPropertyBindings properties{context,functions};
    math::NativeIdentityBindings<Product*> identities{context};
    std::shared_ptr<std::atomic_bool> lease=std::make_shared<std::atomic_bool>(true);
    tree::TreeNodeContextBindings<Product> values{context,identities,properties,lease};
    tree::TreeNodeContextClasses<Product> classes{context,properties,values,lease};
    math_options::ObjectRef factory=context.object();
    Fixture(){
        context.set(Value(factory),Key(u"nodeIsKind"),Value(context.function(u"",[](auto&,Value,
            std::span<const Value>){return Value(true);} )));
        context.set(Value(factory),Key(u"create"),Value(context.function(u"",[this](auto&,
            Value,std::span<const Value>)->Value{
            const std::array<Value,1> args{Value(factory)};
            return Value(context.construct(classes.abstractNodeConstructor(),args));
        })));
    }
    math_options::ObjectRef node(){
        const std::array<Value,1> args{Value(factory)};
        return context.construct(classes.abstractNodeConstructor(),args);
    }
    Value call(math_options::ObjectRef receiver,std::u16string_view name,std::span<const Value> args={}){
        const auto method=context.get(Value(receiver),Key(std::u16string(name)));
        return context.callMember(method,Value(receiver),args,name);
    }
};
}

suite math_tree_context_classes=[] {
    "math_tree_context_class_objects_are_canonical"_test=[] {
        Fixture f;
        expect(f.context.get(Value(f.classes.abstractNodeConstructor()),Key(u"prototype"))==
            Value(f.classes.abstractNodePrototype()));
        expect(f.context.get(Value(f.classes.abstractEmptyNodeConstructor()),Key(u"prototype"))==
            Value(f.classes.abstractEmptyNodePrototype()));
        expect(f.context.prototype(f.classes.abstractEmptyNodePrototype())==
            std::optional<math_options::ObjectRef>(f.classes.abstractNodePrototype()));
        expect(f.context.get(Value(f.classes.abstractNodeConstructor()),Key(u"length"))==Value(3));
        expect(f.context.get(Value(f.classes.abstractEmptyNodeConstructor()),Key(u"length"))==Value(0));
        const auto method=f.context.descriptor(Value(f.classes.abstractNodePrototype()),Key(u"setProperty"));
        expect(method.has_value());
        if(method){expect(method->enumerable);expect(method->writable);expect(method->configurable);}
        const auto kind=f.context.descriptor(Value(f.classes.abstractNodePrototype()),Key(u"kind"));
        expect(kind.has_value());
        if(kind){expect(kind->accessor);expect(!kind->enumerable);expect(kind->configurable);}
    };
    "math_tree_context_constructor_dynamic_order_and_partial_fields"_test=[] {
        Fixture f;std::vector<std::string> trace;
        auto existing=f.context.object();
        f.context.setPrototype(existing,f.classes.abstractNodePrototype());
        const auto baseSetProperty=f.context.get(Value(f.classes.abstractNodePrototype()),Key(u"setProperty"));
        const auto baseSetChildren=f.context.get(Value(f.classes.abstractNodePrototype()),Key(u"setChildren"));
        const auto setProperty=f.context.function(u"",[&](math_options::Context& cx,Value receiver,
            std::span<const Value> args){
            trace.push_back("setProperty");return cx.callMember(baseSetProperty,receiver,args,u"setProperty");
        });
        const auto setChildren=f.context.function(u"",[&](math_options::Context& cx,Value receiver,
            std::span<const Value> args){
            trace.push_back("setChildren");return cx.callMember(baseSetChildren,receiver,args,u"setChildren");
        });
        f.context.set(Value(existing),Key(u"setProperty"),Value(setProperty));
        f.context.set(Value(existing),Key(u"setChildren"),Value(setChildren));
        auto properties=f.context.object();
        const auto getter=f.context.function(u"get",[&](math_options::Context&,Value,
            std::span<const Value>){trace.push_back("propertyGet");return Value(11);});
        f.context.define(properties,Key(u"a"),math_options::Descriptor::access(getter,std::nullopt,true,true));
        auto children=f.context.array(1),child=f.node();
        f.context.set(Value(children),Key(u"0"),Value(child));
        const std::array<Value,3> args{Value(f.factory),Value(properties),Value(children)};
        (void)f.context.call(f.classes.abstractNodeConstructor(),Value(existing),args);
        expect(trace==std::vector<std::string>{"propertyGet","setProperty","setChildren"});
        const auto ownProperties=f.context.get(Value(existing),Key(u"properties"));
        expect(f.context.get(ownProperties,Key(u"a"))==Value(11));
        const auto ownChildren=std::get<math_options::ObjectRef>(
            f.context.get(Value(existing),Key(u"childNodes")).data);
        expect(f.context.arrayLength(ownChildren)==std::uint32_t{1});
        expect(f.context.get(Value(child),Key(u"parent"))==Value(existing));
        auto failed=f.context.object();
        const std::array<Value,2> bad{Value(f.factory),Value(math_options::Null{})};
        bool threw=false;
        try{(void)f.context.call(f.classes.abstractNodeConstructor(),Value(failed),bad);}
        catch(const math_options::Exception&){threw=true;}
        expect(threw);
        expect(f.context.hasOwn(Value(failed),Key(u"factory")));
        expect(f.context.hasOwn(Value(failed),Key(u"parent")));
        expect(f.context.hasOwn(Value(failed),Key(u"properties")));
        expect(f.context.hasOwn(Value(failed),Key(u"childNodes")));
    };
    "math_tree_context_base_constructor_call_existing"_test=[] {
        Fixture f;auto existing=f.context.object();
        f.context.setPrototype(existing,f.classes.abstractNodePrototype());
        const auto source=f.context.object();
        f.context.set(Value(source),Key(u"alpha"),Value(7));
        const std::array<Value,2> args{Value(f.factory),Value(source)};
        const auto result=f.context.call(f.classes.abstractNodeConstructor(),Value(existing),args);
        expect(std::holds_alternative<math_options::Undefined>(result.data));
        expect(f.context.get(Value(existing),Key(u"factory"))==Value(f.factory));
        const auto properties=f.context.get(Value(existing),Key(u"properties"));
        expect(f.context.get(properties,Key(u"alpha"))==Value(7));
        expect(f.context.arrayLength(std::get<math_options::ObjectRef>(
            f.context.get(Value(existing),Key(u"childNodes")).data))==std::uint32_t{0});
    };
    "math_tree_context_empty_call_return_and_noop_children"_test=[] {
        Fixture f;auto existing=f.context.object();
        f.context.setPrototype(existing,f.classes.abstractEmptyNodePrototype());
        const auto child=f.node(),children=f.context.array(1);
        f.context.set(Value(children),Key(u"0"),Value(child));
        const std::array<Value,3> args{Value(f.factory),Value(math_options::Undefined{}),Value(children)};
        const auto result=f.context.call(f.classes.abstractEmptyNodeConstructor(),Value(existing),args);
        expect(result==Value(existing));
        const auto ownChildren=std::get<math_options::ObjectRef>(
            f.context.get(Value(existing),Key(u"childNodes")).data);
        expect(f.context.arrayLength(ownChildren)==std::uint32_t{0});
        const std::array<Value,1> appendArgs{Value(child)};
        expect(f.call(existing,u"appendChild",appendArgs)==Value(child));
        expect(f.context.arrayLength(ownChildren)==std::uint32_t{0});
    };
    "math_tree_context_copy_symbol_and_walk"_test=[] {
        Fixture f;const auto parent=f.node(),child=f.node();
        const auto symbol=f.context.symbol(u"marker");
        const auto properties=std::get<math_options::ObjectRef>(
            f.context.get(Value(parent),Key(u"properties")).data);
        f.context.define(properties,Key(symbol),math_options::Descriptor::data(Value(23)));
        const std::array<Value,1> appendArgs{Value(child)};
        expect(f.call(parent,u"appendChild",appendArgs)==Value(child));
        std::size_t visits=0;
        const auto visitor=f.context.function(u"",[&visits](auto&,Value,std::span<const Value>){
            ++visits;return Value(math_options::Undefined{});
        });
        const std::array<Value,1> walkArgs{Value(visitor)};
        (void)f.call(parent,u"walkTree",walkArgs);
        expect(visits==std::size_t{2});
        const auto matched=std::get<math_options::ObjectRef>(f.call(parent,u"findNodes",
            std::array<Value,1>{Value(u"unknown")}).data);
        expect(f.context.arrayLength(matched)==std::uint32_t{2});
        expect(f.call(parent,u"toString")==Value(u"unknown(unknown())"));
        const auto clone=std::get<math_options::ObjectRef>(f.call(parent,u"copy").data);
        const auto cloneProperties=f.context.get(Value(clone),Key(u"properties"));
        expect(f.context.get(cloneProperties,Key(symbol))==Value(23));
        const auto cloneChildren=std::get<math_options::ObjectRef>(
            f.context.get(Value(clone),Key(u"childNodes")).data);
        expect(f.context.arrayLength(cloneChildren)==std::uint32_t{1});
    };
    "math_tree_context_replace_uses_raw_child_index"_test=[] {
        Fixture f;const auto parent=f.node(),previous=f.node(),next=f.node();
        const std::array<Value,1> appendArgs{Value(previous)};
        (void)f.call(parent,u"appendChild",appendArgs);
        const auto current=f.context.get(Value(parent),Key(u"childNodes"));
        const auto override=f.context.function(u"",[](math_options::Context&,Value,
            std::span<const Value>){return Value(math_options::Undefined{});});
        f.context.set(Value(parent),Key(u"childIndex"),Value(override));
        const std::array<Value,2> replaceArgs{Value(next),Value(previous)};
        expect(f.call(parent,u"replaceChild",replaceArgs)==Value(next));
        expect(f.context.get(current,Key(u"undefined"))==Value(next));
        expect(f.context.get(Value(next),Key(u"parent"))==Value(parent));
    };
    "math_tree_context_array_methods_keep_raw_results"_test=[] {
        Fixture f;const auto parent=f.node(),child=f.node();
        const auto array=f.context.get(Value(parent),Key(u"childNodes"));
        const auto nativePush=f.context.get(array,Key(u"push"));
        const auto push=f.context.function(u"",[nativePush](math_options::Context& cx,Value receiver,
            std::span<const Value> args){
            (void)cx.callMember(nativePush,receiver,args,u"push");
            return Value(u"not a length");
        });
        f.context.set(array,Key(u"push"),Value(push));
        const std::array<Value,1> appendArgs{Value(child)};
        expect(f.call(parent,u"appendChild",appendArgs)==Value(child));
        const auto indexOf=f.context.function(u"",[](math_options::Context&,Value,
            std::span<const Value>){return Value(u"-1");});
        f.context.set(array,Key(u"indexOf"),Value(indexOf));
        expect(f.call(parent,u"childIndex",appendArgs)==Value(u"-1"));
    };
    "math_tree_context_walk_dispatches_own_replacements"_test=[] {
        Fixture f;const auto root=f.node(),child=f.node();
        bool rootCalled=false,childCalled=false;
        const auto rootWalk=f.context.function(u"",[&](math_options::Context& cx,Value receiver,
            std::span<const Value> args){
            rootCalled=true;
            const auto callback=args.empty()?Value(math_options::Undefined{}):args[0];
            const auto data=args.size()>1?args[1]:Value(math_options::Undefined{});
            const std::array<Value,2> visitArgs{receiver,data};
            (void)cx.callMember(callback,Value(math_options::Undefined{}),visitArgs,u"walkTree callback");
            return data;
        });
        f.context.set(Value(root),Key(u"walkTree"),Value(rootWalk));
        const auto found=std::get<math_options::ObjectRef>(f.call(root,u"findNodes",
            std::array<Value,1>{Value(u"unknown")}).data);
        expect(rootCalled);
        expect(f.context.arrayLength(found)==std::uint32_t{1});
        f.context.erase(root,Key(u"walkTree"));
        const auto childWalk=f.context.function(u"",[&](math_options::Context& cx,Value receiver,
            std::span<const Value> args){
            childCalled=true;
            const auto callback=args.empty()?Value(math_options::Undefined{}):args[0];
            const auto data=args.size()>1?args[1]:Value(math_options::Undefined{});
            const std::array<Value,2> visitArgs{receiver,data};
            (void)cx.callMember(callback,Value(math_options::Undefined{}),visitArgs,u"walkTree callback");
            return data;
        });
        f.context.set(Value(child),Key(u"walkTree"),Value(childWalk));
        (void)f.call(root,u"appendChild",std::array<Value,1>{Value(child)});
        const auto visitor=f.context.function(u"",[](math_options::Context&,Value,
            std::span<const Value>){return Value(math_options::Undefined{});});
        (void)f.call(root,u"walkTree",std::array<Value,1>{Value(visitor)});
        expect(childCalled);
    };
};
