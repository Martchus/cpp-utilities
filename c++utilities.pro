projectname = c++utilities
appname = "C++ Utilities"
appauthor = Martchus
QMAKE_TARGET_DESCRIPTION = "Common C++ classes and routines used by my applications such as argument parser, IO and conversion utilities."
VERSION = 3.0.0

# include ../../common.pri when building as part of a subdirs project; otherwise include general.pri
!include(../../common.pri) {
    !include(./general.pri) {
        error("Couldn't find the common.pri or the general.pri file!")
    }
}

TEMPLATE = lib
CONFIG -= qt
CONFIG += shared

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
    application/commandlineutils.cpp \
    io/inifile.cpp

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
    application/commandlineutils.h \
    io/inifile.h

OTHER_FILES += \
    README.md \
    LICENSE

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
