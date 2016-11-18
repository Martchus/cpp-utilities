# C++ utilities
Common C++ classes and routines used by my applications such as argument parser, IO and conversion utilities.

## Features
The library utilizes:
* parsing command-line arguments and providing Bash completion
* dealing with dates and times
* conversion of primitive data types to byte-buffers and vice versa (litte-endian and big-endian)
* common string conversions/operations, eg.
 - character set conversion via iconv
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
* C++ compiler supporting C++11, tested with
 - GNU g++
 - mingw-w64
 - Clang
* CMake, tested 3.5.1 and 3.6.0
* cppunit for unit tests (optional)
* Doxygen for API documentation (optional)
* Graphviz for diagrams in the API documentation (optional)

#### Runtime dependencies
* The c++utilities library itself only needs
 - the C/C++ standard library
 - libiconv (might be part of glibc or provided as extra library)
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
* Building with qmake is not supported anymore.
* The make option ```-j``` can be used for concurrent compilation.
* ```LIB_SUFFIX```, ```LIB_SUFFIX_32``` and ```LIB_SUFFIX_64``` can be set to
  specify a suffix for the library directory, eg. lib*64* or lib*32*. The 32/64 variants are only used when building for 32/64-bit architecture.

#### Building for Windows
Building for Windows with Mingw-w64 cross compiler can be utilized using a small
[cmake wrapper from Fedora](https://aur.archlinux.org/cgit/aur.git/tree/mingw-cmake.sh?h=mingw-w64-cmake):
```
${_arch}-cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/final/install/location" "path/to/source/directory"
make DESTDIR="/temporary/install/location" install-mingw-w64-strip
```
* To create the \*.ico file for the application icon ffmpeg/avconv is required.
* The target ```install-mingw-w64-strip``` in the example will only install files
  suitable for creating a cross-compiler package and additionally strip the binaries.

#### Development builds
During development I find it useful to build all required projects (for instace c++utilities, qtutilities, tagparser and tageditor) as one big project.

This can be easily achieved by using CMake's ```add_subdirectory()``` function. For project files
see the repository [subdirs](https://github.com/Martchus/subdirs).

For a debug build, just use ```-DCMAKE_BUILD_TYPE=Debug```.

#### Arch Linux package
The repository [PKGBUILDs](https://github.com/Martchus/PKGBUILDs) contains files
for building Arch Linux packages.

PKGBUILD files to build for Windows using the Mingw-w64 compiler are also included.

#### RPM packages
RPM \*.spec files can be found at [openSUSE Build Servide](https://build.opensuse.org/project/show/home:mkittler).

#### Gentoo
Packages are provided by perfect7gentleman; checkout his [repository](https://github.com/perfect7gentleman/pg_overlay).

### General notes
* There is a workaround for [GCC Bug 66145](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145) provided
  in io/catchiofailure.h.

## TODO
- remove unused features
