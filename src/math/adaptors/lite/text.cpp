module folia.math.adaptors.lite_adaptor;
import :text;
import std;

namespace folia::math::adaptors::lite {
LiteText::LiteText(std::u16string value) : value_(std::move(value)) {}

LiteTextRef LiteText::create(std::u16string value) {
    return LiteTextRef(new LiteText(std::move(value)));
}

LiteComment::LiteComment(std::u16string value) : LiteText(std::move(value)) {}

LiteCommentRef LiteComment::create(std::u16string rawComment) {
    return LiteCommentRef(new LiteComment(std::move(rawComment)));
}
}
