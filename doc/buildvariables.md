# Build system

\brief Documents variables to control the build system and provided CMake
       modules

## Variables passable as CMake arguments

### Useful variables provided by CMake itself
* `CMAKE_INSTALL_PREFIX=path`: specifies the final install prefix (temporary
  install prefix is set via `make` argument `DESTDIR=path`)
* `CMAKE_BUILD_TYPE=Release/Debug`: specifies whether to do a debug or a release
  build
* `BUILD_SHARED_LIBS=ON/OFF`: whether to build shared libraries (`ON`) or static
  libraries (`OFF`); it is not possible to build both at the same time within the
  same build process
* `CMAKE_SKIP_BUILD_RPATH=OFF`: ensures the rpath is set in the build tree
* `CMAKE_INSTALL_RPATH=rpath`: sets the rpath used when installing
* `CMAKE_CXX_FLAGS`: sets flags to be passed to the C++ compiler
* `CMAKE_FIND_LIBRARY_SUFFIXES`: sets the library suffixes the build script will
  consider, e.g. set to `.a;.lib` to prefer static Windows libraries or to
  `.dll;.dll.a` to prefer shared Windows libraries
* variables provided by [GNUInstallDirs](https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html)

### Custom variables
The following variables are read by the CMake modules provided by `c++utilities`
and `qtutilities`.

None of these are enabled or set by default, unless stated otherwise.

* `LIB_SUFFIX=suffix`: suffix for library install directory
* `LIB_SUFFIX_32=suffix`: suffix for library install directory
    * used when building for 32-bit platforms
    * overrides general `LIB_SUFFIX` when building for 32-bit platforms
* `LIB_SUFFIX_64=suffix`: suffix for library install directory
    * used when building for 64-bit platforms
    * overrides general `LIB_SUFFIX` when building for 64-bit platforms
* `SHELL_COMPLETION_ENABLED=ON/OFF`: enables shell completion in general
  (enabled by default)
* `BASH_COMPLETION_ENABLED=ON/OFF`: enables Bash completion (enabled by
  default)
* `LOGGING_ENABLED=ON/OFF`: enables further loggin in some applications
* `FORCE_OLD_ABI=ON/OFF`: forces use of old C++ ABI
    * sets `_GLIBCXX_USE_CXX11_ABI=0`
    * only relevant when using libstdc++
* `EXCLUDE_TESTS_FROM_ALL=ON/OFF`: excludes tests from the all target
  (enabled by default)
* `APPEND_GIT_REVISION=ON/OFF`: whether the build script should attempt to
  append the Git revision and the latest commit ID to the version
    * displayed via `--help`
    * enabled by default but has no effect when the source directory is
      no Git checkout or Git is not installed
* `CLANG_FORMAT_ENABLED=ON/OFF`: enables tidy target for code formatting via
  `clang-format`
    * can be made unavailable by setting `META_NO_TIDY` in the project file
    * only available if format rules are available
    * also enables tidy check executed via `check` target
* `CMAKE_FORMAT_ENABLED=ON/OFF`: enables tidy target for code formatting via
  `cmake-format`
    * can be made unavailable by setting `META_NO_TIDY` in the project file
    * options can be adjusted by setting `META_CMAKE_FORMAT_OPTIONS` in the
      project file
