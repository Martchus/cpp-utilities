#include <windows.h>

#include <cstdlib>
#include <cwchar>
#include <iostream>
#include <string_view>
#include <system_error>

/*!
 * \brief Returns \a replacement if \a value matches \a key; otherwise returns \a value.
 */
static constexpr std::size_t replace(std::size_t value, std::size_t key, std::size_t replacement)
{
    return value == key ? replacement : value;
}

/*!
 * \brief Sets the console up and launches the "main" application.
 */
int main()
{
    // ensure environment variables are set so the main executable will attach to the parent's console
    // note: This is still required for this wrapper to receive standard I/O. We also still rely on the main
    //       process to enable UTF-8 and virtual terminal processing.
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    SetEnvironmentVariableW(L"ENABLE_CONSOLE", L"1");
    SetEnvironmentVariableW(L"ENABLE_CP_UTF8", L"1");
    SetEnvironmentVariableW(L"ENABLE_HANDLING_VIRTUAL_TERMINAL_PROCESSING", L"1");

    // determine the wrapper executable path
    wchar_t pathBuffer[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, pathBuffer, MAX_PATH)) {
        std::cerr << "Unable to determine wrapper executable path: " << std::error_code(GetLastError(), std::system_category()) << '\n';
        return EXIT_FAILURE;
    }

    // replace "-cli.exe" in the wrapper executable's file name with just ".exe" to make up the main executable path
    const auto path = std::wstring_view(pathBuffer);
    const auto filenameStart = replace(path.rfind(L'\\'), std::wstring_view::npos, 0);
    const auto appendixStart = std::wstring_view(pathBuffer + filenameStart, path.size() - filenameStart).rfind(L"-cli.exe");
    if (appendixStart == std::wstring_view::npos) {
        std::cerr << "Unable to determine main executable path: unexpected wrapper executable name\n";
        return EXIT_FAILURE;
    }
    std::wcscpy(pathBuffer + filenameStart + appendixStart, L".exe");

    // compute startup parameters
    auto commandLine = GetCommandLineW();
    auto processInformation = PROCESS_INFORMATION();
    auto startupInfo = STARTUPINFOW();
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    ZeroMemory(&processInformation, sizeof(processInformation));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    // start main executable in new group and print debug information if that's not possible
    auto res = CreateProcessW(pathBuffer, // path of main executable
        commandLine, // command line arguments
        nullptr, // process handle not inheritable
        nullptr, // thread handle not inheritable
        true, // set handle inheritance to true
        CREATE_NEW_PROCESS_GROUP, // creation flags
        nullptr, // use parent's environment block
        nullptr, // use parent's starting directory
        &startupInfo, // pointer to STARTUPINFO structure
        &processInformation); // pointer to PROCESS_INFORMATION structure
    if (!res) {
        std::cerr << "Unable to launch main executable: " << std::error_code(GetLastError(), std::system_category()) << '\n';
        std::wcerr << L" - assumed path: " << pathBuffer << L'\n';
        std::wcerr << L" - assumed command-line: " << commandLine << L'\n';
        return EXIT_FAILURE;
    }

    // wait for main executable and possible children to terminate and return exit code
    auto exitCode = DWORD();
    WaitForSingleObject(processInformation.hProcess, INFINITE);
    GetExitCodeProcess(processInformation.hProcess, &exitCode);
    return exitCode;
}
