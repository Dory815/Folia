// Root-frozen stubs establish real failing tests before Luna implementation.
module folia.math.input.tex.token_map;
import std;
namespace folia::math::tex {
ParseResult parseResult(ParseResult){throw std::runtime_error("P22b parseResult stub");}
void AbstractTokenMap::initializeSource(Property,ParseMethod){throw std::runtime_error("P22b base constructor stub");}
Property AbstractTokenMap::name()const{throw std::runtime_error("P22b name stub");}
ParseMethod AbstractTokenMap::parser()const{throw std::runtime_error("P22b parser getter stub");}
void AbstractTokenMap::parser(ParseMethod){throw std::runtime_error("P22b parser setter stub");}
ParseMethod AbstractTokenMap::parserFor(const Property&){throw std::runtime_error("P22b parserFor stub");}
ParseResult AbstractTokenMap::parse(const ParseInput&){throw std::runtime_error("P22b base parse stub");}
void RegExpMap::initializeSource(Property,ParseMethod,RegExpRef){throw std::runtime_error("P22b regex constructor stub");}
bool RegExpMap::contains(const Property&){throw std::runtime_error("P22b regex contains stub");}
TokenLookup RegExpMap::lookup(const Property&){throw std::runtime_error("P22b regex lookup stub");}
void AbstractParseMap::initializeSource(Property,ParseMethod){throw std::runtime_error("P22b map field initializer stub");}
TokenLookup AbstractParseMap::lookup(const Property&){throw std::runtime_error("P22b map lookup stub");}
bool AbstractParseMap::contains(const Property&){throw std::runtime_error("P22b map contains stub");}
void AbstractParseMap::add(Property,TokenLookup){throw std::runtime_error("P22b map add stub");}
void AbstractParseMap::remove(const Property&){throw std::runtime_error("P22b map remove stub");}
void CharacterMap::initializeSource(Property,ParseMethod,Property){throw std::runtime_error("P22b character constructor stub");}
ParseResult DelimiterMap::parse(const ParseInput&){throw std::runtime_error("P22b delimiter parse stub");}
void MacroMap::initializeSource(Property,Property,Property){throw std::runtime_error("P22b macro constructor stub");}
ParseMethod MacroMap::parserFor(const Property&){throw std::runtime_error("P22b macro parserFor stub");}
ParseResult MacroMap::parse(const ParseInput&){throw std::runtime_error("P22b macro parse stub");}
ParseResult CommandMap::parse(const ParseInput&){throw std::runtime_error("P22b command parse stub");}
void EnvironmentMap::initializeSource(Property,ParseMethod,Property,Property){throw std::runtime_error("P22b environment constructor stub");}
ParseResult EnvironmentMap::parse(const ParseInput&){throw std::runtime_error("P22b environment parse stub");}
}
