QT       += core gui
QT += multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    playthread.cpp \
    widget.cpp

HEADERS += \
    playthread.h \
    widget.h

FORMS += \
    widget.ui


# msvc编译器(这里是设置源码为utf-8(默认为gbk))
msvc{
    QMAKE_CFLAGS += -source-charset:utf-8
    QMAKE_CXXFLAGS += -source-charset:utf-8
}

INCLUDEPATH += $$PWD/../../include

CONFIG(debug, debug|release) {

  message("debug")
} else {
  message("release")
}

#WINDOWS平台
win32 {
    contains(QT_ARCH, x86_64) {
        message("x64")
        # 输出目录
        DESTDIR = $$PWD/../../bin/win64/
        # 依赖模块
        LIBS += \
                -L$$PWD/../../lib/win64 -lavformat \
                -L$$PWD/../../lib/win64 -lavcodec \
                -L$$PWD/../../lib/win64 -lavutil \
                -L$$PWD/../../lib/win64 -lswscale \
                -L$$PWD/../../lib/win64 -lswresample \



    } else {
        message("win32")
        # 输出目录
        DESTDIR = $$PWD/../../bin/win32/
        # 依赖模块
        LIBS += \
                -L$$PWD/../../lib/win32 -lavformat \
                -L$$PWD/../../lib/win32 -lavcodec \
                -L$$PWD/../../lib/win32 -lavutil \
                -L$$PWD/../../lib/win32 -lswscale\
                -L$$PWD/../../lib/win32 -lswresample\

    }
}

RESOURCES += \
    image.qrc
RC_ICONS =player.ico
