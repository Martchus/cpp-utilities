# c++utilities
Common C++ classes and routines used by my applications such as argument parser, IO and conversion utilities.

## Build instructions
Building the library is simple:
```
INSTALL_ROOT=/where/you/want/to/install qmake-qt5 c++utilities.pro
make && make install
```
As you can see, the qmake build system is used. However the library itself does *not* depend on Qt.

It is also possible to build the library using CMake:
```
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/where/you/want/to/install project/directory
make && make install
```

In any case, the make option *-j* might be used to compile multiple files in parallel.

The repository PKGBUILDs (also on GitHub) contains files for building Arch Linux packages. A PKGBUILD file to build for Windows using the Mingw-w64 compiler is also included.
