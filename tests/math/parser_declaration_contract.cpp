// Root compile-only contracts for actual Source class ownership and field types.
// This file does not instantiate or accept the still-unimplemented parser.
#include "support/folia_test.hpp"
import folia.math.input.tex.parser;
namespace {
using namespace folia::math;
using namespace folia::math::tex;
static_assert(std::derived_from<ParseOptions,NodeFactoryConfiguration>);
static_assert(std::derived_from<ParseOptions,StackItemFactoryConfiguration>);
static_assert(std::derived_from<TexParser,ParserContext>);
static_assert(std::derived_from<TeX,core::AbstractInputJax>);
static_assert(std::same_as<decltype(ParseOptions::options),options::Value>);
static_assert(std::same_as<decltype(ParseOptions::packageData),ParserShared<SourceStringMap<options::Value>>>);
static_assert(std::same_as<decltype(ParseOptions::parsers),options::Value>);
static_assert(std::same_as<decltype(ParseOptions::nodeFactory),options::Value>);
static_assert(std::same_as<decltype(ParseOptions::columnParser),options::Value>);
static_assert(std::same_as<decltype(ParseOptions::mathItem),core::MathItemReference>);
static_assert(std::same_as<decltype(TexParser::configuration),ParserBorrowed<ParseOptions>>);
static_assert(std::same_as<decltype(TexParser::stack),ParserShared<Stack>>);
static_assert(std::same_as<decltype(ArrayItem::parser),ParserBorrowed<TexParser>>);
static_assert(std::same_as<decltype(ArrayItem::table),ParserShared<tree::ChildArray<mml::MmlNode>>>);
// Tags Source fields are live Context properties on the actual instance.
// They are accessed through the native owner's field view, rather than a
// second typed field whose value could drift from the Context graph.
static_assert(std::same_as<decltype(std::declval<AbstractTags&>().field(u"configuration")),options::Value>);
static_assert(std::same_as<decltype(std::declval<AbstractTags&>().field(u"enTag")),options::Value>);
static_assert(std::same_as<decltype(std::declval<TexParser&>().GetDelimiter(u"name")),Property>);
static_assert(std::same_as<decltype(std::declval<TexParser&>().GetArgument(u"name")),Property>);
static_assert(std::same_as<decltype(std::declval<TexParser&>().GetBrackets(u"name")),Property>);
static_assert(std::same_as<decltype(std::declval<SubsupItem&>().checkItem(nullptr)),CheckType>);
static_assert(std::derived_from<EqnArrayItem,ArrayItem>);
static_assert(std::derived_from<MstyleItem,BeginItem>);
static_assert(std::derived_from<NoTags,AbstractTags>);
static_assert(std::derived_from<AllTags,AbstractTags>);
template<class...T>constexpr bool allActualBaseItems(){return (std::derived_from<T,BaseItem>&&...);}
static_assert(allActualBaseItems<StartItem,StopItem,OpenItem,CloseItem,NullItem,PrimeItem,
    SubsupItem,OverItem,LeftItem,Middle,RightItem,BreakItem,BeginItem,EndItem,StyleItem,
    PositionItem,CellItem,MmlItem,FnItem,NotItem,NonscriptItem,DotsItem,ArrayItem,
    EqnArrayItem,MstyleItem,EquationItem>());
struct EntrySignatureProbe:ArrayItem {
    using ArrayItem::getEntry;
};
static_assert(std::same_as<decltype(std::declval<EntrySignatureProbe&>().getEntry()),options::Value>);
static_assert(!std::copy_constructible<ParserRuntime>);
static_assert(!std::copy_constructible<TagsFactory>);
}
