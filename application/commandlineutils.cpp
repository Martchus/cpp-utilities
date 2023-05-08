#include "./commandlineutils.h"
#include "./argumentparserprivate.h"

#include "../io/ansiescapecodes.h"

#include <iostream>
#include <string>

#include <fcntl.h>
#ifdef PLATFORM_WINDOWS
#include <cstring>
#include <io.h>
#include <tchar.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

using namespace std;

namespace CppUtilities {

/*!
 * \brief Prompts for confirmation displaying the specified \a message.
 */
bool confirmPrompt(const char *message, Response defaultResponse)
{
    cout << message;
    cout << ' ' << '[';
    cout << (defaultResponse == Response::Yes ? 'Y' : 'y');
    cout << '/' << (defaultResponse == Response::No ? 'N' : 'n');
    cout << ']' << ' ';
    cout.flush();
    for (string line;;) {
        getline(cin, line);
        if (line == "y" || line == "Y" || (defaultResponse == Response::Yes && line.empty())) {
            return true;
        } else if (line == "n" || line == "N" || (defaultResponse == Response::No && line.empty())) {
            return false;
        } else {
            cout << "Please enter [y] or [n]: ";
            cout.flush();
        }
    }
}

/*!
 * \brief Returns whether the specified env variable is set to a non-zero and non-white-space-only value.
 */
std::optional<bool> isEnvVariableSet(const char *variableName)
{
    const char *envValue = std::getenv(variableName);
    if (!envValue) {
        return std::nullopt;
    }
    for (; *envValue; ++envValue) {
        switch (*envValue) {
        case '0':
        case ' ':
            break;
        default:
            return true;
        }
    }
    return false;
}

/*!
 * \brief Returns the current size of the terminal.
 * \remarks Unknown members of the returned TerminalSize are set to zero.
 */
TerminalSize determineTerminalSize()
{
    TerminalSize size;
#ifndef PLATFORM_WINDOWS
    ioctl(STDOUT_FILENO, TIOCGWINSZ, reinterpret_cast<winsize *>(&size));
#else
    CONSOLE_SCREEN_BUFFER_INFO consoleBufferInfo;
    if (const HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE)) {
        GetConsoleScreenBufferInfo(stdHandle, &consoleBufferInfo);
        if (consoleBufferInfo.dwSize.X > 0) {
            size.columns = static_cast<unsigned short>(consoleBufferInfo.dwSize.X);
        }
        if (consoleBufferInfo.dwSize.Y > 0) {
            size.rows = static_cast<unsigned short>(consoleBufferInfo.dwSize.Y);
        }
    }
#endif
    return size;
}

#ifdef PLATFORM_WINDOWS
/*!
 * \brief Returns whether Mintty is used.
 */
static bool isMintty()
{
    static const auto mintty = [] {
        const char *const msyscon = std::getenv("MSYSCON");
        const char *const termprog = std::getenv("TERM_PROGRAM");
        return (msyscon && std::strstr(msyscon, "mintty")) || (termprog && std::strstr(termprog, "mintty"));
    }();
    return mintty;
}

/*!
 * \brief Enables virtual terminal processing (and thus processing of ANSI escape codes) of the console
 *        determined by the specified \a nStdHandle.
 * \sa https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
 */
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

/*!
 * \brief Enables virtual terminal processing (and thus processing of ANSI escape codes) of the console
 *        or disables use of ANSI escape codes if that's not possible.
 */
bool handleVirtualTerminalProcessing()
{
    // try to enable virtual terminal processing
    if (enableVirtualTerminalProcessing(STD_OUTPUT_HANDLE) && enableVirtualTerminalProcessing(STD_ERROR_HANDLE)) {
        return true;
    }
    // disable use of ANSI escape codes otherwise if it makes sense
    if (isMintty()) {
        return false; // no need to disable escape codes if it is just mintty
    }
    if (const char *const term = std::getenv("TERM"); term && std::strstr(term, "xterm")) {
        return false; // no need to disable escape codes if it is some xterm-like terminal
    }
    return EscapeCodes::enabled = false;
}

/*!
 * \brief Closes stdout, stdin and stderr and stops the console.
 * \remarks Internally used by startConsole() to close the console when the application exits.
 */
void stopConsole()
{
    fclose(stdout);
    fclose(stdin);
    fclose(stderr);
    if (auto *const consoleWindow = GetConsoleWindow()) {
        FreeConsole();
    }
}

/*!
 * \brief Ensure the process has a console attached and properly setup.
 * \remarks
 * - Only available (and required) under Windows where otherwise standard I/O is not possible via the console (unless
 *   when using Mintty).
 * - Attaching a console breaks redirections/pipes so this needs to be opted-in by setting the environment variable
 *   `ENABLE_CONSOLE=1`.
 * - Note that this is only useful to start a console from a GUI application. It is not necassary to call this function
 *   from a console application.
 * - The console is automatically closed when the application exits.
 * - This function alone does not provide good results. It still breaks redirections in PowerShell and other shells and
 *   after the application exists the command prompt is not displayed. A CLI-wrapper is required for proper behavior. The
 *   build system automatically generates one when the CMake variable BUILD_CLI_WRAPPER is set. Note that this CLI-wrapper
 *   still relies on this function (and thus sets `ENABLE_CONSOLE=1`). Without this standard I/O would still not be
 *   possible via the console. The part for skipping in case there's a redirection is still required. Otherwise
 *   redirections/pipes are broken when using the CLI-wrapper as well.
 * \sa
 * - https://docs.microsoft.com/en-us/windows/console/AttachConsole
 * - https://docs.microsoft.com/en-us/windows/console/AllocConsole
 * - https://docs.microsoft.com/en-us/windows/console/SetConsoleCP
 * - https://docs.microsoft.com/en-us/windows/console/SetConsoleOutputCP
 */
