// Native candidate for the 36 frozen BaseItems CJS parse observations.
// This file is deliberately outside test auto-discovery. It consumes the
// actual ParseOptions owner and constructs the actual TexParser/MML products;
// it provides observations only and contains no replacement parser behavior.
import folia.math.input.tex.parser;
import folia.math.input.tex.tex_error;
import std;

namespace folia::math::tex::candidate {
struct Observation {
    std::u16string input;
    double index=0;
    std::u16string remaining;
    mml::NodeValue result=Missing{};
    std::string errorName;
    std::u16string errorId;
    std::u16string errorMessage;
};

struct ItemObservation {
    std::string sourceClass;
    std::string kind;
    bool isOpen=false,isClose=false,isFinal=false,copyEnv=false;
    double size=0;
    Property global=Missing{},env=Missing{};
    StackProperty name=Missing{};
    options::ObjectRef contextIdentity{};
    bool hasContextIdentity=false;
    options::Value constructorLength=options::Undefined{};
    options::Value prototypeKind=options::Undefined{};
    bool kindIsGetter=false;
    bool hasOwnErrors=false;
};

std::vector<ItemObservation> observeRegisteredItems(ParserRuntime& runtime,ParseOptions& parseOptions) {
    auto* binding=std::get_if<std::shared_ptr<StackItemFactory>>(&parseOptions.itemFactory);
    if(!binding||!*binding)throw options::Exception("TypeError",u"Actual ParseOptions StackItemFactory is unavailable");
    auto& factory=**binding;
    auto& context=runtime.context();
    using Arguments=std::vector<std::any>;
    const std::array<std::pair<std::string_view,std::string_view>,26> classes{{
        {"StartItem","start"},{"StopItem","stop"},{"OpenItem","open"},{"CloseItem","close"},
        {"NullItem","null"},{"PrimeItem","prime"},{"SubsupItem","subsup"},{"OverItem","over"},
        {"LeftItem","left"},{"Middle","middle"},{"RightItem","right"},{"BreakItem","break"},
        {"BeginItem","begin"},{"EndItem","end"},{"StyleItem","style"},{"PositionItem","position"},
        {"CellItem","cell"},{"MmlItem","mml"},{"FnItem","fn"},{"NotItem","not"},
        {"NonscriptItem","nonscript"},{"DotsItem","dots"},{"ArrayItem","array"},
        {"EqnArrayItem","eqnarray"},{"EquationItem","equation"},{"MstyleItem","mstyle"}}};
    std::vector<ItemObservation> result;
    result.reserve(classes.size());
    for(const auto& [className,kind]:classes) {
        Arguments args;
        if(className=="StartItem")args.emplace_back(Property(PropertyList{{"isInner",false}}));
        else if(className=="LeftItem")args.emplace_back(Property("("));
        else if(className=="Middle") {args.emplace_back(Property("|"));args.emplace_back(Property("red"));}
        else if(className=="RightItem") {args.emplace_back(Property(")"));args.emplace_back(Property("blue"));}
        else if(className=="BreakItem") {args.emplace_back(Property("newline"));args.emplace_back(Property(true));}
        else if(className=="EqnArrayItem") {args.emplace_back(Property("fixture-tag"));args.emplace_back(Property(true));args.emplace_back(Property(false));}
        else if(className=="EquationItem")args.emplace_back(Property("fixture-tag"));
        else if(className=="MstyleItem") {args.emplace_back(Property(PropertyList{{"mathcolor","red"}}));args.emplace_back(Property("fixture-style"));}
        auto* item=factory.create(kind,tree::FactoryArgs(args.data(),args.size()));
        if(!item)throw options::Exception("TypeError",u"Actual StackItemFactory returned no item");
        const auto reference=factory.reference(item);
        ItemObservation observation;
        observation.sourceClass=className;
        observation.kind=item->kind();
        observation.isOpen=item->isOpen();observation.isClose=item->isClose();
        observation.isFinal=item->isFinal();observation.copyEnv=item->copyEnv();
        observation.size=item->Size();observation.global=item->global;observation.env=item->env();
        observation.name=item->getName();
        if(const auto identity=runtime.stackItemIdentities.find(reference)) {
            observation.contextIdentity=*identity;observation.hasContextIdentity=true;
        }
        const auto classReference=factory.getNodeClass(kind);
        const auto constructor=classReference?runtime.itemClassIdentities.find(classReference):std::optional<options::ObjectRef>{};
        if(constructor) {
            observation.constructorLength=context.get(options::Value(*constructor),options::Key(u"length"));
            const auto prototype=context.get(options::Value(*constructor),options::Key(u"prototype"));
            if(const auto* prototypeObject=std::get_if<options::ObjectRef>(&prototype.data)) {
                observation.prototypeKind=context.get(options::Value(*prototypeObject),options::Key(u"kind"));
                const auto descriptor=context.descriptor(options::Value(*prototypeObject),options::Key(u"kind"));
                observation.kindIsGetter=descriptor&&descriptor->accessor&&descriptor->getter.has_value();
            }
            observation.hasOwnErrors=context.hasOwn(options::Value(*constructor),options::Key(u"errors"));
        }
        result.push_back(std::move(observation));
    }
    return result;
}

Observation observe(ParseOptions& parseOptions,std::u16string source) {
    Observation observation;
    observation.input=source;
    const auto parser=TexParser::make(std::move(source),PropertyList{},parseOptions);
    try {
        observation.result=parser->mml();
    } catch(const TexError& error) {
        observation.errorName="TexError";
        observation.errorId=error.id;
        observation.errorMessage=error.message;
    } catch(const options::Exception& error) {
        observation.errorName=error.name;
        observation.errorMessage=error.message;
    }
    observation.index=parser->i;
    observation.remaining=parser->string();
    return observation;
}

std::vector<Observation> observe(ParseOptions& parseOptions,
                                std::span<const std::u16string> sources) {
    std::vector<Observation> observations;
    observations.reserve(sources.size());
    for(const auto& source:sources)observations.push_back(observe(parseOptions,source));
    return observations;
}
}
