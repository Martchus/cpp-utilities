# C++ utilities
Common C++ classes and routines used by my applications such as argument parser, IO and conversion utilities.

## Features
The library utilizes:
* parsing command-line arguments
* dealing with dates and times
* conversion of primitive data types to byte-buffers and vice versa (litte-endian and big-endian)
* common string conversions/operations, eg.
 - split, join, find and replace
 - conversion from number to string and vice verca
 - encoding/decoding base-64
* IO
 - reading/writing primitive data types of various sizes (little-endian and big-endian)
 - reading/writing terminated strings and size-prefixed strings
 - reading/writing INI files
 - reading bitwise
* building with CMake by providing some modules and templates

## Build instructions
### Requirements
#### Build-only dependencies
* C++ compiler supporting C++11 (I've tested GNU g++, Clang and mingw-w64 yet.)
* CMake (I've only tested 3.5.1 so far.)
* cppunit for unit tests (optional)
* Doxygen for API documentation (optional)

#### Runtime dependencies
* The c++utilities library itself only needs the C/C++ standard library.
* For dependencies of my other projects check the README.md of these projects.

### How to build
Just run:
```
cd "path/to/build/directory"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/final/install/location" "path/to/projectdirectory"
make
make check # build and run unit tests (optional)
make c++utilities_apidoc # build API documentation (optional)
make DESTDIR="/temporary/install/location" install
```

#### General notes
* The make option *-j* can be used for concurrent compilation.
* Building with qmake is not supported anymore.

#### Building for Windows
Building for Windows with Mingw-w64 cross compiler can be utilized using a small
[cmake wrapper from Fedora](https://aur.archlinux.org/cgit/aur.git/tree/mingw-cmake.sh?h=mingw-w64-cmake):
```
${_arch}-cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/final/install/location" "path/to/projectdirectory"
make DESTDIR="/temporary/install/location" install-mingw-w64-strip
```
* To create the \*.ico file for the application icon ffmpeg/avconv is required.
* The target ```install-mingw-w64-strip``` in the example will only install files
  suitable for creating a cross-compiler package and additionally strip the binaries.

#### Development builds
During development I find it useful to build all required projects (for instace c++utilities, qtutilities, tagparser and tageditor) as one big project.

This can be easily achieved by using CMakes ```add_subdirectory()``` function. For project files
see the repository [subdirs](https://github.com/Martchus/subdirs).

For a debug build, just use ```-DCMAKE_BUILD_TYPE=Debug```.

### Creating Arch Linux package
The repository [PKGBUILDs](https://github.com/Martchus/PKGBUILDs) contains files for building Arch Linux packages.
PKGBUILD files to build for Windows using the Mingw-w64 compiler are also included.

### Notes
* Because of [GCC Bug 66145](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145) usage of the new libstdc++ ABI
  is currently disabled. Linking against cppunit built using new libstdc++ ABI isn't possible.

## TODO
- rewrite argument parser (the API might change slightly)
- remove unused features
