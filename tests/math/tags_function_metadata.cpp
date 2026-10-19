#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.input.tex.find_tex_host;
import folia.math.util.regexp_host;
import folia.math.util.unicode_normalization;
import folia.math.util.option_functions;

namespace {
using namespace boost::ut;
using namespace folia::math;
namespace t = folia::math::tex;
namespace o = folia::math::options;

struct Graph {
    o::Context context;
    OptionFunctionBindings functions{context};
    regexp_host::RegExpBindings regexp{context};
    Unicode16Normalization unicode;
    core::InputJaxModule input{context};
    core::FindMathModule findMath{context};
    t::FindTeXModule findTeX{context,findMath,regexp};
    t::ParserRuntime runtime{context,functions,regexp,unicode,input,findTeX};
    Graph(){runtime.initializeParserRuntime();}
};

struct MethodSpec { std::u16string_view name; double length; std::string_view label; };

void checkMethod(o::Context& context,o::ObjectRef prototype,const MethodSpec& spec){
    const auto value=context.get(o::Value(prototype),o::Key(std::u16string(spec.name)));
    const auto* function=std::get_if<o::ObjectRef>(&value.data);
    expect(function&&context.kind(*function)==o::ObjectKind::function) << spec.label << " is a callable prototype method";
    if(!function||context.kind(*function)!=o::ObjectKind::function)return;
    expect(context.get(o::Value(*function),o::Key(u"name"))==o::Value(u"")) << spec.label << " has Source's anonymous method name";
    expect(context.get(o::Value(*function),o::Key(u"length"))==o::Value(spec.length)) << spec.label << " has Source arity";
    const auto descriptor=context.descriptor(o::Value(prototype),o::Key(std::u16string(spec.name)));
    expect(descriptor&& !descriptor->accessor && descriptor->writable && descriptor->enumerable && descriptor->configurable)
        << spec.label << " retains Source's writable enumerable configurable method descriptor";
}

void checkAccessor(o::Context& context,o::ObjectRef prototype,std::u16string_view property,
                   std::u16string_view getterName,double getterLength,
                   std::optional<std::u16string_view> setterName,std::optional<double> setterLength,
                   std::string_view label){
    const auto descriptor=context.descriptor(o::Value(prototype),o::Key(std::u16string(property)));
    expect(descriptor&&descriptor->accessor&&!descriptor->enumerable&&descriptor->configurable)
        << label << " retains Source's non-enumerable configurable accessor descriptor";
    if(!descriptor||!descriptor->accessor)return;
    expect(descriptor->getter.has_value()) << label << " has its Source getter";
    if(descriptor->getter){
        const o::Value getter(*descriptor->getter);
        expect(context.get(getter,o::Key(u"name"))==o::Value(std::u16string(getterName))) << label << " getter name matches Source";
        expect(context.get(getter,o::Key(u"length"))==o::Value(getterLength)) << label << " getter arity matches Source";
    }
    expect(descriptor->setter.has_value()==setterName.has_value()) << label << " setter presence matches Source";
    if(descriptor->setter&&setterName){
        const o::Value setter(*descriptor->setter);
        expect(context.get(setter,o::Key(u"name"))==o::Value(std::u16string(*setterName))) << label << " setter name matches Source";
        expect(context.get(setter,o::Key(u"length"))==o::Value(*setterLength)) << label << " setter arity matches Source";
    }
}

suite tagsClassMetadata=[] {
    "math_tags_class_and_instance_functions_match_source_metadata"_test=[] {
        Graph graph;
        auto& cx=graph.context;auto& runtime=graph.runtime;
        const std::array<std::pair<o::Value,std::u16string_view>,5> constructors{{
            {runtime.labelClass,u"Label"},{runtime.tagInfoClass,u"TagInfo"},
            {runtime.abstractTagsClass,u"AbstractTags"},{runtime.noTagsClass,u"NoTags"},
            {runtime.allTagsClass,u"AllTags"}}};
        for(const auto& [value,name]:constructors){
            const auto* constructor=std::get_if<o::ObjectRef>(&value.data);
            expect(constructor!=nullptr) << "Tags constructor is present in the runtime";
            if(!constructor)continue;
            expect(cx.get(o::Value(*constructor),o::Key(u"name"))==o::Value(std::u16string(name))) << "Tags constructor Source name";
            const double arity=name==u"Label"?2.0:name==u"TagInfo"?8.0:0.0;
            expect(cx.get(o::Value(*constructor),o::Key(u"length"))==o::Value(arity)) << "Tags constructor Source arity";
            const auto lengthDescriptor=cx.descriptor(o::Value(*constructor),o::Key(u"length"));
            expect(lengthDescriptor&&!lengthDescriptor->writable&&!lengthDescriptor->enumerable&&lengthDescriptor->configurable)
                << "Tags constructor length has Source's standard descriptor";
        }
        const auto* baseConstructor=std::get_if<o::ObjectRef>(&runtime.abstractTagsClass.data);
        const auto* noneConstructor=std::get_if<o::ObjectRef>(&runtime.noTagsClass.data);
        const auto* allConstructor=std::get_if<o::ObjectRef>(&runtime.allTagsClass.data);
        expect(baseConstructor&&noneConstructor&&allConstructor) << "Tags class constructors are available";
        if(!baseConstructor||!noneConstructor||!allConstructor)return;
        const auto base=std::get<o::ObjectRef>(cx.get(o::Value(*baseConstructor),o::Key(u"prototype")).data);
        const auto none=std::get<o::ObjectRef>(cx.get(o::Value(*noneConstructor),o::Key(u"prototype")).data);
        const auto all=std::get<o::ObjectRef>(cx.get(o::Value(*allConstructor),o::Key(u"prototype")).data);
        const auto checkDerivedConstructor=[&](o::ObjectRef prototype,o::ObjectRef constructor,bool enumerable,std::string_view label){
            const auto descriptor=cx.descriptor(o::Value(prototype),o::Key(u"constructor"));
            expect(descriptor&&!descriptor->accessor&&descriptor->writable&&descriptor->enumerable==enumerable&&descriptor->configurable)
                << label << " prototype constructor descriptor matches emitted Source class shape";
            if(descriptor)expect(descriptor->value==o::Value(constructor)) << label << " prototype constructor points to the canonical class function";
        };
        checkDerivedConstructor(base,*baseConstructor,false,"AbstractTags");
        checkDerivedConstructor(none,*noneConstructor,true,"NoTags");
        checkDerivedConstructor(all,*allConstructor,true,"AllTags");
        const std::array<MethodSpec,18> baseMethods{{
            {u"start",3,"AbstractTags.start"},{u"end",0,"AbstractTags.end"},
            {u"tag",2,"AbstractTags.tag"},{u"notag",0,"AbstractTags.notag"},
            {u"formatUrl",2,"AbstractTags.formatUrl"},{u"formatTag",1,"AbstractTags.formatTag"},
            {u"formatRef",1,"AbstractTags.formatRef"},{u"formatId",1,"AbstractTags.formatId"},
            {u"formatNumber",1,"AbstractTags.formatNumber"},{u"autoTag",0,"AbstractTags.autoTag"},
            {u"clearTag",0,"AbstractTags.clearTag"},{u"getTag",1,"AbstractTags.getTag"},
            {u"resetTag",0,"AbstractTags.resetTag"},{u"reset",1,"AbstractTags.reset"},
            {u"startEquation",1,"AbstractTags.startEquation"},{u"finishEquation",1,"AbstractTags.finishEquation"},
            {u"finalize",2,"AbstractTags.finalize"},{u"makeId",0,"AbstractTags.makeId"}}};
        for(const auto& method:baseMethods)checkMethod(cx,base,method);
        checkMethod(cx,base,{u"makeTag",0,"AbstractTags.makeTag"});
        checkAccessor(cx,base,u"env",u"get",0.0,std::nullopt,std::nullopt,"AbstractTags.env");
        checkAccessor(cx,base,u"noTag",u"get",0.0,std::nullopt,std::nullopt,"AbstractTags.noTag");
        checkAccessor(cx,base,u"label",u"get",0.0,std::u16string_view(u"set"),1.0,"AbstractTags.label");
        checkMethod(cx,none,{u"autoTag",0,"NoTags.autoTag"});
        checkMethod(cx,none,{u"getTag",0,"NoTags.getTag"});
        checkMethod(cx,all,{u"finalize",2,"AllTags.finalize"});
        const auto tags=runtime.tagsFactory.create(u"none");
        const auto identity=tags->sourceIdentity();
        const auto enTagValue=cx.get(identity,o::Key(u"enTag"));
        const auto* enTag=std::get_if<o::ObjectRef>(&enTagValue.data);
        expect(enTag&&cx.kind(*enTag)==o::ObjectKind::function) << "Tags instance owns its enTag function";
        if(!enTag||cx.kind(*enTag)!=o::ObjectKind::function)return;
        expect(cx.get(o::Value(*enTag),o::Key(u"name"))==o::Value(u"")) << "enTag retains Source's anonymous function name";
        expect(cx.get(o::Value(*enTag),o::Key(u"length"))==o::Value(2)) << "enTag retains its two Source parameters";
        const auto enTagDescriptor=cx.descriptor(o::Value(identity),o::Key(u"enTag"));
        expect(enTagDescriptor&&!enTagDescriptor->accessor&&enTagDescriptor->writable&&enTagDescriptor->enumerable&&enTagDescriptor->configurable)
            << "enTag retains Source's own function field descriptor";
    };
};
} // namespace
