#pragma once

// Test-only reader for scalar tables emitted by layout-source-table.cjs.
// The header names Source observation fields; '~' represents absent/null.
// Each fixture row is interpreted by its family test, never by production code.
namespace folia::math::test {
class SourceRow {
    std::string_view file_;
    std::map<std::string, std::size_t, std::less<>> const& columns_;
    std::vector<std::string> const& cells_;

    [[noreturn]] void invalid(std::string_view field) const {
        throw std::runtime_error(std::string(file_) + " case " + cells_.front() +
            ": missing or invalid " + std::string(field));
    }
    std::string const& cell(std::string_view key) const {
        auto it = columns_.find(key);
        if (it == columns_.end() || cells_[it->second] == "~") invalid(key);
        return cells_[it->second];
    }
public:
    SourceRow(std::string_view file,
              std::map<std::string, std::size_t, std::less<>> const& columns,
              std::vector<std::string> const& cells)
        : file_(file), columns_(columns), cells_(cells) {}

    bool present(std::string_view key) const {
        auto it = columns_.find(key);
        return it != columns_.end() && cells_[it->second] != "~";
    }
    double number(std::string_view key) const {
        auto const& value = cell(key);
        double result{};
        auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), result);
        if (error != std::errc{} || end != value.data() + value.size() ||
            !std::isfinite(result)) invalid(key);
        return result;
    }
    bool boolean(std::string_view key) const {
        auto const& value = cell(key);
        if (value == "true") return true;
        if (value == "false") return false;
        invalid(key);
    }
    std::string text(std::string_view key) const { return cell(key); }

    bool require(bool condition, std::string_view field) const {
        boost::ut::expect(condition) << file_ << " case " << cells_.front() << " field " << field;
        return condition;
    }
    void check(double actual, std::string_view key) const {
        boost::ut::expect(actual == number(key)) << file_ << " case " << cells_.front()
            << " field " << key << " actual " << actual << " expected " << number(key);
    }
    void check(bool actual, std::string_view key) const {
        require(actual == boolean(key), key);
    }
    void check(std::string_view actual, std::string_view key) const {
        require(actual == text(key), key);
    }
    BBox box(std::string_view prefix) const {
        BBox result;
        for (auto [name, member] : numeric_box_fields()) {
            result.*member = number(std::string(prefix) + "." + name);
        }
        const auto width = std::string(prefix) + ".pwidth";
        result.pwidth = present(width) ? text(width) : "";
        return result;
    }
    void check_box(BBox const& actual, std::string_view prefix) const {
        for (auto [name, member] : numeric_box_fields()) {
            check(actual.*member, std::string(prefix) + "." + name);
        }
        check(std::string_view(actual.pwidth), std::string(prefix) + ".pwidth");
    }
private:
    static std::array<std::pair<char const*, double BBox::*>, 11> numeric_box_fields() {
        return std::array<std::pair<char const*, double BBox::*>, 11>{{
            {"w", &BBox::w}, {"h", &BBox::h}, {"d", &BBox::d},
            {"scale", &BBox::scale}, {"rscale", &BBox::rscale},
            {"L", &BBox::L}, {"R", &BBox::R}, {"ic", &BBox::ic},
            {"oc", &BBox::oc}, {"sk", &BBox::sk}, {"dx", &BBox::dx}}};
    }
};

class SourceTable {
    std::string file_;
    std::size_t expected_count_;

    static std::vector<std::string> tokens(std::string const& line) {
        std::istringstream stream(line);
        std::vector<std::string> values;
        while (stream >> std::ws && !stream.eof()) {
            std::string value;
            if (!(stream >> std::quoted(value))) throw std::runtime_error("Malformed Source table row");
            values.push_back(std::move(value));
        }
        return values;
    }
public:
    SourceTable(std::string file, std::size_t expected_count)
        : file_(std::move(file)), expected_count_(expected_count) {}

    template<class Check> void for_each(Check&& check) const {
        std::ifstream input(std::string(FOLIA_MATH_FIXTURE_DIR) + "/" + file_);
        if (!input) throw std::runtime_error("Cannot open Source fixture " + file_);
        std::string line;
        if (!std::getline(input, line) || line != "# folia-layout-source-v1")
            throw std::runtime_error("Unknown Source fixture schema " + file_);
        std::map<std::string, std::size_t, std::less<>> columns;
        std::size_t rows = 0;
        while (std::getline(input, line)) {
            if (line.empty() || line.front() == '#') continue;
            auto values = tokens(line);
            if (columns.empty()) {
                if (values.empty() || values.front() != "id")
                    throw std::runtime_error("Missing Source fixture columns " + file_);
                for (std::size_t i = 0; i < values.size(); ++i)
                    if (!columns.emplace(values[i], i).second)
                        throw std::runtime_error("Duplicate Source fixture column " + values[i]);
                continue;
            }
            if (values.size() != columns.size())
                throw std::runtime_error("Wrong Source fixture field count " + file_);
            SourceRow row(file_, columns, values);
            if (row.number("id") != static_cast<double>(++rows))
                throw std::runtime_error("Wrong Source fixture case order " + file_);
            check(row);
        }
        if (!input.eof() || rows != expected_count_)
            throw std::runtime_error("Incomplete Source fixture " + file_);
    }
};
}
