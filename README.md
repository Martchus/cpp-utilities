# c++utilities
Common C++ classes and routines used by my applications such as argument parser, IO and conversion utilities.

## Build instructions
### Requirements
* C++ compiler supporting C++11 (I've tested GNU g++, Clang and mingw-w64 yet.)
* CMake to build
* cppunit to build and run unit tests after building
* The c++utilities library only depends on the C++ standard library. For Dependencies of my other projects
  see the README.md of these projects.

### How to build
Just run:
```
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/where/you/want/to/install" "path/to/projectdirectory"
make
make check # build and run unit tests (optional)
make install
```

Building with qmake is also possible:
```
INSTALL_ROOT="/where/you/want/to/install" qmake-qt5 "path/to/projectfile"
make && make install
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
The repository PKGBUILDs (also on GitHub) contains files for building Arch Linux packages.
PKGBUILD files to build for Windows using the Mingw-w64 compiler are also included.

### Notes
* Because of [GCC Bug 66145](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145) usage of the new libstdc++ ABI
  is currently disabled. Linking against cppunit built using new libstdc++ ABI isn't possible.

## TODO
- rewrite argument parser
- remove unused features
