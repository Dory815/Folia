// Same-owner forwards for the genuine cyclic MathJax TeX class graph.
export module folia.math.input.tex.parser:types;
export import folia.math.core.input_jax;
export import folia.math.input.tex.find_tex;
export import folia.math.input.tex.stack_item_factory;
export import folia.math.input.tex.map_handler;
export import folia.math.input.tex.tex_error;
export import folia.math.util.native_identities;
export import folia.math.util.string_map;
export import folia.math.util.unicode_normalization;
import std;
export namespace folia::math::tex {
class ParserRuntime;
class Configuration;
class ParserConfiguration;
class ParseOptions;
class TexParser;
class Tags;
class AbstractTags;
class ColumnParser;
class ArrayItem;
class TeX;
class Label;
class TagInfo;
struct TagsClassInfo;
class KeyValueDef;
template<class T>using ParserShared=std::variant<Missing,PropertyNull,std::shared_ptr<T>>;
template<class T>using ParserBorrowed=std::variant<Missing,PropertyNull,T*>;
using ConfigurationRef=std::shared_ptr<Configuration>;
using ParserConfigurationRef=std::shared_ptr<ParserConfiguration>;
using ParseOptionsRef=std::shared_ptr<ParseOptions>;
using TexParserRef=std::shared_ptr<TexParser>;
using TagsRef=std::shared_ptr<Tags>;
using TeXRef=std::shared_ptr<TeX>;
using KeyValueType=std::shared_ptr<KeyValueDef>;
using TagsClassRef=std::shared_ptr<TagsClassInfo>;
using LabelRef=std::shared_ptr<Label>;
using TagInfoRef=std::shared_ptr<TagInfo>;
// Source ordinary records/arrays/callback tuples retain Context handles.
using ProcessorValue=options::Value;
using ProcessorListValue=options::Value;
using StackItemConfigValue=options::Value;
using TagsConfigValue=options::Value;
using TagFormat=options::Value; // string | string[]; clearTag also passes null.
using ParserArray=std::shared_ptr<std::vector<TexParserRef>>;
using TagInfoArray=std::shared_ptr<std::vector<TagInfoRef>>;
using StackItemClassRef=tree::FactoryClassRef<BaseItem>;
}
