# meta data
projectname = c++utilities
appname = "C++ Utilities"
appauthor = Martchus
QMAKE_TARGET_DESCRIPTION = "Common C++ classes and routines used by my applications such as argument parser, IO and conversion utilities."
VERSION = 3.1.1

# include ../../common.pri when building as part of a subdirs project; otherwise include general.pri
!include(../../common.pri) {
    !include(./general.pri) {
        error("Couldn't find the common.pri or the general.pri file!")
    }
}

# basic configuration: shared library, no Qt
TEMPLATE = lib
CONFIG -= qt
CONFIG += shared

# add project files
HEADERS += \
    application/argumentparser.h \
    application/commandlineutils.h \
    application/failure.h \
    application/fakeqtconfigarguments.h \
    application/global.h \
    chrono/datetime.h \
    chrono/period.h \
    chrono/timespan.h \
    conversion/binaryconversion.h \
    conversion/binaryconversionprivate.h \
    conversion/conversionexception.h \
    conversion/stringconversion.h \
    conversion/types.h \
    conversion/widen.h \
    io/ansiescapecodes.h \
    io/binaryreader.h \
    io/binarywriter.h \
    io/bitreader.h \
    io/copy.h \
    io/inifile.h \
    io/path.h \
    math/math.h \
    misc/memory.h \
    misc/random.h

SOURCES += \
    application/argumentparser.cpp \
    application/commandlineutils.cpp \
    application/failure.cpp \
    application/fakeqtconfigarguments.cpp \
    chrono/datetime.cpp \
    chrono/period.cpp \
    chrono/timespan.cpp \
    conversion/binaryconversion.cpp \
    conversion/conversionexception.cpp \
    conversion/stringconversion.cpp \
    io/ansiescapecodes.cpp \
    io/binaryreader.cpp \
    io/binarywriter.cpp \
    io/bitreader.cpp \
    io/inifile.cpp \
    io/path.cpp \
    math/math.cpp \
    misc/random.cpp

OTHER_FILES += \
    README.md \
    LICENSE \
    CMakeLists.txt \
    resources/config.h.in \
    resources/windows.rc.in

# installs
mingw-w64-install {
    target.path = $$(INSTALL_ROOT)
    target.extra = install -m755 -D $${OUT_PWD}/release/lib$(TARGET).a $$(INSTALL_ROOT)/lib/lib$(TARGET).a
    INSTALLS += target
    dlltarget.path = $$(INSTALL_ROOT)
    dlltarget.extra = install -m755 -D $${OUT_PWD}/release/$(TARGET) $$(INSTALL_ROOT)/bin/$(TARGET)
    INSTALLS += dlltarget
} else {
    target.path = $$(INSTALL_ROOT)/lib
    INSTALLS += target
}
for(dir, $$list(application io conversion chrono math misc)) {
    eval(inc_$${dir} = $${dir})
    inc_$${dir}.path = $$(INSTALL_ROOT)/include/$$projectname/$${dir}
    inc_$${dir}.files = $${dir}/*.h
    INSTALLS += inc_$${dir}
}
