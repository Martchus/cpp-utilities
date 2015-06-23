#include "commandlineutils.h"

#include <string>
#include <iostream>

using namespace std;

namespace ApplicationUtilities {

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

} // namespace ApplicationUtilities

