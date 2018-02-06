#-------------------------------------------------
#
# Project created by QtCreator 2017-08-04T16:12:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VideoPlayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#DEPENDPATH  += /home/wangkuan/work/code/opensrc/ffmpeg/
INCLUDEPATH += /home/wangkuan/work/code/opensrc/ffmpeg/ffmpeg-3.3

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    ch264decoder.cpp \


HEADERS += \
        mainwindow.h \
    ch264decoder.h \


FORMS += \
        mainwindow.ui

LIBS += -L/home/wangkuan/work/code/git_root/netmarch/video_player/VideoPlayer/lib/  \
        -L/home/wangkuan/work/code/opensrc/ffmpeg/ffmpeg-3.3/libavcodec    \
        -L/home/wangkuan/work/code/opensrc/ffmpeg/ffmpeg-3.3/libavformat    \
        -L/home/wangkuan/work/code/opensrc/ffmpeg/ffmpeg-3.3/libavutil    \
        -L/home/wangkuan/work/code/opensrc/ffmpeg/ffmpeg-3.3/libswscale    \
        -L/home/wangkuan/work/code/opensrc/ffmpeg/ffmpeg-3.3/libswresample    \
        -lavcodec -lavformat -lavutil -lswscale -lswresample -lz -ldl -llzma    \
        -L$$PWD/../../00-common/lib/debug/    \
        -lnvr -lvtdu -ldataswitch -losp -lrt -lgb28181 -lresolv -lmsgcenter -ldev\
         -ldebug -loscbb -lsdp -lxml

#-L./lib
