# C++ utilities
Useful C++ classes and routines such as argument parser, IO and conversion utilities.

## Features
The library utilizes:

* parsing command-line arguments and providing Bash completion
    - supports nested arguments
    - supports operations (no `--` or `-` prefix, eg. `git status`)
    - can check for invalid or uncombinable arguments
    - can print help automatically
    - provides automatic Bash completion for argument names
    - allows customizing Bash completion for argument values
* dealing with dates and times
* conversion of primitive data types to byte-buffers and vice versa (litte-endian and big-endian)
* common string conversions/operations, eg.
    - character set conversions via iconv
    - split, join, find and replace
    - conversion from number to string and vice verca
    - encoding/decoding base-64
    - building string without multiple heap allocations ("string builder")
* IO
    - reading/writing primitive data types of various sizes (little-endian and big-endian)
    - reading/writing terminated strings and size-prefixed strings
    - reading/writing INI files
    - reading bitwise
    - writing formatted output using ANSI escape sequences
* using SFINAE by providing additional traits, eg. for checking whether a type is iteratable
* testing with *CppUnit*
    - finding testfiles and make working copies of testfiles
    - assert standard output
    - various helper
* building with CMake by providing some modules and templates

Besides, the library provides a few useful algorithms and data structures:

* min(), max() for any number of arguments
* digitsum(), factorial(), powerModulo(), inverseModulo(), orderModulo()
* Damerau–Levenshtein distance
* *N*-dimensional array

## Build instructions
### Requirements
#### Build-only dependencies
* C++ compiler supporting C++17, tested with
    - clang++ to compile for GNU/Linux and Android
    - g++ to compile for GNU/Linux and Windows
* CMake (at least 3.3.0)
* cppunit for unit tests (optional)
* Doxygen for API documentation (optional)
* Graphviz for diagrams in the API documentation (optional)
* clang-format for tidying (optional)
* llvm-profdata, llvm-cov and cppunit for source-based code coverage analysis (optional)

#### Runtime dependencies
* The c++utilities library itself only needs
    * C++ standard library supporting C++17, tested with
        - libstdc++ under GNU/Linux and Windows
        - libc++ under GNU/Linux and Android
    * iconv (might be part of glibc or provided as extra library)
* For dependencies of my other projects check the README.md of these projects.

### How to build
Just run:
```
cd "path/to/build/directory"
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="/final/install/location" \
      "path/to/projectdirectory"
make tidy # format source files (optional, must be enabled via CLANG_FORMAT_ENABLED)
make
make check # build and run tests (optional)
make coverage # build and run tests measuring test coverage (optional, must be enabled via CLANG_SOURCE_BASED_COVERAGE_ENABLED)
make apidoc # build API documentation (optional)
make DESTDIR="/temporary/install/location" install
```

#### General notes
* The make option ```-j``` can be used for concurrent compilation.
* ```LIB_SUFFIX```, ```LIB_SUFFIX_32``` and ```LIB_SUFFIX_64``` can be set to
  specify a suffix for the library directory, eg. lib*64* or lib*32*. The 32/64 variants are only used when building for 32/64-bit architecture.
* By default the build system will *build* static libs. To *build* shared libraries *instead*, set `BUILD_SHARED_LIBS=ON`.
* By default the build system will prefer *linking against* shared libraries. To force *linking against* static libraries set `STATIC_LINKAGE=ON`.
  However, this will only affect applications. To force linking statically when building shared libraries set `STATIC_LIBRARY_LINKAGE=ON`.
* If thread local storage is not supported by your compiler/platform (might be the case on MacOS), you can disable making use of it
  via `ENABLE_THREAD_LOCAL=OFF`.
* For more detailed documentation, see the documentation about build variables (in
  [directory doc](https://github.com/Martchus/cpp-utilities/blob/master/doc/buildvariables.md) and
  in Doxygen version accessible via "Related Pages").
* The repository [PKGBUILDs](https://github.com/Martchus/PKGBUILDs) contains build scripts for GNU/Linux, Windows and MacOS X in form
  of Arch Linux packages. These scripts can be used as an example also when building under another platform.

#### Building for Windows
* Building for Windows with GCC as cross compiler and mingw-w64 can be simplified by using a small
  [Cmake wrapper and a custom toolchain file](https://aur.archlinux.org/cgit/aur.git/tree/mingw-cmake.sh?h=mingw-w64-cmake):
  ```
  ${_arch}-cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/final/install/location" "path/to/source/directory"
  make DESTDIR="/temporary/install/location" install-mingw-w64-strip
  ```
 
* To create the \*.ico file for the application icon ffmpeg/avconv is required.
* The target ```install-mingw-w64-strip``` can be used as in the example above to only install files
  suitable for creating a cross-compiler package and additionally strip the binaries.

#### Building for MacOS X
* Building for MacOS X is possible using [osxcross](https://github.com/tpoechtrager/osxcross).
* Here is a Homebrew formula to build Tag Editor (without GUI): https://gist.github.com/rakkesh/0b13b8fca5dd1d57d98537ef1dd2e0dd

#### Development builds
During development I find it useful to build all required projects (for instace c++utilities, qtutilities, tagparser and tageditor) as one big project.

This can be easily achieved by using CMake's ```add_subdirectory()``` function. For project files
see the repository [subdirs](https://github.com/Martchus/subdirs). For an example, see
[build instructions for Syncthing Tray](https://github.com/Martchus/syncthingtray#building-this-straight).

For a debug build, just use ```-DCMAKE_BUILD_TYPE=Debug```.

#### Arch Linux package
The repository [PKGBUILDs](https://github.com/Martchus/PKGBUILDs) contains files
for building Arch Linux packages.

PKGBUILD files to cross compile for Windows using mingw-w64 and for MacOS X using osxcross are also included.

#### RPM packages for openSUSE
RPM \*.spec files can be found at [openSUSE Build Servide](https://build.opensuse.org/project/show/home:mkittler).
Packages are available for x86_64, aarch64 and armv7l. Since GCC provided by Leap is too old, only Tumbleweed packages
are up-to-date.

#### Gentoo
Packages are provided by perfect7gentleman; checkout his [repository](https://github.com/perfect7gentleman/pg_overlay).

#### Cygwin
Scripts to build with Cygwin are provided by svnpenn. Checkout his
[repository](https://github.com/svnpenn/glade).

### General notes
* There is a workaround for [GCC Bug 66145](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145) provided
  in io/catchiofailure.h.
