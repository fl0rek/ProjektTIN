#-------------------------------------------------
#
# Project created by QtCreator 2017-04-28T20:10:55
#
#-------------------------------------------------
include(Model/model.pri)
include(View/view.pri)

QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Game
TEMPLATE = app
CONFIG  += c++14
QMAKE_CXXFLAGS += -std=c++14
INCLUDEPATH += /usr/include/boost/ ../libtlv/include/ ../include
LIBS += -L/usr/include/boost -lboost_serialization
#QMAKE_POST_LINK += ./GameApp/Tests
# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp \
    util.cpp \
    ../libtlv/src/Tlv.cpp

HEADERS  += \
    util.h \
    debug.h \
    ../libtlv/include/Tlv.h \
    ../include/tags.h


RESOURCES += \
    res.qrc
