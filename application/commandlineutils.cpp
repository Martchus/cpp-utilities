#include "./commandlineutils.h"

#include <string>
#include <iostream>

#ifdef PLATFORM_WINDOWS
# include <windows.h>
# include <fcntl.h>
#endif

using namespace std;

namespace ApplicationUtilities {

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
    for(string line; ;) {
        getline(cin, line);
        if(line == "y" || line == "Y" || (defaultResponse == Response::Yes && line.empty())) {
            return true;
        } else if(line == "n" || line == "N" || (defaultResponse == Response::No && line.empty())) {
            return false;
        } else {
            cout << "Please enter [y] or [n]: ";
            cout.flush();
        }
    }
}

#ifdef PLATFORM_WINDOWS
/*!
 * \brief Starts the console and sets the console output code page to UTF-8.
 * \remarks This method is only available on Windows and used to start a console from a GUI application.
 */
void startConsole()
{
    AttachConsole(ATTACH_PARENT_PROCESS);
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.X = 200;
    coninfo.dwSize.Y = 500;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
    // redirect stdout
    auto stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_OUTPUT_HANDLE));
    auto conHandle = _open_osfhandle(stdHandle, _O_TEXT);
    auto fp = _fdopen(conHandle, "w");
    *stdout = *fp;
    setvbuf(stdout, NULL, _IONBF, 0);
    // redirect stdin
    stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_INPUT_HANDLE));
    conHandle = _open_osfhandle(stdHandle, _O_TEXT);
    fp = _fdopen(conHandle, "r");
    *stdin = *fp;
    setvbuf(stdin, NULL, _IONBF, 0);
    // redirect stderr
    stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_ERROR_HANDLE));
    conHandle = _open_osfhandle(stdHandle, _O_TEXT);
    fp = _fdopen(conHandle, "w");
    *stderr = *fp;
    setvbuf(stderr, NULL, _IONBF, 0);
    // set console to handle UTF-8 IO correctly
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    // sync
    ios::sync_with_stdio(true);
}
#endif

} // namespace ApplicationUtilities
