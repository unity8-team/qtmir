include(../../test-includes.pri)
include(../../common/common.pri)

TARGET = qpa_acceptance_tests

INCLUDEPATH += \
    ../../../src/platforms/mirserver

SOURCES += \
    acceptance_tests.cpp \
    mir_test_framework/server_runner.cpp

# need to link in the QPA plugin too for access to MirServerConfiguration
LIBS += -Wl,-rpath,$${PWD}/../../../src/platforms/mirserver \
    -L../../../src/platforms/mirserver -lqpa-mirserver

