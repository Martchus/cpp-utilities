#include <boost/process/args.hpp>
#include <boost/process/child.hpp>
#include <boost/process/exe.hpp>
#include <boost/process/group.hpp>
#include <boost/process/io.hpp>

#include <tchar.h>
#include <windows.h>

#include <cstdlib>
#include <cwchar>
#include <iostream>
#include <system_error>
#include <string_view>

static bool enableVirtualTerminalProcessing(DWORD nStdHandle)
{
    auto stdHandle = GetStdHandle(nStdHandle);
    if (stdHandle == INVALID_HANDLE_VALUE) {
        return false;
    }
    auto dwMode = DWORD();
    if (!GetConsoleMode(stdHandle, &dwMode)) {
        return false;
    }
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(stdHandle, dwMode);
}

int wmain(int argc, wchar_t *argv[])
{
    namespace bp = boost::process;

    // setup console
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    if (enableVirtualTerminalProcessing(STD_OUTPUT_HANDLE) && enableVirtualTerminalProcessing(STD_ERROR_HANDLE)) {
        SetEnvironmentVariable(L"ENABLE_ESCAPE_CODES", L"1");
    }

    // determine the wrapper executable path
    wchar_t pathBuffer[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, pathBuffer, MAX_PATH)) {
        std::cerr << "Unable to determine wrapper executable path: " << std::error_code(GetLastError(), std::system_category()) << '\n';
        return EXIT_FAILURE;
    }

    // replace "-cli.exe" with just ".exe" to determine the main executable path
    const auto path = std::wstring_view(pathBuffer);
    auto filenameStart = path.rfind(L'\\');
    if (filenameStart == std::wstring_view::npos) {
        filenameStart = 0;
    }
    const auto appendixStart = std::wstring_view(pathBuffer + filenameStart, path.size() - filenameStart).rfind(L"-cli.exe");
    if (appendixStart == std::wstring_view::npos) {
        std::cerr << "Unable to determine main executable path: unexpected wrapper executable name\n";
        return EXIT_FAILURE;
    }
    std::wcscpy(pathBuffer + filenameStart + appendixStart, L".exe");

    // launch main executable via group and wait for its termination
    auto ec = std::error_code();
    auto group = bp::group();
    auto child = bp::child(bp::exe(pathBuffer), bp::args(++argv), bp::std_out > stdout, bp::std_err > stderr, bp::std_in < stdin, ec);
    if (ec) {
        std::cerr << "Unable to launch \"" << argv[0] << "\": " << ec.message() << '\n';
        return EXIT_FAILURE;
    }
    group.wait(ec);
    if (ec) {
        std::cerr << "Unable to wait for group to terminate: " << ec.message() << '\n';
        return EXIT_FAILURE;
    }
    return child.exit_code();
}
