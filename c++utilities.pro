projectname = c++utilities
VERSION = 1.0.6

# include ../../common.pri when building as part of a subdirs project; otherwise include general.pri
!include(../../common.pri) {
    !include(./general.pri) {
        error("Couldn't find the common.pri or the general.pri file!")
    }
}

CONFIG -= qt
win32 {
    CONFIG += dll
}

SOURCES += \
    application/argumentparser.cpp \
    application/failure.cpp \
    io/binarywriter.cpp \
    io/binaryreader.cpp \
    io/path.cpp \
    conversion/stringconversion.cpp \
    conversion/conversionexception.cpp \
    chrono/timespan.cpp \
    chrono/datetime.cpp \
    chrono/period.cpp \
    math/math.cpp \
    application/fakeqtconfigarguments.cpp \
    io/ansiescapecodes.cpp \
    misc/random.cpp \
    io/bitreader.cpp \
    application/commandlineutils.cpp

HEADERS += \
    application/global.h \
    application/argumentparser.h \
    application/failure.h \
    io/binarywriter.h \
    io/binaryreader.h \
    io/path.h \
    conversion/types.h \
    conversion/widen.h \
    conversion/binaryconversion.h \
    conversion/stringconversion.h \
    conversion/conversionexception.h \
    chrono/timespan.h \
    chrono/datetime.h \
    chrono/period.h \
    math/math.h \
    io/copy.h \
    conversion/binaryconversionprivate.h \
    application/fakeqtconfigarguments.h \
    io/ansiescapecodes.h \
    misc/memory.h \
    misc/random.h \
    io/bitreader.h \
    application/commandlineutils.h

OTHER_FILES += \
    README.md \
    LICENSE \
    pkgbuild/default/PKGBUILD \
    pkgbuild/mingw-w64/PKGBUILD

# installs
target.path = $$(INSTALL_ROOT)/lib
INSTALLS += target
for(dir, $$list(application io conversion chrono math misc)) {
    eval(inc_$${dir} = $${dir})
    inc_$${dir}.path = $$(INSTALL_ROOT)/include/$$projectname/$${dir}
    inc_$${dir}.files = $${dir}/*.h
    INSTALLS += inc_$${dir}
}
