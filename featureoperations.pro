#-------------------------------------------------
#
# Project created by QtCreator 2013-03-29T09:57:39
#
#-------------------------------------------------

CONFIG += plugin

QT       -= gui

TARGET = featureoperations
TEMPLATE = lib

include(global.pri)

DEFINES += FEATUREOPERATIONS_LIBRARY

HEADERS += \
    featureoperations/featureoperationsmodule.h \
    featureoperations/gridding.h

SOURCES += \
    featureoperations/featureoperationsmodule.cpp \
    featureoperations/gridding.cpp

OTHER_FILES += \ 
    featureoperations/featureoperations.json

LIBS += -L$$PWD/../libraries/$$PLATFORM$$CONF/core/ -lilwiscore

QMAKE_POST_LINK += $${QMAKE_COPY} $$PWD/../libraries/$$PLATFORM$$CONF/$$TARGET/lib$${TARGET}.so $$PWD/../output/$$PLATFORM$$CONF/bin/extensions/$$TARGET
