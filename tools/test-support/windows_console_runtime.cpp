// Console tests and probes must report failures without opening desktop UI.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <exception>
#include <typeinfo>
#include <crtdbg.h>

// Configure the CRT before test suites and probe globals are initialized.
#pragma init_seg(lib)
namespace {
[[noreturn]] void reportTermination() noexcept {
    if (const auto error = std::current_exception()) {
        try {
            std::rethrow_exception(error);
        } catch (const std::exception& caught) {
            std::fprintf(stderr, "Unhandled exception: %s: %s\n",
                         typeid(caught).name(), caught.what());
        } catch (...) {
            std::fputs("Unhandled non-standard exception\n", stderr);
        }
    } else {
        std::fputs("terminate() without an active exception\n", stderr);
    }
    void* frames[48]{};
    const auto count = CaptureStackBackTrace(0, 48, frames, nullptr);
    for (unsigned short index = 0; index < count; ++index) {
        HMODULE module = nullptr;
        if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              reinterpret_cast<LPCWSTR>(frames[index]), &module)) {
            wchar_t name[MAX_PATH]{};
            (void)GetModuleFileNameW(module, name, MAX_PATH);
            const auto offset = reinterpret_cast<std::uintptr_t>(frames[index]) -
                                reinterpret_cast<std::uintptr_t>(module);
            std::fprintf(stderr, "frame[%hu] %p module=%ls rva=0x%zx\n",
                         index, frames[index], name, offset);
        }
    }
    std::fflush(stderr);
    std::abort();
}
struct ConsoleTestRuntime {
    ConsoleTestRuntime() noexcept {
        SetErrorMode(GetErrorMode() | SEM_FAILCRITICALERRORS |
                     SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
        (void)_set_error_mode(_OUT_TO_STDERR);
        (void)_set_abort_behavior(_WRITE_ABORT_MSG,
                                 _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
        (void)std::set_terminate(reportTermination);
#ifdef _DEBUG
        constexpr int kinds[] = {_CRT_WARN, _CRT_ERROR, _CRT_ASSERT};
        for (const int kind : kinds) {
            (void)_CrtSetReportMode(kind, _CRTDBG_MODE_FILE);
            (void)_CrtSetReportFile(kind, _CRTDBG_FILE_STDERR);
        }
#endif
    }
};
ConsoleTestRuntime consoleTestRuntime;
}
