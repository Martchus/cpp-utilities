cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

project(c++utilities)

# add project files
set(HEADER_FILES
    application/argumentparser.h
    application/commandlineutils.h
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
    io/ansiescapecodes.h
    io/binaryreader.h
    io/binarywriter.h
    io/bitreader.h
    io/buffersearch.h
    io/copy.h
    io/inifile.h
    io/path.h
    io/nativefilestream.h
    io/misc.h
    misc/flagenumclass.h
    misc/math.h
    misc/multiarray.h
    misc/parseerror.h
    misc/traits.h
    misc/levenshtein.h
    tests/testutils.h
    tests/cppunit.h
    tests/outputcheck.h)
set(SRC_FILES
    application/argumentparserprivate.h
    application/argumentparser.cpp
    application/commandlineutils.cpp
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
    io/buffersearch.cpp
    io/inifile.cpp
    io/path.cpp
    io/nativefilestream.cpp
    io/misc.cpp
    misc/math.cpp
    misc/parseerror.cpp
    misc/levenshtein.cpp
    tests/testutils.cpp)

set(TEST_HEADER_FILES)
set(TEST_SRC_FILES
    tests/cppunit.cpp
    tests/conversiontests.cpp
    tests/iotests.cpp
    tests/chronotests.cpp
    tests/argumentparsertests.cpp
    tests/traitstests.cpp
    tests/mathtests.cpp
    tests/misctests.cpp)

set(CMAKE_MODULE_FILES
    cmake/modules/BasicConfig.cmake
    cmake/modules/ConfigHeader.cmake
    cmake/modules/LibraryTarget.cmake
    cmake/modules/TestUtilities.cmake
    cmake/modules/TestTarget.cmake
    cmake/modules/AppUtilities.cmake
    cmake/modules/AppTarget.cmake
    cmake/modules/WindowsResources.cmake
    cmake/modules/TemplateFinder.cmake
    cmake/modules/Doxygen.cmake
    cmake/modules/ListToString.cmake
    cmake/modules/ShellCompletion.cmake
    cmake/modules/DevelUtilities.cmake
    cmake/modules/3rdParty.cmake)
set(CMAKE_TEMPLATE_FILES
    cmake/templates/bash-completion.sh.in
    cmake/templates/Config.cmake.in
    cmake/templates/config.h.in
    cmake/templates/desktop.in
    cmake/templates/appdata.xml.in
    cmake/templates/doxygen.in
    cmake/templates/global.h.in
    cmake/templates/version.h.in
    cmake/templates/template.pc.in)
if (WIN32)
    list(APPEND CMAKE_TEMPLATE_FILES cmake/templates/windows.rc.in cmake/templates/windows-cli-wrapper.rc.in
         cmake/templates/cli-wrapper.cpp)
endif ()
set(EXCLUDED_FILES cmake/templates/cli-wrapper.cpp)

set(DOC_FILES README.md doc/buildvariables.md doc/testapplication.md)
set(EXTRA_FILES tests/calculateoverallcoverage.awk coding-style.clang-format)

# required to include CMake modules from own project directory
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_MODULE_PATH}")

# meta data
set(META_PROJECT_NAME c++utilities)
set(META_PROJECT_VARNAME CPP_UTILITIES)
set(META_APP_NAME "C++ Utilities")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "Useful C++ classes and routines such as argument parser, IO and conversion utilities")
set(META_VERSION_MAJOR 5)
set(META_VERSION_MINOR 27)
set(META_VERSION_PATCH 2)

# find required 3rd party libraries
include(3rdParty)
option(USE_ICONV "uses iconv/libiconv; if disabled character set conversions and features depending on it are not available"
       ON)
if (USE_ICONV)
    use_iconv(AUTO_LINKAGE REQUIRED)
else ()
    list(APPEND META_PUBLIC_COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_NO_ICONV)
endif ()

# do not rely on legacy behavior of FlagEnumClass (TODO v6: remove macro, make new behavior the default)
list(APPEND META_PRIVATE_COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_FLAG_ENUM_CLASS_NO_LEGACY_AND)

# configure use of native file buffer and its backend implementation if enabled
set(REQUIRED_BOOST_COMPONENTS "")
set(USE_NATIVE_FILE_BUFFER_BY_DEFAULT OFF)
if (WIN32
    OR ANDROID
    OR (UNIX AND NOT APPLE))
    set(USE_NATIVE_FILE_BUFFER_BY_DEFAULT ON)
endif ()
option(USE_NATIVE_FILE_BUFFER "enables use of native file buffer, affects ABI" ${USE_NATIVE_FILE_BUFFER_BY_DEFAULT})
option(FORCE_BOOST_IOSTREAMS_FOR_NATIVE_FILE_BUFFER "forces use of Boost.Iostreams for native file buffer" OFF)
if (USE_NATIVE_FILE_BUFFER)
    list(APPEND META_PUBLIC_COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_USE_NATIVE_FILE_BUFFER)
    set(NATIVE_FILE_STREAM_IMPL_FILES io/nativefilestream.cpp tests/iotests.cpp)

    # check whether __gnu_cxx::stdio_filebuf is available
    try_compile(
        GNU_CXX_STDIO_FILEBUF_AVAILABLE ${CMAKE_CURRENT_BINARY_DIR}
        SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/feature_detection/stdio_filebuf.cpp
        OUTPUT_VARIABLE GNU_CXX_STDIO_FILEBUF_CHECK_LOG)

    # use __gnu_cxx::stdio_filebuf if available or fallback to boost::iostreams::stream_buffer
    if (GNU_CXX_STDIO_FILEBUF_AVAILABLE AND NOT FORCE_BOOST_IOSTREAMS_FOR_NATIVE_FILE_BUFFER)
        message(STATUS "Using __gnu_cxx::stdio_filebuf for NativeFileStream")
        foreach (NATIVE_FILE_STREAM_IMPL_FILE ${NATIVE_FILE_STREAM_IMPL_FILES})
            set_property(
                SOURCE ${NATIVE_FILE_STREAM_IMPL_FILE}
                APPEND
                PROPERTY COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_USE_GNU_CXX_STDIO_FILEBUF)
        endforeach ()
    else ()
        message(STATUS "Using boost::iostreams::stream_buffer<boost::iostreams::file_descriptor_sink> for NativeFileStream")
        list(APPEND REQUIRED_BOOST_COMPONENTS iostreams)
        foreach (NATIVE_FILE_STREAM_IMPL_FILE ${NATIVE_FILE_STREAM_IMPL_FILES})
            set_property(
                SOURCE ${NATIVE_FILE_STREAM_IMPL_FILE}
                APPEND
                PROPERTY COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_USE_BOOST_IOSTREAMS)
        endforeach ()
    endif ()
