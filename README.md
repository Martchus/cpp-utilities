# C++ utilities
Useful C++ classes and routines such as argument parser, IO and conversion utilities.

## Features
The library contains helpers for:

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
* using standard IO streams
    - reading/writing primitive data types of various sizes (little-endian and big-endian)
    - reading/writing terminated strings and size-prefixed strings
    - reading/writing INI files
    - reading bitwise (from a buffer; not using standard IO streams)
    - writing formatted output using ANSI escape sequences
    - instantiating a standard IO stream from a native file descriptor to support UTF-8 encoded
      file paths under Windows and Android's `content://` URLs
* using SFINAE by providing additional traits, eg. for checking whether a type is iterable
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

## API/ABI stability
The following counts for `c++utilities` and my other libraries unless stated otherwise:

* Different major versions are incompatible (API- and ABI-wise). Different major versions can be
  installed within the same prefix using the CMake variable `CONFIGURATION_NAME` (see documentation
  about build variables mentioned below).
* Minor versions are backwards compatible (API- and ABI-wise) to previous ones within the same major
  version.
* Patch versions are interchangeable (API- and ABI-wise) within the same major/minor version.
* Some functions or classes are experimental. They might be modified in an incompatible way or even
  removed in the next minor or patch release.

## Build instructions
These build instructions apply to `c++utilities` but also to my other projects using it.

### Requirements
#### Build-only dependencies
* C++ compiler supporting C++17, tested with
    - g++ to compile for GNU/Linux and Windows
    - clang++ to compile for GNU/Linux and Android
