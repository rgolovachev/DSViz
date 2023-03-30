QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
include ( ./qwt/qwt.prf )

CONFIG += c++17

CONFIG += qwt

### comment this when push
#SOURCES += Debug/debug.cpp
#HEADERS += Debug/debug.h
###


# решил забить на исключения, qwt вроде не должна их бросать
# (сделал grep по слову throw в папке src и не нашел ничего)
# надеюсь правильно компиляторы проверяю

win32-msvc*|win64-msvc* {
    QMAKE_CXXFLAGS += /EHsc
    QMAKE_CXXFLAGS += /O2
}

*-g++|*-clang++ {
    QMAKE_CXXFLAGS += -fno-exceptions
    QMAKE_CXXFLAGS += -O3
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    App/app.cpp \
    Core/controller.cpp \
    Core/model.cpp \
    Core/vnode.cpp \
    main.cpp \
    App/mainwindow.cpp \
    Core/view.cpp

HEADERS += \
    App/app.h \
    Core/controller.h \
    App/mainwindow.h \
    Core/model.h \
    Common/node.h \
    Observer/observer.h \
    Common/query.h \
    Core/view.h \
    Core/vnode.h

FORMS += \
    App/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
