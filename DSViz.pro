QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
# здесь кажется нужно указать свой путь до библиотеки qwt
include ( /usr/local/qwt-6.2.0/features/qwt.prf )

CONFIG += c++17

CONFIG += qwt

QMAKE_CXXFLAGS += -O3

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Debug/debug.cpp \
    main.cpp \
    App/mainwindow.cpp \
    Observer/observer.cpp \
    Core/view.cpp

HEADERS += \
    App/app.h \
    Core/controller.h \
    Debug/debug.h \
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
