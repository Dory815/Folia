// MathJax 4.1.3 input/tex/Tags.ts; Apache-2.0. Actual record constructors.
module folia.math.input.tex.parser;
import std;
namespace folia::math::tex {
Label::Label(std::u16string tagValue,std::u16string idValue)
    :tag(std::move(tagValue)),id(std::move(idValue)){
}
TagInfo::TagInfo(std::u16string envValue,bool taggableValue,bool defaultTagsValue,
                 options::Value tagValue,std::u16string tagIdValue,
                 TagFormat tagFormatValue,bool noTagValue,std::u16string labelIdValue)
    :env(std::move(envValue)),taggable(taggableValue),defaultTags(defaultTagsValue),
     tag(std::move(tagValue)),tagId(std::move(tagIdValue)),tagFormat(std::move(tagFormatValue)),
     noTag(noTagValue),labelId(std::move(labelIdValue)){
}
}
