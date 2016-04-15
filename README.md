# c++utilities
Common C++ classes and routines used by my applications such as argument parser, IO and conversion utilities.

## Features
The library utilizes:
* parsing command-line arguments
* chronology (DateTime and TimeSpan classes)
* conversion of primitive data types to byte-buffers (litte-endian, big-endian) and vice versa
* common string conversions/operations, eg.
 - split, join, findAndReplace
 - numberToString, stringToNumber
 - encodeBase64, decodeBase64
* IO
 - reading/writing primitive data types of various sizes (little-endian, big-endian)
 - reading/writing terminated strings and size-prefixed strings
 - bitwise reading
 - parsing INI files

## Build instructions
### Requirements
* C++ compiler supporting C++11 (I've tested GNU g++, Clang and mingw-w64 yet.)
* CMake to build
* cppunit to build and run unit tests
* The c++utilities library only depends on the C++ standard library. For dependencies of my other projects
  check the README.md of these projects.

### How to build
Just run:
```
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/where/you/want/to/install" "path/to/projectdirectory"
make
make check # build and run unit tests (optional)
make install
```

Building for Windows with Mingw-w64 cross compiler can be utilized using a small
[cmake wrapper from Fedora](https://aur.archlinux.org/cgit/aur.git/tree/mingw-cmake.sh?h=mingw-w64-cmake):
```
${_arch}-cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/where/you/want/to/install" "path/to/projectdirectory"
make && make install
```
To create *.ico files for application icons ffmpeg is required.

In any case, the make option *-j* can be used for concurrent compilation.

### Creating Arch Linux package
The repository [PKGBUILDs](https://github.com/Martchus/PKGBUILDs) contains files for building Arch Linux packages.
PKGBUILD files to build for Windows using the Mingw-w64 compiler are also included.

### Notes
* Because of [GCC Bug 66145](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145) usage of the new libstdc++ ABI
  is currently disabled. Linking against cppunit built using new libstdc++ ABI isn't possible.

## TODO
- rewrite argument parser (the API might change slightly)
- remove unused features
