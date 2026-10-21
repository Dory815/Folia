#pragma once

#include <memory>
#include <stop_token>
#include <string_view>

#include "media/MathJaxRenderer.h"

namespace winrt::Folia
{
    class NativeMathSession
    {
    public:
        NativeMathSession();
        ~NativeMathSession();

        NativeMathSession(NativeMathSession const&) = delete;
        NativeMathSession& operator=(NativeMathSession const&) = delete;
        NativeMathSession(NativeMathSession&&) = delete;
        NativeMathSession& operator=(NativeMathSession&&) = delete;

        MathJaxSvg Render(
            std::string_view tex,
            bool display,
            float em,
            float containerWidth,
            std::stop_token stop = {});

    private:
        struct State;
        std::unique_ptr<State> state;
    };
}
