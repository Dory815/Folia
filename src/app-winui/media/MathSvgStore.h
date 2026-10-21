#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace winrt::Folia
{
    class MathSvgStore
    {
    public:
        struct Range
        {
            std::uint64_t offset = 0;
            std::uint64_t size = 0;
        };

        MathSvgStore();
        ~MathSvgStore();
        MathSvgStore(MathSvgStore const&) = delete;
        MathSvgStore& operator=(MathSvgStore const&) = delete;
        MathSvgStore(MathSvgStore&&) noexcept;
        MathSvgStore& operator=(MathSvgStore&&) noexcept;

        Range Append(std::string_view svg);
        std::shared_ptr<std::string const> Read(Range range) const;
        std::filesystem::path const& Path() const noexcept;

    private:
        struct State;
        std::unique_ptr<State> state;
    };
}
