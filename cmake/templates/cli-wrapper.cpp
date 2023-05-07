#define _CRT_SECURE_NO_WARNINGS 1

#include <boost/process/args.hpp>
#include <boost/process/child.hpp>
#include <boost/process/exe.hpp>
#include <boost/process/group.hpp>
#include <boost/process/io.hpp>

#include <tchar.h>
#include <windows.h>
#include <strsafe.h>

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

void ErrorExit(PTSTR lpszFunction)

// Format a readable error message, display a message box,
// and exit from the application.
{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
                                       (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
                    LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                    TEXT("%s failed with error %d: %s"),
                    lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(1);
}

int wmain(int argc, wchar_t *argv[])
{
    namespace bp = boost::process;

    // setup console
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    SetEnvironmentVariableW(L"ENABLE_CONSOLE", L"1");
    SetEnvironmentVariableW(L"ENABLE_CP_UTF8", L"1");
    if (enableVirtualTerminalProcessing(STD_OUTPUT_HANDLE) && enableVirtualTerminalProcessing(STD_ERROR_HANDLE)) {
        SetEnvironmentVariableW(L"ENABLE_ESCAPE_CODES", L"1");
    }

    // determine the wrapper executable path
    wchar_t pathBuffer[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, pathBuffer, MAX_PATH)) {
        std::cerr << "Unable to determine wrapper executable path: " << std::error_code(GetLastError(), std::system_category()) << '\n';
        return EXIT_FAILURE;
    }

    std::wcerr << L"exe:" << pathBuffer << L"\n";

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

    std::wcerr << L"exe 2:" << pathBuffer << L"\n";

    // make vector of args (Boost.Proces doesn't just take argv)
    auto args = std::vector<std::wstring_view>();
    args.reserve(--argc);
    for (++argv; argv && argc; ++argv, --argc) {
        args.emplace_back(*argv);
    }

    const auto pathBuffer2 = L"D:\\applications\\windows_installed\\msys64\\usr\\bin\\echo.exe";

    SECURITY_ATTRIBUTES saAttr;

    printf("\n->Start of parent execution.\n");

    // Set the bInheritHandle flag so pipe handles are inherited.

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT.
    HANDLE g_hChildStd_IN_Rd = NULL;
    HANDLE g_hChildStd_IN_Wr = NULL;
    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;

    if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) )
        ErrorExit(TEXT("StdoutRd CreatePipe"));

    // Ensure the read handle to the pipe for STDOUT is not inherited.

    if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
        ErrorExit(TEXT("Stdout SetHandleInformation"));

    // Create a pipe for the child process's STDIN.

    if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
        ErrorExit(TEXT("Stdin CreatePipe"));

    // Ensure the write handle to the pipe for STDIN is not inherited.

    if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
        ErrorExit(TEXT("Stdin SetHandleInformation"));

    auto commadLine = GetCommandLineW();
    auto processInformation = PROCESS_INFORMATION();
    auto startupInfo = STARTUPINFOW();
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    ZeroMemory(&processInformation, sizeof(processInformation));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    //startupInfo.hStdError = g_hChildStd_OUT_Wr;
    //startupInfo.hStdOutput = g_hChildStd_OUT_Wr;
    //startupInfo.hStdInput = g_hChildStd_IN_Rd;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;
    //int cstderr = _dup( _fileno( stderr ));

    auto res = CreateProcessW(
        pathBuffer,                // path of main executable
        commadLine,                // command line arguments
        nullptr,                   // process handle not inheritable
        nullptr,                   // thread handle not inheritable
        true,                      // set handle inheritance to true
        CREATE_NEW_PROCESS_GROUP,  // creation flags
        nullptr,                   // use parent's environment block
        nullptr,                   // use parent's starting directory
        &startupInfo,              // pointer to STARTUPINFO structure
        &processInformation);      // pointer to PROCESS_INFORMATION structure
    if (!res) {
        std::cerr << "Unable to launch main executable: " << std::error_code(GetLastError(), std::system_category()) << '\n';
        return EXIT_FAILURE;
    }

    auto exitCode = DWORD();

    DWORD dwRead, dwWritten;
    constexpr auto BUFSIZE = 1024;
    CHAR chBuf[BUFSIZE];
    BOOL bSuccess = FALSE;
    //HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    //SetStdHandle(STD_OUTPUT_HANDLE, hWrite);
    /*
    for (;;)
    {
        bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if( ! bSuccess || dwRead == 0 ) break;

        bSuccess = WriteFile(hParentStdOut, chBuf,
                             dwRead, &dwWritten, NULL);
        if (! bSuccess ) break;
    }
*/

    WaitForSingleObject(processInformation.hProcess, INFINITE);
    GetExitCodeProcess(processInformation.hProcess, &exitCode);
    return exitCode;

    /*

    // launch main executable via group and wait for its termination
    auto ec = std::error_code();
    auto group = bp::group();
    // not required: bp::std_out > stdout, bp::std_err > stderr, bp::std_in < stdin
    auto child = bp::child(group, bp::exe(pathBuffer), bp::args(args), ec);
    if (ec) {
        std::wcerr << L"Unable to launch \"" << pathBuffer << L"\": ";
        std::cerr << ec.message() << '\n';
        return EXIT_FAILURE;
    }
    //std::cerr << "launched group: " << group.native_handle() << "\n";
    group.wait(ec);
    //child.wait(ec);
    if (ec) {
        std::cerr << "Unable to wait for group to terminate: " << ec.message() << '\n';
        return EXIT_FAILURE;
    }
    return child.exit_code();

    */
}
