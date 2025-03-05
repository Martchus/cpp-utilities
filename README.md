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
  This can be safely done when building `c++utilities` *only* for applications that don't use `std::fstream`
  anyway such as Syncthing Tray. Then disabling makes sense to avoid depending on a `libstdc++` vendor
  extension or Boost.Iostreams. Otherwise disabling is not recommended as it will lead to the following
  limitations:
    * Error messages when opening files will not contain the cause of the error.
    * Opening files under paths with non-ASCII characters will not work on Windows.
    * Password Manager will not compile for Android.
* The Qt-based applications support bundling icon themes by specifying e.g.
  `BUILTIN_ICON_THEMES=breeze;breeze-dark`.
    * This variable must be set when building the application (not when building any of the libraries).
    * The specified icon themes need to be installed in the usual location. Otherwise, use e.g.
      `BUILTIN_ICON_THEMES_SEARCH_PATH=D:/programming/misc/breeze-icons/usr/share/icons` to specify the
      search path.
* For more details, checkout the documentation about build variables (in the
  [directory `doc`](https://github.com/Martchus/cpp-utilities/blob/master/doc/buildvariables.md) and
  in the Doxygen version accessible under "Related Pages").
* The repository [PKGBUILDs](https://github.com/Martchus/PKGBUILDs) contains build scripts for GNU/Linux,
  Android, Windows and MacOS X in form of Arch Linux packages using `ninja`. These scripts can be used as an
  example also when building under/for other platforms.

#### Windows-specific notes
* To create application icons the tool `ffmpeg`/`avconv` is required.
* Windows builds are mainly conducted using mingw-w64/GCC/LLVM so using them is recommended. Building with
  MSVC should be possible as well but it is not as well tested.
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

#### Using Visual Studio Code
The following sections document useful extensions. Most of them require the "vscode marketplace" which might
not be available in all distributions of Visual Studio Code (e.g. when using the Arch Linux `code` package one
needs to install the `code-marketplace` package in addition).

Note that most of this has only been tested under GNU/Linux so far.

##### clangd
To use `clangd` via Visual Studio Code install the
[extension](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) and
add `-DCMAKE_EXPORT_COMPILE_COMMANDS=1` to the CMake arguments. Then link the `compile_commands.json` file
from the build directory into the source directory. When using the `subdirs` project you can create a symlink
to the subdirs project in the individual projects (e.g.
`compile_commands.json -> ../subdirs/syncthingtray/compile_commands.json`) and a symlink to the actual build
directory in the subdirs project (e.g.
`compile_commands.json -> /…/build/presets/syncthingtray/arch-android-x86_64/compile_commands.json`).

##### Building and debugging
Install the extension [C/C++](https://github.com/microsoft/vscode-cpptools) from Microsoft. To avoid having
to create manual build tasks and launch configurations, install the
[CMake extension](https://github.com/microsoft/vscode-cmake-tools) from Microsoft.

The extension [Native Debug](https://github.com/WebFreak001/code-debug) works as well.

Presumably [CodeLLDB](https://github.com/vadimcn/codelldb) (which also supports Rust) would work as well
but I haven't tested that yet.

The repository [subdirs](https://github.com/Martchus/subdirs) contains example configuration covering
Syncthing Tray and a few other projects. It configures many tasks and launcher configurations for building and
more manually. Not all projects and targets are covered, though. So it makes most sense to rely on the CMake
extension to configure and build the various projects (using presets, see section below). A launcher config to 
launch the active/selected CMake target exists.

##### Qt
To work on my C++ projects that use Qt it might be useful to install their
[extension pack](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.qt).
Compile the code with `-DQT_QML_GENERATE_QMLLS_INI=ON` for the QML language server to work.

The repository [subdirs](https://github.com/Martchus/subdirs) also contains a GDB init script and
configuration for the extension [C/C++](https://github.com/microsoft/vscode-cpptools) to make pretty printing
of Qt data types work in GDB using the printers from KDevelop.

#### CMake presets
There are some generic [presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) available.

Use `cmake --list-presets` to list all presets. Note that some presets

* are specific to certain Arch Linux packaging found in the AUR and my PKGBUILDs repository. Those presets
  start with `arch-`.
* are specific to certain build setups/toolchains under Windows. Those presets start with `win-`.

All `cmake` commands need to be executed within the source directory. Builds will be created within a
sub-directory of the path specified via the environment variable `BUILD_DIR`.

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

##### Remarks about special presets
The presets starting with `arch-` are for use under Arch Linux (or an Arch Linux container). Do *not* use them
unless you know what you are doing. When creating a normal build under Arch Linux it is recommended to still
use e.g. `devel-qt6`.

Use the preset `arch-android` to cross-compile for Android using `android-*` packages (see next section for
details).

Use the presets starting with `arch-*-w64-mingw32` to cross-compile for Windows (i686/x86_64) using
`mingw-w64` packages. Use the presets starting with `arch-mingw-w64-clang` to cross compile for Windows
(aarch64) using `mingw-w64-clang-aarch64` packages.

Use the presets starting with `arch-static-compat-devel` to create a self-contained executable that is also
usable under older GNU/Linux distributions using `static-compat` packages (see
[PKGBUILDs](https://github.com/Martchus/PKGBUILDs#static-gnulinux-libraries) for details about it).

##### Remarks about building for Android
Note that this might not be necassary; one can usually develop and test most parts of the mobile UI of e.g.
Syncthing Tray natively on the development host thanks to the cross-platform nature of Qt.

I recommended building for Android under Arch Linux (or an Arch Linux container, see last paragraphs of this
section) using `android-*` packages found on the AUR and my
[binary repository](https://martchus.dyn.f3l.de/repo/arch/ownstuff). The commands in this section assume this
kind of build environment. For building on Windows, checkout the section
"[Building under Windows for Android](#building-under-windows-for-android)" below.

---

First, create a key for signing the APK (always required; otherwise the APK file won't install):
```
# set variables for creating a keystore and allowing androiddeployqt to find it
export QT_ANDROID_KEYSTORE_PATH=/path/to/keystore-dir/$USER-devel QT_ANDROID_KEYSTORE_ALIAS=$USER-devel QT_ANDROID_KEYSTORE_STORE_PASS=$USER-devel QT_ANDROID_KEYSTORE_KEY_PASS=$USER-devel

# create keystore (do this only once)
mkdir -p "${QT_ANDROID_KEYSTORE_PATH%/*}"
pushd "${QT_ANDROID_KEYSTORE_PATH%/*}"
keytool -genkey -v -keystore "$QT_ANDROID_KEYSTORE_ALIAS" -alias "$QT_ANDROID_KEYSTORE_ALIAS" -keyalg RSA -keysize 2048 -validity 10000
popd
```

Note that `QT_ANDROID_KEYSTORE_PATH` needs to point to a particular keystore *file* (and *not* the containing
directory).

---

Example for building `c++utilities`, `passwordfile`, `qtutilities` and `passwordmanager` in one step to create
an Android APK for aarch64 assuming required `android-*` packages are already installed:

```
# unset any potentially problematic Java options
export _JAVA_OPTIONS=

# configure and build using CMake presets and helpers from android-cmake package
source android-env aarch64
export BUILD_DIR=…
cd "$SOURCES/subdirs/passwordmanager"
cmake --preset arch-android -DBUILTIN_ICON_THEMES='breeze;breeze-dark'
cmake --build --preset arch-android

# install the app
adb install "$BUILD_DIR/passwordmanager/arch-android-arm64-v8a/android-build//build/outputs/apk/release/android-build-release-signed.apk"
```

---

To use a container you can create a suitable image using the `imgbuild` script from the PKGBUILDs repo, see
its [README](https://github.com/Martchus/PKGBUILDs/blob/master/README.md#container-image-building-packages-within-a-container).

After creating a container from that image like it is done in
[the example script from the PKGBUILDs repo](https://github.com/Martchus/PKGBUILDs/blob/master/devel/container/create-devel-container-example)
you can install required dependencies via `pacman`, e.g. for Syncthing Tray one would install:

```
podman container exec -it archlinux-devel-container \
  pacman -Syu clang ninja git extra-cmake-modules qt6-{base,tools,declarative,shadertools} android-cmake android-aarch64-qt6-{base,declarative,tools,translations,svg} go perl-yaml-libyaml
```

You use `keytool` from within the container in the same way as shown above:
```
podman container exec -it -e QT_ANDROID_KEYSTORE_PATH -e QT_ANDROID_KEYSTORE_ALIAS -e QT_ANDROID_KEYSTORE_STORE_PASS -e QT_ANDROID_KEYSTORE_KEY_PASS \
  archlinux-devel-container keytool …
```

When setting the environment variables, make sure `QT_ANDROID_KEYSTORE_PATH` points to the path of the kestore
file *within* the container.


Then the build can be invoked like this:

```
podman container exec -it -e QT_ANDROID_KEYSTORE_PATH -e QT_ANDROID_KEYSTORE_ALIAS -e QT_ANDROID_KEYSTORE_STORE_PASS -e QT_ANDROID_KEYSTORE_KEY_PASS \
 archlinux-devel-container \
 bash -c '
  cd /src/c++/cmake/subdirs/syncthingtray
  source android-env aarch64
  export BUILD_DIR=/build/presets
  cmake --preset arch-android
  cmake --build --preset arch-android'
```

You can also use `adb` from the container, see the
[examples in the PKGBUILDs repo](https://github.com/Martchus/PKGBUILDs/blob/master/README.md#deploydebug-android-package-using-tooling-from-android-sdk-package).

###### Further details
* The Android packages for the dependencies Boost, Qt, iconv, OpenSSL and Kirigami are provided on the AUR and
  by my [PKGBUILDs](http://github.com/Martchus/PKGBUILDs) repo.
* The latest Java version that is currently supported is version 17, see QTBUG-119223.
* Use `QT_QUICK_CONTROLS_STYLE=Material` and `QT_QUICK_CONTROLS_MOBILE=1` to test the Qt Quick GUI like it would
  be shown under Android via a normal desktop build.
* One can open the Gradle project that is created within the build directory in Android Studio and run the app in
  the emulator.

##### Remarks for building on Windows
To create a development build on Windows, it is most straight forward to use the `devel-qt6` preset. To create
a debug build (e.g. to debug with GDB) use the `debug-qt6` preset. Set the `BUILD_DIR` environment variable to
specify the directory to store build artefacts.

I recommended to conduct the build in an MSYS2 mingw64/ucrt64/… shell. There are different
[environments](https://www.msys2.org/docs/environments) to choose from. I recommended UCRT64 for my projects
but MINGW64 will work as well. In theory CLANG64 and CLANGARM64 will work as well but `libc++` is not tested as
much (especilly on Windows) so expect some tough edges. The 32-bit environments will not work for anything
requiring Qt 6 or later.

Run the following commands to build one of my applications and its `c++utilities`/`qtutilities` dependencies
in one go (in this example Syncthing Tray):
```
# set prefix of package names depending on what env you want to use, see https://www.msys2.org/docs/environments
prefix=mingw-w64-ucrt-x86_64      # in UCRT64 shell, recommended and used in all further examples
prefix=mingw-w64-x86_64           # in MINGW64 shell
prefix=mingw-w64-i686             # in MINGW32 shell
prefix=mingw-w64-clang-x86_64     # in CLANG64 shell
prefix=mingw-w64-clang-aarch64    # in CLANGARM64 shell

# install dependencies; you may strip down this list depending on the application and features to enable
pacman -Syu git perl-YAML-Tiny $prefix-gcc $prefix-ccache $prefix-cmake $prefix-boost $prefix-cppunit $prefix-qt6-base $prefix-qt6-declarative $prefix-qt6-tools $prefix-qt6-svg $prefix-clang-tools-extra $prefix-doxygen $prefix-ffmpeg $prefix-go $prefix-libarchive

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
prefix=mingw-w64-ucrt-x86_64
pacman -Syu git $prefix-gcc $prefix-ccache $prefix-cmake $prefix-boost $prefix-cppunit

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
    * Especially `…-go` is only required when building Syncthing Tray with built-in Syncthing-library enabled.
      To build in an MSYS2 shell one needs to invoke e.g. `export GOROOT=/ucrt64/lib/go` or
      `export GOROOT=/mingw64/lib/go` so Go can find its root.
    * All Qt-related dependencies are generally only required for building with Qt GUI, e.g. Tag Editor
      and Password Manager can be built without Qt GUI. The libraries `c++utilities` and `tagparser` don't
      require Qt at all.
* To run the binaries from the Windows terminal, you need to add the mingw-w64 libraries from the MSYS2
  installation to the path, e.g. `$Env:PATH = "$Env:MSYS2_ROOT\ucrt64\bin"` or
  `$Env:PATH = "$Env:MSYS2_ROOT\mingw64\bin"`.
* You can also easily install Qt Creator via MSYS2 using `pacman -S $prefix-qt-creator`. In Qt
  Creator you can import the build configured via presets on the command-line as existing build. This also
  works for the MSVC build mentioned below. This way not much tinkering in the Qt Creator settings is
  required. I had to set the debugger path to use GDB, though.
* You must *not* use the presets containing `mingw-w64` in their name as those are only intended for
  cross-compilation on Arch Linux.

###### Building with MSVC
To build with MSVC you can use the `win-x64-msvc-static` preset. This preset (and all presets inheriting from it) need
various additional environment variables to be set and you need to install dependencies from various sources:
* `MSYS2_ROOT` and `MSYS2_PREFIX`: for Perl (only used by `qtforkawesome` so far), `clang-format`, Doxygen, FFmpeg and
  Go (only used by `libsyncthing`) provided via MSYS2 packages; install the following packages:
  ```
  prefix=mingw-w64-ucrt-x86_64 # see "Remarks for building on Windows" for details and other options
  pacman -Syu perl-YAML $prefix-clang-tools-extra $prefix-doxygen $prefix-ffmpeg $prefix-go
  ```
    * `MSYS2_ROOT` must be set to the main install directory of MSYS2 (that also contains all the executables for the
      different shells/environments).
    * `MSYS2_PREFIX` must be set to the prefix of the environment you want to use. That is one of the values mentioned
      in the "Prefix" column on the [table of MSYS2 environments](https://www.msys2.org/docs/environments), e.g.
      `MSYS2_PREFIX=/ucrt64` for the UCRT64 environment.
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
would become something like this for UCRT64:
```
`& "$Env:MSYS2_ROOT\ucrt64\bin\cmake.exe" --preset win-x64-msvc-static -DCMAKE_MAKE_PROGRAM="$Env:MSYS2_ROOT\ucrt64\bin\ninja.exe" .
```
or for MINGW64:
```
`& "$Env:MSYS2_ROOT\mingw64\bin\cmake.exe" --preset win-x64-msvc-static -DCMAKE_MAKE_PROGRAM="$Env:MSYS2_ROOT\mingw64\bin\ninja.exe" .
```

To run the resulting binaries, you'll need to make sure the Qt libraries are in the search path, e.g. using
`$Env:PATH = "$Env:QT_ROOT\bin"`.

Note that you don't need to install all Visual Studio has to offer. A customized installation with just
C++ core features, MSVC x86/x64 build tools, Windows SDK and vcpkg should be enough. In Qt's online installer
you can also uncheck everything except the MSVC build of Qt.

If the compilation of the resource file doesn't work you can use `-DWINDOWS_RESOURCES_ENABLED=OFF` to continue
the build regardless.

###### Building under Windows for Android
Building for Android under Windows is still experimental and not regularly tested. It is generally supported
by CMake, Android tooling and Qt, though. So as long as all dependencies are installed correctly by *some*
means it is supposed to work. The following instructions describe how one could approach the installation of
the required dependencies.

---

To build for Android under Windows one needs to install the Android NDK and additional libraries depending
on the concrete library/app and wanted features. To build anything that depends on Qt one also needs the
Android SDK and Qt for Android.

<details>
<summary>Install Android SDK and NDK</summary>

The easiest way to install the SDK is to install [Android Studio](https://developer.android.com/studio).
Its setup wizard allows to install the SDK and other useful tools. The Gradle project files created by Qt can
also be opened with it. This setup will also use the version of Java that comes with Android Studio.

The NDK needs to be [downloaded separately](https://developer.android.com/ndk/downloads).
</details>

<details>
<summary>Install Qt</summary>

The easiest way to install Qt is via the official [Qt installer](https://www.qt.io/download-qt-installer-oss).
The open source version is sufficient but a Qt account is required.
</details>

<details>
<summary>Install additional native libraries for Android</summary>

Additional libraries can be installed via MSYS2 using my Arch Linux packaging. Note that this is not generally
required to build Syncthing Tray as use of libiconv, Boost, OpenSSL and CppUnit is optional (so only Qt is
required besides the C/C++ standard libraries). However, the following instructions and the CMake preset make
use of MSYS2 and the `android-cmake` package. The OpenSSL package is also very likely wanted for TLS support.

To install additional libraries via MSYS2, add my Arch Linux repository to
`/etc/pacman.conf`:

```
[ownstuff]
SigLevel = Required DatabaseOptional
Server = https://ftp.f3l.de/~martchus/$repo/os/$arch
Server = https://martchus.dyn.f3l.de/repo/arch/$repo/os/$arch
```

After following [instructions for importing my GPG key](https://martchus.dyn.f3l.de/repo/arch/ownstuff) you
can install Android packages, e.g.:

```
pacman -Syu android-cmake android-{x86-64,aarch64}-{boost,libiconv,openssl,cppunit} \
  --assume-installed android-ndk --assume-installed android-sdk
````

You may even install a few KDE libraries like Kirigami:
```
pacman -S android-{aarch64,x86-64}-kirigami --assume-installed=android-{aarch64,x86-64}-qt6-{base,declarative,shadertools,svg,5compat}
```

Whether this will actually work at runtime hasn't been tested yet. One definitely has to make sure that the
used version of Qt is at least as new as the version the KDE libraries from my repo have been linked against.

The libraries will end up under `/opt/android-libs` within your MSYS2 installation. Do not install any non
`android-*-` packages, though. They will have file conflicts with packages provided by MSYS2 and are not usable
under Windows anyway.

The Qt packages for Android cannot be used as well because they rely on the Qt packaging provided by Arch Linux
for tooling. (Maybe the Qt packages provided by MSYS2 mingw-w64 packages could be used for tooling. This hasn't
been tested yet, though.)

To search for available Android packages on my repo per architecture one can use e.g.
`pacman -Ss android-aarch64-`.
</details>

<details>
<summary>Conduct build</summary>

Set the following environment variables:

* `ANDROID_HOME`: path to the Android SDK
* `ANDROID_NDK_HOME`: path to the Android NDK
* `ANDROID_STUDIO_HOME`: Android studio install directory (for adding Java to `PATH` and setting `JAVA_HOME`)
* `QT_PLATFORMS_ROOT`: directory containing Qt platform directories installed via the official Qt installer,
  e.g. `D:/programming/qt/6.8.0`
* `QT_ANDROID_ARCH`: `x86_64`/`arm64_v8a`/`armv7`/`x86`
* `QT_ANDROID_KEYSTORE_PATH`: path of directory containing Android keystores
* `QT_ANDROID_KEYSTORE_ALIAS`: name of Android keystore to use
* `QT_ANDROID_KEYSTORE_STORE_PASS`: keystore store password
* `QT_ANDROID_KEYSTORE_KEY_PASS`: keystore key password
* `MSYS2_ROOT`: install directory of MSYS2

Then the build can be conducted in a MSYS2 shell, e.g.:

```
source android-env x86-64 # or aarch64
cmake --preset win-android
cmake --build --preset win-android
```
</details>

### Packaging
The repositories mentioned below contain packages for `c++utilities` itself but also for my other projects.
For a more comprehensive list of repositories providing my other projects such as Syncthing Tray, checkout
the README of those projects instead.

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
Copyright © 2015-2025 Marius Kittler

All code is licensed under [GPL-2-or-later](LICENSE).
