// Console tests and probes must report failures without opening desktop UI.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <cstdlib>
#include <crtdbg.h>

// Configure the CRT before test suites and probe globals are initialized.
#pragma init_seg(lib)
namespace {
struct ConsoleTestRuntime {
    ConsoleTestRuntime() noexcept {
        SetErrorMode(GetErrorMode() | SEM_FAILCRITICALERRORS |
                     SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
        (void)_set_error_mode(_OUT_TO_STDERR);
        (void)_set_abort_behavior(_WRITE_ABORT_MSG,
                                 _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
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
