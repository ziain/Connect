TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
   # lwconnect.cpp \
    lwnetwork_connect.cpp \
    lwthread.cpp

HEADERS += \
    #lwconnect.h \
    lwnetwork_connect.h \
    lwthread.h

include (./3rd/3rd.pri)

LIBS += -lpthread

DEFINES += LOG_OPEN