else ()
    message(STATUS "Using std::fstream for NativeFileStream")
endif ()

# configure use of Boost.Process for launching test applications on Windows
if (WIN32)
    option(USE_BOOST_PROCESS "enables use of Boost.Process to launch test applications" ON)
    if (USE_BOOST_PROCESS)
        list(APPEND REQUIRED_BOOST_COMPONENTS filesystem)
        list(APPEND META_PUBLIC_COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_BOOST_PROCESS)
        list(APPEND PRIVATE_LIBRARIES ws2_32) # needed by Boost.Asio
        use_package(TARGET_NAME Threads::Threads PACKAGE_NAME Threads PACKAGE_ARGS REQUIRED)
    endif ()
endif ()

# configure usage of Boost
if (REQUIRED_BOOST_COMPONENTS)
    set(BOOST_ARGS REQUIRED COMPONENTS ${REQUIRED_BOOST_COMPONENTS})
    use_package(TARGET_NAME Boost::boost PACKAGE_NAME Boost PACKAGE_ARGS "${BOOST_ARGS}")
    foreach (COMPONENT ${REQUIRED_BOOST_COMPONENTS})
        use_package(TARGET_NAME Boost::${COMPONENT} PACKAGE_NAME Boost PACKAGE_ARGS "${BOOST_ARGS}")
    endforeach ()
endif ()

# configure required libraries for std::filesystem
option(USE_STANDARD_FILESYSTEM
       "uses std::filesystem; if disabled Bash completion for files and directories and archiving utilities are disabled" ON)
if (USE_STANDARD_FILESYSTEM)
    list(APPEND META_PRIVATE_COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_USE_STANDARD_FILESYSTEM)
    use_standard_filesystem()
else ()
    message(
        WARNING
            "The use of std::filesystem has been disabled. Bash completion for files and directories will not work and archiving utilities are disabled."
    )
endif ()

# configure usage of libarchive
option(USE_LIBARCHIVE "uses libarchive; if disabled archiving utilities will not be available" OFF)
if (USE_LIBARCHIVE)
    if (NOT USE_STANDARD_FILESYSTEM)
        message(FATAL_ERROR "Unable to use USE_LIBARCHIVE without USE_STANDARD_FILESYSTEM.")
    endif ()
    use_package(TARGET_NAME LibArchive::LibArchive PACKAGE_NAME LibArchive)
    list(APPEND HEADER_FILES io/archive.h)
    list(APPEND SRC_FILES io/archive.cpp)
    list(APPEND META_PUBLIC_COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_USE_LIBARCHIVE)
else ()
    set(EXCLUDED_FILES io/archive.h io/archive.cpp)
endif ()

# configure whether escape codes should be enabled by default
option(ENABLE_ESCAPE_CODES_BY_DEAULT "enables usage of escape codes by default" ON)
if (ENABLE_ESCAPE_CODES_BY_DEAULT)
    set_property(
        SOURCE application/argumentparser.cpp io/ansiescapecodes.cpp tests/argumentparsertests.cpp
        APPEND
        PROPERTY COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_ESCAPE_CODES_ENABLED_BY_DEFAULT)
else ()
    message(STATUS "Disabling use of escape codes by default.")
endif ()

# configure use of thread_local
option(ENABLE_THREAD_LOCAL "enables use of Thread-Local Storage" ON)
if (NOT ENABLE_THREAD_LOCAL)
    set_property(
        SOURCE conversion/stringconversion.cpp
        APPEND
        PROPERTY COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_NO_THREAD_LOCAL)
endif ()

# configure use of platform-specific APIs for optimizing CopyHelper
option(USE_PLATFORM_SPECIFIC_API_FOR_OPTIMIZING_COPY_HELPER
       "enables use of platform-specific APIs for optimizing CopyHelper" OFF)
if (USE_PLATFORM_SPECIFIC_API_FOR_OPTIMIZING_COPY_HELPER)
    list(APPEND META_PUBLIC_COMPILE_DEFINITIONS ${META_PROJECT_VARNAME}_USE_PLATFORM_SPECIFIC_API_FOR_OPTIMIZING_COPY_HELPER)
endif ()

# apply basic configuration
include(BasicConfig)

# set the package name of c++utilities itself as it is used in the config module template
set(CPP_UTILITIES_PACKAGE "${NAMESPACE_PREFIX}${META_PROJECT_NAME}${META_CONFIG_SUFFIX}")

# include further modules to apply configuration
include(WindowsResources)
include(LibraryTarget)
include(TestTarget)
include(Doxygen)
include(ConfigHeader)