void startConsole()
{
    // skip if ENABLE_CONSOLE is set to 0 or not set at all
    if (const auto e = isEnvVariableSet("ENABLE_CONSOLE"); !e.has_value() || !e.value()) {
        return;
    }

    // check whether there's a redirection; skip messing with any streams then to not break redirections/pipes
    auto pos = std::fpos_t();
    std::fgetpos(stdout, &pos);
    const auto skipstdout = pos >= 0;
    std::fgetpos(stderr, &pos);
    const auto skipstderr = pos >= 0;
    std::fgetpos(stdin, &pos);
    const auto skipstdin = pos >= 0;
    const auto skip = skipstdout || skipstderr || skipstdin;

    // attach to the parent process' console or allocate a new console if that's not possible
    if (!skip && (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole())) {
        FILE* fp;
#ifdef _MSC_VER
        // take care of normal streams
        if (!skipstdout) {
            freopen_s(&fp, "CONOUT$", "w", stdout);
            std::cout.clear();
            std::clog.clear();
        }
        if (!skipstderr) {
            freopen_s(&fp, "CONOUT$", "w", stderr);
            std::cerr.clear();
        }
        if (!skipstdin) {
            freopen_s(&fp, "CONIN$", "r", stdin);
            std::cin.clear();
        }
        // take care of wide streams
        auto hConOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        auto hConIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (!skipstdout) {
            SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
            std::wcout.clear();
            std::wclog.clear();
        }
        if (!skipstderr) {
            SetStdHandle(STD_ERROR_HANDLE, hConOut);
            std::wcerr.clear();
        }
        if (!skipstdin) {
            SetStdHandle(STD_INPUT_HANDLE, hConIn);
            std::wcin.clear();
        }
#else
        // redirect stdout
        auto stdHandle = std::intptr_t();
        auto conHandle = int();
        if (!skipstdout) {
            stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_OUTPUT_HANDLE));
            conHandle = _open_osfhandle(stdHandle, _O_TEXT);
            fp = _fdopen(conHandle, "w");
            *stdout = *fp;
            setvbuf(stdout, nullptr, _IONBF, 0);
        }
        // redirect stdin
        if (!skipstdin) {
            stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_INPUT_HANDLE));
            conHandle = _open_osfhandle(stdHandle, _O_TEXT);
            fp = _fdopen(conHandle, "r");
            *stdin = *fp;
            setvbuf(stdin, nullptr, _IONBF, 0);
        }
        // redirect stderr
        if (!skipstderr) {
            stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_ERROR_HANDLE));
            conHandle = _open_osfhandle(stdHandle, _O_TEXT);
            fp = _fdopen(conHandle, "w");
            *stderr = *fp;
            setvbuf(stderr, nullptr, _IONBF, 0);
        }
        // sync
        ios::sync_with_stdio(true);
#endif
        // ensure the console prompt is shown again when app terminates
        std::atexit(stopConsole);
    }

    // set console character set to UTF-8
    if (const auto e = isEnvVariableSet("ENABLE_CP_UTF8"); !e.has_value() || e.value()) {
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
    }

    // enable virtual terminal processing or disable ANSI-escape if that's not possible
    if (const auto e = isEnvVariableSet("ENABLE_HANDLING_VIRTUAL_TERMINAL_PROCESSING"); !e.has_value() || e.value()) {
        handleVirtualTerminalProcessing();
    }
}

/*!
 * \brief Convert command line arguments to UTF-8.
 * \remarks Only available on Windows (on other platforms we can assume passed arguments are already UTF-8 encoded).
 */
pair<vector<unique_ptr<char[]>>, vector<char *>> convertArgsToUtf8()
{
    pair<vector<unique_ptr<char[]>>, vector<char *>> res;
    int argc;

    LPWSTR *argv_w = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv_w || argc <= 0) {
        return res;
    }

    res.first.reserve(static_cast<size_t>(argc));
    res.second.reserve(static_cast<size_t>(argc));
    for (LPWSTR *i = argv_w, *end = argv_w + argc; i != end; ++i) {
        int requiredSize = WideCharToMultiByte(CP_UTF8, 0, *i, -1, nullptr, 0, 0, 0);
        if (requiredSize <= 0) {
            break; // just stop on error
        }

        auto argv = make_unique<char[]>(static_cast<size_t>(requiredSize));
        requiredSize = WideCharToMultiByte(CP_UTF8, 0, *i, -1, argv.get(), requiredSize, 0, 0);
        if (requiredSize <= 0) {
            break;
        }

        res.second.emplace_back(argv.get());
        res.first.emplace_back(std::move(argv));
    }

    LocalFree(argv_w);
    return res;
}
#endif

} // namespace CppUtilities
