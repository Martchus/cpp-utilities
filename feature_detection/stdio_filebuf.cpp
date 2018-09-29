// assume __gnu_cxx::stdio_filebuf is available if this program compiles

#include <ext/stdio_filebuf.h>
#include <iostream>

int main() {
    const int fd = 0;
    __gnu_cxx::stdio_filebuf<char> buf(fd, std::ios_base::in);
    return buf.fd();
}