* CMake (at least 3.17.0) and Ninja or GNU Make
* cppunit for unit tests (optional)
* Doxygen for API documentation (optional)
* Graphviz for diagrams in the API documentation (optional)
* clang-format and cmake-format for tidying (optional)
* llvm-profdata, llvm-cov and cppunit for source-based code coverage analysis (optional)
* [appstreamcli](https://www.freedesktop.org/wiki/Distributions/AppStream/) for validation
  of generated AppStream files (optional)

#### Runtime dependencies
* The `c++utilities` library itself only needs
    * C++ standard library supporting C++17, tested with
        - libstdc++ under GNU/Linux and Windows
        - libc++ under GNU/Linux and Android
    * glibc with iconv support or standalone iconv library
    * libstdc++ or Boost.Iostreams for `NativeFileStream` (optional, use `USE_NATIVE_FILE_BUFFER=OFF` to disable)
    * Boost.Process for `execApp()` test helper under Windows (optional, use `USE_BOOST_PROCESS=OFF` to disable)
    * libarchive (optional, for archiving utilities only, use `USE_LIBARCHIVE=ON` to enable)
* My other projects have further dependencies such as Qt. Checkout the README of these
  projects for further details.

### How to build
Generic example using Ninja:
```
cmake -G Ninja \
      -S "path/to/source/directory" \
      -B "path/to/build/directory" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="/final/install/location"
# build the binaries
cmake --build "path/to/build/directory"
# format source files (optional, must be enabled via CLANG_FORMAT_ENABLED)
cmake --build "path/to/build/directory" --target tidy
# build and run tests (optional)
cmake --build "path/to/build/directory" --target check
# build and run tests measuring test coverage (optional, must be enabled via CLANG_SOURCE_BASED_COVERAGE_ENABLED)
cmake --build "path/to/build/directory" --target coverage
# build API documentation (optional)
cmake --build "path/to/build/directory" --target apidoc
# install binaries, headers and additional files
DESTDIR="/temporary/install/location" \
  cmake --install "path/to/build/directory"
```

This example is rather generic. For a development build I recommended using CMakePresets as
documented in the "[CMake presets](#cmake-presets)" section below. It also contains more concrete
instructions for building on Windows.

#### General notes
* By default the build system will *build* static libs. To *build* shared libraries *instead*, set
  `BUILD_SHARED_LIBS=ON`.
* By default the build system will prefer *linking against* shared libraries. To force *linking against*
  static libraries set `STATIC_LINKAGE=ON`. However, this will only affect applications. To force linking
  statically when building shared libraries set `STATIC_LIBRARY_LINKAGE=ON`.
* If thread local storage is not supported by your compiler/platform (might be the case on MacOS), you can
  disable making use of it via `ENABLE_THREAD_LOCAL=OFF`.
* To disable use of `std::filesystem`, set `USE_STANDARD_FILESYSTEM=OFF`. Note that the Bash completion will
  not be able to suggest files and directories and the archiving utilities cannot be enabled with
  `USE_STANDARD_FILESYSTEM=OFF`. Note that this will only help with `c++utilities` itself. My other projects
  might use `std::filesystem` unconditionally.
* To disable `NativeFileStream` (and make it just a regular `std::fstream`), set `USE_NATIVE_FILE_BUFFER=OFF`.
  Note that handling paths with non-ASCII characters will then cease to work on Windows.
* The Qt-based applications support bundeling icon themes by specifying e.g.
  `BUILTIN_ICON_THEMES=breeze;breeze-dark`.
    * This variable must be set when building the application (not when building any of the libraries).
    * The specified icon themes need to be installed in the usual location. Otherwise, use e.g.
      `BUILTIN_ICON_THEMES_SEARCH_PATH=D:/programming/misc/breeze-icons/usr/share/icons` to specify the
      search path.
* For more detailed documentation, see the documentation about build variables (in
  [directory doc](https://github.com/Martchus/cpp-utilities/blob/master/doc/buildvariables.md) and
  in Doxygen version accessible via "Related Pages").
* The repository [PKGBUILDs](https://github.com/Martchus/PKGBUILDs) contains build scripts for GNU/Linux,
  Android, Windows and MacOS X in form of Arch Linux packages using `ninja`. These scripts can be used as an
  example also when building under/for other platforms.

#### Windows-specific notes
* To create application icons the tool `ffmpeg`/`avconv` is required.
* Windows builds are mainly conducted using mingw-w64/GCC so using them is recommended. Building with MSVC
  should be possible as well but it is not as well tested.
* When using `BUILTIN_ICON_THEMES`, the icon theme still needs to be installed as if it was installed on a
  GNU/Linux system. So simply grab e.g. the Arch Linux package `breeze-icons` and extract it somewhere. Do
  *not* use the package from MSYS2 or what comes with builds from KDE's binary factory.

#### MacOS-specific notes
* To create application icons the tool `png2icns` is required.
* Building for MacOS X under GNU/Linux is possible using [osxcross](https://github.com/tpoechtrager/osxcross).
* MacOS X builds are not tested regularly but should generally work (maybe with minor tweaks necassary).
* There is a [Homebrew formula](https://gist.github.com/rakkesh/0b13b8fca5dd1d57d98537ef1dd2e0dd) to
  build Tag Editor (without GUI).
* There are [MacPorts packages](https://www.macports.org/ports.php?by=name&substr=syncthingtray-devel)
  to build Syncthing Tray.

#### Development builds
During development I find it useful to build all required projects (for instance c++utilities, qtutilities,
tagparser and tageditor) as one big project.

This can be easily achieved by using CMake's `add_subdirectory()` function. For project files see the repository
[subdirs](https://github.com/Martchus/subdirs). For an example, see
[build instructions for Syncthing Tray](https://github.com/Martchus/syncthingtray#building-this-straight) or
[build instructions for Tag Editor](https://github.com/Martchus/tageditor#building-this-straight). The `subdirs`
repository also contains the script `sync-all.sh` to clone all possibly relevant repositories and keep them
up-to-date later on.

For a debug build, use `-DCMAKE_BUILD_TYPE=Debug`. To tweak various settings (e.g. warnings) for development,
use `-DENABLE_DEVEL_DEFAULTS=ON`.

#### CMake presets
There are some generic [presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) available
but also some specific to certain Arch Linux packaging found in the AUR and my PKGBUILDs repository.

Use `cmake --list-presets` to list all presets. All `cmake` commands need to be executed within the source
directory. Builds will be created within a sub-directory of the path specified via the environment variable
`BUILD_DIR`.

The most useful presets for development are likely `devel`, `devel-qt6` and `debug`. Note that the `devel`
preset (and all presets inheriting from it) use `ccache` which therefore needs to be installed.

Here is a simple example to build with the `devel-qt6` preset:
```
export BUILD_DIR=$HOME/builds                    # set build directory via environment variable
cmake --preset devel-qt6                         # configure build
cmake --build --preset devel-qt6 -- -v           # conduct build
cmake --build --preset devel-qt6 --target check  # run tests
cmake --build --preset devel-qt6 --target tidy   # apply formatting
```

Note that these presets are supposed to cover all of my projects (so some of them aren't really making a
difference when just building `c++utilities` itself). To use presets in other projects, simply symlink the
file `CMakePresets.json` into the source directory of those projects. This is also done by the "subdirs"
projects mentioned in the previous section.

After invoking the configuration via the command-line, you can also open the project in Qt Creator and import
it as an existing build (instead of adding a new build configuration).

##### Remarks for building on Windows
To create a development build on Windows, it is most straight forward to use the `devel-qt6` preset in a
MSYS2 mingw64 shell. Set the `BUILD_DIR` environment variable to specify the directory to store build
artefacts.

Run the following commands to build one of my applications and its `c++utilities`/`qtutilities` dependencies
in one go (in this example Syncthing Tray):
```
# install dependencies; you may strip down this list depending on the application and features to enable
pacman -Syu git perl-YAML mingw-w64-x86_64-gcc mingw-w64-x86_64-ccache mingw-w64-x86_64-cmake mingw-w64-x86_64-boost mingw-w64-x86_64-cppunit mingw-w64-x86_64-qt6-base mingw-w64-x86_64-qt6-declarative mingw-w64-x86_64-qt6-tools mingw-w64-x86_64-qt6-svg mingw-w64-x86_64-clang-tools-extra mingw-w64-x86_64-doxygen mingw-w64-x86_64-ffmpeg mingw-w64-x86_64-go mingw-w64-x86_64-libarchive

# clone repositories as mentioned under "Building this straight" in the application's README file
cd /path/to/store/sources
...
git clone ...
...

# configure and invoke the build
cd subdirs/syncthingtray
cmake --preset devel-qt6
cmake --build "$BUILD_DIR/syncthingtray/devel-qt6" devel-qt6 -- -v
```

Run the following commands to build libraries individually (in this example `tagparser`) and
installing them in some directory (in this example `$BUILD_DIR/install`) for use in another
project:
```
# install dependencies
pacman -Syu git mingw-w64-x86_64-gcc mingw-w64-x86_64-ccache mingw-w64-x86_64-cmake mingw-w64-x86_64-boost mingw-w64-x86_64-cppunit

# clone relevant repositories, e.g. here just tagparser and its dependency c++utilities
cd /path/to/store/sources
git config core.symlinks true
git clone https://github.com/Martchus/cpp-utilities.git c++utilities
git clone https://github.com/Martchus/tagparser.git

# configure and invoke the build and installation of the projects individually
cmake --preset devel-qt6 -S c++utilities -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/install"
cmake --build "$BUILD_DIR/c++utilities/devel-qt6" --target install -- -v
ln -rs c++utilities/CMakePresets.json tagparser/CMakePresets.json
cmake --preset devel-qt6 -S tagparser -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/install"
cmake --build "$BUILD_DIR/tagparser/devel-qt6" --target install -- -v
```

Note that:
* Not all those dependencies are required by all my projects and some are just optional.
    * The second example to just build `c++utilities` and `tagparser` already shows a stripped-down list
      of dependencies.
    * Especially `mingw-w64-x86_64-go` is only required when building Syncthing Tray with built-in
      Syncthing-library enabled. To build in an MSYS2 shell one needs to invoke `export GOROOT=/mingw64/lib/go`
      so Go can find its root.
    * All Qt-related dependencies are generally only required for building with Qt GUI, e.g. Tag Editor
      and Password Manager can be built without Qt GUI. The libraries `c++utilities` and `tagparser` don't
      require Qt at all.
* You can also easily install Qt Creator via MSYS2 using `pacman -S mingw-w64-x86_64-qt-creator`.
* You must *not* use the presets containing `mingw-w64` in their name as those are only intended for cross-compilation
  on Arch Linux.

###### Building with MSVC
To build with MSVC you can use the `win-x64-msvc-static` preset. This preset (and all presets inheriting from it) need
various additional environment variables to be set and you need to install dependencies from various sources:
* `MSYS2_ROOT`: for Perl (only used by `qtforkawesome` so far), `clang-format`, Doxygen, FFmpeg and Go (only
  used by `libsyncthing`) provided via MSYS2 packages; install the following packages:
  ```
  pacman -Syu perl-YAML mingw-w64-x86_64-clang-tools-extra mingw-w64-x86_64-doxygen mingw-w64-x86_64-ffmpeg mingw-w64-x86_64-go
  ```
* `MSVC_ROOT`: for compiler and stdlib usually installed as part of Visual Studio setup, e.g.
  `C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.34.31933`
* `WIN_KITS_ROOT`: for Windows platform headers/libraries usually installed as part of Visual Studio setup,
  e.g. `C:/Program Files (x86)/Windows Kits/10`
* `WIN_KITS_VERSION`: the relevant subdirectory within `WIN_KITS_ROOT`, usually a version number like `10.0.22621.0`
* `QT_ROOT`: for Qt libraries provided by the official Qt installer, e.g. `D:/programming/qt/6.5.0/msvc2019_64`
* `QT_TOOLS`:  for additional build tools provided by the official Qt installer, e.g. `D:/programming/qt/Tools`
* `VCPKG_ROOT`: directory of VCPKG checkout used for other dependencies; install the following packages:
  ```
  vcpkg install boost-system:x64-windows-static boost-iostreams:x64-windows-static boost-filesystem:x64-windows-static boost-hana:x64-windows-static boost-process:x64-windows-static boost-asio:x64-windows-static libiconv:x64-windows-static zlib:x64-windows-static openssl:x64-windows-static cppunit:x64-windows-static libarchive'[bzip2,crypto,zstd]':x64-windows-static
  ```

When building with MSVC, do *not* use any of the MSYS2 shells. The environment of those shells leads to
build problems. You can however use CMake and Ninja from MSYS2's mingw-w64 packaging (instead of the CMake
version from Qt's installer). Then you need to specify the Ninja executable manually so the CMake invocation
would become something like this:
```
`& "$Env:MSYS2_ROOT\mingw64\bin\cmake.exe" --preset win-x64-msvc-static -DCMAKE_MAKE_PROGRAM="$Env:MSYS2_ROOT\mingw64\bin\ninja.exe" .
```

To run the resulting binaries, you'll need to make sure the Qt libraries are in the search path, e.g. using
`$Env:PATH = "$Env:QT_ROOT\bin"`.

Note that you don't need to install all Visual Studio has to offer. A customized installation with just
C++ core features, MSVC x86/x64 build tools, Windows SDK and vpkg should be enough. In Qt's online installer
you can also uncheck everything except the MSVC build of Qt itself.

If the compilation of the resource file doesn't work you can use `-DWINDOWS_RC_FILE=OFF` to continue the
build regardless.

##### Remarks about special presets
The presets starting with `arch-` are for use under Arch Linux. Do *not* use them unless you know what you
are doing. When creating a normal build under Arch Linux it is recommended to still use e.g. `devel-qt6`.

Use the presets starting with `arch-*-w64-mingw32` to cross-compile for Windows using `mingw-w64` packages.
Use the presets starting with `arch-static-compat-devel` to create a self-contained executable that is also
usable under older GNU/Linux distributions using `static-compat` packages (see
[PKGBUILDs](https://github.com/Martchus/PKGBUILDs#static-gnulinux-libraries) for details about it).

### Packaging
The mentioned repositories contain packages for `c++utilities` itself but also for my other projects.
However, the README files of my other projects contain a more exhaustive list.

#### Arch Linux package
The repository [PKGBUILDs](https://github.com/Martchus/PKGBUILDs) contains files for building Arch Linux
packages of the latest release and the Git master.

PKGBUILDs to cross compile for Android, Windows (using mingw-w64) and for MacOS X (using osxcross) are
included as well.

#### RPM packages for openSUSE and Fedora
RPM \*.spec files can be found at [openSUSE Build Servide](https://build.opensuse.org/project/show/home:mkittler).
Packages are available for several architectures.

There is also a [sub project](https://build.opensuse.org/project/show/home:mkittler:vcs) containing the builds
from the Git master branch.

#### Gentoo
Checkout [Case_Of's overlay](https://codeberg.org/Case_Of/gentoo-overlay)
or [perfect7gentleman's overlay](https://gitlab.com/Perfect_Gentleman/PG_Overlay).

## Copyright notice and license
Copyright © 2015-2024 Marius Kittler

All code is licensed under [GPL-2-or-later](LICENSE).