* `CLANG_TIDY_ENABLED=ON/OFF`: enables target `static-check` for static code
  analysis with `clang-tidy`
    * can be made unavailable by setting `META_NO_STATIC_ANALYSIS` in the
      project file
    * the variable [CMAKE_<LANG>_CLANG_TIDY](https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_CLANG_TIDY.html#variable:CMAKE_%3CLANG%3E_CLANG_TIDY)
      provided by CMake itself provides a similar functionality
* `CLANG_SOURCE_BASED_COVERAGE_ENABLED=ON/OFF`: enables `coverage` target to
  determine source-based test coverage using Clang/llvm
    * only available when building with Clang under UNIX
    * coverage report is stored in build directory
* `ENABLE_INSTALL_TARGETS=ON/OFF`: enables creation of install targets (enabled
  by default)
* `ENABLE_ESCAPE_CODES_BY_DEAULT=ON/OFF`: enables use of escape codes for formatted
  output by default
    * enabled by default
    * see ApplicationUtilities::NoColorArgument and EscapeCodes::enabled
    * has to be set when building `c++utilities`; projects using that build of
      `c++utilities` will then use this default
* `USE_NATIVE_FILE_BUFFER=ON/OFF`: use native functions to open file streams
    * Allows opening files which have non-ASCII in the path under Windows.
    * Allows to use native file descriptors which is required to open files
      from URLs provided by Android's Storage Access Framework.
    * Changing this option alters the ABI of `c++utilities` and other libraries
      exposing that ABI (eg. `tagparser` and `passwordfile`).
    * This feature is implemented in `c++utilities`, so the option must be specified
      when building `c++utilities`. Specifying it only when building eg. `tagparser`
      has *no* effect.
* `USE_STANDARD_FILESYSTEM=ON/OFF`: enables use of `std::filesystem` (default)
    * Disabing this is required when building for MacOS and Android at the time of
      writing this documentation.
    * Bash completion will not be able to suggest files and directory when disabled.
* `USER_DEFINED_ADDITIONAL_LIBRARIES`: specifies additional libraries to link against
  (added after any other libraries to the linker line)
    * Use case: Some platforms/standard libraries/compilers require linking against
      additional libraries for certain features. Adding additional libraries for
      `std::filesystem` is covered by use_standard_filesystem(). However, it makes no
      sense to add such a function for every specific platform/feature. For instance
      it seems to be required to add "-latomic" to the linker line for
      armv7-none-linux-androideabi/libc++/clang (not for aarch64-none-linux-androidabi)
      but it is better to cover such details on packaging-level and only allow to pass
      such flags here.
    * Using `CMAKE_EXE_LINKER_FLAGS` or `CMAKE_SHARED_LINKER_FLAGS` is often not helpful
      because the additional flags need to be added at the end of the linker line most
      of the time.
* `CONFIGURATION_NAME`: specifies a name to be incorporated into install paths
    * Builds with different configuration names can be installed alongside within the
      same install prefix.
    * Use cases
        * Installing static and shared libraries within the same prefix.
        * Installing different versions of the library within the same prefix.
        * Installing the version linked against Qt 5 and the version linked against
          Qt 6 within the same prefix (when Qt 6 is released).
    * Does not affect library names, because their file names might differ anyways
      between different configurations (e.g. static vs. shared libraries).
    * Set `CONFIGURATION_TARGET_SUFFIX` in accordance so library names are affected
      as well.
    * Set `CONFIGURATION_PACKAGE_SUFFIX` to *use* libraries built with
      `CONFIGURATION_NAME`.

#### Variables for specifying location of 3rd party dependencies
The build script tries to find the required dependencies at standard loctions
using the CMake functions
[`find_library`](https://cmake.org/cmake/help/latest/command/find_library.html)
and
[`find_package`](https://cmake.org/cmake/help/latest/command/find_package.html).
The behaviour of those functions can be controlled by setting some variables, eg.
using a toolchain file. Checkout the CMake documentation for this.

If the detection does not work as expected or a library from a non-standard
location should be used one can also just pass the library path directly by specifying
the relevant CMake cache variable on the CMake invocation.

#### Windows specific
* `FORCE_UTF8_CODEPAGE=ON/OFF`: forces use of UTF-8 codepage in terminal
* `WINDOWS_RESOURCES_ENABLED=ON/OFF`: enables creating resources for
  application meta data and icon (enabled by default)

#### MacOS specific
* `BUNDLE_INSTALL_DESTINATION=/some/path`: specifies the install destination for
  application bundles; if not specified, the default bin directory is used

#### Qt specific
* `WIDGETS_GUI=ON/OFF`: enables Qt Widgets GUI for projects where it is
  available and optional
* `QUICK_GUI=ON/OFF`: enables Qt Quick GUI for projects where it is available
  and optional
* `QT_PACKAGE_PREFIX=Qt5`: sets the prefix for Qt packages, by default `Qt5`
    * use `QT_PACKAGE_PREFIX=Qt6` to build against Qt 6
* `KF_PACKAGE_PREFIX=KF5`: sets the prefix for KDE Frameworks packages, by
  default `KF5`
* `ENABLE_QT_TRANSLATIONS=ON/OFF`: enables translations for Qt applications,
  enabled by default
* `BUILTIN_TRANSLATIONS=ON/OFF`: enables built-in translations instead of
  installing translations as separate files
* `BUILTIN_TRANSLATIONS_OF_QT=ON/OFF`: enables built-in translations also for Qt's
  own translation files when building an application
    * enabled by default when `BUILTIN_TRANSLATIONS` is enabled
    * useful when deploying Qt itself anyways, e.g. Windows builds
    * better turned off when using system Qt and the system provides a translations
      package for Qt, e.g. GNU/Linux builds
    * when building a library Qt's translations are never built-in
    * has no effect when `BUILTIN_TRANSLATIONS` is disabled
* `BUILTIN_ICON_THEMES=breeze;breeze-dark;...`: specifies icon themes to
  built-in
* `BUILTIN_ICON_THEMES_IN_LIBRARIES=breeze;breeze-dark;...`: same as above but
  also affects libraries
* `SVG_SUPPORT=ON/OFF`: enables linking against the static SVG image format
  plugin
    * enabled by default
    * requires the Qt Svg module
    * only relevant when using static Qt
* `SVG_ICON_SUPPORT=ON/OFF`: enables linking against the static SVG icon engine
  plugin provided by the Qt Svg module
    * enabled by default
    * requires the Qt Svg module
    * only relevant when using static Qt
    * required to use Breeze icon theme (or any other SVG icon theme)
* `IMAGE_FORMAT_SUPPORT`: enables linking against the specified static image
  format plugins
    * comma-separated list
    * by default set to "Gif;ICO;Jpeg", so support for Gif, ICO and Jpeg is
      enabled by default
    * note that PNG support is not provided via a plugin, so it should be
      always available and not be affected by this option
    * further image formats require building the plugins contained by the
      additional `qtimageformats` repository
    * only relevant when using static Qt
* `QT_PLUGIN_DIR`: specified the directory to install Qt plugins to
    * When installing plugins to a non-standard directory (e.g. during development)
      set the `QT_PLUGIN_PATH` environment variable accordingly so the plugin can
      be found.
    * The install directory defaults to `qmake -query QT_INSTALL_PLUGINS` unless
      that directory would be outside the specified `CMAKE_INSTALL_PREFIX`.
* `WEBVIEW_PROVIDER=auto/webkit/webengine/none`: specifies the Qt module to use
  for the web view
* `JS_PROVIDER=qml/script/none`: specifies the Qt module to use
  for the JavaScript engine
* `WEBVIEW_PROVIDER=webengine/webkit/none`: specifies the Qt module to use
  for the built-in web view


## Variables to be set in project file
The following variables are read by the CMake modules provided by c++utilities
and qtutilities.

### Meta-data and overall project configuration
* `META_PROJECT_NAME=name`: specifies the project name which is used as the
  application/library name, mustn't contain spaces
* `META_APP_NAME=The Name`: specifies a more readible version of the project
  name used for instance in about dialog and desktop file
* `META_APP_AUTHOR`: specifies the author shown in for instance in about
  dialog
* `META_APP_DESCRIPTION`: specifies a description shown for instance in about
  dialog and desktop file
* `META_GENERIC_NAME`: specifies a generic name for the desktop file
* `META_VERSION_MAJOR/MINOR/PATCH=number`: specifies the application/library
  version, default is 0
* `META_PROJECT_TYPE=application/library/plugin/qtplugin`: specifies whether
  to build an application, a library or a plugin
* `META_CXX_STANDARD=11/14/..`: specifies the C++ version, default is 14
* `META_NO_TIDY`: disables availability of enabling formatting via
  `CLANG_FORMAT_ENABLED` for this project
* `META_NO_INSTALL_TARGETS`: the project is not meant to be installed, eg.
  private test helper; prevents creation of install targets
* `META_ADD_DEFAULT_CPP_UNIT_TEST_APPLICATION`: implicitely add a source file
  with a main()-entry point to test target for running CppUnit test cases

### Files
* `HEADER_FILES`/`SRC_FILES`: specifies C++ header/source files
* `TEST_HEADER_FILES`/`TEST_SRC_FILES`: specifies C++ header/source files of the
  tests
* `TS_FILES`: specifies Qt translations
* `RES_FILES`: specifies Qt resource files
* `DBUS_FILES`: specifies files for Qt DBus
* `WIDGETS_HEADER_FILES`/`WIDGETS_SRC_FILES`: specifies C++ header/source files
  only required for Qt Widgets GUI
* `QML_HEADER_FILES`/`QML_SRC_FILES`/`QML_RES_FILES`: specifies C++
  header/source files and Qt resouce files only required for Qt Quick GUI
* `EXCLUDED_FILES`: specifies C++ files which are part of the project (and therefore
  should be formatted by the `tidy` target, added to the documentation and considered
  for translations) but are excluded from the actual build; when adding files
  conditionally to the other `_FILES` variables, be sure to add them at least
  to `EXCLUDED_FILES`
* `DOC_FILES`: additional markdown files to be inlcuded in the documentation
  created via Doxygen; the first file is used as the main page
* `REQUIRED_ICONS`: names of the icons required by the application and the
  used libraries (can be generated with
  [findicons](https://github.com/Martchus/findicons))
* `CMAKE_MODULE_FILES`/`CMAKE_TEMPLATE_FILES`: specifies CMake modules/templates
  provides by the project; those files are installed so they can be used by
  other projects

### Additional build variables
* `META_PRIVATE_COMPILE_DEFINITIONS`/`META_PUBLIC_COMPILE_DEFINITIONS`: specifies
  private/public compile definitions
* `META_PRIVATE_COMPILE_OPTIONS`/`META_PUBLIC_COMPILE_OPTIONS`: specifies
  private/public compile options
* `META_SOVERSION`: specifies the soversion for libraries (defaults to
  `META_VERSION_MAJOR`)
* `PUBLIC_LIBRARIES`/`PRIVATE_LIBRARIES`: specifies the public/private libraries
   to link against
* `PUBLIC_INCLUDE_DIRS`/`PRIVATE_INCLUDE_DIRS`: specifies the public/private include
  directories
* `ADDITIONAL_QT_MODULES`: specifies additional Qt modules to link against
* `ADDITIONAL_KF_MODULES`: specifies additional KDE frameworks modules to link
  against
* `META_QT_VERSION`: specifies the minimum Qt version to require

## Provided modules
c++utilities and qtutilities provide CMake modules to reduce boilerplate code
in the CMake files of my projects. Those modules implement the functionality
controlled by the variables documented above. Most important modules are:

* `BaseConfig`: does basic configuration, reads most of the `META`-variables
* `QtConfig`: does basic Qt-related configuration, reads most of the Qt-specific
  variables documented above
* `QtGuiConfig`: does Qt-related configuration for building a Qt Widgets or
  Qt Quick based GUI application/library
    * must be included *before* `QtConfig`
* `WebViewProviderConfig`: configures the webview provider
    * used by Tag Editor and Syncthing Tray to select between Qt WebEngine,
      Qt WebKit or disabling the built-in webview
* `LibraryTarget`: does further configuration for building dynamic and static
  libraries and plugins; `META_PROJECT_TYPE` can be left empty or set explicitely
  to `library`
* `AppTarget`: does further configuration for building an application;
  `META_PROJECT_TYPE` must be set to `application`
* `ShellCompletion`: enables shell completion
    * only works when using the argument parser provided by the
      `ApplicationUtilities::ArgumentParser` class of course
* `TestTarget`: adds the test target `check`
    * `check` target is *not* required by target `all`
    * test target is comprised of files specified in `TEST_HEADER_FILES`/`TEST_SRC_FILES`
      variables
    * test target will automatically link against `cppunit` which is the test
      framework used by most my projects; set `META_NO_CPP_UNIT=OFF` in the project
      file to prevent this
* `TestUtilities`: provides the `configure_test_target` macro for creating test
  targets manually
    * see `CMakeLists.txt` of `qtutilities` for an example
* `AndroidApk`: adds a target to create an APK package for Android using
  androiddeployqt, ignored on other platforms
* `Doxygen`: adds a target to generate documentation using Doxygen
* `WindowsResources`: handles creation of Windows resources to set application
  meta data and icon, ignored on other platforms
* `ConfigHeader`: generates `resources/config.h`, must be included as the last
  module (when all configuration is done)

Since those modules make use of the variables explained above, the modules must
be included *after* setting those variables. The inclusion order of the modules
matters as well.

For an example, checkout the project file of c++utilities itself. The project
files of [Syncthing Tray](https://github.com/Martchus/syncthingtray) should
cover everything (library, plugin, application, tests, desktop file, Qt
resources and translations, ...).
