TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++17

SOURCES += \
        main.cpp

LIBS += -lboost_serialization

CONFIG(debug, release|debug):DEFINES += DEBUG
CONFIG(release, release|debug):DEFINES += NDEBUG
