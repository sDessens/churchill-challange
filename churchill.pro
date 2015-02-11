TEMPLATE = lib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
TARGET = stefan

SOURCES += \
    src/solution.cpp \
    src/dll.cpp \
    src/binary_search.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    src/point_search.h \
    src/dll.h \
    src/solution.h \
    src/util.h \
    src/timer.h \
    src/aligned_allocator.h \
    src/binary_search.h \
    src/rank_heap.h

DEFINES += CHURCHILL_EXPORTS

*-msvc-* {
    QMAKE_CXXFLAGS += \
        /Zi \  # create external .pdb
        /GS-\  # disable buffer security checks
        /GL \  # global optimizations
        /arch:AVX
    QMAKE_CXXFLAGS_RELEASE += \
        /FA \  # generate .asm
        /O2 \
        /EHsc
    QMAKE_LFLAGS_RELEASE += \
        /OPT:REF    # remove unused functions
}
*mingw* {
    QMAKE_CXXFLAGS += \
        -std=c++11 \
        -march=native \
        -Wno-unused-function
    QMAKE_CXXFLAGS_RELEASE += \
        -O3 \
        -fno-rtti \
        -mtune=native
    QMAKE_LFLAGS +=   -static -static-libgcc
}
