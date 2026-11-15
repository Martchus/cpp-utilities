---
name: cmake-commands
description: |
  Essential rules and commands for building my C++ projects using CMake. Use
  whenever the user asks to build, compile, or run the project.
triggers:
  - build
  - compile
  - cmake
  - c++utilities
  - qtutilities
  - qtforkawesome
  - syncthingtray
  - passwordfile
  - passwordmanager
  - tagparser
  - tageditor
  - auto-makepkg
  - pianobooster
---

# C++ CMake build instructions
When the project needs to be built, you must follow these rules strictly (unless
explicitly instructed otherwise).

## General rules and constraints
* **Use an existing build:** Do NOT configure and conduct a 2nd build. Instead,
  use the commands mentioned in the "Build command" section which make use of an
  existing build.
* **Ask when unsure:** If the relevant build command for the current project and
  build host and relevant target cannot be clearly determined, ask the user.
* **Do not get hung up on general build problems:** If e.g. a dependency is
  missing, do not try to fix or work around it. Ask the user to take care of it,
  e.g. to install a package for the missing dependency. Do NOT install missing
  packages yourself. Do NOT change the configuration or environment yourself.
* **Variables:** The variable `BUILD_DIR` is expected to be set in the
  environment, pointing to the directory where builds for all projects and
  CMake presets are found. If `BUILD_DIR` is unset or empty, ask the user rather
  than guessing or configuring a new build directory in-tree.

## Overall checkout and build structure
We usually build an application (e.g. `syncthingtray`) together with utility
libraries (e.g. `c++utilities` and `qtutilities`). All relevant checkouts are
created next to each other, so from e.g. `syncthingtray` you can access files
from `c++utilities` via `../c++utilities`. As we build utility libraries
together with the main application repository, changes made in e.g.
`../c++utilities` and `../qtutilities` are automatically covered and hence
effective when using the build command mentioned in the next section. There is
no need to build utility libraries separately.

Do NOT investigate system paths like `/usr/include/c++utilities` to explore
utility libraries, as this is NOT the version being built.

## Build command
Use the following command structure to execute the build:
```
cmake --build "$BUILD_DIR"/<project-name>/<preset>
```

You must use the correct project name and CMake preset based on the platform and
current context (see further sub sections).

You may invoke CMake without `--build` using `-D` flags to change build options.
However, this is normally NOT needed and should only been done if necessary.

You may invoke CMake with `--target …` to invoke additional targets.

Example (debug Qt-based build of Syncthing Tray under Arch Linux):
```
cmake --build "$BUILD_DIR"/syncthingtray/debug-kde
```

### Project name
Substitute `project-name` as follows:

* `syncthingtray`: use when developing `c++utilities`, `qtutilities` or
  `qtforkawesome` for `syncthingtray` or `syncthingtray` itself
* `passwordmanager`: use when developing `c++utilities`, `qtutilities` or
  `passwordfile` for `passwordmanager` or `passwordmanager` itself
* `tageditor`: use when developing `c++utilities`, `qtutilities` or
  `tagparser` for `tageditor` or `tageditor` itself
* `auto-makepkg`: use when developing `c++utilities` or `reflective-rapidjson`
  for `auto-makepkg` or `auto-makepkg` itself
* `pianobooster`: use when developing `c++utilities` or `qtutilities`
  for `PianoBooster` or `PianoBooster` itself
* For any other project not listed above, `project-name` matches the project's
  checkout directory name.

### Preset
Substitute `preset` as follows:

* `debug-qt6`: use for development builds unless any of the next points is a
  better match
* `debug-kde`: use for development builds of `syncthingtray` under GNU/Linux
* `debug`: use for development builds using Qt 5 (only useful when testing
  compatibility with Qt 5 specifically, test that only if instructed)
* `arch-android-x86_64`: cross compile for Android/x86_64 under Arch Linux (only
  use that to test modifications in Android-specific code)
  * You must ensure the correct environment: `source android-env x86-64`
* `arch-android-aarch64`: cross compile for Android/ARM under Arch Linux (only
  use that if dealing with problems specific to Android/ARM)
  * You must ensure the correct environment: `source android-env aarch64`
* `arch-x86_64-w64-mingw32-static-qt6`: cross compile for Windows under Arch
  Linux (only use that to test modifications in Windows-specific code)
* `arch-mingw-w64-clang-devel-static-aarch64-w64-mingw32`: cross compile for
  Windows on ARM under Arch Linux (only do that if dealing with problems
  specific to Windows on ARM)
  * You must use ensure the correct environment:
    `source mingw-clang-env aarch64-w64-mingw32`
* `arch-static-compat`: create a mostly statically linked GNU/Linux executable
  under Arch Linux (only do that if instructed)

## Ensure sources are tidied
Invoke CMake like this to ensure sources are consistently formatted:
```
cmake --build "$BUILD_DIR"/<project-name>/<preset>
```

Run this *before* doing changes to make sure you start on a clean state (and
commit the diff with the message `Apply clang-format` or `Apply cmake-format` if
needed before doing further changes).

Run this before making a commit to ensure the committed changes are formatted.

Avoid running this in the middle of development/testing to avoid triggering
unnecessary recompilations.

## Invoking tests and built executables
You can invoke tests and other built executables directly from their location
within the build directory. It is NOT needed to invoke the install target. Note
that running test executables directly will not automatically rebuild code
changes, so build the relevant target first before executing them.

To build everything and invoke all tests in one go, use the following command:
```
cmake --build "$BUILD_DIR"/<project-name>/<preset> --target check
```
This is usually NOT the fastest option, though.

To speed things up, it makes more sense to run only relevant tests by invoking
the specific test executable directly. It is even possible to execute only a
specific test class or function.

CppUnit examples:
```
"$BUILD_DIR"/syncthingtray/debug-kde/c++utilities/c++utilities-devel_tests --units IoTests
"$BUILD_DIR"/syncthingtray/debug-kde/syncthingtray/syncthingconnector/syncthingconnector-devel_tests --units IgnorePatternTests::testGreediness
```

Qt Test examples:
```
"$BUILD_DIR"/syncthingtray/debug-kde/qtutilities/qtutilities-devel_buttonoverlay_tests
"$BUILD_DIR"/syncthingtray/debug-kde/syncthingtray/syncthingwidgets/syncthingwidgets-devel_app_tests AppTests::test_advancedPage
```
