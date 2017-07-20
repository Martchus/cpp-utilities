# Build system

\brief Documents variables to control the build system and provided CMake
       modules

## Variables passable as CMake arguments

### Useful variables provided by CMake itself
* `CMAKE_INSTALL_PREFIX=path`: specifies the final install prefix (temporary
  install prefix is set via `make` argument `DESTDIR=path`)
* `CMAKE_BUILD_TYPE=Release/Debug`: specifies whether to do a debug or a release
  build
* `CMAKE_SKIP_BUILD_RPATH=OFF`: ensures the rpath is set in the build tree
* `CMAKE_INSTALL_RPATH=rpath`: sets the rpath used when installing

### Custom variables
The following variables are read by the CMake modules provided by c++utilities
and qtutilities.

None of these are enabled or set by default, unless stated otherwise.

* `LIB_SUFFIX=suffix`: suffix for library install directory
* `LIB_SUFFIX_32=suffix`: suffix for library install directory
    * used when building for 32-bit platforms
    * overrides general `LIB_SUFFIX` when building for 32-bit platforms
* `LIB_SUFFIX_64=suffix`: suffix for library install directory
    * used when building for 64-bit platforms
    * overrides general `LIB_SUFFIX` when building for 64-bit platforms
* `ENABLE_STATIC_LIBS=ON/OFF`: enables building static libs
* `DISABLE_SHARED_LIBS=ON/OFF`: disables building shared libs
* `STATIC_LINKAGE=ON/OFF`: enables linking applications *preferably* against
  static libraries
    * by default dynamic libraries are preferred
    * only affect building applications
* `STATIC_LIBRARY_LINKAGE=ON/OFF`: enables linking dynamic libraries *preferably*
  against static libraries
    * by default linking against dynamic libraries is preferred
    * only affects building dynamic libraries
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
    * displayed via --help
    * enabled by default but has no effect when the source directory is
      no Git checkout or Git is not installed
* `CLANG_FORMAT_ENABLED=ON/OFF`: enables tidy target for code formatting via
  `clang-format`
    * can be made unavailable by setting `META_NO_TIDY` in the project file
    * only available if format rules are available
    * also enables tidy check executed via `check` target
* `CLANG_SOURCE_BASED_COVERAGE_ENABLED=ON/OFF`: enables `coverage` target to
  determine source-based test coverage using Clang/llvm
    * only available when building with Clang under UNIX
    * coverage report is stored in build directory
* `ENABLE_INSTALL_TARGETS=ON/OFF`: enables creation of install targets (enabled
  by default)

#### Windows specific
* `USE_NATIVE_FILE_BUFFER=ON/OFF`: use native function to open file streams
  to pass unicode file names correctly, changing this alters ABI
* `FORCE_UTF8_CODEPAGE=ON/OFF`: forces use of UTF-8 codepage in terminal
* `WINDOWS_RESOURCES_ENABLED=ON/OFF`: enables creating resources for
  application meta data and icon (enabled by default)

#### Qt specific
* `WIDGETS_GUI=ON/OFF`: enables Qt Widgets GUI for projects where it is
  available and optional
* `QUICK_GUI=ON/OFF`: enables Qt Quick GUI for projects where it is available
  and optional
* `BUILTIN_TRANSLATIONS=ON/OFF`: enables built-in translations in applications
* `BUILTIN_ICON_THEMES=breeze;breeze-dark;...`: specifies icon themes to
  built-in
* `BUILTIN_ICON_THEMES_IN_LIBRARIES=breeze;breeze-dark;...`: same as above but
  also affects libraries
* `SVG_SUPPORT=ON/OFF`: enables SVG support if not enabled anyways by the
  project
* `SVG_ICON_SUPPORT=ON/OFF`: enables SVG icon support (only affect static
  builds where the required Qt plugin will be built-in if this variable is
  enabled)
* `WEBVIEW_PROVIDER=auto/webkit/webengine/none`: specifies the Qt module to use
  for the web view
* `JS_PROVIDER=auto/script/qml/none`: specifies the Qt module to use
  for the JavaScript engine
* `QT_LINKAGE=AUTO_LINKAGE/STATIC/SHARED`: specifies whether to use static
  or shared version of Qt (only works with Qt packages provided in the AUR)
* `ADDITIONAL_QT_MODULES=Network;Concurrent;...`: specifies additional Qt
  modules to link against (only use for modules which can not be added
  automatically)


## Variables to be set in project file
The following variables are read by the CMake modules provided by c++utilities
and qtutilities.

### Meta data
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
* `DOC_FILES`: additional markdown files to be inlcuded in the documentation
  created via Doxygen; the first file is used as the main page
* `DOC_ONLY_FILES`: specifies documentation-only files
* `REQUIRED_ICONS`: names of the icons required by the application and the
  used libraries (can be generated with `qtutilities/scripts/required_icons.sh`)
* `CMAKE_MODULE_FILES`/`CMAKE_TEMPLATE_FILES`: specifies CMake modules/templates
  provides by the project; those files are installed so they can be used by
  other projects

### Additional build variables
* `META_PRIVATE/PUBLIC_COMPILE_DEFINITIONS`: specifies private/public compile
  definitions
* `LIBRARIES`: specifies libraries to link against
* `META_PUBLIC_QT_MODULES`: specifies Qt modules used in the public library
  interface

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
    * test target uses files specified in `TEST_HEADER_FILES`/`TEST_SRC_FILES`
      variables
    * test target will automatically link against `cppunit` which is the test
      framework used by all my projects; set `META_NO_CPP_UNIT=OFF` in the project
      file to prevent this
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
