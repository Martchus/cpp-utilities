cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# add project files
set(HEADER_FILES
    application/argumentparser.h
    application/commandlineutils.h
    application/failure.h
    application/fakeqtconfigarguments.h
    application/global.h
    chrono/datetime.h
    chrono/period.h
    chrono/timespan.h
    chrono/format.h
    conversion/binaryconversion.h
    conversion/binaryconversionprivate.h
    conversion/conversionexception.h
    conversion/stringconversion.h
    conversion/stringbuilder.h
    conversion/types.h
    conversion/widen.h
    io/ansiescapecodes.h
    io/binaryreader.h
    io/binarywriter.h
    io/bitreader.h
    io/copy.h
    io/inifile.h
    io/path.h
    io/catchiofailure.h
    io/nativefilestream.h
    io/misc.h
    math/math.h
    misc/memory.h
    misc/multiarray.h
    misc/random.h
    misc/traits.h
    misc/levenshtein.h
    tests/testutils.h
    tests/cppunit.h
    tests/outputcheck.h
)
set(SRC_FILES
    application/argumentparserprivate.h
    application/argumentparser.cpp
    application/commandlineutils.cpp
    application/failure.cpp
    application/fakeqtconfigarguments.cpp
    chrono/datetime.cpp
    chrono/period.cpp
    chrono/timespan.cpp
    conversion/conversionexception.cpp
    conversion/stringconversion.cpp
    io/ansiescapecodes.cpp
    io/binaryreader.cpp
    io/binarywriter.cpp
    io/bitreader.cpp
    io/inifile.cpp
    io/path.cpp
    io/catchiofailure.cpp
    io/nativefilestream.cpp
    io/misc.cpp
    math/math.cpp
    misc/random.cpp
    misc/levenshtein.cpp
    tests/testutils.cpp
)

set(TEST_HEADER_FILES

)
set(TEST_SRC_FILES
    tests/cppunit.cpp
    tests/conversiontests.cpp
    tests/iotests.cpp
    tests/chronotests.cpp
    tests/argumentparsertests.cpp
    tests/traitstests.cpp
    tests/mathtests.cpp
    tests/misctests.cpp
)

set(CMAKE_MODULE_FILES
    cmake/modules/BasicConfig.cmake
    cmake/modules/ConfigHeader.cmake
    cmake/modules/LibraryTarget.cmake
    cmake/modules/TestTarget.cmake
    cmake/modules/AppTarget.cmake
    cmake/modules/WindowsResources.cmake
    cmake/modules/TemplateFinder.cmake
    cmake/modules/Doxygen.cmake
    cmake/modules/ListToString.cmake
    cmake/modules/ShellCompletion.cmake
    cmake/modules/3rdParty.cmake
)
set(CMAKE_TEMPLATE_FILES
    cmake/templates/bash-completion.sh.in
    cmake/templates/Config.cmake.in
    cmake/templates/SharedConfig.cmake.in
    cmake/templates/StaticConfig.cmake.in
    cmake/templates/config.h.in
    cmake/templates/desktop.in
    cmake/templates/appdata.xml.in
    cmake/templates/doxygen.in
    cmake/templates/global.h.in
    cmake/templates/template.pc.in
)
set(SCRIPT_FILES
)
if(MINGW)
    list(APPEND CMAKE_TEMPLATE_FILES
        cmake/templates/windows.rc.in
    )
    list(APPEND SCRIPT_FILES
        scripts/wine.sh
    )
endif()

set(DOC_FILES
    README.md
    doc/buildvariables.md
    doc/testapplication.md
)
set(EXTRA_FILES
    tests/calculateoverallcoverage.awk
    coding-style.clang-format
)

# required to include CMake modules from own project directory
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_MODULE_PATH}")

# meta data
set(META_PROJECT_NAME c++utilities)
set(META_PROJECT_VARNAME CPP_UTILITIES)
set(META_APP_NAME "C++ Utilities")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "Useful C++ classes and routines such as argument parser, IO and conversion utilities")
set(META_FEATURES_FOR_COMPILER_DETECTION_HEADER cxx_thread_local)
set(META_VERSION_MAJOR 4)
set(META_VERSION_MINOR 15)
set(META_VERSION_PATCH 0)

# find required 3rd party libraries
include(3rdParty)
use_iconv(AUTO_LINKAGE REQUIRED)

# configure use of native file buffer
option(USE_NATIVE_FILE_BUFFER "enables use of native file buffer under Windows, affects ABI (required for unicode filenames under Windows)" OFF)
if(USE_NATIVE_FILE_BUFFER)
    list(APPEND META_PUBLIC_COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_USE_NATIVE_FILE_BUFFER)
endif()

# configure forcing UTF-8 code page under Windows
option(FORCE_UTF8_CODEPAGE "forces use of UTF-8 code page under Windows via ApplicationUtilities::startConsole()" OFF)
if(FORCE_UTF8_CODEPAGE)
    list(APPEND META_PRIVATE_COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_FORCE_UTF8_CODEPAGE)
endif()

# configure whether escape codes should be enabled by default
option(ENABLE_ESCAPE_CODES_BY_DEAULT "enables usage of escape codes by default" ON)
if(ENABLE_ESCAPE_CODES_BY_DEAULT)
    set_source_files_properties(
        application/argumentparser.cpp
        io/ansiescapecodes.cpp
        tests/argumentparsertests.cpp
        PROPERTIES COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_ESCAPE_CODES_ENABLED_BY_DEFAULT
    )
else()
    message(STATUS "Disabling use of escape codes by default.")
endif()

# configure use of thread_local
option(ENABLE_THREAD_LOCAL "enables use of Thread-Local Storage" ON)
if(NOT ENABLE_THREAD_LOCAL)
    set_source_files_properties(
        conversion/stringconversion.cpp
        PROPERTIES COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_NO_THREAD_LOCAL
    )
endif()

# include modules to apply configuration
include(BasicConfig)
include(WindowsResources)
include(LibraryTarget)
include(TestTarget)
include(Doxygen)
include(ConfigHeader)
