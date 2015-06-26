# c++utilities
Common C++ classes and routines used by my applications such as argument parser, IO and conversion utilities.

## Build instructions
Building the library is simple:
```
INSTALL_ROOT=/where/you/want/to/install qmake-qt5 "c++utilities.pro" -r -spec linux-g++
make && make install
```
As you can see, the qmake build system is used. However the library itself does *not* depend on Qt.

The repository PKGBUILDs (also on GitHub) contains files for building Arch Linux packages. A PKGBUILD file to build for Windows using the Mingw-w64 compiler is also included.
