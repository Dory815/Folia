#include "support/folia_test.hpp"
import folia.math.util.unicode_normalization;

using namespace boost::ut;
using folia::math::Unicode16Normalization;

namespace {
struct Sequence { std::u16string_view input, output; };
#include "fixtures/unicode/normalization-source.inc"

std::u16string decodeUnits(std::string_view hex) {
    if (hex.size() % 4) throw std::runtime_error("Invalid UTF-16 fixture length");
    std::u16string result;
    for (std::size_t i = 0; i < hex.size(); i += 4) {
        unsigned unit{};
        auto [end, error] = std::from_chars(hex.data() + i, hex.data() + i + 4, unit, 16);
        if (error != std::errc{} || end != hex.data() + i + 4)
            throw std::runtime_error("Invalid UTF-16 fixture digit");
        result.push_back(static_cast<char16_t>(unit));
    }
    return result;
}

std::array<std::u16string, 10> decodeRow(std::string_view line) {
    std::array<std::u16string, 10> fields;
    for (std::size_t i = 0; i < fields.size(); ++i) {
        auto tab = line.find('\t');
        if ((i + 1 < fields.size()) != (tab != std::string_view::npos))
            throw std::runtime_error("Invalid NFD fixture column count");
        fields[i] = decodeUnits(line.substr(0, tab));
        if (tab != std::string_view::npos) line.remove_prefix(tab + 1);
    }
    return fields;
}

std::u16string encodePoint(std::uint32_t point) {
    if (point < 0x10000) return std::u16string(1, static_cast<char16_t>(point));
    point -= 0x10000;
    return {static_cast<char16_t>(0xd800 + (point >> 10)),
            static_cast<char16_t>(0xdc00 + (point & 1023))};
}
}

suite unicode_normalization = [] {
    "unicode normalization official corpus"_test = [] {
        Unicode16Normalization host;
        std::ifstream input(std::filesystem::path(FOLIA_MATH_FIXTURE_DIR) /
                            "unicode/normalization-corpus.tsv");
        if (!input) throw std::runtime_error("Cannot open NFD corpus fixture");
        std::size_t row{};
        for (std::string line; std::getline(input, line); ++row) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            auto fields = decodeRow(line);
            for (std::size_t column = 0; column < 5; ++column)
                expect(host.nfd(fields[column]) == fields[column + 5])
                    << "corpus row " << row << " column " << column;
        }
        expect(row == corpusRows);
    };

    "unicode normalization UTF16 sequences"_test = [] {
        Unicode16Normalization host;
        for (std::size_t i = 0; i < std::size(sequences); ++i)
            expect(host.nfd(sequences[i].input) == sequences[i].output) << "sequence " << i;
    };

    "unicode normalization all codepoint planes"_test = [] {
        Unicode16Normalization host;
        for (std::uint32_t plane = 0; plane < std::size(planeDigests); ++plane) {
            std::uint64_t digest = 0xcbf29ce484222325ULL;
            auto update = [&](std::uint32_t value) { digest = (digest ^ value) * 0x100000001b3ULL; };
            for (auto point = plane * 65536; point < (plane + 1) * 65536; ++point) {
                auto output = host.nfd(encodePoint(point));
                update(point);
                update(static_cast<std::uint32_t>(output.size()));
                for (auto unit : output) update(unit);
            }
            expect(digest == planeDigests[plane]) << "plane " << plane;
        }
    };
};
