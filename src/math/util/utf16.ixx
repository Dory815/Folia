// UTF-8/UTF-16 boundary helpers used by the MathJax host adaptation.
// UTF-16 strings retain isolated surrogates internally; only encode() applies
// TextEncoder's required U+FFFD replacement at the external UTF-8 boundary.
export module folia.math.util.utf16;
export import folia.math.util.utf16_types;
import folia.math.cancellation;
import std;

export namespace folia::math::utf16 {

std::optional<char32_t> codePointAt(std::u16string_view text, std::size_t index) {
    if (index >= text.size()) {
        return std::nullopt;
    }

    const auto first = static_cast<std::uint16_t>(text[index]);
    if (first >= 0xD800 && first <= 0xDBFF && index + 1 < text.size()) {
        const auto second = static_cast<std::uint16_t>(text[index + 1]);
        if (second >= 0xDC00 && second <= 0xDFFF) {
            const auto high = static_cast<std::uint32_t>(first - 0xD800);
            const auto low = static_cast<std::uint32_t>(second - 0xDC00);
            return static_cast<char32_t>(0x10000 + (high << 10) + low);
        }
    }
    // JavaScript codePointAt returns a lone surrogate's numeric code unit.
    return static_cast<char32_t>(first);
}

std::vector<char32_t> codePoints(std::u16string_view text) {
    std::vector<char32_t> result;
    result.reserve(text.size());
    for (std::size_t i = 0; i < text.size();) {
        cancellation::checkpoint(cancellation::CheckpointStage::Serialization);
        const auto point = codePointAt(text, i);
        if (!point) {
            break;
        }
        result.push_back(*point);
        i += *point > 0xFFFF ? 2 : 1;
    }
    return result;
}

Input decode(std::string_view bytes) {
    Input result;
    result.text.reserve(bytes.size());
    result.byteOffsets.reserve(bytes.size() + 1);

    const auto byte = [&](std::size_t i) {
        return static_cast<std::uint8_t>(static_cast<unsigned char>(bytes[i]));
    };
    const auto continuation = [](std::uint8_t value) {
        return value >= 0x80 && value <= 0xBF;
    };
    const auto append_utf16 = [&](std::uint32_t cp, std::size_t start) {
        if (cp <= 0xFFFF) {
            result.text.push_back(static_cast<char16_t>(cp));
            result.byteOffsets.push_back(start);
            return;
        }
        cp -= 0x10000;
        result.text.push_back(static_cast<char16_t>(0xD800 + (cp >> 10)));
        result.byteOffsets.push_back(start);
        result.text.push_back(static_cast<char16_t>(0xDC00 + (cp & 0x3FF)));
        result.byteOffsets.push_back(start);
    };

    for (std::size_t i = 0; i < bytes.size();) {
        cancellation::checkpoint(cancellation::CheckpointStage::TeX);
        const std::size_t start = i;
        const std::uint8_t lead = byte(i);
        std::uint32_t cp = 0;
        std::size_t length = 0;
        if (lead <= 0x7F) {
            cp = lead;
            length = 1;
        } else if (lead >= 0xC2 && lead <= 0xDF) {
            cp = lead & 0x1F;
            length = 2;
        } else if (lead >= 0xE0 && lead <= 0xEF) {
            cp = lead & 0x0F;
            length = 3;
        } else if (lead >= 0xF0 && lead <= 0xF4) {
            cp = lead & 0x07;
            length = 4;
        } else {
            throw DecodeError(start);
        }

        if (length > bytes.size() - start) {
            throw DecodeError(start);
        }
        for (std::size_t j = 1; j < length; ++j) {
            const std::uint8_t next = byte(start + j);
            if (!continuation(next)) {
                throw DecodeError(start);
            }
            if (j == 1 && ((lead == 0xE0 && next < 0xA0) ||
                           (lead == 0xED && next > 0x9F) ||
                           (lead == 0xF0 && next < 0x90) ||
                           (lead == 0xF4 && next > 0x8F))) {
                throw DecodeError(start);
            }
            cp = (cp << 6) | (next & 0x3F);
        }
        // The lead/second-byte bounds above reject overlong encodings,
        // UTF-8 encodings of surrogates, and values above U+10FFFF.
        append_utf16(cp, start);
        i += length;
    }
    result.byteOffsets.push_back(bytes.size());
    return result;
}

std::string encode(std::u16string_view text) {
    std::string result;
    result.reserve(text.size() * 3);
    const auto append_utf8 = [&](std::uint32_t cp) {
        if (cp <= 0x7F) {
            result.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FF) {
            result.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            result.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            result.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    };

    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto first = static_cast<std::uint16_t>(text[i]);
        if (first >= 0xD800 && first <= 0xDBFF) {
            if (i + 1 < text.size()) {
                const auto second = static_cast<std::uint16_t>(text[i + 1]);
                if (second >= 0xDC00 && second <= 0xDFFF) {
                    const auto cp = 0x10000u +
                        ((static_cast<std::uint32_t>(first - 0xD800)) << 10) +
                        static_cast<std::uint32_t>(second - 0xDC00);
                    append_utf8(cp);
                    ++i;
                    continue;
                }
            }
            append_utf8(0xFFFD); // TextEncoder replacement for lone high surrogate.
        } else if (first >= 0xDC00 && first <= 0xDFFF) {
            append_utf8(0xFFFD); // TextEncoder replacement for lone low surrogate.
        } else {
            append_utf8(first);
        }
    }
    return result;
}

} // namespace folia::math::utf16
